
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

class QAction;
class QLineEdit;
class QLabel;


#include <vector>
#include <string>
using namespace std;

class QListView;
class VizmoAttributeSelectionGUI: public QToolBar
{
    Q_OBJECT 
public:
    VizmoAttributeSelectionGUI(QMainWindow * parent=NULL,char *name=NULL);
    void reset();
private:
    QListView *list;  
};