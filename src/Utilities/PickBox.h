#ifndef PICKBOX_H_
#define PICKBOX_H_

#include <vector>
using namespace std;

#include <qgl.h>

//definition for a 2D box
struct Box{
  Box() : m_left(-1), m_right(-1), m_bottom(-1), m_top(-1) {}

  double m_left, m_right, m_bottom, m_top; //left, right,top,bottom
};

//Handle user picking
class PickBox {
  public:
    enum Highlight {LEFT = 1, RIGHT = 2, TOP = 4, BOTTOM = 8};

    PickBox() : m_leftMouseButton(false), m_w(0), m_h(0),
    m_highlightedPart(0), m_resizing(false) {}

    void Draw();

    void MousePressed(QMouseEvent* _e);
    void MouseReleased(QMouseEvent* _e);
    void MouseMotion(QMouseEvent* _e);
    bool PassiveMouseMotion(QMouseEvent* _e);

    ///////////////////////////////////////////////////////////////////////////
    // Access
    const Box& GetBox() const {return m_pickBox;}
    bool IsPicking() const {return m_leftMouseButton;}

    //need be updated when window size changed
    void SetWinSize(int _w, int _h) {m_w = _w; m_h = _h;}

  private:
    Box m_pickBox;
    bool m_leftMouseButton; //true:left mouse button down
    int m_w, m_h; //width and height of window
    int m_highlightedPart;
    bool m_resizing;
};

#endif
