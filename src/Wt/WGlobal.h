// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WGLOBAL_H_
#define WGLOBAL_H_

#include <Wt/WFlags.h>
#include <Wt/Core/observing_ptr.hpp>
#include <memory>

/*! \file */

namespace Wt {

  /* Since we target C++11, we're sorely missing std::make_unique */
  namespace cpp14 {
#ifndef WT_TARGET_JAVA
    template<typename T, typename ...Args>
    std::unique_ptr<T> make_unique( Args&& ...args )
    {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
#else // WT_TARGET_JAVA
    template<typename T>
    std::unique_ptr<T> make_unique();
    template<typename T, typename Arg1>
    std::unique_ptr<T> make_unique(Arg1 arg1);
    template<typename T, typename Arg1, typename Arg2>
    std::unique_ptr<T> make_unique(Arg1 arg1, Arg2 arg2);
    template<typename T, typename Arg1, typename Arg2, typename Arg3>
    std::unique_ptr<T> make_unique(Arg1 arg1, Arg2 arg2, Arg3 arg3);
    template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
    std::unique_ptr<T> make_unique(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4);
    template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
    std::unique_ptr<T> make_unique(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4, Arg5);
#endif // WT_TARGET_JAVA
  }

  class EventSignalBase;
  class JSlot;
  class WAbstractArea;
  class WAbstractSpinBox;
  class WAbstractItemDelegate;
  class WAbstractItemModel;
  class WAbstractItemView;
  class WAbstractMedia;
  class WAbstractListModel;
  class WAbstractProxyModel;
  class WAbstractTableModel;
  class WAbstractToggleButton;
  class WAggregateProxyModel;
  class WAnimation;
  class WAnchor;
  class WApplication;
  class WAudio;
  class WBatchEditProxyModel;
  class WBorder;
  class WBorderLayout;
  class WBoxLayout;
  class WBreak;
  class WBrush;
  class WButtonGroup;
  class WCalendar;
  class WCanvasPaintDevice;
  class WCheckBox;
  class WCircleArea;
  class WColor;
  class WCombinedLocalizedStrings;
  class WComboBox;
  class WCompositeWidget;
  class WContainerWidget;
  class WCssDecorationStyle;
  class WCssRule;
  class WCssStyleSheet;
  class WCssTemplateRule;
  class WCssTextRule;
  class WCssTheme;
  class WDate;
  class WDateEdit;
  class WDatePicker;
  class WDateTime;
  class WDateValidator;
  class WDefaultLayout;
  class WDefaultLoadingIndicator;
  class WDialog;
  class WDoubleSpinBox;
  class WDoubleValidator;
  class WDropEvent;
  class WEnvironment;
  class WEvent;
  class WException;
  class WFileResource;
  class WFileUpload;
  class WFitLayout;
  class WFlashObject;
  class WFont;
  class WFormModel;
  class WFormWidget;
  class WGestureEvent;
  class WGLWidget;
  class WGoogleMap;
  class WGridLayout;
  class WGroupBox;
  class WHBoxLayout;
  class WMouseEvent;
  class WIcon;
  class WIconPair;
  class WImage;
  class WInPlaceEdit;
  class WIntValidator;
  class WInteractWidget;
  class WIOService;
  class WItemDelegate;
  class WItemSelectionModel;
  class WKeyEvent;
  class WLabel;
  class WLayout;
  class WLayoutItem;
  class WLeafletMap;
  class WLength;
  class WLengthValidator;
  class WLineEdit;
  class WLineF;
  class WLink;
  class WLinkedCssStyleSheet;
  class WLocale;
  class WLocalDateTime;
  class WLoadingIndicator;
  class WLocalizedStrings;
  class WLogEntry;
  class WLogger;
  class WMatrix4x4;
  class WMediaPlayer;
  class WMemoryResource;
  class WMenu;
  class WMenuItem;
  class WMessageBox;
  class WMessageResourceBundle;
  class WMessageResources;
  class WModelIndex;
  class WMouseEvent;
  class WNavigationBar;
  class WObject;
  class WOverlayLoadingIndicator;
  class WPaintDevice;
  class WPaintedWidget;
  class WPainter;
  class WPainterPath;
  class WPanel;
  class WPdfImage;
  class WPen;
  class WPoint;
  class WPointF;
  class WPolygonArea;
  class WPopupMenu;
  class WPopupWidget;
  class WProgressBar;
  class WPushButton;
  class WRadioButton;
  class WRandom;
  class WReadOnlyProxyModel;
  class WRasterImage;
  class WRectArea;
  class WRectF;
  class WRegExpValidator;
  class WResource;
  class WScrollEvent;
  class WSelectionBox;
  class WServer;
  class WShadow;
  class WSlider;
  class WSocketNotifier;
  class WSortFilterProxyModel;
  class WSound;
  class WSpinBox;
  class WSplitButton;
  class WStackedWidget;
  class WStandardItem;
  class WStandardItemModel;
  class WStreamResource;
  class WString;
  class WStringListModel;
  class WSuggestionPopup;
  class WSvgImage;
  class WTabWidget;
  class WTable;
  class WTableCell;
  class WTableColumn;
  class WTableRow;
  class WTableView;
  class WTemplate;
  class WText;
  class WTextArea;
  class WTextEdit;
  class WToolBar;
  class WTheme;
  class WTime;
  class WTimer;
  class WTouchEvent;
  class WTransform;
  class WTree;
  class WTreeNode;
  class WTreeTable;
  class WTreeTableNode;
  class WTreeView;
  class WVBoxLayout;
  class WValidationStatus;
  class WValidator;
  class WVectorImage;
  class WVideo;
  class WViewWidget;
  class WVirtualImage;
  class WVmlImage;
  class WWebWidget;
  class WWidget;
  class WWidgetItem;

