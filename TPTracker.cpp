#include "TPTracker.h"

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "Bedrock/BaseLib/string_format.h"
#include "Bedrock/UtilLib/PNGDecompressor.h"
#include "GW2API.h"
#include "Language.h"
#include "OverlayConfig.h"

using namespace jsonxx;

LIGHTWEIGHT_CRITICALSECTION itemCacheCritSec;

std::unordered_map<int32_t, GW2ItemData> itemDataCache;

TBOOL HasGW2ItemData(int32_t itemID) {
  CLightweightCriticalSection cs(&itemCacheCritSec);
  return itemDataCache.find(itemID) != itemDataCache.end();
}

GW2ItemData GetGW2ItemData(int32_t itemID) {
  CLightweightCriticalSection cs(&itemCacheCritSec);
  if (itemDataCache.find(itemID) != itemDataCache.end())
    return itemDataCache[itemID];
  return GW2ItemData();
}

void SetGW2ItemData(GW2ItemData& data) {
  CLightweightCriticalSection cs(&itemCacheCritSec);
  itemDataCache[data.itemID] = data;
}

std::string FetchHTTPS(std::string_view url, std::string_view path);

__inline std::string ToGold(int32_t value) {
  int32_t copper = value % 100;
  value /= 100;
  int32_t silver = value % 100;
  value /= 100;

  std::string result;
  if (value) {
    result += FormatString("%d", value) + DICT("gold") + " ";
    result += FormatString("%.2d", silver) + DICT("silver") + " ";
    result += FormatString("%.2d", copper) + DICT("copper");
  } else {
    if (silver) {
      result += FormatString("%d", silver) + DICT("silver") + " ";
      result += FormatString("%.2d", copper) + DICT("copper");
    } else
      result = FormatString("%d", copper) + DICT("copper");
  }

  return result;
}

