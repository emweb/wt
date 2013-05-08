#include "Wt/WApplication"
#include "Wt/WMatchValidator"

#ifndef WT_DEBUG_JS
#include "js/WMatchValidator.min.js"
#endif

namespace Wt {

	WValidator::Result WMatchValidator::validate(const WT_USTRING& strInput) const
	{
		if(0 == m_pCompareWidget) {
			return Result(Invalid, invalidDataText());
		}

		if(strInput.empty()) {
			return WValidator::validate(strInput);
		}

		if(strInput != m_pCompareWidget->valueText()) {
			return Result(Invalid, mismatchText());
		}

		return Result(Valid);
	}

	std::string WMatchValidator::javaScriptValidate() const
	{
		loadJavaScript(WApplication::instance());

		WStringStream js;

		js << "new " WT_CLASS ".WMatchValidator(";

		if(0 == m_pCompareWidget) {
			js << "null";
		} else {
			js << m_pCompareWidget->jsRef();
		}

		js << ',' << isMandatory() << ','
			<< invalidBlankText().jsStringLiteral() << ','
			<< invalidDataText().jsStringLiteral() << ','
			<< mismatchText().jsStringLiteral() << ");";

		return js.str();
	}

#ifndef WT_TARGET_JAVA
	void WMatchValidator::createExtConfig(std::ostream& config) const
	{
		if(!invalidDataText().empty()) {
			config << ",invalidData:" << invalidDataText().jsStringLiteral();
		}

		if(!mismatchText().empty()) {
			config << ",mismatch:" << mismatchText().jsStringLiteral();
		}

		WValidator::createExtConfig(config);
	}
#endif //WT_TARGET_JAVA

	void WMatchValidator::loadJavaScript(WApplication *app)
	{
		LOAD_JAVASCRIPT(app, "js/WMatchValidator.js", "WMatchValidator", wtjs1);
	}

} // namespace Wt

