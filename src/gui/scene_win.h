#ifndef _SCENE_WIN_H_
#define _SCENE_WIN_H_

/**
* This file defines class for scene window of vimzo2. 
* The scene window uses opengl widget.
*/

///////////////////////////////////////////////////////////////////////////////
#include <qgl.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>
// BSS
#include <GL/gli.h>
#include <list>
using namespace std;

class VizGLWin : public QGLWidget
{
    Q_OBJECT
        
public:
    
    VizGLWin( QWidget * parent=0, const char * name=0 );
    void resetCamera();
    
    // BSS
    vector<gliObj> *objs2;
    
    void getWidthHeight(int *,int *);
    
    void setClearColor(double r, double g, double b) const {
		glClearColor( r , g, b, 0 );
	}
    //Collision Detection related vars.
    bool CDOn;

    // reset tranformation tool
    // it calls class gli::gliReset()
    void resetTransTool();

signals:
    
    void selectByRMB();
    void clickByRMB();
    void selectByLMB();
    void clickByLMB();
    void MRbyGLI();

protected:
    
    void initializeGL();
    void resizeGL( int, int );
    void paintGL();
    void mousePressEvent ( QMouseEvent * );
    void mouseReleaseEvent ( QMouseEvent * );
    void mouseMoveEvent ( QMouseEvent * );
    void keyPressEvent ( QKeyEvent * e );
	
    void drawText(); //this should not be here but it is here for now

public slots:
	void showGrid();
	void showAxis();
    
private slots:
    
    void togleSlectionSlot();
    void getBoxDimensions(int *,int *,int *,int *); 
    void simulateMouseUpSlot();
    
private:
    
    void SetLight();
    void SetLightPos(){ //set light position
        static GLfloat light_position[] = { 250.0f, 250.0f, 250.0f, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        static GLfloat light_position2[] = { -250.0f, 250.0f, -250.0f, 1.0f };
        glLightfv(GL_LIGHT1, GL_POSITION, light_position2);
    }
	
    void drawText(list<string>& info);
    bool takingSnapShot;

    bool m_bShowGrid, m_bShowAxis;

};


#endif /*_SCENE_WIN_H_ */