void TPTracker::OnDraw(CWBDrawAPI* API) {
  CWBFont* f = GetFont(GetState());
  int32_t size = f->GetLineHeight();

  if (!HasConfigValue("TPTrackerOnlyShowOutbid"))
    SetConfigValue("TPTrackerOnlyShowOutbid", 0);

  if (!HasConfigValue("TPTrackerShowBuys"))
    SetConfigValue("TPTrackerShowBuys", 1);

  if (!HasConfigValue("TPTrackerShowSells"))
    SetConfigValue("TPTrackerShowSells", 1);

  if (!HasConfigValue("TPTrackerNextSellOnly"))
    SetConfigValue("TPTrackerNextSellOnly", 0);

  int32_t onlyShowOutbid = GetConfigValue("TPTrackerOnlyShowOutbid");
  int32_t nextSellOnly = GetConfigValue("TPTrackerNextSellOnly");

  GW2::APIKeyManager::Status status =
      GW2::apiKeyManager.DisplayStatusText(API, f);
  GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();

  if (key && key->valid &&
      (GetTime() - lastFetchTime > 150000 || !lastFetchTime) && !beingFetched &&
      !fetchThread.joinable()) {
    beingFetched = true;
    fetchThread = std::thread([this, key]() {
      auto qbuys =
          "{\"buys\":" + key->QueryAPI("v2/commerce/transactions/current/buys");
      auto qsells = "{\"sells\":" +
                    key->QueryAPI("v2/commerce/transactions/current/sells");

      Object json;
      Object json2;
      json.parse(qbuys);
      json2.parse(qsells);

      std::vector<TransactionItem> incoming;
      std::vector<TransactionItem> outgoing;

      std::vector<int32_t> unknownItems;
      std::vector<int32_t> priceCheckList;

      if (json.has<Array>("buys")) {
        auto buyData = json.get<Array>("buys").values();

        for (auto& x : buyData) {
          if (!x->is<Object>()) continue;

          Object& item = x->get<Object>();

          TransactionItem itemData;
          if (!TPTracker::ParseTransaction(item, itemData)) continue;
          incoming.push_back(itemData);

          if (itemDataCache.find(itemData.itemID) == itemDataCache.end())
            unknownItems.push_back(itemData.itemID);

          if (std::find(priceCheckList.begin(), priceCheckList.end(),
                        itemData.itemID) == priceCheckList.end()) {
            priceCheckList.push_back(itemData.itemID);
          }
        }
      }

      if (json2.has<Array>("sells")) {
        auto buyData = json2.get<Array>("sells").values();

        for (auto& x : buyData) {
          if (!x->is<Object>()) continue;

          Object& item = x->get<Object>();

          TransactionItem itemData;
          if (!TPTracker::ParseTransaction(item, itemData)) continue;
          outgoing.push_back(itemData);

          if (itemDataCache.find(itemData.itemID) == itemDataCache.end())
            unknownItems.push_back(itemData.itemID);

          if (std::find(priceCheckList.begin(), priceCheckList.end(),
                        itemData.itemID) == priceCheckList.end()) {
            priceCheckList.push_back(itemData.itemID);
          }
        }
      }

      std::string itemIds;

      if (!unknownItems.empty()) {
        for (const auto& i : unknownItems) {
          itemIds += std::to_string(i) + ',';
        }

        // https://api.guildwars2.com/v2/items?ids=28445,12452
        auto items =
            "{\"items\":" + key->QueryAPI("v2/items?ids=" + itemIds) + "}";

        Object itemjson;
        itemjson.parse(items);

        if (itemjson.has<Array>("items")) {
          auto items = itemjson.get<Array>("items").values();

          for (auto& x : items) {
            if (!x->is<Object>()) continue;

            Object& item = x->get<Object>();

            GW2ItemData itemData;
            if (!item.has<String>("name") || !item.has<Number>("id")) continue;
            itemData.name = item.get<String>("name");
            itemData.itemID = int32_t(item.get<Number>("id"));
            if (item.has<String>("icon")) {
              auto iconFile = item.get<String>("icon");
              if (iconFile.find("https://render.guildwars2.com/") == 0) {
                auto png =
                    FetchHTTPS("render.guildwars2.com", iconFile.substr(29));

                uint8_t* imageData = nullptr;
                int32_t xres, yres;
                if (DecompressPNG((uint8_t*)png.c_str(), png.size(), imageData,
                                  xres, yres)) {
                  ARGBtoABGR(imageData, xres, yres);
                  CRect area = CRect(0, 0, xres, yres);
                  itemData.icon = GetApplication()->GetAtlas()->AddImage(
                      imageData, xres, yres, area);
                }
              }
            }

            SetGW2ItemData(itemData);
          }
        }
      }

      {
        for (const auto& i : priceCheckList) {
          itemIds += std::to_string(i) + ',';
        }

        // https://api.guildwars2.com/v2/commerce/prices?ids=19684,19709
        auto items = "{\"items\":" +
                     key->QueryAPI(("v2/commerce/prices?ids=" + itemIds)) + "}";

        Object itemjson;
        itemjson.parse(items);

        if (itemjson.has<Array>("items")) {
          auto items = itemjson.get<Array>("items").values();

          for (auto& x : items) {
            if (!x->is<Object>()) continue;

            Object& item = x->get<Object>();

            if (!item.has<Number>("id") || !item.has<Object>("buys") ||
                !item.has<Object>("sells"))
              continue;

            int32_t id = int32_t(item.get<Number>("id"));
            if (!HasGW2ItemData(id)) continue;

            Object buys = item.get<Object>("buys");
            Object sells = item.get<Object>("sells");
            if (!buys.has<Number>("unit_price") ||
                !sells.has<Number>("unit_price"))
              continue;

            GW2ItemData itemData = GetGW2ItemData(id);
            itemData.buyPrice = int32_t(buys.get<Number>("unit_price"));
            itemData.sellPrice = int32_t(sells.get<Number>("unit_price"));
            SetGW2ItemData(itemData);
          }
        }
      }

      {
        CLightweightCriticalSection cs(&itemCacheCritSec);
        buys = incoming;
        sells = outgoing;
      }

      beingFetched = false;
    });
  }

  if (!beingFetched && fetchThread.joinable()) {
    lastFetchTime = GetTime();
    fetchThread.join();
  }

  {
    CLightweightCriticalSection cs(&itemCacheCritSec);

    int32_t posy = 0;
    int32_t lh = f->GetLineHeight();

    if (!buys.empty() && GetConfigValue("TPTrackerShowBuys")) {
      std::vector<int32_t> showedAlready;

      int32_t textPosy = posy;
      int32_t writtenCount = 0;

      posy += lh + 2;

      for (size_t x = 0; x < buys.size(); x++) {
        if (!HasGW2ItemData(buys[x].itemID)) continue;

        auto& itemData = GetGW2ItemData(buys[x].itemID);
        TBOOL outbid = buys[x].price < itemData.buyPrice;

        if (nextSellOnly &&
            std::find(showedAlready.begin(), showedAlready.end(),
                      itemData.itemID) != showedAlready.end()) {
          continue;
        }

        if (!onlyShowOutbid || outbid) {
          int32_t price = buys[x].price;
          if (nextSellOnly) {
            for (size_t y = x; y < buys.size(); y++)
              if (buys[y].itemID == buys[x].itemID)
                price = max(buys[y].price, buys[x].price);
          }

          if (itemData.icon)
            API->DrawAtlasElement(itemData.icon,
                                  CRect(lh, posy, lh * 2 + 5, posy + lh + 5),
                                  false, false, true, true, 0xffffffff);
          auto text = itemData.name + " " + ToGold(price);
          if (buys[x].quantity > 1)
            text = FormatString("%d ", buys[x].quantity) + text;
          f->Write(API, text, CPoint(int(lh * 2.5 + 3), posy + 3),
                   !outbid ? 0xffffffff : 0xffee6655);
          writtenCount++;
          posy += lh + 6;
          if (nextSellOnly) {
            if (std::find(showedAlready.begin(), showedAlready.end(),
                          itemData.itemID) == showedAlready.end()) {
              showedAlready.push_back(itemData.itemID);
            }
          }
        }
      }
      posy += 2;

      if (writtenCount)
        f->Write(API, DICT(onlyShowOutbid ? "outbidbuys" : "buylist"),
                 CPoint(0, textPosy), 0xffffffff);
      else
        posy -= lh + 4;
    }

    if (!sells.empty() && GetConfigValue("TPTrackerShowSells")) {
      std::vector<int32_t> showedAlready;

      int32_t textPosy = posy;
      int32_t writtenCount = 0;

      posy += lh + 2;

      for (size_t x = 0; x < sells.size(); x++) {
        if (!HasGW2ItemData(sells[x].itemID)) continue;
        auto& itemData = GetGW2ItemData(sells[x].itemID);
        TBOOL outbid = sells[x].price > itemData.sellPrice;

        if (nextSellOnly &&
            std::find(showedAlready.begin(), showedAlready.end(),
                      itemData.itemID) != showedAlready.end()) {
          continue;
        }

        if (!onlyShowOutbid || outbid) {
          int32_t price = sells[x].price;
          if (nextSellOnly) {
            for (size_t y = x; y < sells.size(); y++)
              if (sells[y].itemID == sells[x].itemID)
                price = min(sells[y].price, sells[x].price);
          }

          if (itemData.icon)
            API->DrawAtlasElement(itemData.icon,
                                  CRect(lh, posy, lh * 2 + 5, posy + lh + 5),
                                  false, false, true, true, 0xffffffff);
          auto text = itemData.name + " " + ToGold(price);
          if (sells[x].quantity > 1)
            text = FormatString("%d ", sells[x].quantity) + text;
          f->Write(API, text, CPoint(int(lh * 2.5 + 3), posy + 3),
                   !outbid ? 0xffffffff : 0xffee6655);
          writtenCount++;
          posy += lh + 6;
          if (nextSellOnly) {
            if (std::find(showedAlready.begin(), showedAlready.end(),
                          itemData.itemID) == showedAlready.end()) {
              showedAlready.push_back(itemData.itemID);
            }
          }
        }
      }

      if (writtenCount)
        f->Write(API, DICT(onlyShowOutbid ? "outbidsells" : "selllist"),
                 CPoint(0, textPosy), 0xffffffff);
    }
  }

  DrawBorder(API);
}

TBOOL TPTracker::ParseTransaction(Object& object, TransactionItem& output) {
  if (!object.has<Number>("id") || !object.has<Number>("item_id") ||
      !object.has<Number>("price") || !object.has<Number>("quantity"))
    return false;
  output.transactionID = int32_t(object.get<Number>("id"));
  output.itemID = int32_t(object.get<Number>("item_id"));
  output.price = int32_t(object.get<Number>("price"));
  output.quantity = int32_t(object.get<Number>("quantity"));
  return true;
}

TPTracker::TPTracker(CWBItem* Parent, CRect Position)
    : CWBItem(Parent, Position) {}

TPTracker::~TPTracker() {
  if (fetchThread.joinable()) fetchThread.join();
}

CWBItem* TPTracker::Factory(CWBItem* Root, CXMLNode& node, CRect& Pos) {
  return TPTracker::Create(Root, Pos).get();
}

TBOOL TPTracker::IsMouseTransparent(CPoint& ClientSpacePoint,
                                    WBMESSAGE MessageType) {
  return true;
}
