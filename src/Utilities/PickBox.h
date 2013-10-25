#ifndef PICKBOX_H_
#define PICKBOX_H_

#include <vector>
using namespace std;

#include <qgl.h>

//definition for a 2D box
struct Box{
  Box() : m_left(0), m_right(0), m_bottom(0), m_top(0) {}

  double m_left, m_right, m_bottom, m_top; //left, right,top,bottom
};

class Model;
vector<Model*>& GetPickedSceneObjs();

//Handle user picking
class PickBox {
  public:

    PickBox() : m_leftMouseButton(false), m_w(0), m_h(0) {}

    void Draw();

    void MousePressed(QMouseEvent* _e);
    void MouseReleased(QMouseEvent* _e);
    void MouseMotion(QMouseEvent* _e);

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
};

PickBox& GetPickBox();

//Get the picking box dimensions
void PickBoxDim(int *xOffset,int *yOffset,int *w,int *h);

#endif
