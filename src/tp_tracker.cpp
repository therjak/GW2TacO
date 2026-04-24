module;
#include <algorithm>
#include <format>
#include <future>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "src/gw2_tactical.h"
#include "src/util/jsonxx.h"
#include "src/util/png_decompressor.h"

module taco.tp_tracker;

import math;
import taco.gw2;
import taco.language;
import taco.overlay_config;
import time;
import whiteboard;

using math::Point;
using math::Rect;

namespace {
std::mutex item_data_cache_mtx;
std::unordered_map<int32_t, GW2ItemData> item_data_cache;

bool HasGW2ItemData(int32_t item_id) {
  std::lock_guard<std::mutex> lock_guard(item_data_cache_mtx);
  return item_data_cache.find(item_id) != item_data_cache.end();
}

GW2ItemData GetGW2ItemData(int32_t item_id) {
  std::lock_guard<std::mutex> lock_guard(item_data_cache_mtx);
  if (item_data_cache.find(item_id) != item_data_cache.end()) {
    return item_data_cache[item_id];
  }
  return {};
}

void SetGW2ItemData(GW2ItemData& data) {
  std::lock_guard<std::mutex> lock_guard(item_data_cache_mtx);
  item_data_cache[data.item_id] = data;
}

bool ParseTransaction(jsonxx::Object& object, TransactionItem& output) {
  if (!object.has<jsonxx::Number>("id") ||
      !object.has<jsonxx::Number>("item_id") ||
      !object.has<jsonxx::Number>("price") ||
      !object.has<jsonxx::Number>("quantity") ||
      !object.has<jsonxx::String>("created")) {
    return false;
  }
  output.transaction_id = int32_t(object.get<jsonxx::Number>("id"));
  output.item_id = int32_t(object.get<jsonxx::Number>("item_id"));
  output.price = int32_t(object.get<jsonxx::Number>("price"));
  output.quantity = int32_t(object.get<jsonxx::Number>("quantity"));
  output.created = object.get<jsonxx::String>("created");
  return true;
}

std::vector<TransactionItem> ParseTransactionList(const std::string& json_data,
                                                  const std::string& root_key) {
  std::vector<TransactionItem> result;
  jsonxx::Object json;
  json.parse(json_data);

  if (json.has<jsonxx::Array>(root_key)) {
    auto data = json.get<jsonxx::Array>(root_key).values();

    for (auto& x : data) {
      if (!x->is<jsonxx::Object>()) continue;

      jsonxx::Object& item = x->get<jsonxx::Object>();

      TransactionItem item_data;
      if (ParseTransaction(item, item_data)) {
        result.push_back(item_data);
      }
    }
  }

  return result;
}

std::vector<GW2ItemData> ParseGW2Items(const std::string& items_json) {
  std::vector<GW2ItemData> result;
  jsonxx::Object item_json;
  item_json.parse(items_json);

  if (item_json.has<jsonxx::Array>("items")) {
    auto items = item_json.get<jsonxx::Array>("items").values();

    for (auto& x : items) {
      if (!x->is<jsonxx::Object>()) continue;

      jsonxx::Object& item = x->get<jsonxx::Object>();

      GW2ItemData item_data;
      if (!item.has<jsonxx::String>("name") ||
          !item.has<jsonxx::Number>("id")) {
        continue;
      }
      item_data.name = item.get<jsonxx::String>("name");
      item_data.item_id = int32_t(item.get<jsonxx::Number>("id"));
      if (item.has<jsonxx::String>("icon")) {
        item_data.icon_file = item.get<jsonxx::String>("icon");
      }

      result.push_back(item_data);
    }
  }

  return result;
}

}  // namespace

std::string FetchHTTPS(std::string_view url, std::string_view path);

__inline std::string ToGold(int32_t value) {
  int32_t copper = value % 100;
  value /= 100;
  int32_t silver = value % 100;
  value /= 100;

  std::string result;
  if (value) {
    result = std::format("{:d}{:s} {:02d}{:s} {:02d}{:s}", value, DICT("gold"),
                         silver, DICT("silver"), copper, DICT("copper"));
  } else {
    if (silver) {
      result = std::format("{:d}{:s} {:02d}{:s}", silver, DICT("silver"),
                           copper, DICT("copper"));
    } else {
      result = std::format("{:d}{:s}", copper, DICT("copper"));
    }
  }

  return result;
}

