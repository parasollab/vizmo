#include "vizmo2.h"
#include "roadmap.h"
#include "MapObj/SimpleCfg.h"

///////////////////////////////////////////////////////////////////////////////// 
//// Include Qt Headers
#include <qapplication.h>
#include <qaction.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qtoolbutton.h>
#include <qwidget.h>
#include <qinputdialog.h>
#include <qcolordialog.h> 
#include <qobjectlist.h>
#include <qcursor.h>

///////////////////////////////////////////////////////////////////////////////
//Icons
#include "icon/shapes1.xpm"
#include "icon/pallet.xpm"
#include "icon/make.xpm"

VizmoRoadmapGUI::VizmoRoadmapGUI(QMainWindow *parent,char *name)
:QToolBar(parent, name){
    
    setLabel("CC's features");
    createGUI();
    m_bEditModel=false;
    m_addNode=false;
    m_addEdge=false;
}

void VizmoRoadmapGUI::createGUI()
{
    
    sizeAction = new QAction
    ("Size",QPixmap(icon_shapes1),"&Size",CTRL+Key_S,this);
    connect(sizeAction,SIGNAL(activated()), this, SLOT(changeSize()) );
    sizeAction->addTo(this);
    sizeAction->setEnabled(false);

    colorAction = new QAction
    ("Random",QPixmap(icon_pallet),"&Random Color",CTRL+Key_R,this);
    connect(colorAction,SIGNAL(activated()), this, SLOT(changeColor()) );
    colorAction->addTo(this);
    colorAction->setEnabled(false);

    editAction = new QAction
    ("Edit",QPixmap(icon_make),"Edit &Map",CTRL+Key_E,this,"",true);
    connect(editAction,SIGNAL(activated()), this, SLOT(editMap()) );
    editAction->addTo(this);
    editAction->setEnabled(false);

    addNodeAction = new QAction
    ("New Node",QPixmap(icon_make),"Add &Node",CTRL+Key_E,this,"",true);
    connect(addNodeAction,SIGNAL(activated()), this, SLOT(addNode()) );
    addNodeAction->setEnabled(false);

    addEdgeAction = new QAction
    ("New Edge",QPixmap(icon_make),"Add &Edge",CTRL+Key_E,this,"",true);
    connect(addEdgeAction,SIGNAL(activated()), this, SLOT(addEdge()) );
    addEdgeAction->setEnabled(false);

    l =  new QListBox( this );
    l->setFocusPolicy( QWidget::StrongFocus ); 
    l->insertItem( QString::fromLatin1( "Robot" ));
    l->insertItem( QString::fromLatin1( "Box" ));
    l->insertItem( QString::fromLatin1( "Point" ));
    l->setSelected(2,true);
    connect(l,SIGNAL(clicked(QListBoxItem *)),this,SLOT(getSelectedItem()));
    size=0.5;

    //Find all toolbar button and show text
    QObjectList * l = queryList( "QToolButton" );
    QObjectListIt it( *l );             // iterate over the buttons
    QObject * obj;
    while ( (obj=it.current()) != 0 ) { // for each found object...
        ++it;
        ((QToolButton*)obj)->setUsesTextLabel ( true );
    }
}

void VizmoRoadmapGUI::reset()
{
    if( GetVizmo().IsRoadMapLoaded() && 
        GetVizmo().isRoadMapShown() ){
        editAction->setEnabled(true);
        sizeAction->setEnabled(true);
        colorAction->setEnabled(true);
    }
    else{
        editAction->setEnabled(false);
        sizeAction->setEnabled(false);
        colorAction->setEnabled(false);
    }
}

void VizmoRoadmapGUI::getSelectedItem()
{
    string s;
    for ( unsigned int i = 0; i < l->count(); i++ ) {
        QListBoxItem *item = l->item( i );
        // if the item is selected...
        if ( item->selected() )
            s = (string)item->text().ascii();
    }
    m_shapeString = s;
    GetVizmo().ChangeNodesShape(s);
    emit callUpdate(); //set an update event
}

void VizmoRoadmapGUI::changeSize(){
    
    bool ok = false;
    size = QInputDialog::getDouble(tr("Change Roadmap Node Size"), 
                 tr("Enter a positive number to scale the nodes"),
                 size, 0, 1, 2,  &ok,  this);
    if(ok){
        string shape;
        for ( unsigned int i = 0; i < l->count(); i++ ){
            QListBoxItem *item = l->item( i );
            // if the item is selected...
            if ( item->selected() )
                shape = (string)item->text().ascii();
        }
        GetVizmo().ChangeNodesSize(size, shape);
        emit callUpdate(); //set an updat event
    }

}

