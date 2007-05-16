#include "vizmo2.h"
#include "obj_property.h"
#include "icon/eye.xpm"

#include <q3grid.h>
#include <q3vbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qvalidator.h>
#include <qlayout.h>
//Added by qt3to4:
#include <QPixmap>
#include <QVBoxLayout>

BasicPropertyDialog * propertyDlg=NULL;

void InvokeObjPropertyDialog(QWidget *parent)
{
	if( propertyDlg!=NULL ){
		propertyDlg->hide();
		delete propertyDlg;
	}
	propertyDlg=new BasicPropertyDialog(parent);
	propertyDlg->show();
}

BasicPropertyDialog::BasicPropertyDialog
(QWidget *parent, const char *name, Qt::WFlags f)
:QDialog(parent,name,f)
{
	vector<gliObj>& sel=GetVizmo().GetSelectedItem(); 
	gliObj first=sel.front();
	setIcon(QPixmap(icon_eye));

	///////////////////////////////////////////////////////////////////////
    QVBoxLayout* vbox = new QVBoxLayout(this,8);
    vbox->setAutoAdd(TRUE);
    Q3Grid* controls = new Q3Grid(2,Qt::Horizontal,this);
    controls->setSpacing(-1); controls->setMargin(1);

    (new QLabel("Translate <b>X</b>:     ",controls))->setAlignment(Qt::AlignRight);
    QLineEdit * tx=new QLineEdit(controls);
    tx->setValidator(new QDoubleValidator(tx));
    
    (new QLabel("Translate <b>Y</b>:     ",controls))->setAlignment(Qt::AlignRight);
    QLineEdit * ty=new QLineEdit(controls);
    ty->setValidator(new QDoubleValidator(ty));
    
    (new QLabel("Translate <b>Z</b>:     ",controls))->setAlignment(Qt::AlignRight);
    QLineEdit * tz=new QLineEdit(controls);
    tz->setValidator(new QDoubleValidator(tz)); 

    (new QLabel("Scale <b>X</b>:     ",controls))->setAlignment(Qt::AlignRight);
    QLineEdit * sx=new QLineEdit(controls);
    sx->setValidator(new QDoubleValidator(sx));
    
    (new QLabel("Scale <b>Y</b>:     ",controls))->setAlignment(Qt::AlignRight);
    QLineEdit * sy=new QLineEdit(controls);
    sy->setValidator(new QDoubleValidator(sy));
    
    (new QLabel("Scale <b>Z</b>:     ",controls))->setAlignment(Qt::AlignRight);
    QLineEdit * sz=new QLineEdit(controls);
    sz->setValidator(new QDoubleValidator(sz)); 

	new QLabel("",controls);
	new QLabel("",controls);
    QPushButton *go = new QPushButton("Done",controls);
    connect(go,SIGNAL(clicked()),this,SLOT(accept()));
}

