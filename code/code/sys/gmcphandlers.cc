#include "gmcphandlers.h"
#include "being.h"
#include "connect.h"
#include "room.h"
#include "person.h"

#include "json.hpp"

namespace {
  unsigned char GMCP = 201;
  unsigned char iac = 255;             /* interpret as command: */
  unsigned char dont = 254;            /* you are not to use option */
  unsigned char do_ = 253;             /* please, you use option */
  unsigned char wont = 252;            /* I won't use option */
  unsigned char will = 251;            /* I will use option */
  unsigned char sb = 250;              /* interpret as subnegotiation */
  // unsigned char se = 240;              /* end sub negotiation */

  void handleCoreHello(sstring const& s, Descriptor& d) {
    auto hello = s.substr(sizeof("Core.Hello"));
    try {
      auto js = nlohmann::json::parse(hello);
      d.mudclient = js.at("client");
      d.clientversion = js.at("version");
    } catch (nlohmann::json::parse_error const&) {
      vlogf(LOG_MISC, format("Client sent bad Core.Hello: %s") % hello);
    } catch (const std::range_error&) {
      vlogf(LOG_MISC, format("Client sent bad Core.Hello: %s") % hello);
    }
  }

  void handleDiscord(sstring const& s, Descriptor& d)
  {
    sstring discordInfoCmd("External.Discord.Info { \"inviteurl\": \"https://discord.gg/pTcaQuk\", \"applicationid\": \"SneezyMUD\" }");
    d.sendGmcp(discordInfoCmd, true);
  }

  void handleRemember(sstring const& s, Descriptor& d)
  {
    sstring arg = s.dropWord();

    auto player = dynamic_cast<TPerson*>(d.character);
    if (player)
      player->doRemember(false, arg);
  }

  void handleRememberPlayer(sstring const& s, Descriptor& d)
  {
    sstring arg = s.dropWord();

    auto player = dynamic_cast<TPerson*>(d.character);
    if (player)
      player->doRememberPlayer(false, arg);
  }

  void handleRetrieve(sstring const& s, Descriptor& d)
  {
    sstring key = s.word(1);

    auto player = dynamic_cast<TPerson*>(d.character);
    if (player)
      player->doRetrieve(false, key);
  }

  std::map<std::string, std::function<void(std::string, Descriptor&)>> commandHandlers = {
    {"Core.Supports.Set", [](std::string, Descriptor&){}}, // squelch
    {"Core.Hello", handleCoreHello},
    {"remember", handleRemember},
    {"rememberplayer", handleRememberPlayer},
    {"retrieve", handleRetrieve},
    {"External.Discord.Hello", handleDiscord},
  };

  void handleGmcpCommand(sstring const& s, Descriptor* d)
  {
    assert(d);
    decltype(commandHandlers)::iterator it;

    if (s == "request sectors") {
      sstring str;
      for (int i = 0; i < MAX_SECTOR_TYPES; i++) {
        str += format(", { \"id\" : %d, \"name\" : \"%s\", \"color\" : %d }")
          % i
          % TerrainInfo[i]->name
          % TerrainInfo[i]->color;
      }
      str = sstring("room.sectors { \"sectors\" : [ ") + str.substr(2) + " ] }";
      d->sendGmcp(str, true);
    }
    else if (s == "request area") {
      TRoom* roomp = d->character->roomp;
      sstring area = format(
          "room.area { \"id\":\"%d\", \"name\": \"%s\", \"x\": 0, \"y\": 0, \"z\": 0, \"col\": \"\", \
          \"flags\": \"quiet\" }")
        % roomp->getZone()->zone_nr
        % roomp->getZone()->name;
      d->sendGmcp(area, true);
    }
    else if ((it = commandHandlers.find(s.word(0))) != commandHandlers.end()) {
      it->second(s, *d);
    }
    else
      vlogf(LOG_MISC, format("Telnet: Unknown GMCP command '%s' ") % s);
  }
}

sstring handleTelnetOpts(sstring& s, Descriptor* d)
{
  sstring result;
  size_t iac_pos = s.find(iac);
  if (iac_pos == sstring::npos)
    return "";

  // Ugliest hack ever: clients don't bother to negotiate for GMCP. Therefore, if they ever send any Telnet commands, assume they can handle subchannels.
  d->gmcp = true;

  // I wonder if this ever happens
  if (iac_pos > s.length() - 2) {
    vlogf(LOG_MISC, "Telnet: truncated Telnet IAC");
    return "";
  }

  unsigned char cmd = s[iac_pos + 1];

  if (cmd == will || cmd == do_ || cmd == wont || cmd == dont) {
    // I wonder if this ever happens
    if (iac_pos > s.length() - 3) {
      vlogf(LOG_MISC, "Telnet: truncated Telnet IAC WILL/DO/WONT/DONT");
      return "";
    }

    unsigned char arg = s[iac_pos+2];

    if (cmd == will && arg == GMCP) { // let's have a lil' GMCP
      // IAC DO GMCP
      d->gmcp = true;
      result = "\xff\0xfc\xc9";
    }
    else if (cmd == do_ && arg == GMCP) { // I can handle GMCP should you wish
      // IAC WILL GMCP
      d->gmcp = true;
      result = "\xff\0xfb\xc9";
    }
    else if (cmd == will) { // Anything else is unsupported
      // IAC DONT ...
      vlogf(LOG_MISC, format("Telnet: Unsupported protocol request: IAC WILL 0x%2x") % static_cast<int>(arg));
      result = format("\xff\xfe%c") % arg;
    }
    else if (cmd == do_) { // Anything else is unsupported
      // IAC WONT ...
      vlogf(LOG_MISC, format("Telnet: Unsupported protocol request: IAC DO 0x%02x") % static_cast<int>(arg));
      result = format("\xff\xfd%c") % arg;
    }
    else {
      if (arg == GMCP)
        d->gmcp = false;
      // vlogf(LOG_MISC, format("Telnet: Unsupported IAC DONT/WONT: 0x%02x") % static_cast<int>(arg));
    }

    // so that it won't get into general processing
    s.erase(iac_pos, 3);
    return result + handleTelnetOpts(s, d); // also handle other commands in the same string
  }
  else if (cmd == sb) {
    // eat everything up to and including IAC SE

    // I wonder if this ever happens
    if (iac_pos > s.length() - 5) {
      vlogf(LOG_MISC, "Telnet: truncated Telnet IAC SB");
      return "";
    }

    unsigned char arg = s[iac_pos+2];
    size_t begin = iac_pos + 3;
    size_t end = s.find("\xff\xf0"); // IAC SE
    if (end == sstring::npos) {
      vlogf(LOG_MISC, format("Telnet: Truncated IAC SB 0x%02x") % static_cast<int>(arg));
      return "";
    }
    sstring client_gmcp_cmd = s.substr(begin, end-begin);
    s.erase(iac_pos, end + 1 - iac_pos);

    if (arg == GMCP) {
      handleGmcpCommand(client_gmcp_cmd, d);
      return handleTelnetOpts(s, d);
    }
    else {
      // vlogf(LOG_MISC, format("Telnet: Got unhandled IAC SB 0x%02x: '%s'")
      // % static_cast<int>(arg) % client_gmcp_cmd);
      return handleTelnetOpts(s, d);
    }
  }
  else
    return "";
}
