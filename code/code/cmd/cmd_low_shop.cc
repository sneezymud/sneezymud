#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "being.h"
#include "database.h"
#include "db.h"
#include "enum.h"
#include "extern.h"
#include "log.h"
#include "low.h"
#include "materials.h"
#include "obj.h"
#include "parse.h"
#include "room.h"
#include "shop.h"
#include "sstring.h"

namespace {
  sstring formatProducing(std::string_view producing) {
    if (producing.empty()) {
      return "(none)";
    }

    // Arbitrary limit for displaying full list
    static constexpr size_t MAX_PRODUCING_WIDTH = 20;

    if (producing.length() <= MAX_PRODUCING_WIDTH) {
      return producing;
    }

    const auto itemCount = std::ranges::count(producing, ',') + 1;
    return std::format("{} items", itemCount);
  }

  struct ShopListRow {
      sstring shopNr;
      sstring keeper;
      sstring room;
      sstring ownedBy;
      sstring types;
      sstring materials;
      sstring producing;
  };

  struct ShopListColumn {
      std::string_view header;
      sstring ShopListRow::* field;
  };

  constexpr auto SHOP_LIST_COLUMNS = std::to_array<ShopListColumn>({
    {"Shop#", &ShopListRow::shopNr},
    {"Keeper", &ShopListRow::keeper},
    {"Room", &ShopListRow::room},
    {"Owned By", &ShopListRow::ownedBy},
    {"Types", &ShopListRow::types},
    {"Materials", &ShopListRow::materials},
    {"Producing", &ShopListRow::producing},
  });

  constexpr auto DEFAULT_WIDTHS = []() {
    std::array<size_t, SHOP_LIST_COLUMNS.size()> widths{};
    for (size_t i = 0; i < SHOP_LIST_COLUMNS.size(); ++i) {
      widths[i] = SHOP_LIST_COLUMNS[i].header.length();
    }
    return widths;
  }();

  void listShops(const TBeing& ch, const sstring& arg) {
    std::optional<int> zoneFilter;
    if (!arg.empty()) {
      zoneFilter = convertTo<int>(arg);
      const auto zoneCount = zone_table.size();
      if (*zoneFilter < 0 || std::cmp_greater_equal(*zoneFilter, zoneCount)) {
        ch.sendTo(
          std::format("Zone must be between 0 and {}.\n\r", zoneCount - 1));
        return;
      }
    }

    const auto whereClause =
      zoneFilter
        ? std::format(" WHERE s.in_room BETWEEN {} AND {}",
            zone_table[*zoneFilter].bottom, zone_table[*zoneFilter].top)
        : std::string{};

    TDatabase db(DB_SNEEZY);
    db.query(std::vformat(
      "SELECT s.shop_nr, s.keeper, s.in_room, "
      "m.name AS keeper_name, r.name AS room_name, "
      "so.corp_id, c.name AS corp_name, "
      "GROUP_CONCAT(DISTINCT st.type ORDER BY st.type) AS type_ids, "
      "GROUP_CONCAT(DISTINCT sm.mat_type ORDER BY sm.mat_type) AS mat_ids, "
      "GROUP_CONCAT(DISTINCT sp.producing ORDER BY sp.producing) AS producing"
      " FROM shop s"
      " LEFT JOIN mob m ON s.keeper = m.vnum"
      " LEFT JOIN room r ON s.in_room = r.vnum"
      " LEFT JOIN shopowned so ON s.shop_nr = so.shop_nr"
      " LEFT JOIN corporation c ON so.corp_id = c.corp_id"
      " LEFT JOIN shoptype st ON s.shop_nr = st.shop_nr"
      " LEFT JOIN shopmaterial sm ON s.shop_nr = sm.shop_nr"
      " LEFT JOIN shopproducing sp ON s.shop_nr = sp.shop_nr"
      "{}"
      " GROUP BY s.shop_nr, s.keeper, s.in_room, m.name, r.name, "
      "so.corp_id, c.name"
      " ORDER BY s.shop_nr",
      std::make_format_args(whereClause))
        .c_str());

    auto widths = DEFAULT_WIDTHS;
    std::vector<ShopListRow> rows;
    rows.reserve(static_cast<size_t>(db.rowCount()));

    while (db.fetchRow()) {
      const sstring typeIds = db["type_ids"];
      const sstring matIds = db["mat_ids"];
      const sstring corpId = db["corp_id"];
      auto& row = rows.emplace_back(ShopListRow{
        .shopNr = db["shop_nr"],
        .keeper = std::format("{} ({})", db["keeper_name"], db["keeper"]),
        .room = std::format("{} ({})", db["room_name"], db["in_room"]),
        .ownedBy = corpId.empty()
                     ? sstring("(none)")
                     : std::format("{} ({})", db["corp_name"], corpId),
        .types = typeIds.empty() ? "(none)" : typeIds,
        .materials = matIds.empty() ? "(all)" : matIds,
        .producing = formatProducing(db["producing"]),
      });

      for (size_t i = 0; i < SHOP_LIST_COLUMNS.size(); ++i) {
        widths[i] = std::max(widths[i],
          (row.*(SHOP_LIST_COLUMNS[i].field)).lengthNoColor());
      }
    }

    std::ostringstream output;

    for (size_t i = 0; i < SHOP_LIST_COLUMNS.size(); ++i) {
      output << std::format("{:<{}} ", SHOP_LIST_COLUMNS[i].header, widths[i]);
    }
    output << "\n\r";

    const size_t totalWidth =
      std::accumulate(widths.begin(), widths.end(), size_t{0}) + widths.size() -
      1;
    output << std::string(totalWidth, '-') << "\n\r";

    for (const auto& row : rows) {
      for (size_t i = 0; i < SHOP_LIST_COLUMNS.size(); ++i) {
        const auto& field = row.*(SHOP_LIST_COLUMNS[i].field);
        // Adjust width to account for invisible color codes
        const auto adjustedWidth =
          widths[i] + field.length() - field.lengthNoColor();
        output << std::format("{:<{}} ", field, adjustedWidth);
      }
      output << "\n\r";
    }

    output << std::format("\n\r{} shop(s) found.\n\r", rows.size());

    if (ch.desc) {
      ch.desc->page_string(output.str(), SHOWNOW_NO, ALLOWREP_YES);
    }
  }

