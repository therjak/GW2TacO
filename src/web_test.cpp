#include <gtest/gtest.h>

#include <string>
#include <unordered_map>

import math;
import taco.web;

TEST(ParseAchievementsTest, ParsesAchievementsCorrectly) {
  const std::string json_data = R"({
      "achievements": [
          {
              "id": 1,
              "current": 10,
              "max": 50,
              "done": false,
              "repeated": 0,
              "unlocked": true
          },
          {
              "id": 5,
              "bits": [0, 1, 3],
              "done": true,
              "repeated": 2
          },
          {
              "id": 102,
              "done": true
          },
          {
              "id": 150,
              "bits": [2, 4],
              "done": false
          }
      ]
  })";

  auto result = ParseAchievements(json_data);

  EXPECT_EQ(result.size(), 4);

  // ID 1: Not done, no bits
  EXPECT_FALSE(result[1].done);
  EXPECT_TRUE(result[1].bits.empty());

  // ID 5: Done. ParseAchievements clears bits when done == true.
  EXPECT_TRUE(result[5].done);
  EXPECT_TRUE(result[5].bits.empty());

  // ID 102: Done, no bits.
  EXPECT_TRUE(result[102].done);
  EXPECT_TRUE(result[102].bits.empty());

  // ID 150: Not done, has bits.
  EXPECT_FALSE(result[150].done);
  ASSERT_EQ(result[150].bits.size(), 2);
  EXPECT_EQ(result[150].bits[0], 2);
  EXPECT_EQ(result[150].bits[1], 4);
}

TEST(ParseAchievementsTest, HandlesEmptyAchievements) {
  auto result = ParseAchievements(R"({"achievements": []})");
  EXPECT_TRUE(result.empty());
}

TEST(ParseAchievementsTest, HandlesMissingAchievementsKey) {
  auto result = ParseAchievements(R"({"some_other_key": [1, 2, 3]})");
  EXPECT_TRUE(result.empty());
}

TEST(ParseAchievementsTest, HandlesInvalidJson) {
  auto result = ParseAchievements(R"(invalid json text)");
  EXPECT_TRUE(result.empty());
}

TEST(ParseTokenInfoTest, ParsesTokenInfoCorrectly) {
  const std::string json_data = R"({
      "id": "A1B2C3D4-E5F6-7890-1234-567890ABCDEF",
      "name": "MyToken",
      "permissions": ["account", "characters", "pvp"]
  })";

  auto result = ParseTokenInfo(json_data);

  EXPECT_EQ(result.id, "A1B2C3D4-E5F6-7890-1234-567890ABCDEF");
  EXPECT_TRUE(result.name.has_value());
  EXPECT_EQ(result.name.value(), "MyToken");
  EXPECT_TRUE(result.permissions.has_value());
  EXPECT_EQ(result.permissions.value().size(), 3);
}

TEST(ParseTokenInfoTest, HandlesInvalidJson) {
  auto result = ParseTokenInfo(R"(invalid)");
  EXPECT_TRUE(result.id.empty());
}

TEST(ParseAccountInfoTest, ParsesAccountInfoCorrectly) {
  const std::string json_data = R"({
      "id": "A1B2C3D4-E5F6-7890-1234-567890ABCDEF",
      "name": "User.1234",
      "age": 1000,
      "world": 1001,
      "guilds": ["111", "222"],
      "guild_leader": ["111"],
      "created": "2015-01-01T00:00:00Z",
      "access": ["HeartOfThorns", "PathOfFire"],
      "commander": true,
      "fractal_level": 100,
      "daily_ap": 500,
      "monthly_ap": 200,
      "wvw_rank": 500,
      "last_modified": "2023-01-01T00:00:00Z"
  })";

  auto result = ParseAccountInfo(json_data);

  EXPECT_EQ(result.id, "A1B2C3D4-E5F6-7890-1234-567890ABCDEF");
  EXPECT_TRUE(result.name.has_value());
  EXPECT_EQ(result.name.value(), "User.1234");
  EXPECT_EQ(result.age.value(), 1000);
  EXPECT_EQ(result.world.value(), 1001);
  EXPECT_TRUE(result.guilds.has_value());
  ASSERT_EQ(result.guilds.value().size(), 2);
  EXPECT_EQ(result.guilds.value()[0], "111");
  EXPECT_EQ(result.guilds.value()[1], "222");
  EXPECT_TRUE(result.guild_leader.has_value());
  ASSERT_EQ(result.guild_leader.value().size(), 1);
  EXPECT_EQ(result.guild_leader.value()[0], "111");
  EXPECT_TRUE(result.created.has_value());
  EXPECT_EQ(result.created.value(), "2015-01-01T00:00:00Z");
  EXPECT_TRUE(result.access.has_value());
  ASSERT_EQ(result.access.value().size(), 2);
  EXPECT_EQ(result.access.value()[0], "HeartOfThorns");
  EXPECT_EQ(result.access.value()[1], "PathOfFire");
  EXPECT_TRUE(result.commander.value());
  EXPECT_EQ(result.fractal_level.value(), 100);
  EXPECT_EQ(result.daily_ap.value(), 500);
  EXPECT_EQ(result.monthly_ap.value(), 200);
  EXPECT_EQ(result.wvw_rank.value(), 500);
  EXPECT_TRUE(result.last_modified.has_value());
  EXPECT_EQ(result.last_modified.value(), "2023-01-01T00:00:00Z");
}

