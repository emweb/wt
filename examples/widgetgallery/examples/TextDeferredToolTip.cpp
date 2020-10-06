#include <Wt/WText.h>
#include <Wt/WString.h>
#include <Wt/WContainerWidget.h>

class Text : public Wt::WText
{
public:
  Text() : WText(){}

  Wt::WString calculateToolTip() const
  {
    return "Deferred tooltip";
  }

  virtual Wt::WString toolTip() const
  {
    return calculateToolTip();
  }
};

SAMPLE_BEGIN(TextDeferredToolTip)

auto text = std::make_unique<Text>();
text->setText("Text");
text->setDeferredToolTip(true);

SAMPLE_END(return std::move(text))