  void listItemTypes(const TBeing& ch, const sstring&) {
    std::ostringstream str;
    str << "Item Types for shop addtype/rmtype\n\r";
    str << "Restricts shop to only buy/sell items of these types.\n\r";
    str << "-----------------------------------------------------------\n\r";

    size_t i = 0;
    for (const auto* info : ItemInfo) {
      if (info && info->name) {
        str << std::format("{:3} - {}\n\r", i, info->name);
      }
      ++i;
    }

    if (ch.desc) {
      ch.desc->page_string(str.str(), SHOWNOW_NO, ALLOWREP_YES);
    }
  }

  void listMaterials(const TBeing& ch, const sstring&) {
    std::ostringstream o;
    o << "Material Types for shop addmat/rmmat\n\r";
    o << "Restricts shop to only buy/sell items made of these materials.\n\r";
    o << "-----------------------------------------------------------\n\r";

    size_t i = 0;
    for (const auto& mat : material_nums) {
      const std::string_view name = mat.mat_name;
      if (!name.empty()) {
        o << std::format("{:3} - {}\n\r", i, name);
      }
      ++i;
    }

    if (ch.desc) {
      ch.desc->page_string(o.str(), SHOWNOW_NO, ALLOWREP_YES);
    }
  }

  bool requireShopExists(TDatabase& db, const TBeing& ch, int shopNr) {
    db.query(
      "SELECT s.*, m.name AS keeper_name, r.name AS room_name "
      "FROM shop s "
      "LEFT JOIN mob m ON s.keeper = m.vnum "
      "LEFT JOIN room r ON s.in_room = r.vnum "
      "WHERE s.shop_nr=%i",
      shopNr);
    if (db.fetchRow()) {
      return true;
    }
    ch.sendTo(std::format("Shop {} does not exist.\n\r", shopNr));
    return false;
  }

  std::ostringstream appendShopTypes(int shopNr) {
    std::ostringstream str;
    str << "Item Types Traded:\n\r";

    TDatabase db(DB_SNEEZY);
    db.query("select type from shoptype where shop_nr=%i order by type",
      shopNr);

    if (!db.rowCount()) {
      str << "  (none)\n\r";
      return str;
    }

    while (db.fetchRow()) {
      const int dbType = convertTo<int>(db["type"]);
      const itemTypeT itemType = mapFileToItemType(dbType);
      const itemInfo* info = ItemInfo[itemType];
      const std::string_view name =
        itemType < MAX_OBJ_TYPES && info ? info->name : "(unknown)";
      str << std::format("  {} - {}\n\r", dbType, name);
    }

    return str;
  }

  std::ostringstream appendProducingItems(int shopNr) {
    std::ostringstream str;
    str << "Producing Infinitely:\n\r";

    TDatabase db(DB_SNEEZY);
    db.query(
      "select p.producing, o.short_desc from shopproducing p "
      "left join obj o on p.producing = o.vnum "
      "where p.shop_nr=%i order by p.producing",
      shopNr);

    if (!db.rowCount()) {
      str << "  (none)\n\r";
      return str;
    }

    while (db.fetchRow()) {
      str << std::format("  {} - {}\n\r", db["producing"], db["short_desc"]);
    }

    return str;
  }

  constexpr size_t MATERIAL_COUNT = std::size(material_nums);

  std::ostringstream appendMaterialRestrictions(int shopNr) {
    std::ostringstream str;
    str << "Material Restrictions:\n\r";

    TDatabase db(DB_SNEEZY);
    db.query(
      "select mat_type from shopmaterial where shop_nr=%i order by mat_type",
      shopNr);

    if (!db.rowCount()) {
      str << "  (none - accepts all materials)\n\r";
      return str;
    }

    while (db.fetchRow()) {
      const int matType = convertTo<int>(db["mat_type"]);
      const std::string_view name =
        matType >= 0 && std::cmp_less(matType, MATERIAL_COUNT)
          ? &material_nums[matType].mat_name[0]
          : "(invalid)";
      str << std::format("  {} - {}\n\r", matType, name);
    }

    return str;
  }

  void showShopInfo(const TBeing& ch, const sstring& arg) {
    sstring argument = arg;
    sstring shopNrArg;
    argument = one_argument(argument, shopNrArg);

    if (shopNrArg.empty()) {
      ch.sendTo("Syntax: low shop info <shop_nr>\n\r");
      return;
    }

    const int shopNr = convertTo<int>(shopNrArg);

    TDatabase db(DB_SNEEZY);
    if (!requireShopExists(db, ch, shopNr)) {
      return;
    }

    static constexpr std::string_view DIVIDER =
      "-----------------------------------------------------------------------";

    std::ostringstream str;

    str << std::format("Shop #{} Information\n\r", shopNr);
    str << std::string(DIVIDER.size(), '=') << "\n\r";
    str << std::format("{:>11}: {} ({})\n\r", "Keeper", db["keeper_name"],
      db["keeper"]);
    str << std::format("{:>11}: {} ({})\n\r", "Room", db["room_name"],
      db["in_room"]);

    {
      TDatabase corpDb(DB_SNEEZY);
      corpDb.query(
        "SELECT so.corp_id, c.name AS corp_name "
        "FROM shopowned so "
        "LEFT JOIN corporation c ON so.corp_id = c.corp_id "
        "WHERE so.shop_nr=%i",
        shopNr);
      if (corpDb.fetchRow()) {
        str << std::format("{:>11}: {} ({})\n\r", "Owned By",
          corpDb["corp_name"], corpDb["corp_id"]);
      } else {
        str << std::format("{:>11}: (not owned)\n\r", "Owned By");
      }
    }

    str << DIVIDER << "\n\r";
    str << "Values:\n\r";
    str << std::format("  {:>11}: {}\n\r", "profit_buy", db["profit_buy"]);
    str << std::format("  {:>11}: {}\n\r", "profit_sell", db["profit_sell"]);
    str << std::format("  {:>11}: {}\n\r", "temper1", db["temper1"]);
    str << std::format("  {:>11}: {}\n\r", "temper2", db["temper2"]);
    str << std::format("  {:>11}: {}\n\r", "flags", db["flags"]);
    str << std::format("  {:>11}: {}\n\r", "open1", db["open1"]);
    str << std::format("  {:>11}: {}\n\r", "close1", db["close1"]);
    str << std::format("  {:>11}: {}\n\r", "open2", db["open2"]);
    str << std::format("  {:>11}: {}\n\r", "close2", db["close2"]);

    str << DIVIDER << "\n\r";
    str << appendShopTypes(shopNr).view();

    str << DIVIDER << "\n\r";
    str << appendProducingItems(shopNr).view();

    str << DIVIDER << "\n\r";
    str << appendMaterialRestrictions(shopNr).view();

    str << DIVIDER << "\n\r";
    str << "Messages:\n\r";
    str << std::format("  {:>13}: {}\n\r", "no_such_item1",
      db["no_such_item1"]);
    str << std::format("  {:>13}: {}\n\r", "no_such_item2",
      db["no_such_item2"]);
    str << std::format("  {:>13}: {}\n\r", "do_not_buy", db["do_not_buy"]);
    str << std::format("  {:>13}: {}\n\r", "missing_cash1",
      db["missing_cash1"]);
    str << std::format("  {:>13}: {}\n\r", "missing_cash2",
      db["missing_cash2"]);
    str << std::format("  {:>13}: {}\n\r", "message_buy", db["message_buy"]);
    str << std::format("  {:>13}: {}\n\r", "message_sell", db["message_sell"]);

    if (ch.desc) {
      ch.desc->page_string(str.str(), SHOWNOW_NO, ALLOWREP_YES);
    }
  }