TEST(ParseAccountInfoTest, HandlesInvalidJson) {
  auto result = ParseAccountInfo(R"(invalid)");
  EXPECT_TRUE(result.id.empty());
}

TEST(ParseWvwObjectivesTest, ParsesCorrectly) {
  const std::string json_data = R"([
    {
      "id": "38-11",
      "name": "Aldon's Ledge",
      "type": "Tower",
      "sector_id": 835,
      "map_id": 38,
      "map_type": "Center",
      "coord": [11463.8, 12330.4, -18.6653],
      "label_coord": [11463.8, 12330.4],
      "marker": "https://render.guildwars2.com/file/marker.png",
      "chat_link": "[&DTgDAAA=]",
      "upgrade_id": 55
    }
  ])";
  auto result = ParseWvwObjectives(json_data);
  ASSERT_EQ(result.size(), 1);
  EXPECT_EQ(result[0].id, "38-11");
  EXPECT_EQ(result[0].name, "Aldon's Ledge");
  EXPECT_EQ(result[0].type, "Tower");
  EXPECT_EQ(result[0].sector_id, 835);
  EXPECT_EQ(result[0].map_id, 38);
  EXPECT_EQ(result[0].map_type, "Center");
  ASSERT_EQ(result[0].coord.size(), 3);
  EXPECT_FLOAT_EQ(result[0].coord[0], 11463.8f);
  ASSERT_EQ(result[0].label_coord.size(), 2);
  EXPECT_FLOAT_EQ(result[0].label_coord[0], 11463.8f);
  EXPECT_EQ(result[0].marker, "https://render.guildwars2.com/file/marker.png");
  EXPECT_EQ(result[0].chat_link, "[&DTgDAAA=]");
  EXPECT_EQ(result[0].upgrade_id, 55);
}

TEST(ParseWvwObjectivesTest, HandlesInvalidJson) {
  auto result = ParseWvwObjectives(R"(invalid)");
  EXPECT_TRUE(result.empty());
}

TEST(ParseWvwMapDataTest, ParsesCorrectly) {
  const std::string json_data = R"({
    "id": 38,
    "name": "Eternal Battlegrounds",
    "min_level": 80,
    "max_level": 80,
    "default_floor": 1,
    "type": "Public",
    "floors": [1, 2],
    "region_id": 6,
    "region_name": "The Mists",
    "continent_id": 2,
    "continent_name": "The Mists",
    "map_rect": [
      [-43008, -43008],
      [43008, 43008]
    ],
    "continent_rect": [
      [5376, 9216],
      [8448, 12288]
    ]
  })";
  auto result = ParseWvwMapData(json_data);
  EXPECT_EQ(result.id, 38);
  EXPECT_EQ(result.name, "Eternal Battlegrounds");
  EXPECT_EQ(result.min_level, 80);
  EXPECT_EQ(result.max_level, 80);
  EXPECT_EQ(result.default_floor, 1);
  EXPECT_EQ(result.type, "Public");
  ASSERT_EQ(result.floors.size(), 2);
  EXPECT_EQ(result.floors[0], 1);
  EXPECT_EQ(result.floors[1], 2);
  EXPECT_EQ(result.region_id, 6);
  EXPECT_EQ(result.region_name, "The Mists");
  EXPECT_EQ(result.continent_id, 2);
  EXPECT_EQ(result.continent_name, "The Mists");
  EXPECT_TRUE(result.map_rect.has_value());
  EXPECT_EQ(result.map_rect->x1, -43008);
  EXPECT_EQ(result.map_rect->y1, -43008);
  EXPECT_EQ(result.map_rect->x2, 43008);
  EXPECT_EQ(result.map_rect->y2, 43008);
  EXPECT_TRUE(result.continent_rect.has_value());
  EXPECT_EQ(result.continent_rect->x1, 5376);
  EXPECT_EQ(result.continent_rect->y1, 9216);
  EXPECT_EQ(result.continent_rect->x2, 8448);
  EXPECT_EQ(result.continent_rect->y2, 12288);
}

