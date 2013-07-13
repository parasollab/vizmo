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
        m_LMB_DOWN=false; m_W=m_H=0; 
    }

    ///////////////////////////////////////////////////////////////////////////
    // Core
    void Draw( void );
    void MP( QMouseEvent * e ); //mouse button
    void MR( QMouseEvent * e ); //mouse button
    void MM( QMouseEvent * e );  //mouse motion

    ///////////////////////////////////////////////////////////////////////////
    // Access
    const gliBox& getPickBox() const { return m_PickBox; }
    bool isPicking() const { return m_LMB_DOWN; }
    //need be updated when window size changed
    void setWinSize(int W, int H) { m_W=W; m_H=H; }

private:

    gliBox m_PickBox;
    bool m_LMB_DOWN;
    int m_W, m_H; //width and height of window
};

///////////////////////////////////////////////////////////////////////////
//Singleton
gliPickBox & gliGetPickBox();

#endif //_GLI_PICKBox_H_
