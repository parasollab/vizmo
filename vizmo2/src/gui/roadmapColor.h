#include "vizmo2.h"

/** 
* This class deifines the animation GUI for vizmo2
*/

#ifdef WIN32
#pragma warning(disable : 4786)
#endif

///////////////////////////////////////////////////////////////////////////////
// QT Headhers
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <vector>
#include <string>

using namespace std;

#include <qwidget.h>


class VizmoRoadmapColorGUI: public QToolBar
{
    Q_OBJECT
        
public:
    
    VizmoRoadmapColorGUI(QMainWindow * parent=NULL,char *name=NULL);
    void reset();

public slots:
    void changeColor();      //Changes CC's color

signals:
    void callUpdate();

private:

    // private:
	double size;
};