TEST(ParseWvwMapDataTest, HandlesInvalidJson) {
  auto result = ParseWvwMapData(R"(invalid)");
  EXPECT_EQ(result.id, 0);
}

TEST(ParseWvwMatchTest, ParsesCorrectly) {
  const std::string json_data = R"({
    "id": "2-1",
    "start_time": "2023-01-01T00:00:00Z",
    "end_time": "2023-01-08T00:00:00Z",
    "scores": { "red": 100, "blue": 200, "green": 300 },
    "worlds": { "red": 1001, "blue": 1002, "green": 1003 },
    "all_worlds": {
      "red": [1001, 1004],
      "blue": [1002, 1005],
      "green": [1003, 1006]
    },
    "deaths": { "red": 10, "blue": 20, "green": 30 },
    "kills": { "red": 1, "blue": 2, "green": 3 },
    "victory_points": { "red": 10, "blue": 20, "green": 30 },
    "maps": [
      {
        "id": 38,
        "type": "Center",
        "scores": { "red": 50, "blue": 50, "green": 50 },
        "bonuses": [ { "type": "Bloodlust", "owner": "red" } ],
        "deaths": { "red": 5, "blue": 5, "green": 5 },
        "kills": { "red": 1, "blue": 1, "green": 1 },
        "objectives": [
          {
            "id": "38-1",
            "type": "Castle",
            "owner": "red",
            "last_flipped": "2023-01-01T01:00:00Z",
            "claimed_by": "guild1",
            "claimed_at": "2023-01-01T01:05:00Z",
            "points_tick": 35,
            "points_capture": 0,
            "guild_upgrades": [ 1, 2 ],
            "yaks_delivered": 10
          }
        ]
      }
    ],
    "skirmishes": [
      {
        "id": 1,
        "scores": { "red": 10, "blue": 20, "green": 30 },
        "map_scores": [
          {
            "type": "Center",
            "scores": { "red": 5, "blue": 10, "green": 15 }
          }
        ]
      }
    ]
  })";
  auto result = ParseWvwMatch(json_data);
  EXPECT_EQ(result.id, "2-1");
  EXPECT_EQ(result.start_time, "2023-01-01T00:00:00Z");
  EXPECT_EQ(result.scores["red"], 100);
  EXPECT_EQ(result.worlds["blue"], 1002);
  ASSERT_EQ(result.all_worlds["green"].size(), 2);
  EXPECT_EQ(result.all_worlds["green"][0], 1003);
  EXPECT_EQ(result.deaths["red"], 10);
  EXPECT_EQ(result.kills["blue"], 2);
  EXPECT_EQ(result.victory_points["green"], 30);
  ASSERT_EQ(result.maps.size(), 1);
  EXPECT_EQ(result.maps[0].id, 38);
  EXPECT_EQ(result.maps[0].type, "Center");
  EXPECT_EQ(result.maps[0].scores["red"], 50);
  ASSERT_EQ(result.maps[0].bonuses.size(), 1);
  EXPECT_EQ(result.maps[0].bonuses[0].type, "Bloodlust");
  EXPECT_EQ(result.maps[0].bonuses[0].owner, "red");
  ASSERT_EQ(result.maps[0].objectives.size(), 1);
  EXPECT_EQ(result.maps[0].objectives[0].id, "38-1");
  EXPECT_EQ(result.maps[0].objectives[0].type, "Castle");
  EXPECT_EQ(result.maps[0].objectives[0].owner, "red");
  EXPECT_EQ(result.maps[0].objectives[0].last_flipped, "2023-01-01T01:00:00Z");
  EXPECT_EQ(result.maps[0].objectives[0].claimed_by, "guild1");
  EXPECT_EQ(result.maps[0].objectives[0].points_tick, 35);
  ASSERT_EQ(result.maps[0].objectives[0].guild_upgrades.size(), 2);
  EXPECT_EQ(result.maps[0].objectives[0].yaks_delivered, 10);
  ASSERT_EQ(result.skirmishes.size(), 1);
  EXPECT_EQ(result.skirmishes[0].id, 1);
  EXPECT_EQ(result.skirmishes[0].scores["red"], 10);
  ASSERT_EQ(result.skirmishes[0].map_scores.size(), 1);
  EXPECT_EQ(result.skirmishes[0].map_scores[0].type, "Center");
  EXPECT_EQ(result.skirmishes[0].map_scores[0].scores["blue"], 10);
}

TEST(ParseWvwMatchTest, HandlesInvalidJson) {
  auto result = ParseWvwMatch(R"(invalid)");
  EXPECT_TRUE(result.id.empty());
}