  void createShop(const TBeing& ch, const sstring& arg) {
    sstring argument = arg;
    sstring shopNrArg;
    argument = one_argument(argument, shopNrArg);

    sstring keeperVnumArg;
    argument = one_argument(argument, keeperVnumArg);

    sstring roomVnumArg;
    argument = one_argument(argument, roomVnumArg);

    if (shopNrArg.empty() || keeperVnumArg.empty() || roomVnumArg.empty()) {
      ch.sendTo(
        "Syntax: low shop create <shop_nr> <keeper_vnum> <room_vnum>\n\r");
      return;
    }

    const int shopNr = convertTo<int>(shopNrArg);
    if (shopNr < 0) {
      ch.sendTo("Shop number must be non-negative.\n\r");
      return;
    }

    const int keeperVnum = convertTo<int>(keeperVnumArg);
    if (real_mobile(keeperVnum) < 0) {
      ch.sendTo(
        std::format("Mob with vnum {} does not exist.\n\r", keeperVnum));
      return;
    }

    const int roomVnum = convertTo<int>(roomVnumArg);
    if (!real_roomp(roomVnum)) {
      ch.sendTo(std::format("Room with vnum {} does not exist.\n\r", roomVnum));
      return;
    }

    TDatabase db(DB_SNEEZY);

    db.query("SELECT shop_nr FROM shop WHERE keeper=%i", keeperVnum);
    if (db.fetchRow()) {
      ch.sendTo(std::format("Mob {} is already the keeper of shop {}.\n\r",
        keeperVnum, db["shop_nr"]));
      return;
    }

    db.query("SELECT shop_nr FROM shop WHERE in_room=%i", roomVnum);
    if (db.fetchRow()) {
      ch.sendTo(std::format("Room {} is already the location of shop {}.\n\r",
        roomVnum, db["shop_nr"]));
      return;
    }

    db.query("select shop_nr from shop where shop_nr=%i", shopNr);
    if (db.fetchRow()) {
      ch.sendTo(
        std::format("Shop {} already exists. Use 'low shop modify' or "
                    "'low shop delete' first.\n\r",
          shopNr));
      return;
    }

    if (!db.query("insert into shop (shop_nr, keeper, in_room, profit_buy, "
                  "profit_sell, no_such_item1, no_such_item2, do_not_buy, "
                  "missing_cash1, missing_cash2, message_buy, message_sell, "
                  "temper1, temper2, flags, open1, close1, "
                  "open2, close2) values (%i, %i, %i, 1.1, 0.9, "
                  "'I don''t have that right now.  Try \"list\".', "
                  "'I''ve never heard of that!', "
                  "'Go sell it to someone who cares!', "
                  "'I can''t afford that!', "
                  "'I don''t think you have that kind of money!', "
                  "'That''ll be %%d talens.  Thank you!', "
                  "'Here is your %%d talens.  Thank you!', "
                  "2, 1, 0, 0, 96, 0, 0)",
          shopNr, keeperVnum, roomVnum)) {
      ch.sendTo("Database error: failed to create shop.\n\r");
      return;
    }

    static constexpr std::string_view CREATE_MSG =
      "Shop {} created with keeper {} in room {}.\n\r\n\r"
      "Use 'low shop addtype' to add item types this shop trades.\n\r"
      "Changes take effect after 'boot zone' or server reboot.\n\r";

    ch.sendTo(std::format(CREATE_MSG, shopNr, keeperVnum, roomVnum));
    vlogf(LOG_LOW, std::format("{} created shop {} (keeper={}, room={})",
                     ch.getName(), shopNr, keeperVnum, roomVnum));
  }

  // Field types for shop modify validation
  enum class ShopFieldType : uint8_t {
    INVALID,
    FLOAT,
    INT_HOUR,
    INT_TEMPER,
    INT_FLAGS,
    STRING,
    VNUM_MOB,
    VNUM_ROOM
  };

  struct ShopFieldDef {
      std::string_view name;
      ShopFieldType type;
  };

  constexpr auto SHOP_FIELD_DEFS = std::to_array<ShopFieldDef>({
    {"profit_buy", ShopFieldType::FLOAT},
    {"profit_sell", ShopFieldType::FLOAT},
    {"open1", ShopFieldType::INT_HOUR},
    {"close1", ShopFieldType::INT_HOUR},
    {"open2", ShopFieldType::INT_HOUR},
    {"close2", ShopFieldType::INT_HOUR},
    {"temper1", ShopFieldType::INT_TEMPER},
    {"temper2", ShopFieldType::INT_TEMPER},
    {"flags", ShopFieldType::INT_FLAGS},
    {"no_such_item1", ShopFieldType::STRING},
    {"no_such_item2", ShopFieldType::STRING},
    {"do_not_buy", ShopFieldType::STRING},
    {"missing_cash1", ShopFieldType::STRING},
    {"missing_cash2", ShopFieldType::STRING},
    {"message_buy", ShopFieldType::STRING},
    {"message_sell", ShopFieldType::STRING},
    {"keeper", ShopFieldType::VNUM_MOB},
    {"in_room", ShopFieldType::VNUM_ROOM},
  });

