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

class QListBox;

#include <qwidget.h>


class VizmoRoadmapNodesShapeGUI: public QToolBar
//class VizmoRoadmapNodesShapeGUI: public QWidget
{
    Q_OBJECT
        
public:
    
    VizmoRoadmapNodesShapeGUI(QMainWindow * parent=NULL,char *name=NULL);
	void reset();

public slots:
    void changeSize();      //Changes road map node's size

signals:
    void callUpdate();

private slots:
	void getSelectedItem();

private:

    // private:
    QListBox *l;
	double size;
};