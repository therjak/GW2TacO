module;
#include <memory>

export module whiteboard:root;

import :css_item;
import :gui_item;

export class CWBApplication;

export class CWBRoot : public CWBGuiType<"root", CWBItem> {
 public:
  CWBRoot(CWBItem* Parent, const math::CRect& Pos);
  static inline std::unique_ptr<CWBRoot> Create(const math::CRect& Pos) {
    return std::make_unique<CWBRoot>(nullptr, Pos);
  }
  ~CWBRoot() override;

  void SetApplication(CWBApplication* Application);
  bool Initialize(CWBItem* Parent, const math::CRect& Position) override;

 private:
  bool MessageProc(const CWBMessage& Message) override;
  void OnDraw(CWBDrawAPI* API) override;
};