  namespace Auth {
    class AbstractPasswordService;
    class AbstractUserDatabase;
    class AuthModel;
    class AuthService;
    class AuthWidget;
    class FacebookService;
    class FormBaseModel;
    class GoogleService;
    class HashFunction;
    class Identity;
    class Login;
    class LostPasswordWidget;
    class OAuthService;
    class PasswordHash;
    class PasswordPromptDialog;
    class PasswordService;
    class PasswordStrengthValidator;
    class PasswordVerifier;
    class RegistrationModel;
    class RegistrationWidget;
    class Token;
    class UpdatePasswordWidget;
    class User;

    namespace Dbo {
      template <class UserType> class AuthInfo;
      template <class DboType> class UserDatabase;
    }
  }

  namespace Chart {
    class SeriesIterator;
    class WAbstractChart;
    class WAxis;
    class WCartesianChart;
    class WChartPalette;
    class WDataSeries;
    class WPieChart;
    class WStandardPalette;
  }

  namespace Dbo {
    struct dbo_default_traits;

    class Call;
    class Exception;
    class FieldInfo;
    class FixedSqlConnectionPool;
    class ObjectNotFoundException;
    class SaveBaseAction;
    class Session;
    class SqlConnection;
    class SqlConnectionPool;
    class SqlStatement;
    class StaleObjectException;
    class Transaction;

    template <class C> class collection;
    template <class C> struct dbo_traits;
    template <typename V, class Enable> struct sql_value_traits;
    template <class C> class ptr;
    template <class Result, typename BindStrategy> class Query;
    template <class Result> class QueryModel;

    namespace backend {
      class Firebird;
      class Postgres;
      class Sqlite3;
    }
  }

  namespace Http {
    class Client;
    class Message;
    class Request;
    class Response;
    class ResponseContinuation;
    class UploadedFile;
    class WtClient;
  }

  namespace Json {
    class Array;
    class Object;
    class TypeException;
    class Value;
  }

  namespace Mail {
    class Client;
    class Mailbox;
    class Message;
  }

  namespace Payment {
    class Address;
    class Customer;
    class Money;
    class Order;
    class OrderItem;
    class PayPalExpressCheckout;
    class PayPalService;
    class Result;
    class Approval;
  }

