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
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qlistview.h> 
#include <qsplitter.h> 

class QAction;
class QLineEdit;
class QLabel;


#include <vector>
#include <string>
using namespace std;

class VizmoListViewItem : public QListViewItem
{
public:
    VizmoListViewItem(QListView * parent):QListViewItem(parent){ model=NULL; }
    VizmoListViewItem(QListViewItem * parent):QListViewItem(parent){model=NULL;}
    CGLModel * model;
};

///////////////////////////////////////////////////////////////////////////////
class QListView;

class VizmoItemSelectionGUI: public QToolBar
{
    Q_OBJECT
        
public:
    
    VizmoItemSelectionGUI(QMainWindow * parent=NULL,char *name=NULL);
	
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
	
	QListView * listview;
	int maxNoModels;
	list<VizmoListViewItem*> items;
	
};