TEST(ParseTransactionListTest, ParsesCorrectly) {
  const std::string json_data = R"([
    {
      "id": 1234567890123,
      "item_id": 456,
      "price": 100,
      "quantity": 2,
      "created": "2023-01-01T00:00:00Z"
    }
  ])";
  auto result = ParseTransactionList(json_data);
  ASSERT_EQ(result.size(), 1);
  EXPECT_EQ(result[0].transaction_id, 1234567890123LL);
  EXPECT_EQ(result[0].item_id, 456);
  EXPECT_EQ(result[0].price, 100);
  EXPECT_EQ(result[0].quantity, 2);
  EXPECT_EQ(result[0].created, "2023-01-01T00:00:00Z");
}

TEST(ParseTransactionListTest, HandlesInvalidJson) {
  auto result = ParseTransactionList(R"(invalid)");
  EXPECT_TRUE(result.empty());
}

TEST(ParseCommercePricesTest, ParsesCorrectly) {
  const std::string json_data = R"({
    "items": [
      {
        "id": 123,
        "whitelisted": true,
        "buys": {
          "quantity": 100,
          "unit_price": 50
        },
        "sells": {
          "quantity": 200,
          "unit_price": 60
        }
      }
    ]
  })";
  auto result = ParseCommercePrices(json_data);
  ASSERT_EQ(result.size(), 1);
  EXPECT_EQ(result[0].id, 123);
  EXPECT_TRUE(result[0].whitelisted);
  EXPECT_EQ(result[0].buys.quantity, 100);
  EXPECT_EQ(result[0].buys.unit_price, 50);
  EXPECT_EQ(result[0].sells.quantity, 200);
  EXPECT_EQ(result[0].sells.unit_price, 60);
}

TEST(ParseCommercePricesTest, HandlesInvalidJson) {
  auto result = ParseCommercePrices(R"(invalid)");
  EXPECT_TRUE(result.empty());
}

TEST(ParseGW2ItemsTest, ParsesCorrectly) {
  const std::string json_data = R"({
    "items": [
      {
        "name": "Copper Ore",
        "id": 19697,
        "icon": "https://render.guildwars2.com/file/icon.png"
      }
    ]
  })";
  auto result = ParseGW2Items(json_data);
  ASSERT_EQ(result.size(), 1);
  EXPECT_EQ(result[0].name, "Copper Ore");
  EXPECT_EQ(result[0].item_id, 19697);
  EXPECT_EQ(result[0].icon_file, "https://render.guildwars2.com/file/icon.png");
}

TEST(ParseGW2ItemsTest, HandlesInvalidJson) {
  auto result = ParseGW2Items(R"(invalid)");
  EXPECT_TRUE(result.empty());
}

TEST(ParseAccountAchievementTest, ParsesCorrectly) {
  const std::string json_data = R"({
      "id": 1,
      "current": 10,
      "max": 50,
      "done": false,
      "repeated": 0,
      "unlocked": true,
      "bits": [0, 1, 3]
  })";

  auto result = ParseAccountAchievement(json_data);
  EXPECT_EQ(result.id, 1);
  EXPECT_EQ(result.current.value(), 10);
  EXPECT_EQ(result.max.value(), 50);
  EXPECT_FALSE(result.done);
  EXPECT_EQ(result.repeated.value(), 0);
  EXPECT_TRUE(result.unlocked.value());
  ASSERT_EQ(result.bits.size(), 3);
  EXPECT_EQ(result.bits[0], 0);
  EXPECT_EQ(result.bits[1], 1);
  EXPECT_EQ(result.bits[2], 3);
}

TEST(ParseAccountAchievementTest, ParsesArrayFormat) {
  const std::string json_data = R"([{
      "id": 1,
      "done": true
  }])";
  auto result = ParseAccountAchievement(json_data);
  EXPECT_EQ(result.id, 1);
  EXPECT_TRUE(result.done);
}

TEST(ParseAccountAchievementTest, HandlesInvalidJson) {
  auto result = ParseAccountAchievement(R"(invalid)");
  EXPECT_EQ(result.id, 0);
}

TEST(ParseArrayTest, ParsesCorrectly) {
  const std::string json_data = R"(["element1", "element2"])";
  auto result = ParseArray(json_data);
  EXPECT_EQ(result.size(), 2);
  EXPECT_TRUE(result.contains("element1"));
  EXPECT_TRUE(result.contains("element2"));
}

TEST(ParseArrayTest, HandlesInvalidJson) {
  auto result = ParseArray(R"(invalid)");
  EXPECT_TRUE(result.empty());
}