  constexpr ShopFieldType getShopFieldType(std::string_view field) {
    const auto* it =
      std::ranges::find(SHOP_FIELD_DEFS, field, &ShopFieldDef::name);
    return it != SHOP_FIELD_DEFS.end() ? it->type : ShopFieldType::INVALID;
  }

  std::optional<sstring> validateShopFieldValue(ShopFieldType fieldType,
    std::string_view field, std::string_view value, const TBeing& ch,
    int shopNr) {
    static constexpr std::string_view QUERY =
      "update shop set {}={} where shop_nr={}";

    switch (fieldType) {
      case ShopFieldType::FLOAT: {
        const auto floatVal = convertTo<double>(value);
        if (floatVal <= 0.0) {
          ch.sendTo(
            std::format("Value for {} must be a positive number.\n\r", field));
          return std::nullopt;
        }
        return std::format(QUERY, field, floatVal, shopNr);
      }

      case ShopFieldType::INT_HOUR: {
        const int hourVal = convertTo<int>(value);
        if (hourVal < 0 || (hourVal > 23 && hourVal != 96)) {
          ch.sendTo(std::format(
            "Value for {} must be 0-23 (hour) or 96 (always open).\n\r",
            field));
          return std::nullopt;
        }
        return std::format(QUERY, field, hourVal, shopNr);
      }

      case ShopFieldType::INT_TEMPER: {
        const int temperVal = convertTo<int>(value);
        if (temperVal < 0 || temperVal > 3) {
          ch.sendTo(std::format("Value for {} must be 0-3.\n\r", field));
          return std::nullopt;
        }
        return std::format(QUERY, field, temperVal, shopNr);
      }

      case ShopFieldType::INT_FLAGS: {
        const int flagsVal = convertTo<int>(value);
        if (flagsVal < 0) {
          ch.sendTo(
            std::format("Value for {} must be non-negative.\n\r", field));
          return std::nullopt;
        }
        return std::format(QUERY, field, flagsVal, shopNr);
      }

      case ShopFieldType::STRING: {
        // Escape the string to prevent SQL injection and wrap in quotes to make
        // it valid SQL. Also double any % characters to prevent
        // TDatabase::query format issues.
        const sstring escapedValue =
          std::format("'{}'", sstring(value).escape().replaceString("%", "%%"));
        return std::format(QUERY, field, escapedValue, shopNr);
      }

      case ShopFieldType::VNUM_MOB: {
        const int keeperVnum = convertTo<int>(value);
        if (real_mobile(keeperVnum) < 0) {
          ch.sendTo(
            std::format("Mob with vnum {} does not exist.\n\r", keeperVnum));
          return std::nullopt;
        }
        TDatabase db(DB_SNEEZY);
        db.query("SELECT shop_nr FROM shop WHERE keeper=%i AND shop_nr!=%i",
          keeperVnum, shopNr);
        if (db.fetchRow()) {
          ch.sendTo(std::format("Mob {} is already the keeper of shop {}.\n\r",
            keeperVnum, db["shop_nr"]));
          return std::nullopt;
        }
        return std::format(QUERY, field, keeperVnum, shopNr);
      }

      case ShopFieldType::VNUM_ROOM: {
        const int roomVnum = convertTo<int>(value);
        if (!real_roomp(roomVnum)) {
          ch.sendTo(
            std::format("Room with vnum {} does not exist.\n\r", roomVnum));
          return std::nullopt;
        }
        TDatabase db(DB_SNEEZY);
        db.query("SELECT shop_nr FROM shop WHERE in_room=%i AND shop_nr!=%i",
          roomVnum, shopNr);
        if (db.fetchRow()) {
          ch.sendTo(
            std::format("Room {} is already the location of shop {}.\n\r",
              roomVnum, db["shop_nr"]));
          return std::nullopt;
        }
        return std::format(QUERY, field, roomVnum, shopNr);
      }

      case ShopFieldType::INVALID:
      default:
        ch.sendTo(
          std::format("Unknown field '{}'. Use 'low shop' for valid fields. "
                      "Field names must be exact.\n\r",
            field));
        return std::nullopt;
    }
  }

  void modifyShop(const TBeing& ch, const sstring& arg) {
    sstring argument = arg;
    sstring shopNrArg;
    argument = one_argument(argument, shopNrArg);

    sstring fieldArg;
    argument = one_argument(argument, fieldArg);

    if (shopNrArg.empty() || fieldArg.empty() || argument.empty()) {
      ch.sendTo("Syntax: low shop modify <shop_nr> <field> <value>\n\r");
      return;
    }

    const int shopNr = convertTo<int>(shopNrArg);
    const sstring value = argument.trim();

    TDatabase db(DB_SNEEZY);
    if (!requireShopExists(db, ch, shopNr)) {
      return;
    }

    const sstring field = fieldArg.lower();
    const ShopFieldType fieldType = getShopFieldType(field);
    const std::optional<sstring> query =
      validateShopFieldValue(fieldType, field, value, ch, shopNr);

    if (!query.has_value()) {
      return;
    }

    if (!db.query(query.value().c_str())) {
      ch.sendTo("Database error: failed to modify shop.\n\r");
      return;
    }

    static constexpr std::string_view MODIFY_MSG =
      "Shop {}: {} set to '{}'.\n\r"
      "Changes take effect after 'boot zone' or server reboot.\n\r";

    ch.sendTo(std::format(MODIFY_MSG, shopNr, field, value));
    vlogf(LOG_LOW, std::format("{} modified shop {}: {}='{}'", ch.getName(),
                     shopNr, field, value));
  }

  std::optional<std::pair<itemTypeT, itemInfo>> getValidItemInfo(
    int itemTypeInt, const TBeing& ch) {
    const auto itemType = static_cast<itemTypeT>(itemTypeInt);

    if (!ItemInfo[itemType] || !ItemInfo[itemType]->name) {
      ch.sendTo(std::format("Item type {} is not defined.\n\r", itemTypeInt));
      return std::nullopt;
    }

    return std::make_pair(itemType, *ItemInfo[itemType]);
  }

