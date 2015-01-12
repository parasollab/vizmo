#ifndef PICKBOX_H_
#define PICKBOX_H_

#include <vector>
using namespace std;

#include <QtGui>

////////////////////////////////////////////////////////////////////////////////
/// \brief Defines an oriented 2D box.
/// \details The box is defined by four doubles indicating the maximum and
/// minimum x/y values.
////////////////////////////////////////////////////////////////////////////////
struct Box{

  Box() : m_left(-1), m_right(-1), m_bottom(-1), m_top(-1) {}

  double m_left;   ///< The x-value of the left side of the box.
  double m_right;  ///< The x-value of the right side of the box.
  double m_bottom; ///< The y-value of the bottom of the box.
  double m_top;    ///< The y-value of the top of the box.

};

////////////////////////////////////////////////////////////////////////////////
/// \brief Creates a dashed-line box in the OpenGL scene to allow the user to
/// select a specific area for screen capture.
////////////////////////////////////////////////////////////////////////////////
class PickBox {

  public:

    ////////////////////////////////////////////////////////////////////////////
    /// Defines a bit field for describing which parts of the pick box are
    /// currently under the user's influence.
    enum Highlight {None = 0, Left = 1, Right = 2, Top = 4, Bottom = 8, All = 16};

    PickBox() : m_picking(false),
    m_highlightedPart(0), m_resizing(false), m_translating(false) {}

    void Draw(); ///< Render the pick box in the OpenGL scene.

    //event handlers
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Handle mouse button press events, which initiate drawing,
    /// translation, and rotation.
    /// \param[in] _e The mouse event to handle.
    void MousePressed(QMouseEvent* _e);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Handle mouse button release events, which specify that the pick
    /// box is no longer being moved or resized.
    /// \param[in] _e The mouse event to handle.
    void MouseReleased(QMouseEvent* _e);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Handle active mouse move events, which move and resize the pick
    /// box.
    /// \param[in] _e The mouse event to handle.
    void MouseMotion(QMouseEvent* _e);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Handle passive mouse motion, which is used to determine which
    /// part(s) of the pick box the user is trying to grab.
    /// \param[in] _e The mouse event to handle.
    bool PassiveMouseMotion(QMouseEvent* _e);

    //access
    const Box& GetBox() const {return m_pickBox;}
    bool IsPicking() const {return m_picking;}

  private:

    Box m_pickBox, m_origBox; ///< Current box and box used for translation.
    bool m_picking;           ///< True when using the pick box, false otherwise.
    int m_highlightedPart;    ///< Bitmask for the current highlighted parts.
    bool m_resizing, m_translating; ///< True when resizing/translating the box.
    QPoint m_clicked;         ///< Window location of latest mouse click.
};

#endif
