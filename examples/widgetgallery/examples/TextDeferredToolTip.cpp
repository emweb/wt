#include <Wt/WText>
#include <Wt/WString>
#include <Wt/WContainerWidget>

class Text : public Wt::WText
{
public:
  Text() : Wt::WText(){}

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

Text *text = new Text();
text->setText("Text");
text->setDeferredToolTip(true);

SAMPLE_END(return text)