  void addShopType(const TBeing& ch, const sstring& arg) {
    sstring argument = arg;
    sstring shopNrArg;
    argument = one_argument(argument, shopNrArg);

    sstring itemTypeArg;
    argument = one_argument(argument, itemTypeArg);

    if (shopNrArg.empty() || itemTypeArg.empty()) {
      ch.sendTo(
        "Syntax: low shop addtype <shop_nr> <item_type>\n\r"
        "Use 'low shop types' for valid item type numbers.\n\r");
      return;
    }

    const int shopNr = convertTo<int>(shopNrArg);

    TDatabase db(DB_SNEEZY);
    if (!requireShopExists(db, ch, shopNr)) {
      return;
    }

    const int itemTypeInt = convertTo<int>(itemTypeArg);
    if (itemTypeInt < 0 || itemTypeInt >= MAX_OBJ_TYPES) {
      ch.sendTo(
        std::format("Invalid item type {}. See 'low shop types' for valid item "
                    "type numbers.\n\r",
          itemTypeInt));
      return;
    }

    const auto maybeItemInfo = getValidItemInfo(itemTypeInt, ch);

    if (!maybeItemInfo.has_value()) {
      return;
    }

    const auto& [itemType, itemInfo] = maybeItemInfo.value();

    const int dbType = mapItemTypeToFile(itemType);
    db.query("select type from shoptype where shop_nr=%i and type=%i", shopNr,
      dbType);

    if (db.fetchRow()) {
      ch.sendTo(std::format("Shop {} already trades item type {} ({}).\n\r",
        shopNr, itemTypeInt, itemInfo.name));
      return;
    }

    if (!db.query("insert into shoptype (shop_nr, type) values (%i, %i)",
          shopNr, dbType)) {
      ch.sendTo("Database error: failed to add item type to shop.\n\r");
      return;
    }

    ch.sendTo(std::format("Shop {} now trades item type {} ({}).\n\r", shopNr,
      itemTypeInt, itemInfo.name));
    vlogf(LOG_LOW, std::format("{} added type {} to shop {}", ch.getName(),
                     itemTypeInt, shopNr));
  }

  void removeShopType(const TBeing& ch, const sstring& arg) {
    sstring argument = arg;
    sstring shopNrArg;
    argument = one_argument(argument, shopNrArg);

    sstring itemTypeArg;
    argument = one_argument(argument, itemTypeArg);

    if (shopNrArg.empty() || itemTypeArg.empty()) {
      ch.sendTo("Syntax: low shop rmtype <shop_nr> <item_type>\n\r");
      return;
    }

    const int itemTypeInt = convertTo<int>(itemTypeArg);
    if (itemTypeInt < 0 || itemTypeInt >= MAX_OBJ_TYPES) {
      ch.sendTo(
        std::format("Invalid item type {}. See 'low shop types' for valid item "
                    "type numbers.\n\r",
          itemTypeInt));
      return;
    }

    const auto maybeItemInfo = getValidItemInfo(itemTypeInt, ch);
    if (!maybeItemInfo.has_value()) {
      return;
    }

    const auto& [itemType, itemInfo] = maybeItemInfo.value();

    // Confirm shop exists first just for extra precaution when deleting
    const int shopNr = convertTo<int>(shopNrArg);
    TDatabase db(DB_SNEEZY);
    if (!requireShopExists(db, ch, shopNr)) {
      return;
    }

    if (!db.query("delete from shoptype where shop_nr=%i and type=%i", shopNr,
          mapItemTypeToFile(itemType))) {
      ch.sendTo("Database error: failed to remove item type.\n\r");
      return;
    }

    if (db.rowCount() > 0) {
      ch.sendTo(std::format("Removed item type {} ({}) from shop {}.\n\r",
        itemTypeInt, itemInfo.name, shopNr));
      vlogf(LOG_LOW, std::format("{} removed type {} ({}) from shop {}",
                       ch.getName(), itemTypeInt, itemInfo.name, shopNr));
    } else {
      ch.sendTo(std::format("Shop {} was not trading item type {}.\n\r", shopNr,
        itemTypeInt));
    }
  }

  void addShopProduct(const TBeing& ch, const sstring& arg) {
    sstring argument = arg;
    sstring shopNrArg;
    argument = one_argument(argument, shopNrArg);

    sstring objVnumArg;
    argument = one_argument(argument, objVnumArg);

    if (shopNrArg.empty() || objVnumArg.empty()) {
      ch.sendTo("Syntax: low shop addprod <shop_nr> <obj_vnum>\n\r");
      return;
    }

    const int shopNr = convertTo<int>(shopNrArg);
    TDatabase db(DB_SNEEZY);
    if (!requireShopExists(db, ch, shopNr)) {
      return;
    }

    const int objVnum = convertTo<int>(objVnumArg);
    if (real_object(objVnum, true) < 0) {
      ch.sendTo(std::format("Object vnum {} does not exist.\n\r", objVnum));
      return;
    }

    db.query(
      "select producing from shopproducing where shop_nr=%i and producing=%i",
      shopNr, objVnum);

    if (db.fetchRow()) {
      ch.sendTo(std::format("Shop {} already produces object {}.\n\r", shopNr,
        objVnum));
      return;
    }

    if (!db.query(
          "insert into shopproducing (shop_nr, producing) values (%i, %i)",
          shopNr, objVnum)) {
      ch.sendTo("Database error: failed to add product to shop.\n\r");
      return;
    }

    ch.sendTo(std::format(
      "Shop {} now produces object {} (infinite stock).\n\r", shopNr, objVnum));
    vlogf(LOG_LOW, std::format("{} added producing {} to shop {}", ch.getName(),
                     objVnum, shopNr));
  }

  void removeShopProduct(const TBeing& ch, const sstring& arg) {
    sstring argument = arg;
    sstring shopNrArg;
    argument = one_argument(argument, shopNrArg);

    sstring objVnumArg;
    argument = one_argument(argument, objVnumArg);

    if (shopNrArg.empty() || objVnumArg.empty()) {
      ch.sendTo("Syntax: low shop rmprod <shop_nr> <obj_vnum>\n\r");
      return;
    }

    const int shopNr = convertTo<int>(shopNrArg);
    TDatabase db(DB_SNEEZY);
    if (!requireShopExists(db, ch, shopNr)) {
      return;
    }

    const int objVnum = convertTo<int>(objVnumArg);
    if (!db.query("delete from shopproducing where shop_nr=%i and producing=%i",
          shopNr, objVnum)) {
      ch.sendTo("Database error: failed to remove product.\n\r");
      return;
    }

    if (db.rowCount() > 0) {
      ch.sendTo(std::format("Removed producing object {} from shop {}.\n\r",
        objVnum, shopNr));
      vlogf(LOG_LOW, std::format("{} removed producing {} from shop {}",
                       ch.getName(), objVnum, shopNr));
    } else {
      ch.sendTo(std::format("Shop {} was not producing object {}.\n\r", shopNr,
        objVnum));
    }
  }

