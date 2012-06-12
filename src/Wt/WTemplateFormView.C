/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAbstractToggleButton"
#include "Wt/WFormWidget"
#include "Wt/WLogger"
#include "Wt/WText"
#include "Wt/WTemplateFormView"

namespace Wt {

  LOGGER("WTemplateFormView");

WTemplateFormView::WTemplateFormView(const WString& text,
				     WContainerWidget *parent)
  : WTemplate(text, parent)
{
  addFunction("id", WT_TEMPLATE_FUNCTION(id));
  addFunction("tr", WT_TEMPLATE_FUNCTION(tr));
}

WTemplateFormView::WTemplateFormView(WContainerWidget *parent)
  : WTemplate(parent)
{
  addFunction("id", WT_TEMPLATE_FUNCTION(id));
  addFunction("tr", WT_TEMPLATE_FUNCTION(tr));
}

WFormWidget *WTemplateFormView::createFormWidget(WFormModel::Field field)
{
  return 0;
}

void WTemplateFormView::updateViewField(WFormModel *model,
					WFormModel::Field field)
{
  const std::string var = field;

  if (model->isVisible(field)) {
    setCondition("if:" + var, true);
    WFormWidget *edit = resolve<WFormWidget *>(var);
    if (!edit) {
      edit = createFormWidget(field);
      if (!edit) {
	LOG_ERROR("updateViewField: createFormWidget('"
		  << field << "') returned 0");
	return;
      }
      bindWidget(var, edit);
    }

    WAbstractToggleButton *b = dynamic_cast<WAbstractToggleButton *>(edit);
    if (b) {
      boost::any v = model->value(field);
      if (v.empty() || boost::any_cast<bool>(v) == false)
	b->setChecked(false);
      else
	b->setChecked(true);
    } else
      edit->setValueText(model->valueText(field));

    // TODO support other types, e.g. combo boxes and date fields ?

    WText *info = resolve<WText *>(var + "-info");
    if (!info) {
      info = new WText();
      bindWidget(var + "-info", info);
    }

    bindString(var + "-label", model->label(field));

    const WValidator::Result& v = model->validation(field);
    info->setText(v.message());

    indicateValidation(field, model->isValidated(field),
		       info, edit, v);

    edit->setDisabled(model->isReadOnly(field));
    edit->toggleStyleClass("Wt-disabled", edit->isDisabled());
  } else {
    setCondition("if:" + var, false);
    bindEmpty(var);
    bindEmpty(var + "-info");    
  }
}

void WTemplateFormView::indicateValidation(WFormModel::Field field,
					   bool validated,
					   WText *info,
					   WFormWidget *edit,
					   const WValidator::Result& validation)
{
  info->setText(validation.message());

  if (validated) {
    switch (validation.state()) {
    case WValidator::InvalidEmpty:
    case WValidator::Invalid:
      edit->removeStyleClass("Wt-valid", true);
      edit->addStyleClass("Wt-invalid", true);
      info->addStyleClass("Wt-error", true);

      break;
    case WValidator::Valid:
      edit->removeStyleClass("Wt-invalid", true);
      edit->addStyleClass("Wt-valid", true);
      info->removeStyleClass("Wt-error", true);

      break;
    }
  } else {
    edit->removeStyleClass("Wt-valid", true);
    edit->removeStyleClass("Wt-invalid", true);
    info->removeStyleClass("Wt-error", true);
  }
}

void WTemplateFormView::updateModelField(WFormModel *model,
					 WFormModel::Field field)
{
  WFormWidget *edit = resolve<WFormWidget *>(field);
  if (edit) {
    WAbstractToggleButton *b = dynamic_cast<WAbstractToggleButton *>(edit);
    if (b)
      model->setValue(field, b->isChecked());
    else
      model->setValue(field, edit->valueText());
  }
}

void WTemplateFormView::updateModel(WFormModel *model)
{
  std::vector<WFormModel::Field> fields = model->fields();

  for (unsigned i = 0; i < fields.size(); ++i)
    updateModelField(model, fields[i]);
}

void WTemplateFormView::updateView(WFormModel *model)
{
  std::vector<WFormModel::Field> fields = model->fields();

  for (unsigned i = 0; i < fields.size(); ++i)
    updateViewField(model, fields[i]);
}

}
