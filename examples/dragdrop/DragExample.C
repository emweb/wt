#include "DragExample.h"
#include "Character.h"
#include <Wt/WEnvironment.h>
#include <Wt/WImage.h>
#include <Wt/WApplication.h>

/**
 * \defgroup dragexample Drag and Drop example
 */
/*@{*/

/*! \brief Create an image which can be dragged.
 *
 * The image to be displayed when dragging is given by smallurl, and
 * configured with the given mime type
 */
WImage *createDragImage(const char *url, const char *smallurl,
			const char *mimeType,
			WContainerWidget *p)
{
  WImage *result = p->addWidget(std::make_unique<WImage>(url));
  WImage *dragImage = p->addWidget(std::make_unique<WImage>(smallurl));
  dragImage->setMargin(-15, Side::Left | Side::Top);

  /*
   * Set the image to be draggable, showing the other image (dragImage)
   * to be used as the widget that is visually dragged.
   */
  result->setDraggable(mimeType, dragImage, true);

  return result;
}

DragExample::DragExample():
  WContainerWidget()
{
  this->addWidget(std::make_unique<WText>("<p>Help these people with their decision by dragging one of "
            "the pills.</p>"));

  if (!wApp->environment().javaScript()) {
    this->addWidget(std::make_unique<WText>("<i>This examples requires that javascript support is "
              "enabled.</i>"));
  }

  WContainerWidget *pills = this->addWidget(std::make_unique<WContainerWidget>());
  pills->setContentAlignment(AlignmentFlag::Center);

  createDragImage("icons/blue-pill.jpg",
		  "icons/blue-pill-small.png",
		  "blue-pill", pills);
  createDragImage("icons/red-pill.jpg",
		  "icons/red-pill-small.png",
		  "red-pill", pills);

  WContainerWidget *dropSites = this->addWidget(std::make_unique<WContainerWidget>());

  dropSites->addWidget(std::make_unique<Character>("Neo"));
  dropSites->addWidget(std::make_unique<Character>("Morpheus"));
  dropSites->addWidget(std::make_unique<Character>("Trinity"));

}

/*@}*/
