module;
#include <memory>

export module whiteboard:root;

import :css_item;
import :gui_item;

export namespace gui {

class CWBApplication;

class CWBRoot : public CWBGuiType<"root", CWBItem> {
 public:
  CWBRoot();
  static inline std::unique_ptr<CWBRoot> Create(const math::CRect& Pos) {
    auto p = std::make_unique<CWBRoot>();
    p->Initialize(nullptr, Pos);
    return p;
  }
  ~CWBRoot() override;

  void SetApplication(CWBApplication* Application);

 private:
  bool MessageProc(const CWBMessage& Message) override;
  void OnDraw(CWBDrawAPI* API) override;
};

}  // namespace gui
