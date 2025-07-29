#include "WD3Widget.h"

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>

#define JS_OBJ(s) WD3Widget::JsObject(s)

class D3Test: public WD3Widget {
public:
  D3Test()
    : colorToggle_(false),
      resizeToggle_(false),
      borderToggle_(false)
  {
    resize(800, 600);
  }

  // Toggle the color of the circle
  void toggleColor() {
    colorToggle_ = !colorToggle_;
    scheduleRender();
  }

  // Toggle the size of the squares
  void toggleResize() {
    resizeToggle_ = !resizeToggle_;
    scheduleRender();
  }

protected:
  void initializeD3() override
  {
    // Create a group of squares with data binding
    squares_ = body().append("g")
                        .attr("fill", "steelblue")
                      .selectAll()
                      .data(JS_OBJ("[{x: 50, y: 50}, {x: 150, y: 150}, {x: 250, y: 250}]"))
                      .join("rect")
                        .attr("width", "50")
                        .attr("height", "50")
                        .attr("x", JS_OBJ("(d) => d.x"))
                        .attr("y", JS_OBJ("(d) => d.y"));

    // Toggle border on square click
    squares_.clicked().connect([=](){
      borderToggle_ = !borderToggle_;
      squares_.attr("stroke", borderToggle_ ? "orange" : "none")
              .attr("stroke-width", borderToggle_ ? "5" : "0")
              .pushUpdates();

      scheduleRender();
    });

    // Create a group and a circle that grows on click
    colorChanger_ = body().append("g");
    growingCircle_ = colorChanger_.append("circle")
                                    .attr("cx", "400")
                                    .attr("cy", "300")
                                    .attr("r", "50")
                                    .on("click", JS_OBJ("(e) => { let target = d3.select(e.currentTarget); target.attr('r', parseInt(target.attr('r')) + 1); }"));

  }

  void updateD3() override
  {
    // Change circle color
    if (colorToggle_) {
      colorChanger_.attr("fill", "red");
    } else {
      colorChanger_.attr("fill", "blue");
    }

    // JS function to resize squares
    auto jsResize = JS_OBJ("(selection, w, h) => { selection.attr('width', w); selection.attr('height', h); }");

    // Resize squares based on toggle
    if (resizeToggle_) {
      squares_.call(jsResize, "100", "100");
    } else {
      squares_.call(jsResize, "50", "50");
    }

    // Push updates to the client
    colorChanger_.pushUpdates();
    squares_.pushUpdates();
  }

private:
  bool colorToggle_, resizeToggle_, borderToggle_;
  WD3Widget::Selection colorChanger_, growingCircle_, squares_;
};


class D3App : public Wt::WApplication {
public:
  D3App(const Wt::WEnvironment &env)
    : Wt::WApplication(env)
  {
    auto d3Widget = root()->addNew<D3Test>();

    // Button to change circle color
    auto colorBtn = root()->addNew<Wt::WPushButton>("Change Circle Color");
    colorBtn->clicked().connect([=]() {
      d3Widget->toggleColor();
    });

    // Button to resize squares
    auto resizeBtn = root()->addNew<Wt::WPushButton>("Resize Squares");
    resizeBtn->clicked().connect([=]() {
      d3Widget->toggleResize();
    });
  }
};

std::unique_ptr<Wt::WApplication> createApplication(const Wt::WEnvironment &env)
{
  return std::make_unique<D3App>(env);
}

int main(int argc, char *argv[])
{
  return Wt::WRun(argc, argv, &createApplication);
}