  namespace Render {
    class WPdfRenderer;
  }

using Core::observing_ptr;

/*! \brief Enumeration that indicates a direction.
 */
enum class Orientation {
  Horizontal = 0x1, //!< Horizontal
  Vertical   = 0x2  //!< Vertical
};

W_DECLARE_OPERATORS_FOR_FLAGS(Orientation)

/*! \brief Enumeration that indicates a standard button.
 *
 * Multiple buttons may be specified by logically or'ing these values
 * together, e.g.
 * \code
 * StandardButton::Ok | StandardButton::Cancel
 * \endcode
 *
 * \sa WMessageBox
 */
enum class StandardButton {
  None = 0x00,      //!< No button
  Ok = 0x01,        //!< An OK button.
  Cancel = 0x02,    //!< A Cancel button.
  Yes = 0x04,       //!< A Yes button.
  No = 0x08,        //!< A No button.
  Abort = 0x10,     //!< An Abort button.
  Retry = 0x20,     //!< A Retry button.
  Ignore = 0x40,    //!< An Ignore button.
  YesAll = 0x80,    //!< A Yes-to-All button.
  NoAll = 0x100     //!< A No-to-All button.
};
W_DECLARE_OPERATORS_FOR_FLAGS(StandardButton)

/*! \brief Enumeration that indiciates a standard icon.
 *
 * \sa WMessageBox
 */
enum class Icon {
  None = 0,         //!< No icon.
  Information = 1,  //!< An information icon.
  Warning = 2,      //!< A warning icon.
  Critical = 3,     //!< A critical icon.
  Question = 4      //!< A question icon.
};

/*! \brief Enumeration that indicates how items may be selected.
 *
 * \sa WTreeView::setSelectionMode()
 */
enum class SelectionMode {
  None = 0,    //!< No selections
  Single = 1,  //!< Single selection only
  Extended = 3 //!< Multiple selection
};

/*! \brief Enumeration that indicates what is being selected.
 *
 * \sa WTreeView::setSelectionBehavior()
 */
enum class SelectionBehavior {
  Items = 0,    //!< Select single items
  Rows = 1      //!< Select only rows
    /*, SelectColumns */
};

/*! \brief Enumeration that indicates how to change a selection.
 *
 * \sa WTreeView::select()
 */
enum class SelectionFlag {
  Select = 1,          //!< Add to selection
  Deselect = 2,        //!< Remove from selection
  ToggleSelect = 3,    //!< Toggle in selection
  ClearAndSelect = 4   //!< Clear selection and add single item
};

/*! \brief Enumeration that indicates a relative location.
 *
 * Values of Side::CenterX, Side::CenterY, and CenterXY are only valid for
 * WCssDecorationStyle::setBackgroundImage()
 *
 * \sa WWidget::setMargin(const WLength& x, WFlags<Side> sides)
 * \sa WWidget::setOffsets(const WLength& x, WFlags<Side> sides)
 * \sa WWidget::setFloatSide(Side s)
 * \sa WWidget::setClearSides(WFlags<Side> sides)
 * \sa WContainerWidget::setPadding(const WLength& x, WFlags<Side> sides)
 * \sa WCssDecorationStyle::setBackgroundImage()
 */
enum class Side {
  Top = 0x1,                          //!< Top side
  Bottom = 0x2,                       //!< Bottom side
  Left = 0x4,                         //!< Left side
  Right = 0x8,                        //!< Right side
  CenterX = 0x10,                     //!< Center horiziontally
  CenterY = 0x20                      //!< Center vertically
};

W_DECLARE_OPERATORS_FOR_FLAGS(Side)

static const WFlags<Side> AllSides = 
  Side::Left | Side::Right | Side::Top | Side::Bottom;

/*! \brief Enumeration that specifies a horizontal or a vertical alignment.
 *
 * The vertical alignment flags are AlignmentFlag::Baseline,
 * AlignmentFlag::Sub, AlignmentFlag::Super, AlignmentFlag::Top,
 * AlignmentFlag::TextTop, AlignmentFlag::Middle,
 * AlignmentFlag::Bottom and AlignmentFlag::TextBottom.
 *
 * The horizontal alignment flags are AlignmentFlag::Left,
 * AlignmentFlag::Right, AlignmentFlag::Center and
 * AlignmentFlag::Justify.
 *
 * When used with setVerticalAlignment(), this applies only
 * to inline widgets and determines how to position itself on the
 * current line, with respect to sibling inline widgets.
 *
 * When used with WTableCell::setContentAlignment(), this determines the
 * vertical alignment of contents within the table cell.
 *
 * When used with WPainter::drawText(), this determines the horizontal and
 * vertical alignment of the text with respect to the bounding rectangle.
 *
 * When used with WContainerWidget::setContentAlignment(), this specifies how
 * contents should be aligned horizontally within the container.
 *
 * Not all values are applicable in all situations. The most commonly
 * used values are AlignmentFlag::Left, AlignmentFlag::Center,
 * AlignmentFlag::Right, AlignmentFlag::Bottom, AlignmentFlag::Middle
 * and AlignmentFlag::Top.
 */
enum class AlignmentFlag {
  /*! \brief Align to the left
   */
  Left=0x1,
  /*! \brief Align to the right
   */
  Right=0x2,
  /*! \brief Align horizontally in the center
   */
  Center=0x4,
  /*! \brief Justify left and right
   */
  Justify=0x8,
  /*! \brief Align at baseline (default alignment).
   */
  Baseline=0x10,
  /*! \brief Align below the baseline (as if subscript).
   */
  Sub=0x20,
  /*! \brief Align above the baseline (as if superscript).
   */
  Super=0x40,
  /*! \brief Align top of widget with top of tallest sibling widget.
   */
  Top=0x80,
  /*! \brief Align top of widget with the top of the parent widget's font.
   */
  TextTop=0x100,
  /*! \brief Align vertically the middle to the middle of the parent widget.
   */
  Middle=0x200,
  /*! \brief Align bottom of widget to the bottom of the lowest sigling widget.
   */
  Bottom=0x400,
  /*! \brief Align bottom of widget to the bottom of parent widget's font.
   */
  TextBottom=0x800
};

W_DECLARE_OPERATORS_FOR_FLAGS(AlignmentFlag)

/*! \brief Combination of all horizontal alignment flags */
static const WFlags<AlignmentFlag> AlignHorizontalMask
= AlignmentFlag::Left | AlignmentFlag::Right | 
  AlignmentFlag::Center | AlignmentFlag::Justify;

/*! \brief Combination of all vertical alignment flags */
static const WFlags<AlignmentFlag> AlignVerticalMask
= AlignmentFlag::Baseline | AlignmentFlag::Sub | 
  AlignmentFlag::Super | AlignmentFlag::Top | 
  AlignmentFlag::TextTop | AlignmentFlag::Middle |
  AlignmentFlag::Bottom | AlignmentFlag::TextBottom;

/*! \brief Enumeration that specifies the way text should be printed.
 *
 * \sa WPainter::drawText(const WRectF& rect, WFlags<AlignmentFlag> alignmentFlags, TextFlag textFlag, const WString& text)
 */
enum class TextFlag {
  SingleLine,   //!< Text will be printed on just one line
  WordWrap      //!< Lines will break at word boundaries
};

/*! \brief Enumeration that specifies a layout mechanism for a widget.
 *
 * The layout mechanism determines how the widget positions itself relative
 * to the parent or sibling widgets.
 *
 * \sa WWidget::setPositionScheme(PositionScheme scheme)
 */
enum class PositionScheme {
  /*! \brief Static position scheme.
   *
   *  The widget is layed-out with other
   *  \link Wt::PositionScheme::Static Static\endlink and
   *  \link Wt::PositionScheme::Relative Relative\endlink
   *  sibling widgets, one after another.
   *
   *  Inline widgets are layed out in horizontal lines (like text),
   *  wrapping around at the end of the line to continue on the next
   *  line. Block widgets are stacked vertically.
   *
   *  Static widgets may also float to the left or
   *  right border, using setFloatSide().
   */
  Static,

