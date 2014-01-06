#ifndef PICKBOX_H_
#define PICKBOX_H_

#include <vector>
using namespace std;

#include <QtGui>

//definition for a 2D box
struct Box{
  Box() : m_left(-1), m_right(-1), m_bottom(-1), m_top(-1) {}

  double m_left, m_right, m_bottom, m_top; //left, right,top,bottom
};

//Handle user picking
class PickBox {
  public:
    enum Highlight {NONE = 0, LEFT = 1, RIGHT = 2, TOP = 4, BOTTOM = 8, ALL = 16};

    PickBox() : m_leftMouseButton(false),
    m_highlightedPart(0), m_resizing(false), m_translating(false) {}

    void Draw();

    void MousePressed(QMouseEvent* _e);
    void MouseReleased(QMouseEvent* _e);
    void MouseMotion(QMouseEvent* _e);
    bool PassiveMouseMotion(QMouseEvent* _e);

    ///////////////////////////////////////////////////////////////////////////
    // Access
    const Box& GetBox() const {return m_pickBox;}
    bool IsPicking() const {return m_leftMouseButton;}

  private:
    Box m_pickBox, m_origBox; //current box and box used for translation
    bool m_leftMouseButton; //true:left mouse button down
    int m_highlightedPart; //mouse over which part of box?
    bool m_resizing, m_translating; //currently resizing/translating the box
    QPoint m_clicked; //point clicked for mouse
};

#endif
