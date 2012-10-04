// TO DO: rethink the way the configuraiotns are updated
// i think we can just call the updateConfiger(int) and make it control all motions
// TO DO: Set up the slider values initially. When loaded, signal slider
// and update slider values.


#include "vizmo2.h"
#include "MainWin.h" 
/** 
* This class deifines the animation GUI for vizmo2
*/

#ifdef WIN32
#pragma warning(disable : 4786)
#endif

///////////////////////////////////////////////////////////////////////////////
// QT Headhers
#include <Q3ListView> 
#include <qsplitter.h> 
//Added by qt3to4:
#include <QLabel>
#include <QToolBar>

class QAction;
class QLineEdit;
class QLabel;

#include <vector>
#include <string>
using namespace std;

class VizmoListViewItem : public Q3ListViewItem
{
  public: //Need to find qt4 alternative for Q3ListViewItem...this may fix CC selection 
    VizmoListViewItem(Q3ListView* _parent):Q3ListViewItem(_parent){m_model=NULL;}
    VizmoListViewItem(Q3ListViewItem* _parent):Q3ListViewItem(_parent){m_model=NULL;}
    CGLModel* m_model;
};

///////////////////////////////////////////////////////////////////////////////
class Q3ListView;

class VizmoItemSelectionGUI : public QToolBar 
{
  Q_OBJECT
        
  public:
    
    VizmoItemSelectionGUI(QString _title, QWidget* _parent = NULL); 	
	
    void reset();
    void fillTree(vector<PlumObject*>& _objs);
    VizmoListViewItem* createItem(VizmoListViewItem* _p, CGLModel* _model);
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
	
    Q3ListView* m_listview;
    int m_maxNoModels;
    vector<VizmoListViewItem*> items;
};