  /*! \brief Relative position scheme.
   *
   *  The widget is first layed out according to
   *  Static layout rules, but after layout, the
   *  widget may be offset relative to where it would be in a static
   *  layout, using setOffsets().
   *
   *  Another common use of a Relative position scheme
   *  (even with no specified offsets) is to provide a new reference
   *  coordinate system for Absolutely positioned widgets.
   */
  Relative,

  /*! \brief Absolute position scheme.
   *
   *  The widget is positioned at an absolute position
   *  with respect to the nearest ancestor widget that is either:
   *  <ul>
   *    <li> a WTableCell </li>
   *    <li> or has a position scheme that is
   *     \link Wt::PositionScheme::Relative Relative\endlink
   *     or \link Wt::PositionScheme::Absolute Absolute\endlink. </li>
   *  </ul>
   */
   Absolute,

  /*! \brief Fixed position scheme.
   *
   *  The widget is positioned at fixed position with respect to
   *  the browser's view-pane.
   */
   Fixed
};

/*! \brief Enumeration for a cursor style
 *
 * \sa WCssDecorationStyle::setCursor(), WAbstractArea::setCursor()
 */
enum class Cursor {
  Arrow,        //!< Arrow, CSS 'default' cursor
  Auto,         //!< Cursor chosen by the browser, CSS 'auto' cursor.
  Cross,        //!< Crosshair, CSS 'cross' cursor
  PointingHand, //!< Pointing hand, CSS 'pointer' cursor
  OpenHand,     //!< Open hand, CSS 'move' cursor
  Wait,         //!< Wait, CSS 'wait' cursor
  IBeam,        //!< Text edit, CSS 'text' cursor
  WhatsThis     //!< Help, CSS 'help' cursor
};

/*! \brief Enumeration that indicates a character encoding.
 *
 * Character encodings are used to represent characters in a stream of
 * bytes.
 *
 * \relates WString
 */
enum class CharEncoding {
  /*! \brief Wt's default encoding
   *
   * Wt's global default encoding, as set through WString::setDefaultEncoding()
   *
   */
  Default,