void VizmoRoadmapGUI::changeColor(){
    GetVizmo().ChangeNodesRandomColor();
    emit callUpdate(); //set an updat event
}

void VizmoRoadmapGUI::editMap()
{
    m_bEditModel=!m_bEditModel;
    if(m_bEditModel){
        m_Map_Changed=false;
        parentWidget()->setCursor(QCursor(crossCursor));
        GetVizmo().GetMap()->getModel()->EnableSelection(true);
        addEdgeAction->setEnabled(true);
        addNodeAction->setEnabled(true);
    }
    else{
        parentWidget()->setCursor(QCursor(arrowCursor));
        GetVizmo().GetMap()->getModel()->EnableSelection(false);
        addEdgeAction->setEnabled(false); addEdgeAction->setOn(false);
        addNodeAction->setEnabled(false); addNodeAction->setOn(false);
        m_addNode=false;
        m_addEdge=false;
        if(m_Map_Changed){
            //update the roadmap CC
        }
    }
}

void VizmoRoadmapGUI::addNode()
{
    m_addNode=!m_addNode;
    //turn off add edge anyway
    m_addEdge=false;
    addEdgeAction->setOn(false);
}

void VizmoRoadmapGUI::addEdge()
{
    m_addEdge=!m_addEdge;
    //turn off add edge anyway
    m_addNode=false;
    addNodeAction->setOn(false);
}

void VizmoRoadmapGUI::handleSelect()
{
    if( !m_bEditModel ) return;

    //find nodes
    m_Nodes.clear();
    vector<gliObj>& sel=GetVizmo().GetSelectedItem();
    typedef vector<gliObj>::iterator OIT;
    for(OIT i=sel.begin();i!=sel.end();i++){
        if( ((CGLModel*)(*i))->GetName()=="Node" ) {
			m_Nodes.push_back((CGLModel*)(*i));
        }//end if
    }//end for

    if( m_addEdge )
        handleAddEdge();
    else if( m_addNode )
        handleAddNode();
    else
        handleEditMap();
}

void VizmoRoadmapGUI::handleAddEdge()
{
    //find two nodes...
}

void VizmoRoadmapGUI::handleAddNode()
{

}

void VizmoRoadmapGUI::handleEditMap()
{
    /*
    static CGLModel* node=NULL;
    static double ot[3];
    static double or[3];

    //find node
    CGLModel* current_node=NULL;
    vector<gliObj>& sel=GetVizmo().GetSelectedItem();
    typedef vector<gliObj>::iterator OIT;
    for(OIT i=sel.begin();i!=sel.end();i++){
        if( ((CGLModel*)(*i))->GetName()=="Node" ) {
            current_node=(CGLModel*)(*i);
            break;
        }
    }
    if( current_node==NULL ) return; //no node is selected

    //find node
    if( current_node==node){
        double diff=fabs(ot[0]-node->tx())+
                    fabs(ot[1]-node->ty())+
                    fabs(ot[2]-node->tz())+
                    fabs(or[0]-node->rx())+
                    fabs(or[1]-node->ry())+
                    fabs(or[2]-node->rz());
        if( diff>1e-10 ){
            m_Map_Changed=true;
            cout<<"node moved"<<endl;
        }
        node=NULL;
    }
    else{
        //remember node
        node=current_node;
        ot[0]=node->tx(); ot[1]=node->ty(); ot[2]=node->tz();
        or[0]=node->rx(); or[1]=node->ry(); or[2]=node->rz();
    }
    */
    if( m_Nodes.empty()==false ){
		CGLModel * n=m_Nodes.front();
        old_T[0]=n->tx(); old_T[1]=n->ty(); old_T[2]=n->tz();
        old_R[0]=n->rx(); old_R[1]=n->ry(); old_R[2]=n->rz();     
    }
}

void VizmoRoadmapGUI::MoveNode()
{
    if( m_Nodes.empty() ) return;
	CGLModel * n=m_Nodes.front();
    double diff=fabs(old_T[0]-n->tx())+
                fabs(old_T[1]-n->ty())+
                fabs(old_T[2]-n->tz())+
                fabs(old_R[0]-n->rx())+
                fabs(old_R[1]-n->ry())+
                fabs(old_R[2]-n->rz());
    if( diff>1e-10 ){
        m_Map_Changed=true;
        ((CSimpleCfg*)n)->GetCC()->ReBuildAll();
		emit callUpdate();
    }   
}
