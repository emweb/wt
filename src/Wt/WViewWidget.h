// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WVIEWWIDGET_H_
#define WVIEWWIDGET_H_

#include <Wt/WWebWidget.h>

namespace Wt {

/*! \class WViewWidget Wt/WViewWidget.h Wt/WViewWidget.h
 *  \brief An abstract base class for an MVC view that is rendered using a
 *         widget.
 *
 * In principle, %Wt widgets are self-contained and manage both their
 * content, behavior and layout. From the point of view of a
 * Model-View-Controller (MVC) design pattern, they implement each of
 * these, except for the view widgets that work in conjunction with
 * WAbstractItemModel. As a developer you can chose to keep Model,
 * View and Controller together or separate them as you wish.
 *
 * This widget facilitates separation of the View from the Model and
 * Controller in a particular way. The %View is rendered as a %Wt
 * widget. The use of this widget provides two benefits. The classic
 * MVC benefit is a decoupling between view and model, which may allow
 * easier maintainance of code. In addition, this widget enforces the
 * View to be stateless, as it is only created transiently on the
 * server. Therefore the View does not require session resources. This
 * may increase scalability for Internet-deployments.
 *
 * The rendered View widget returned by renderView() should reflect
 * the current model state. Whenever the model changes, rerendering
 * can be triggered by calling update().
 *
 * Currently, the View cannot enclose \link WFormWidget
 * WFormWidgets\endlink which would allow direct manipulation of the
 * model (but we are working to remove this limitation in the future,
 * and let the Model/Controller handle editing changes) and the View
 * may only be updated by a complete rerendering of the entire view.
 *
 * The View widget may contain event handling code, but only in one of
 * the following ways:
 * <ul>
 *   <li>event handling implemented directly in JavaScript code</li>
 *   <li>event handling implemented in pre-learned stateless slot
 *     implementations</li>
 * </ul>
 * Thus, currently, event handling code related to the View cannot be
 * implemented at server-side (but we are thinking about a solution for
 * this as well...).
 *
 * \if cpp
 * Implementation example:
 * \code
 * // Shows the contents for a specific role of a model index in a WText widget
 * class SourceView : public Wt::WViewWidget
 * {
 * public:
 *   // role is the ItemDataRole
 *   SourceView(int role)
 *     : role_(role)
 *   { }
 *
 *   // set an index
 *   void setIndex(const Wt::WModelIndex& index) {
 *     if (index != index_
 *         && (!index.isValid() || !index.data(role_).empty())) {
 *       index_ = index;
 *       update(); // trigger rerendering of the view
 *     }
 *   }
 *
 * private:
 *   Wt::WModelIndex index_;
 *   int             role_;
 *
 * protected:
 *   virtual std::unique_ptr<Wt::WWidget> renderView() {
 *     auto result = std::make_unique<Wt::WText>();
 *     result->setInline(false);
 *
 *     if (!index_.isValid())
 *       return result;
 *
 *     cpp17::any d = index_.data(role_);
 *     const std::string& t = cpp17::any_cast<const std::string&>(d);
 *
 *     result->setTextFormat(Wt::TextFormat::Plain);
 *     result->setText(t);
 *
 *     return result;
 *   }
 * };
 * 
 * \endcode
 * \endif
 *
 * <h3>CSS</h3>
 *
 * This widget does not provide styling, 
 * and can be styled using inline or external CSS as appropriate.
 */
class WT_API WViewWidget : public WWebWidget
{
public:
  /*! \brief Creates a new view widget.
   */
  WViewWidget();

  ~WViewWidget();

  /*! \brief Updates the view.
   *
   * Typically, the model will want to update the view when the model
   * has changed.
   *
   * This will trigger a call to renderView() to ask for a new rendering of
   * the view.
   */
  void update();

  virtual void load() override;
  virtual void render(WFlags<RenderFlag> flags) override;
  virtual void refresh() override;

protected:
  /*! \brief Creates a widget that renders the View.
   *
   * This method must be reimplemented to return a widget that renders the
   * view. The returned widget will be deleted by %WViewWidget.
   */
  virtual std::unique_ptr<WWidget> renderView() = 0;

  virtual void updateDom(DomElement& element, bool all) override;
  virtual void propagateRenderOk(bool deep) override;
  virtual DomElementType domElementType() const override;

  virtual void doneRerender() override;

private:
  std::unique_ptr<WWidget> contents_;
  bool needContentsUpdate_;
};

/*! \class WStaticModelView Wt/WViewWidget.h Wt/WViewWidget.h
 *  \brief A widget that implements a view for a non-changing model.
 *
 * This model uses a function object which is passed in the
 * constructor to render the View, and does not react to changes.
 *
 * You may want to use the utility function Wt::makeStaticModel() to create an
 * instance of this class.
 */
template <typename Renderer>
class WStaticModelView : public WViewWidget
{
public:
  /*! \brief Creates a new static model view, given a function
   *         object to render the View widget.
   */
  WStaticModelView(Renderer f)
    : f_(f) { }

protected:
  std::unique_ptr<WWidget> renderView() {
    return f_();
  }

  Renderer f_;
};

/*! \brief Wraps a widget into a view with a non-changing model.
 *
 * The ViewRenderer is called without arguments and must return a
 * newly created widget (WWidget *).
 *
 * \relates WStaticModelView
 */
template <typename R>
std::unique_ptr<WStaticModelView<R> > makeStaticModel(R f)
{
  return std::unique_ptr<WStaticModelView<R> >(new WStaticModelView<R>(f));
}

}

#endif // WVIEWWIDGET_H_
