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
class QToolButton;

#include <qwidget.h>


class VizmoRoadmapGUI: public QToolBar
{
    Q_OBJECT
        
public:
    
    VizmoRoadmapGUI(QMainWindow * parent=NULL,char *name=NULL);
    void reset();
    void createGUI();

public slots:
    void changeSize();      //Changes road map node's size
    void changeColor();      //Changes CC's color
    void setSameColor();     //Set all CC's to the same color

signals:
    void callUpdate();

private slots:
	void getSelectedItem();

private:

    // private:
    QListBox *l;
    double size;
    QToolButton *nodesize, *nodesColor, *nodesSameColor;
    string m_shapeString;
};