  /*! \brief The (server) system locale.
   *
   * Each byte represents a character, according to the locale
   * of the server-side system.
   */
  Local,

  /*! \brief UTF-8 unicode encoding.
   *
   * The byte stream represents unicode characters encoded using
   * UTF-8.
   */
  UTF8
};

/*! \enum PenStyle
 *  \brief Enumeration that indicates a pen style.
 *
 * \ingroup painting
 *
 * \relates WPen
 */
enum class PenStyle {
  None,           //!< Do not stroke
  SolidLine,      //!< Stroke with a solid line
  DashLine,       //!< Stroked with a dashed line
  DotLine,        //!< Stroke with a dotted line
  DashDotLine,    //!< Stroke with a dash dot line
  DashDotDotLine  //!< Stroke with a dash dot dot line
};

/*! \enum PenCapStyle
 *  \brief Enumeration that indicates how line end points are rendered
 *
 * \ingroup painting
 *
 * \relates WPen
 */
enum class PenCapStyle {
  Flat,        //!< Flat ends
  Square,      //!< Square ends (prolongs line with half width)
  Round        //!< Rounded ends (terminates with a half circle)
};

/*! \enum PenJoinStyle
 *  \brief Enumeration that indicates how line joins are rendered.
 *
 * \ingroup painting
 *
 * \relates WPen
 */
enum class PenJoinStyle {
  Miter,      //!< Pointy joins
  Bevel,      //!< Squared-off joins
  Round       //!< Rounded joins
};

/*! \enum BrushStyle
 *  \brief Enumeration that indicates a fill style.
 *
 * \ingroup painting
 *
 * \relates WBrush
 */
enum class BrushStyle {
  None,    //!< Do not fill
  Solid,   //!< Fill with a solid color
  Gradient //!< Fill with a color gradient
};

/*! \enum GradientStyle
 *  \brief Enumeration that indicates a gradient style.
 *
 * \ingroup painting
 *
 * \relates WGradient
 */
enum class GradientStyle {
  Linear, //!< Linear gradient
  Radial //!< Radial gradient
};

/*! \brief Enumeration that specifies where the target of an anchor should
 *         be displayed.
 *
 * \sa WAnchor::setTarget()
 */
enum class LinkTarget {
  Self,      //!< Show Instead of the application
  ThisWindow,//!< Show in the top level frame of the application window
  NewWindow, //!< Show in a separate new tab or window
  Download   //!< Useful only for a downloadable resource
};

/*! \brief Enumeration that indicates the text format.
 *
 * \sa WText::setTextFormat()
 */
enum class TextFormat {
  XHTML,       //!< Format text as XSS-safe XHTML markup'ed text
  UnsafeXHTML, //!< Format text as XHTML markup'ed text
  Plain        //!< Format text as plain text
};

/*! \brief Enumeration for predefined colors
 *
 * \ingroup style
 */
enum class StandardColor {
  /*! \brief Color white.
   */
  White,
  /*! \brief Color black.
   */
  Black,
  /*! \brief Color red.
   */
  Red,
  /*! \brief Color dark red.
   */
  DarkRed,
  /*! \brief Color green.
   */
  Green,
  /*! \brief Color dark green.
   */
  DarkGreen,
  /*! \brief Color blue.
   */
  Blue,
  /*! \brief Color dark blue.
   */
  DarkBlue,
  /*! \brief Color cyan.
   */
  Cyan,
  /*! \brief Color dark cyan.
   */
  DarkCyan,
  /*! \brief Color magenta.
   */
  Magenta,
  /*! \brief Color dark magenta.
   */
  DarkMagenta,
  /*! \brief Color yellow.
   */
  Yellow,
  /*! \brief Color dark yellow.
   */
  DarkYellow,
  /*! \brief Color medium gray.
   */
  Gray,
  /*! \brief Color dark gray.
   */
  DarkGray,
  /*! \brief Color light gray.
   */
  LightGray,
  /*! \brief Color transparent.
   */
  Transparent
};

/*! \brief Enumeration for keyboard modifiers
 *
 * \sa WMouseEvent::modifiers(), WKeyEvent::modifiers()
 *
 * \ingroup signalslot
 */
enum class KeyboardModifier {
  None = 0x0,     //!< No modifiers
  Shift = 0x1,    //!< Shift key pressed
  Control = 0x2,  //!< Control key pressed
  Alt = 0x4,      //!< Alt key pressed
  Meta = 0x8      //!< Meta key pressed ("Windows" or "Command" (Mac) key)
};

W_DECLARE_OPERATORS_FOR_FLAGS(KeyboardModifier)

/*! \brief Enumeration for key codes
 *
 * These are key codes that identify a key on a keyboard. All keys
 * listed here can be identified across all browsers and (Western)
 * keyboards. A %Key is returned by WKeyEvent::key(). If you want to
 * identify a character, you should use the WKeyEvent::charCode()
 * method instead.
 *
 * \sa WInteractWidget::keyWentDown, WInteractWidget::keyWentUp
 *
 * \ingroup signalslot
 */
enum class Key {
  Unknown = 0,     //!< Unknown key.
  Enter = 13,      //!< Enter key.
  Tab = 9,         //!< Tab key.
  Backspace = 8,   //!< Backspace key.
  Shift = 16,      //!< Shift key.
  Control = 17,    //!< Control key.
  Alt = 18,        //!< Alt key.
  PageUp = 33,     //!< Page up key.
  PageDown = 34,   //!< Page down key.
  End = 35,        //!< End key.
  Home = 36,       //!< Home key.
  Left = 37,       //!< Left arrow key.
  Up = 38,         //!< Up arrow key.
  Right = 39,      //!< Right arrow key.
  Down = 40,       //!< Down arrow key.
  Insert = 45,     //!< Insert key.
  Delete = 46,     //!< Delete key.
  Escape = 27,     //!< Escape key.
  F1 = 112,        //!< F1 function key.
  F2 = 113,        //!< F2 function key.
  F3 = 114,        //!< F3 function key.
  F4 = 115,        //!< F4 function key.
  F5 = 116,        //!< F5 function key.
  F6 = 117,        //!< F6 function key.
  F7 = 118,        //!< F7 function key.
  F8 = 119,        //!< F8 function key.
  F9 = 120,        //!< F9 function key.
  F10 = 121,       //!< F10 function key.
  F11 = 122,       //!< F11 function key.
  F12 = 123,       //!< F12 function key.
  Space = ' ',     //!< Space
  A = 'A',         //!< 'A' key
  B = 'B',         //!< 'B' key
  C = 'C',         //!< 'C' key
  D = 'D',         //!< 'D' key
  E = 'E',         //!< 'E' key
  F = 'F',         //!< 'F' key
  G = 'G',         //!< 'G' key
  H = 'H',         //!< 'H' key
  I = 'I',         //!< 'I' key
  J = 'J',         //!< 'J' key
  K = 'K',         //!< 'K' key
  L = 'L',         //!< 'L' key
  M = 'M',         //!< 'M' key
  N = 'N',         //!< 'N' key
  O = 'O',         //!< 'O' key
  P = 'P',         //!< 'P' key
  Q = 'Q',         //!< 'Q' key
  R = 'R',         //!< 'R' key
  S = 'S',         //!< 'S' key
  T = 'T',         //!< 'T' key
  U = 'U',         //!< 'U' key
  V = 'V',         //!< 'V' key
  W = 'W',         //!< 'W' key
  X = 'X',         //!< 'X' key
  Y = 'Y',         //!< 'Y' key
  Z = 'Z',         //!< 'Z' key
  Key_1 = '1',         //!< '1' key
  Key_2 = '2',         //!< '2' key
  Key_3 = '3',         //!< '3' key
  Key_4 = '4',         //!< '4' key
  Key_5 = '5',         //!< '5' key
  Key_6 = '6',         //!< '6' key
  Key_7 = '7',         //!< '7' key
  Key_8 = '8',         //!< '8' key
  Key_9 = '9',         //!< '9' key
  Key_0 = '0',         //!< '0' key
};

/*! \brief Enumeration for the check state of a check box.
 *
 * \sa WCheckBox
 */
enum class CheckState {
  Unchecked,        //!< Unchecked
  PartiallyChecked, //!< Partially checked (for a tri-state checkbox)
  Checked           //!< Checked
};

/*! \brief Enumeration that indicates how to change a selection.
 *
 * \sa WPaintedWidget::update(), WPaintDevice::paintFlags()
 */
enum class PaintFlag {
  Update = 0x1 //!< The canvas is not cleared, but further painted on.
};

W_DECLARE_OPERATORS_FOR_FLAGS(PaintFlag)

/*! \brief Enumeration that indicates a %Wt entrypoint type.
 *
 * An entry point binds a behavior to a public URL. Only the wthttpd
 * connector currently supports multiple entry points.
 */
enum class EntryPointType {
  /*! \brief Specifies a full-screen application.
   *
   * A full screen application manages the entire browser window and
   * provides its own HTML page.
   *
   * \sa WServer::addEntryPoint(), WApplication::root()
   */
  Application,

