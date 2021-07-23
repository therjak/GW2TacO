#include "HPGrid.h"

#include "Bedrock/BaseLib/Sphere.h"
#include "MumbleLink.h"
#include "OverlayConfig.h"

void GW2HPGrid::OnDraw(CWBDrawAPI* API) {
  if (!HasConfigValue("HPGridVisible")) {
    SetConfigValue("HPGridVisible", 1);
  }

  if (!GetConfigValue("HPGridVisible")) {
    return;
  }

  CRect cl = GetClientRect();

  for (auto& grid : Grids) {
    if (mumbleLink.mapID != grid.mapID) {
      continue;
    }

    if (!grid.bSphere.Contains(mumbleLink.charPosition)) {
      continue;
    }

    for (const auto& dp : grid.displayedPercentages) {
      int pos = static_cast<int>(cl.Width() * dp.percentage / 100.0f);
      CRect r = CRect(pos, cl.y1, pos + 1, cl.y2);
      API->DrawRect(r, dp.color);
    }
  }
}

GW2HPGrid::GW2HPGrid(CWBItem* Parent, CRect Position)
    : CWBItem(Parent, Position) {
  LoadGrids();
}

GW2HPGrid::~GW2HPGrid() = default;

CWBItem* GW2HPGrid::Factory(CWBItem* Root, CXMLNode& node, CRect& Pos) {
  return GW2HPGrid::Create(Root, Pos).get();
}

bool GW2HPGrid::IsMouseTransparent(CPoint& ClientSpacePoint,
                                   WBMESSAGE MessageType) {
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

    if (node.HasAttribute("mapid"))
      node.GetAttributeAsInteger("mapid", &gd.mapID);

    if (node.HasAttribute("centerx")) {
      node.GetAttributeAsFloat("centerx", &gd.bSphere.Position.x);
    }
    if (node.HasAttribute("centery")) {
      node.GetAttributeAsFloat("centery", &gd.bSphere.Position.y);
    }
    if (node.HasAttribute("centerz")) {
      node.GetAttributeAsFloat("centerz", &gd.bSphere.Position.z);
    }
    if (node.HasAttribute("radius")) {
      node.GetAttributeAsFloat("radius", &gd.bSphere.Radius);
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
      gd.displayedPercentages.emplace_back(std::move(line));
    }

    Grids.emplace_back(std::move(gd));
  }
}
