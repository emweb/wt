#include "DragExample.h"
#include "Character.h"
#include <Wt/WEnvironment>
#include <Wt/WImage>
#include <Wt/WApplication>

using namespace Wt;

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
  WImage *result = new WImage(url, p);
  WImage *dragImage = new WImage(smallurl, p);
  dragImage->setMargin(-15, Left | Top);

  /*
   * Set the image to be draggable, showing the other image (dragImage)
   * to be used as the widget that is visually dragged.
   */
  result->setDraggable(mimeType, dragImage, true);

  return result;
}

DragExample::DragExample(WContainerWidget *parent):
  WContainerWidget(parent)
{
  new WText("<p>Help these people with their decision by dragging one of "
	    "the pills.</p>", this);

  if (!wApp->environment().javaScript()) {
    new WText("<i>This examples requires that javascript support is "
	      "enabled.</i>", this);
  }

  WContainerWidget *pills = new WContainerWidget(this);
  pills->setContentAlignment(AlignCenter);

  createDragImage("icons/blue-pill.jpg",
		  "icons/blue-pill-small.png",
		  "blue-pill", pills);
  createDragImage("icons/red-pill.jpg",
		  "icons/red-pill-small.png",
		  "red-pill", pills);

  WContainerWidget *dropSites = new WContainerWidget(this);

  new Character("Neo", dropSites);
  new Character("Morpheus", dropSites);
  new Character("Trinity", dropSites);

}

/*@}*/