void TPTracker::OnDraw(gui::CWBDrawAPI* api) {
  gui::CWBFont* f = GetFont(GetState());
  int32_t size = f->GetLineHeight();

  if (!HasConfigValue("TPTrackerOnlyShowOutbid")) {
    SetConfigValue("TPTrackerOnlyShowOutbid", 0);
  }

  if (!HasConfigValue("TPTrackerShowBuys")) {
    SetConfigValue("TPTrackerShowBuys", 1);
  }

  if (!HasConfigValue("TPTrackerShowSells")) {
    SetConfigValue("TPTrackerShowSells", 1);
  }

  if (!HasConfigValue("TPTrackerNextSellOnly")) {
    SetConfigValue("TPTrackerNextSellOnly", 0);
  }

  int32_t only_show_outbid = GetConfigValue("TPTrackerOnlyShowOutbid");
  int32_t next_sell_only = GetConfigValue("TPTrackerNextSellOnly");

  GW2::APIKeyManager::Status status =
      GW2::apiKeyManager.DisplayStatusText(api, f);
  GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();

  if (key && key->Valid() &&
      (GetTime() - last_fetch_time_ > 150000 || !last_fetch_time_)) {
    if (!fetch_task_.valid() || fetch_task_.wait_for(std::chrono::seconds(0)) ==
                                    std::future_status::ready) {
      last_fetch_time_ = GetTime();
      fetch_task_ = std::async(std::launch::async, [this, key]() {
        auto qbuys = "{\"buys\":" +
                     key->QueryAPI("/v2/commerce/transactions/current/buys");
        auto qsells = "{\"sells\":" +
                      key->QueryAPI("/v2/commerce/transactions/current/sells");

        std::vector<TransactionItem> incoming =
            ParseTransactionList(qbuys, "buys");
        std::vector<TransactionItem> outgoing =
            ParseTransactionList(qsells, "sells");

        std::vector<int32_t> unknown_items;
        std::vector<int32_t> price_check_list;

        auto process_items = [&](const std::vector<TransactionItem>& items) {
          for (const auto& item_data : items) {
            if (!HasGW2ItemData(item_data.item_id)) {
              unknown_items.push_back(item_data.item_id);
            }

            if (std::find(price_check_list.begin(), price_check_list.end(),
                          item_data.item_id) == price_check_list.end()) {
              price_check_list.push_back(item_data.item_id);
            }
          }
        };

        process_items(incoming);
        process_items(outgoing);

        std::string item_ids;

        if (!unknown_items.empty()) {
          for (const auto& i : unknown_items) {
            item_ids += std::to_string(i) + ',';
          }

          // https://api.guildwars2.com/v2/items?ids=28445,12452
          auto items =
              "{\"items\":" + key->QueryAPI("/v2/items?ids=" + item_ids) + "}";

          std::vector<GW2ItemData> parsed_items = ParseGW2Items(items);

          for (auto& item_data : parsed_items) {
            if (!item_data.icon_file.empty()) {
              if (item_data.icon_file.find("https://render.guildwars2.com/") ==
                  0) {
                auto png = FetchHTTPS("render.guildwars2.com",
                                      item_data.icon_file.substr(29));

                std::unique_ptr<uint8_t[]> image_data = nullptr;
                int32_t x_res = 0, y_res = 0;
                if (DecompressPNG((uint8_t*)png.c_str(), png.size(), image_data,
                                  x_res, y_res)) {
                  ARGBtoABGR(image_data.get(), x_res, y_res);
                  Rect area = Rect(0, 0, x_res, y_res);
                  item_data.icon = GetApplication()->GetAtlas()->AddImage(
                      image_data.get(), x_res, y_res, area);
                }
              }
            }

            SetGW2ItemData(item_data);
          }
        }

        {
          for (const auto& i : price_check_list) {
            item_ids += std::to_string(i) + ',';
          }

          // https://api.guildwars2.com/v2/commerce/prices?ids=19684,19709
          auto items = "{\"items\":" +
                       key->QueryAPI(("/v2/commerce/prices?ids=" + item_ids)) +
                       "}";

          jsonxx::Object item_json;
          item_json.parse(items);

          if (item_json.has<jsonxx::Array>("items")) {
            auto items = item_json.get<jsonxx::Array>("items").values();

            for (auto& x : items) {
              if (!x->is<jsonxx::Object>()) continue;

              jsonxx::Object& item = x->get<jsonxx::Object>();

              if (!item.has<jsonxx::Number>("id") ||
                  !item.has<jsonxx::Object>("buys") ||
                  !item.has<jsonxx::Object>("sells")) {
                continue;
              }

              int32_t id = int32_t(item.get<jsonxx::Number>("id"));
              if (!HasGW2ItemData(id)) continue;

              jsonxx::Object buys_ = item.get<jsonxx::Object>("buys");
              jsonxx::Object sells_ = item.get<jsonxx::Object>("sells");
              if (!buys_.has<jsonxx::Number>("unit_price") ||
                  !sells_.has<jsonxx::Number>("unit_price")) {
                continue;
              }

              GW2ItemData itemData = GetGW2ItemData(id);
              itemData.buy_price =
                  int32_t(buys_.get<jsonxx::Number>("unit_price"));
              itemData.sell_price =
                  int32_t(sells_.get<jsonxx::Number>("unit_price"));
              SetGW2ItemData(itemData);
            }
          }
        }

        {
          std::lock_guard<std::mutex> lock_guard(transaction_mtx_);
          buys_ = incoming;
          sells_ = outgoing;
        }
      });
    }
  }

  {
    std::lock_guard<std::mutex> lock_guard(transaction_mtx_);

    int32_t posy = 0;
    int32_t lh = f->GetLineHeight();

    if (!buys_.empty() && GetConfigValue("TPTrackerShowBuys")) {
      std::vector<int32_t> showed_already;

      int32_t text_pos_y = posy;
      int32_t written_count = 0;

      posy += lh + 2;

      for (size_t x = 0; x < buys_.size(); x++) {
        if (!HasGW2ItemData(buys_[x].item_id)) continue;

        const auto& itemData = GetGW2ItemData(buys_[x].item_id);
        bool outbid = buys_[x].price < itemData.buy_price;

        if (next_sell_only &&
            std::find(showed_already.begin(), showed_already.end(),
                      itemData.item_id) != showed_already.end()) {
          continue;
        }

        if (!only_show_outbid || outbid) {
          int32_t price = buys_[x].price;
          if (next_sell_only) {
            for (size_t y = x; y < buys_.size(); y++) {
              if (buys_[y].item_id == buys_[x].item_id) {
                price = std::max(buys_[y].price, buys_[x].price);
              }
            }
          }

          if (itemData.icon) {
            api->DrawAtlasElement(itemData.icon,
                                  Rect(lh, posy, lh * 2 + 5, posy + lh + 5),
                                  false, false, true, true, CColor{0xffffffff});
          }
          auto text = itemData.name + " " + ToGold(price);
          if (buys_[x].quantity > 1) {
            text = std::format("{:d} ", buys_[x].quantity) + text;
          }
          f->Write(api, text, Point(static_cast<int>(lh * 2.5 + 3), posy + 3),
                   !outbid ? CColor{0xffffffff} : CColor{0xffee6655});
          written_count++;
          posy += lh + 6;
          if (next_sell_only) {
            if (std::find(showed_already.begin(), showed_already.end(),
                          itemData.item_id) == showed_already.end()) {
              showed_already.push_back(itemData.item_id);
            }
          }
        }
      }
      posy += 2;

      if (written_count) {
        f->Write(api, DICT(only_show_outbid ? "outbidbuys" : "buylist"),
                 Point(0, text_pos_y), CColor{0xffffffff});
      } else {
        posy -= lh + 4;
      }
    }

    if (!sells_.empty() && GetConfigValue("TPTrackerShowSells")) {
      std::vector<int32_t> showed_already;

      int32_t text_pos_y = posy;
      int32_t written_count = 0;

      posy += lh + 2;

      for (size_t x = 0; x < sells_.size(); x++) {
        if (!HasGW2ItemData(sells_[x].item_id)) continue;
        const auto& itemData = GetGW2ItemData(sells_[x].item_id);
        bool outbid = sells_[x].price > itemData.sell_price;

        if (next_sell_only &&
            std::find(showed_already.begin(), showed_already.end(),
                      itemData.item_id) != showed_already.end()) {
          continue;
        }

        if (!only_show_outbid || outbid) {
          int32_t price = sells_[x].price;
          if (next_sell_only) {
            for (size_t y = x; y < sells_.size(); y++) {
              if (sells_[y].item_id == sells_[x].item_id) {
                price = std::min(sells_[y].price, sells_[x].price);
              }
            }
          }

          if (itemData.icon) {
            api->DrawAtlasElement(itemData.icon,
                                  Rect(lh, posy, lh * 2 + 5, posy + lh + 5),
                                  false, false, true, true, CColor{0xffffffff});
          }
          auto text = itemData.name + " " + ToGold(price);
          if (sells_[x].quantity > 1) {
            text = std::format("{:d} ", sells_[x].quantity) + text;
          }
          f->Write(api, text, Point(static_cast<int>(lh * 2.5 + 3), posy + 3),
                   !outbid ? CColor{0xffffffff} : CColor{0xffee6655});
          written_count++;
          posy += lh + 6;
          if (next_sell_only) {
            if (std::find(showed_already.begin(), showed_already.end(),
                          itemData.item_id) == showed_already.end()) {
              showed_already.push_back(itemData.item_id);
            }
          }
        }
      }

      if (written_count) {
        f->Write(api, DICT(only_show_outbid ? "outbidsells" : "selllist"),
                 Point(0, text_pos_y), CColor{0xffffffff});
      }
    }
  }

  DrawBorder(api);
}

TPTracker::TPTracker() : CWBGuiType() {}

TPTracker::~TPTracker() {}

gui::CWBItem* TPTracker::Factory(gui::CWBItem* root, CXMLNode& node,
                                 Rect& pos) {
  return TPTracker::Create(root, pos);
}

bool TPTracker::IsMouseTransparent(const Point& client_space_point,
                                   gui::WBMESSAGE message_type) {
  return true;
}