  void addShopMaterial(const TBeing& ch, const sstring& arg) {
    sstring argument = arg;
    sstring shopNrArg;
    argument = one_argument(argument, shopNrArg);

    sstring matTypeArg;
    argument = one_argument(argument, matTypeArg);

    if (shopNrArg.empty() || matTypeArg.empty()) {
      ch.sendTo("Syntax: low shop addmat <shop_nr> <mat_type>\n\r");
      ch.sendTo(
        "Use 'low shop materials' for valid material type numbers.\n\r");
      return;
    }

    const int shopNr = convertTo<int>(shopNrArg);

    TDatabase db(DB_SNEEZY);
    if (!requireShopExists(db, ch, shopNr)) {
      return;
    }

    const int matType = convertTo<int>(matTypeArg);
    if (matType < 0 || std::cmp_greater_equal(matType, MATERIAL_COUNT)) {
      ch.sendTo(std::format("Invalid material type {}. Must be 0-{}.\n\r",
        matType, MATERIAL_COUNT - 1));
      return;
    }

    const std::string_view matName = &material_nums[matType].mat_name[0];

    if (matName.empty()) {
      ch.sendTo(std::format("Material type {} is not defined.\n\r", matType));
      return;
    }

    db.query(
      "select mat_type from shopmaterial where shop_nr=%i and mat_type=%i",
      shopNr, matType);

    if (db.fetchRow()) {
      ch.sendTo(
        std::format("Shop {} already has material restriction {} ({}).\n\r",
          shopNr, matType, matName));
      return;
    }

    if (!db.query(
          "insert into shopmaterial (shop_nr, mat_type) values (%i, %i)",
          shopNr, matType)) {
      ch.sendTo("Database error: failed to add material restriction.\n\r");
      return;
    }

    ch.sendTo(std::format("Shop {}: added material restriction {} ({}).\n\r",
      shopNr, matType, matName));
    vlogf(LOG_LOW, std::format("{} added material {} to shop {}", ch.getName(),
                     matType, shopNr));
  }

  void removeShopMaterial(const TBeing& ch, const sstring& arg) {
    sstring argument = arg;
    sstring shopNrArg;
    argument = one_argument(argument, shopNrArg);

    sstring matTypeArg;
    argument = one_argument(argument, matTypeArg);

    if (shopNrArg.empty() || matTypeArg.empty()) {
      ch.sendTo("Syntax: low shop rmmat <shop_nr> <mat_type>\n\r");
      return;
    }

    TDatabase db(DB_SNEEZY);
    const int shopNr = convertTo<int>(shopNrArg);

    if (!requireShopExists(db, ch, shopNr)) {
      return;
    }

    const int matType = convertTo<int>(matTypeArg);

    if (matType < 0 || std::cmp_greater_equal(matType, MATERIAL_COUNT)) {
      ch.sendTo(std::format("Invalid material type {}. Must be 0-{}.\n\r",
        matType, MATERIAL_COUNT - 1));
      return;
    }

    if (!db.query("delete from shopmaterial where shop_nr=%i and mat_type=%i",
          shopNr, matType)) {
      ch.sendTo("Database error: failed to remove material restriction.\n\r");
      return;
    }

    if (db.rowCount() > 0) {
      const sstring msg = std::format(
        "Removed material restriction {} from shop {}.\n\r", matType, shopNr);
      ch.sendTo(msg);
      vlogf(LOG_LOW, msg.trim());
      return;
    }

    ch.sendTo(std::format(
      "Shop {} did not have material restriction for material {}.\n\r", shopNr,
      matType));
  }

  struct AccessLevelDef {
      unsigned int bit;
      std::string_view name;
  };

  constexpr auto ACCESS_LEVELS = std::to_array<AccessLevelDef>({
    {.bit = SHOPACCESS_OWNER, .name = "owner"},
    {.bit = SHOPACCESS_INFO, .name = "info"},
    {.bit = SHOPACCESS_RATES, .name = "rates"},
    {.bit = SHOPACCESS_GIVE, .name = "give"},
    {.bit = SHOPACCESS_SELL, .name = "sell"},
    {.bit = SHOPACCESS_ACCESS, .name = "access"},
    {.bit = SHOPACCESS_LOGS, .name = "logs"},
    {.bit = SHOPACCESS_DIVIDEND, .name = "dividend"},
  });

  constexpr unsigned int MAX_ACCESS_LEVEL =
    SHOPACCESS_OWNER | SHOPACCESS_INFO | SHOPACCESS_RATES | SHOPACCESS_GIVE |
    SHOPACCESS_SELL | SHOPACCESS_ACCESS | SHOPACCESS_LOGS | SHOPACCESS_DIVIDEND;

  sstring describeAccess(unsigned int access) {
    if (access == 0) {
      return "(none)";
    }

    std::string result;
    for (const auto& [bit, name] : ACCESS_LEVELS) {
      if (access & bit) {
        if (!result.empty()) {
          result += ", ";
        }
        result += name;
      }
    }
    return result;
  }