  /*! \brief Specifies an application that manages one or more widgets.
   *
   * A widget set application is part of an existing HTML page. One or
   * more HTML elements in that web page may be bound to widgets
   * managed by the application.
   *
   * The application presents itself as a JavaScript file, and
   * therefore should be embedded in the web page using a
   * &lt;script&gt; tag, from within the &lt;body&gt; (since it needs
   * access to the &lt;body&gt;).
   *
   * \note A EntryPointType::WidgetSet application requires JavaScript support
   *
   * \sa WServer::addEntryPoint(), WApplication::bindWidget()
   */
  WidgetSet,

  /*! \brief Specifies a static resource.
   *
   * A static resource binds a WResource to a public URL, and is not bound
   * to a specific session.
   *
   * \if cpp
   * \sa WServer::addResource()
   * \elseif java
   * @see WtServlet#addResource(WResource r, String path)
   * \endif
   */
  StaticResource
};


/*! \enum RenderFlag
 *  \brief Enum for internal use.
 */
enum class RenderFlag {
  Full    = 0x1,
  Update  = 0x2
};

W_DECLARE_OPERATORS_FOR_FLAGS(RenderFlag)

/*! \brief Flags that specify how to match two values.
 *
 * Except when MatchFlag::Exactly, the lexical matching is done (by comparing
 * string representations of the value with the query). This is by default
 * case insensitive, unless MatchFlag::CaseSensitive is OR'ed.
 *
 * \ingroup modelview
 */
enum class MatchFlag {
  Exactly = 0x0,       //!< Same type and value
  StringExactly = 0x1, //!< Lexical match
  StartsWith = 0x2,    //!< Match start with query
  EndsWith = 0x3,      //!< Match end with query
  RegExp = 0x4,        //!< Regular expression match
  WildCard = 0x5,      //!< Wildcard match
  CaseSensitive = 0x10,//!< Case sensitive
  Wrap = 0x20          //!< Wrap around whole model
};

W_DECLARE_OPERATORS_FOR_FLAGS(MatchFlag)

/*! \brief Type part of MatchFlags */
static const WFlags<MatchFlag> MatchTypeMask = 
  MatchFlag::Exactly | MatchFlag::StringExactly |
  MatchFlag::StartsWith | MatchFlag::EndsWith |
  MatchFlag::RegExp | MatchFlag::WildCard;

/*! \brief Flags that indicate table header options
 *
 * \sa WAbstractItemModel::headerFlags()
 *
 * \ingroup modelview
 */
enum class HeaderFlag {
  /*! \brief Flag that indicates that the column can be expanded.
   *
   * \sa WAbstractItemModel::expandColumn()
   */
  ColumnIsCollapsed = 0x1,

