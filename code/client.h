//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __CLIENT_H
#define __CLIENT_H

const char CLIENT_CODE_CHAR = '\200';
const char CLIENT_CODE_STR[] = "\200";

const int CLIENT_INIT         = 100;
const int CLIENT_PROMPT       = 101;
const int CLIENT_EXITS        = 102;
const int CLIENT_INVENTORY    = 103;
const int CLIENT_EQUIPMENT    = 104;
//const int CLIENT_SHOPLIST     = 105;
const int CLIENT_ROOMFOLX     = 106;
const int CLIENT_ROOMOBJS     = 107;
const int CLIENT_ERROR        = 108;
const int CLIENT_CLOSE        = 109;
const int CLIENT_LOGIN        = 110;
const int CLIENT_MOTD         = 111;
const int CLIENT_GLOBAL       = 112;
const int CLIENT_MENU         = 113;
const int CLIENT_PIETY        = 114;
const int CLIENT_CONNECTED    = 115;
const int CLIENT_DISCONNECT   = 117;
const int CLIENT_LOG          = 118;
const int CLIENT_WHO          = 119;
const int CLIENT_STARTEDIT    = 121;
const int CLIENT_CANCELEDIT   = 122;
const int CLIENT_ENABLEWINDOW = 123;
const int CLIENT_NEWCHAR      = 124;
const int CLIENT_NEWACCOUNT   = 125;
const int CLIENT_HITPOINT     = 126;
const int CLIENT_MANA         = 127;
const int CLIENT_FIGHT        = 128;
const int CLIENT_MOVE         = 129;
const int CLIENT_DRAW         = 130;
const int CLIENT_ROOM         = 131;
const int CLIENT_EXP          = 132;
const int CLIENT_GOLD         = 133;
const int CLIENT_COND         = 134;
const int CLIENT_POS          = 135;
const int CLIENT_NOTE         = 136;
const int CLIENT_RENT         = 137;
const int CLIENT_TRACKING     = 138;
const int CLIENT_TRACKOFF     = 139;
const int CLIENT_NOTE_END     = 140;
const int CLIENT_RENT_END     = 141;
const int CLIENT_ROOMNAME     = 142;
const int CLIENT_ROOMDESC     = 143;
const int CLIENT_ROOMHEIGHT   = 144;
const int CLIENT_ROOMMAXCAP   = 145;
const int CLIENT_ROOMSECTOR   = 146;
const int CLIENT_NORMAL       = 147;
const int CLIENT_OFFENSIVE    = 148;
const int CLIENT_DEFENSIVE    = 149;
const int CLIENT_CURRENTROOM  = 150;
const int CLIENT_BUG	      = 151;
const int CLIENT_IDEA	      = 152;
const int CLIENT_TYPO	      = 153;
const int CLIENT_BERSERK      = 154;
const int CLIENT_MUDTIME      = 155;
const int CLIENT_MAIL         = 156;
const int CLIENT_INVIS        = 157;
const int CLIENT_STEALTH      = 158;
const int CLIENT_ACCOUNTSETUP = 159;
const int CLIENT_SHOUT        = 160;
const int CLIENT_TELEPATHY    = 161;
const int CLIENT_TELL         = 162;
const int CLIENT_WIZNET       = 163;
const int CLIENT_SAY          = 164;
const int CLIENT_GROUPTELL    = 165;
const int CLIENT_SHOPLIST     = 166;
const int CLIENT_SHOPLISTEND  = 167;
const int CLIENT_CHECKCHARNAME= 168;
const int CLIENT_CHECKACCOUNTNAME = 169;
const int CLIENT_ROOMEXITS    = 170;
const int CLIENT_ROOMEXTRAS   = 171;
const int CLIENT_GROUPADD     = 172;
const int CLIENT_GROUPDELETE  = 173;
const int CLIENT_GROUP        = 174;
const int CLIENT_GROUPDELETEALL = 175;
const int CLIENT_ROMBLER      = 176;
const int CLIENT_LIFEFORCE    = 177;
const int CLIENT_TONEXTLEVEL  = 178;
const int CLIENT_WHISPER      = 179;
const int CLIENT_ASK          = 180;
const int CLIENT_FTELL        = 181;

const int ERR_BAD_NAME       = 1;
const int ERR_BAD_PASSWD     = 2;
const int ERR_BAD_VERSION    = 3;
const int ERR_NOT_ALLOWED    = 4;
const int ERR_BAD_RACE       = 5;
const int ERR_BAD_CLASS      = 6;
const int ERR_BAD_STAT       = 7;
const int ERR_BAD_TERRAIN    = 8;
const int ERR_BADACCOUNT_NAME = 9;
const int ERR_BADACCOUNT_PASSWORD = 10;

const int DELETE        = 0;
const int ADD           = 1;

extern bool is_client_sstring(char *str);

extern bool Clients;

#endif




