  void showOwnerInfo(const TBeing& ch, int shopNr) {
    TDatabase db(DB_SNEEZY);
    db.query(
      "SELECT so.corp_id, c.name AS corp_name "
      "FROM shopowned so "
      "LEFT JOIN corporation c ON so.corp_id = c.corp_id "
      "WHERE so.shop_nr=%i",
      shopNr);

    std::ostringstream str;
    str << std::format("Shop #{} Ownership\n\r", shopNr);
    str << "==================================================================="
           "====\n\r";

    if (!db.fetchRow()) {
      str << "Status: Not owned (available for purchase)\n\r";
    } else {
      const sstring corpId = db["corp_id"];
      const sstring corpName =
        db["corp_name"][0] ? db["corp_name"] : "(unknown corporation)";

      str << std::format("Status: Owned by {} (corp_id {})\n\r", corpName,
        corpId);
      str << "-----------------------------------------------------------------"
             "------\n\r";
      str << "Player Access:\n\r";

      db.query(
        "SELECT p.name, soa.access FROM shopownedaccess soa "
        "JOIN player p ON soa.player_id=p.id "
        "WHERE soa.shop_nr=%i ORDER BY soa.access DESC, p.name",
        shopNr);

      if (!db.rowCount()) {
        str << "  (no player access entries)\n\r";
      } else {
        while (db.fetchRow()) {
          const auto access = convertTo<unsigned int>(db["access"]);
          str << std::format("  {:>15}: {:>3} ({})\n\r", db["name"], access,
            describeAccess(access));
        }
      }
    }

    str << "-------------------------------------------------------------------"
           "----\n\r";
    str << "Access levels: 1=owner, 2=info, 4=rates, 8=give, 16=sell,\n\r";
    str << "               32=access, 64=logs, 128=dividend (sum for "
           "combined)\n\r";

    if (ch.desc) {
      ch.desc->page_string(str.str(), SHOWNOW_NO, ALLOWREP_YES);
    }
  }

  void setOwner(const TBeing& ch, int shopNr, int corpId) {
    TDatabase db(DB_SNEEZY);

    db.query("SELECT name FROM corporation WHERE corp_id=%i", corpId);
    if (!db.fetchRow()) {
      ch.sendTo(std::format("Corporation {} does not exist.\n\r", corpId));
      return;
    }
    const sstring corpName = db["name"];

    db.query("SELECT corp_id FROM shopowned WHERE shop_nr=%i", shopNr);
    if (db.fetchRow()) {
      if (!db.query("UPDATE shopowned SET corp_id=%i WHERE shop_nr=%i", corpId,
            shopNr)) {
        ch.sendTo("Database error: failed to update ownership.\n\r");
        return;
      }
      ch.sendTo(std::format("Shop {} ownership transferred to {} ({}).\n\r",
        shopNr, corpName, corpId));
    } else {
      if (!db.query(
            "INSERT INTO shopowned (shop_nr, profit_buy, profit_sell, corp_id) "
            "VALUES (%i, 1.1, 0.9, %i)",
            shopNr, corpId)) {
        ch.sendTo("Database error: failed to set ownership.\n\r");
        return;
      }
      ch.sendTo(std::format("Shop {} is now owned by {} ({}).\n\r", shopNr,
        corpName, corpId));
    }

    if (shopNr >= 0 && std::cmp_less(shopNr, shop_index.size())) {
      shop_index[shopNr].owned = true;
      shop_index[shopNr].clearCache();
    }

    vlogf(LOG_LOW, std::format("{} set shop {} owner to corp {}", ch.getName(),
                     shopNr, corpId));
  }

  void removeOwner(const TBeing& ch, int shopNr) {
    TDatabase db(DB_SNEEZY);

    db.query("SELECT corp_id FROM shopowned WHERE shop_nr=%i", shopNr);
    if (!db.fetchRow()) {
      ch.sendTo(std::format("Shop {} is not owned.\n\r", shopNr));
      return;
    }

    // FK cascades handle all shopowned child table cleanup automatically
    if (!db.query("DELETE FROM shopowned WHERE shop_nr=%i", shopNr)) {
      ch.sendTo("Database error: failed to remove ownership.\n\r");
      return;
    }

    if (shopNr >= 0 && std::cmp_less(shopNr, shop_index.size())) {
      shop_index[shopNr].owned = false;
      shop_index[shopNr].clearCache();
    }

    ch.sendTo(std::format("Shop {} ownership removed.\n\r", shopNr));
    vlogf(LOG_LOW,
      std::format("{} removed ownership from shop {}", ch.getName(), shopNr));
  }

  void manageOwner(const TBeing& ch, const sstring& arg) {
    sstring argument = arg;
    sstring shopNrArg;
    argument = one_argument(argument, shopNrArg);

    if (shopNrArg.empty()) {
      ch.sendTo(
        "Syntax: low shop owner <shop_nr>\n\r"
        "        low shop owner <shop_nr> set <corp_id>\n\r"
        "        low shop owner <shop_nr> remove\n\r");
      return;
    }

    const int shopNr = convertTo<int>(shopNrArg);

    TDatabase db(DB_SNEEZY);
    if (!requireShopExists(db, ch, shopNr)) {
      return;
    }

    sstring actionArg;
    argument = one_argument(argument, actionArg);

    if (actionArg.empty()) {
      showOwnerInfo(ch, shopNr);
      return;
    }

    if (is_abbrev(actionArg, "set")) {
      sstring corpIdArg;
      argument = one_argument(argument, corpIdArg);
      if (corpIdArg.empty()) {
        ch.sendTo("Syntax: low shop owner <shop_nr> set <corp_id>\n\r");
        return;
      }
      setOwner(ch, shopNr, convertTo<int>(corpIdArg));
      return;
    }

    if (is_abbrev(actionArg, "remove")) {
      removeOwner(ch, shopNr);
      return;
    }

    ch.sendTo(
      "Unknown action. Use 'set <corp_id>' or 'remove'.\n\r"
      "Syntax: low shop owner <shop_nr> [set <corp_id> | remove]\n\r");
  }

