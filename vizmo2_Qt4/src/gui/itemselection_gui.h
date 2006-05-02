// TO DO: rethink the way the configuraiotns are updated
// i think we can just call the updateConfiger(int) and make it control all motions
// TO DO: Set up the slider values initially. When loaded, signal slider
// and update slider values.


#include "vizmo2.h"

/** 
* This class deifines the animation GUI for vizmo2
*/

#ifdef WIN32
#pragma warning(disable : 4786)
#endif

///////////////////////////////////////////////////////////////////////////////
// QT Headhers
#include <q3mainwindow.h>
#include <q3toolbar.h>
#include <q3listview.h> 
#include <qsplitter.h> 
//Added by qt3to4:
#include <QLabel>

class QAction;
class QLineEdit;
class QLabel;


#include <vector>
#include <string>
using namespace std;

class VizmoListViewItem : public Q3ListViewItem
{
public:
    VizmoListViewItem(Q3ListView * parent):Q3ListViewItem(parent){ model=NULL; }
    VizmoListViewItem(Q3ListViewItem * parent):Q3ListViewItem(parent){model=NULL;}
    CGLModel * model;
};

///////////////////////////////////////////////////////////////////////////////
class Q3ListView;

class VizmoItemSelectionGUI: public Q3ToolBar
{
    Q_OBJECT
        
public:
    
    VizmoItemSelectionGUI(Q3MainWindow * parent=NULL,char *name=NULL);
	
    //VizmoItemSelectionGUI(QWidget * parent, char * name=NULL);
	
    void reset();
    void fillTree(vector<PlumObject*>& objs);
    VizmoListViewItem * createItem(VizmoListViewItem * p, CGLModel * model);
    void trim();
	
public slots:
	void select();
	
signals:
	void itemSelected();
	void callUpdate();
	
private slots:
	void selectionChanged();
	
private:
	
	void clear();
	
	Q3ListView * listview;
	int maxNoModels;
	list<VizmoListViewItem*> items;
	
};




