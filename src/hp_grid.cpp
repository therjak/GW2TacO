module;

#include <cstdio>
#include <memory>
#include <vector>

#include "src/base/color.h"

module taco.hp_grid;

import math;
import taco.mumble_link;
import taco.overlay_config;
import xml;

using math::Point;
using math::Rect;

void GW2HPGrid::OnDraw(gui::CWBDrawAPI* api) {
  if (!GetConfigValue("HPGridVisible")) {
    return;
  }

  Rect cl = GetClientRect();

  for (auto& grid : grids_) {
    if (mumbleLink.map_id != grid.map_id) {
      continue;
    }

    if (!grid.b_sphere.Contains(mumbleLink.charPosition)) {
      continue;
    }

    for (const auto& dp : grid.displayed_percentages) {
      int pos = static_cast<int>(cl.Width() * dp.percentage / 100.0f);
      Rect r = Rect(pos, cl.y1, pos + 1, cl.y2);
      api->DrawRect(r, dp.color);
    }
  }
}

GW2HPGrid::GW2HPGrid() : CWBGuiType() { LoadGrids(); }

GW2HPGrid::~GW2HPGrid() = default;

gui::CWBItem* GW2HPGrid::Factory(gui::CWBItem* root, const CXMLNode& node,
                                 Rect& pos) {
  return GW2HPGrid::Create(root, pos);
}

bool GW2HPGrid::IsMouseTransparent(const Point& client_space_point,
                                   gui::WBMESSAGE message_type) {
  return true;
}

void GW2HPGrid::LoadGrids() {
  auto d = std::make_unique<CXMLDocument>();
  if (!d->LoadFromFile("hpgrids.xml")) {
    return;
  }

  if (!d->GetDocumentNode().GetChildCount("hpgrids")) {
    return;
  }
  CXMLNode root = d->GetDocumentNode().GetChild("hpgrids");

  for (int32_t x = 0; x < root.GetChildCount("grid"); x++) {
    CXMLNode node = root.GetChild("grid", x);
    GridData gd;

    if (node.HasAttribute("mapid")) {
      node.GetAttributeAsInteger("mapid", &gd.map_id);
    }

    if (node.HasAttribute("centerx")) {
      node.GetAttributeAsFloat("centerx", &gd.b_sphere.Position.x);
    }
    if (node.HasAttribute("centery")) {
      node.GetAttributeAsFloat("centery", &gd.b_sphere.Position.y);
    }
    if (node.HasAttribute("centerz")) {
      node.GetAttributeAsFloat("centerz", &gd.b_sphere.Position.z);
    }
    if (node.HasAttribute("radius")) {
      node.GetAttributeAsFloat("radius", &gd.b_sphere.Radius);
    }

    for (int32_t y = 0; y < node.GetChildCount("percentage"); y++) {
      CXMLNode perc = node.GetChild("percentage", y);
      GridLine line;
      if (perc.HasAttribute("value")) {
        perc.GetAttributeAsFloat("value", &line.percentage);
      }
      if (perc.HasAttribute("color")) {
        auto colhex = perc.GetAttributeAsString("color");
        unsigned int val = 0;
        std::sscanf(colhex.c_str(), "%x", &val);
        line.color = CColor(val);
      }
      gd.displayed_percentages.emplace_back(std::move(line));
    }

    grids_.emplace_back(std::move(gd));
  }
}