  void manageAccess(const TBeing& ch, const sstring& arg) {
    sstring argument = arg;
    sstring shopNrArg;
    argument = one_argument(argument, shopNrArg);

    sstring playerArg;
    argument = one_argument(argument, playerArg);

    sstring levelArg;
    argument = one_argument(argument, levelArg);

    if (shopNrArg.empty() || playerArg.empty() || levelArg.empty()) {
      ch.sendTo(
        "Syntax: low shop access <shop_nr> <player> <level>\n\r"
        "\n\r"
        "Access levels (sum for combined):\n\r"
        "    1 = owner      - full control (except sell without explicit "
        "16)\n\r"
        "    2 = info       - view shop information\n\r"
        "    4 = rates      - change buy/sell ratios\n\r"
        "    8 = give       - withdraw money from shop\n\r"
        "   16 = sell       - sell the shop\n\r"
        "   32 = access     - change permissions for other players\n\r"
        "   64 = logs       - view transaction logs\n\r"
        "  128 = dividend   - set dividend rates\n\r");
      return;
    }

    const int shopNr = convertTo<int>(shopNrArg);

    TDatabase db(DB_SNEEZY);
    if (!requireShopExists(db, ch, shopNr)) {
      return;
    }

    db.query("SELECT corp_id FROM shopowned WHERE shop_nr=%i", shopNr);
    if (!db.fetchRow()) {
      ch.sendTo(
        std::format("Shop {} is not owned. Use 'low shop owner {} set "
                    "<corp_id>' first.\n\r",
          shopNr, shopNr));
      return;
    }

    const auto level = convertTo<unsigned int>(levelArg);
    if (level > MAX_ACCESS_LEVEL) {
      ch.sendTo(
        std::format("Access level must be 0-{} (sum of individual levels).\n\r",
          MAX_ACCESS_LEVEL));
      return;
    }

    // Resolve player name to player_id
    db.query("SELECT id FROM player WHERE name='%s'", playerArg.c_str());
    if (!db.fetchRow()) {
      ch.sendTo(std::format("Player '{}' not found.\n\r", playerArg));
      return;
    }
    const auto targetId = convertTo<int>(db["id"]);

    if (!db.query("DELETE FROM shopownedaccess WHERE shop_nr=%i AND "
                  "player_id=%i",
          shopNr, targetId)) {
      ch.sendTo("Database error: failed to update access.\n\r");
      return;
    }

    if (level > 0) {
      if (!db.query("INSERT INTO shopownedaccess (shop_nr, player_id, access) "
                    "VALUES (%i, %i, %i)",
            shopNr, targetId, level)) {
        ch.sendTo("Database error: failed to set access.\n\r");
        return;
      }
      ch.sendTo(std::format("Shop {}: {} access set to {} ({}).\n\r", shopNr,
        playerArg, level, describeAccess(level)));
    } else {
      ch.sendTo(
        std::format("Shop {}: {} access removed.\n\r", shopNr, playerArg));
    }

    vlogf(LOG_LOW, std::format("{} set shop {} access for {} to {}",
                     ch.getName(), shopNr, playerArg, level));
  }

  void deleteShop(const TBeing& ch, const sstring& arg) {
    sstring argument = arg;
    sstring shopNrArg;
    argument = one_argument(argument, shopNrArg);

    if (shopNrArg.empty()) {
      ch.sendTo("Syntax: low shop delete <shop_nr> confirm\n\r");
      return;
    }

    const int shopNr = convertTo<int>(shopNrArg);

    sstring confirmArg;
    argument = one_argument(argument, confirmArg);

    if (confirmArg.lower() != "confirm") {
      ch.sendTo(
        std::format("To delete shop {}, type: low shop delete {} confirm\n\r",
          shopNr, shopNr));
      return;
    }

    TDatabase db(DB_SNEEZY);
    if (!requireShopExists(db, ch, shopNr)) {
      return;
    }

    // FK cascades handle all child table cleanup automatically
    if (!db.query("DELETE FROM shop WHERE shop_nr=%i", shopNr)) {
      ch.sendTo("Database error: failed to delete shop.\n\r");
      return;
    }
    ch.sendTo(std::format("Shop {} and all related data deleted.\n\r", shopNr));
    ch.sendTo("Changes take effect after 'boot zone' or server reboot.\n\r");
    vlogf(LOG_LOW, std::format("{} deleted shop {}", ch.getName(), shopNr));
  }

  struct ShopFnDef {
      std::string_view keyword;
      void (*fn)(const TBeing&, const sstring&);
  };

  constexpr auto SHOP_FNS = std::to_array<ShopFnDef>({
    {"list", listShops},
    {"types", listItemTypes},
    {"materials", listMaterials},
    {"info", showShopInfo},
    {"owner", manageOwner},
    {"access", manageAccess},
    {"create", createShop},
    {"modify", modifyShop},
    {"addtype", addShopType},
    {"rmtype", removeShopType},
    {"addprod", addShopProduct},
    {"rmprod", removeShopProduct},
    {"addmat", addShopMaterial},
    {"rmmat", removeShopMaterial},
    {"delete", deleteShop},
  });

  constexpr const char* const SHOP_USAGE =
    "Syntax:\n\r"
    "  low shop list [zone]\n\r"
    "  low shop types\n\r"
    "  low shop materials\n\r"
    "  low shop info <shop_nr>\n\r"
    "  low shop owner <shop_nr> [set <corp_id> | remove]\n\r"
    "  low shop access <shop_nr> <player> <level>\n\r"
    "  low shop create <shop_nr> <keeper_vnum> <room_vnum>\n\r"
    "  low shop modify <shop_nr> <field> <value>\n\r"
    "  low shop addtype <shop_nr> <item_type>\n\r"
    "  low shop rmtype <shop_nr> <item_type>\n\r"
    "  low shop addprod <shop_nr> <obj_vnum>\n\r"
    "  low shop rmprod <shop_nr> <obj_vnum>\n\r"
    "  low shop addmat <shop_nr> <mat_type>\n\r"
    "  low shop rmmat <shop_nr> <mat_type>\n\r"
    "  low shop delete <shop_nr> confirm\n\r"
    "\n\r"
    "Modifiable fields:\n\r"
    "  profit_buy, profit_sell (e.g. 1.1 = 10%% markup)\n\r"
    "  open1, close1, open2, close2 (hour 0-23, or 96 for always open)\n\r"
    "  temper1, temper2 (0-3)\n\r"
    "  flags (integer)\n\r"
    "  keeper (mob vnum), in_room (room vnum)\n\r"
    "  no_such_item1, no_such_item2, do_not_buy, missing_cash1,\n\r"
    "  missing_cash2, message_buy, message_sell (text messages)\n\r"
    "\n\r"
    "Changes take effect after 'boot zone' or server reboot.\n\r";
}  // namespace

void lowShop(const TBeing& ch, const sstring& arg) {
  sstring argument = arg;
  sstring firstArg;
  argument = one_argument(argument, firstArg);

  if (firstArg.empty()) {
    ch.sendTo(SHOP_USAGE);
    return;
  }

  for (const auto [keyword, fn] : SHOP_FNS) {
    if (is_abbrev(firstArg, keyword)) {
      fn(ch, argument);
      return;
    }
  }

  ch.sendTo(SHOP_USAGE);
}
