#ifndef _GLI_PICKBox_H_
#define _GLI_PICKBox_H_
///////////////////////////////////////////////////////////////////////////
#include "gliDataStructure.h" //defines gliBox

///////////////////////////////////////////////////////////////////////////
#include <vector>
using namespace std;

#include <qgl.h>

//Handle user picking
class gliPickBox
{
public:

    gliPickBox(){
        m_lMB_DOWN=false; m_w=m_h=0;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Core
    void Draw( void );
    void MP( QMouseEvent * e ); //mouse button
    void MR( QMouseEvent * e ); //mouse button
    void MM( QMouseEvent * e );  //mouse motion

    ///////////////////////////////////////////////////////////////////////////
    // Access
    const gliBox& getPickBox() const { return m_pickBox; }
    bool isPicking() const { return m_lMB_DOWN; }
    //need be updated when window size changed
    void setWinSize(int W, int H) { m_w=W; m_h=H; }

private:

    gliBox m_pickBox;
    bool m_lMB_DOWN;
    int m_w, m_h; //width and height of window
};

///////////////////////////////////////////////////////////////////////////
//Singleton
gliPickBox & gliGetPickBox();

#endif //_GLI_PICKBox_H_
