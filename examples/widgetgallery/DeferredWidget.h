#ifndef DEFERRED_WIDGET_H_
#define DEFERRED_WIDGET_H_

#include <Wt/WContainerWidget>

/*
 * A utility container widget which defers creation of its single
 * child widget until the container is loaded (which is done on-demand
 * by a WMenu). The constructor takes the create function for the
 * widget as a parameter.
 *
 * We use this to defer widget creation until needed, which also defers
 * loading auxiliary javascript libraries.
 */
#if !defined(WT_TARGET_JAVA)
template <typename Function>
class DeferredWidget : public Wt::WContainerWidget
{
public:
  DeferredWidget(Function f)
    : f_(f) { }

private:
  void load() {
    addWidget(f_());
    WContainerWidget::load();
  }

  Function f_;
};

template <typename Function>
DeferredWidget<Function> *deferCreate(Function f)
{
  return new DeferredWidget<Function>(f);
}
#else
class DeferredWidget : public Wt::WContainerWidget {
public:
  DeferredWidget(boost::bound f) {}
};
DeferredWidget *deferCreate(boost::bound b) 
{
  return new DeferredWidget(b);
}

#endif

#endif // DEFERRED_WIDGET_H_