  /*! \brief Flag that indicates that the column was expanded to the left.
   *
   * \sa WAbstractItemModel::collapseColumn()
   */
  ColumnIsExpandedLeft = 0x2,

  /*! \brief Flag that indicates that the column was expanded to the right.
   *
   * \sa WAbstractItemModel::collapseColumn()
   */
  ColumnIsExpandedRight = 0x4,

  /*! \brief Flag that indicates that the header can be checked.
   */
  UserCheckable = 0x8,

  /*! \brief Flag that indicates that the item has three states.
   *
   * When set, Wt::ItemDataRole::Checked data is of type Wt::CheckState
   */
  Tristate = 0x10,

  /*! \brief Flag that indicates that the item text (ItemDataRole::Display, ItemDataRole::ToolTip)
   * is HTML
   */
  XHTMLText = 0x40
};

W_DECLARE_OPERATORS_FOR_FLAGS(HeaderFlag)

/*! \brief Enumeration that indicates a meta header type.
 *
 * \sa WApplication::addMetaHeader()
 */
enum class MetaHeaderType {
  Meta,       //!< Of the form &lt;meta name=... content=... &gt;
  Property,   //!< Of the form &lt;meta property=... content=... &gt; 
  HttpHeader  //!< Of the form &lt;meta http-equiv=... content=... &gt;
};

/*! \brief An enumeration describing an event's type.
 *
 * \sa WEvent::eventType()
 */
enum class EventType {
  Other,   //!< An event which is not user- or timer-initiated
  User,    //!< A user-initiated event
  Timer,   //!< A timer-initiated event
  Resource //!< An event which is a resource request
};

/*! \brief An enumeration describing a layout direction.
 */
enum class LayoutDirection {
  LeftToRight, //!< Left-to-Right layout
  RightToLeft, //!< Right-to-left layout
  TopToBottom, //!< Top-to-bottom layout
  BottomToTop  //!< Bottom-to-top layout
};

enum class WebWriteEvent {
  Completed, // Write was successful
  Error      // Write failed
};

enum class WebReadEvent {
  Error,   // Read failed
  Message, // Read a WebSocket message
  Ping     // Read Ping message
};

/*! \brief Enumeration that determines when contents should be loaded.
 */
enum class ContentLoading {
  Lazy,  //!< Lazy loading: on first use
  Eager, //!< Pre-loading: before first use
  NextLevel, //!< Pre-load also next level (if applicable, e.g. for WTreeNode)
};

/*! \brief Enumeration that indicates how much of (a media) is loaded.
 */
enum class MediaReadyState {
  HaveNothing = 0,     //!< No information available
  HaveMetaData = 1,    //!< Metadata loaded: duration, width, height
  HaveCurrentData = 2, //!< Data at playback position is available
  HaveFutureData = 3,  //!< Have data to play for a while
  HaveEnoughData = 4   //!< Enough to reach the end without stalling
};

#ifndef WT_TARGET_JAVA
/*! \brief Enumeration that indicates a regular expression option.
 */
enum class RegExpFlag {
  MatchCaseInsensitive = 0x1 //!< Case insensitive matching
};

W_DECLARE_OPERATORS_FOR_FLAGS(RegExpFlag)
#endif // WT_TARGET_JAVA

#ifdef WT_TARGET_JAVA
struct AutoCloseable {
  virtual void close() = 0;
};
#endif // WT_TARGET_JAVA

}

#endif // WGLOBALS_H_
