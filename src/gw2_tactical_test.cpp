#include <gtest/gtest.h>

#include <string>
#include <unordered_map>

#include "src/gw2_tactical.h"

// Forward declaration of the internal function to be tested.
std::unordered_map<int32_t, Achievement> ParseAchievements(
    const std::string& achievements_data);

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
