/**
 * This file defines class for scene window of vimzo2. 
 * The scene window uses opengl widget.
 */

///////////////////////////////////////////////////////////////////////////////
#include <qgl.h>
// BSS
#include <GL/gli.h>

class VizGLWin : public QGLWidget
{
    Q_OBJECT

public:

    VizGLWin( QWidget * parent=0, const char * name=0 );
    void resetCamera();
    
    // BSS
    vector<gliObj> *objs2;

    double R, G, B;

signals:
    
    void selectByRMB();

protected:

    void initializeGL();
    void resizeGL( int, int );
    void paintGL();
    void mousePressEvent ( QMouseEvent * );
    void mouseReleaseEvent ( QMouseEvent * );
    void mouseMoveEvent ( QMouseEvent * );
    void keyPressEvent ( QKeyEvent * e );

private:

    void SetLight();
    void SetLightPos(){ //set light position
        static GLfloat light_position[] = { 250.0f, 250.0f, 250.0f, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        static GLfloat light_position2[] = { -250.0f, 250.0f, -250.0f, 1.0f };
        glLightfv(GL_LIGHT1, GL_POSITION, light_position2);
    }
};




