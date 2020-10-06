#include <Wt/WPushButton.h>
#include <Wt/WResource.h>
#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>
#include <Wt/Render/WPdfRenderer.h>
#include <Wt/WApplication.h>

#include <hpdf.h>

namespace {
    void HPDF_STDCALL error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no,
		       void *user_data) {
	fprintf(stderr, "libharu error: error_no=%04X, detail_no=%d\n",
		(unsigned int) error_no, (int) detail_no);
    }
}

class ReportResource : public Wt::WResource
{
public:
    ReportResource()
        : WResource()
    {
	suggestFileName("report.pdf");
    }

    virtual ~ReportResource()
    {
	beingDeleted();
    }

    virtual void handleRequest(const Wt::Http::Request& request,
                               Wt::Http::Response& response)
    {
	response.setMimeType("application/pdf");

	HPDF_Doc pdf = HPDF_New(error_handler, 0);

#if HPDF_MAJOR_VERION >= 2 || HPDF_MINOR_VERSION >= 3
	// Note: UTF-8 encoding (for TrueType fonts) is only available since libharu 2.3.0 !
	HPDF_UseUTFEncodings(pdf);
#endif

	renderReport(pdf);

	HPDF_SaveToStream(pdf);
	unsigned int size = HPDF_GetStreamSize(pdf);
	HPDF_BYTE *buf = new HPDF_BYTE[size];
	HPDF_ReadFromStream (pdf, buf, &size);
	HPDF_Free(pdf);
	response.out().write((char*)buf, size);
	delete[] buf;
    }

private:
    void renderReport(HPDF_Doc pdf) {
        renderPdf(Wt::WString::tr("report.example"), pdf);
    }

    void renderPdf(const Wt::WString& html, HPDF_Doc pdf)
    {
	HPDF_Page page = HPDF_AddPage(pdf);
	HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

	Wt::Render::WPdfRenderer renderer(pdf, page);
	renderer.setMargin(2.54);
	renderer.setDpi(96);
	renderer.render(html);
    }
};

SAMPLE_BEGIN(PdfRenderer)
auto container = std::make_unique<Wt::WContainerWidget>();

Wt::WText *text = container->addNew<Wt::WText>(Wt::WString::tr("report.example"));
text->setStyleClass("reset");

Wt::WPushButton *button = container->addNew<Wt::WPushButton>("Create pdf");

auto pdf = std::make_shared<ReportResource>();
button->setLink(Wt::WLink(pdf));

SAMPLE_END(return std::move(container))
