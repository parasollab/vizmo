#ifndef _VIZMO2_OBJ_PROPERTY_H_
#define _VIZMO2_OBJ_PROPERTY_H_

#include <qdialog.h>
void InvokeObjPropertyDialog(QWidget *parent);

class QLineEdit;
class BasicPropertyDialog: public QDialog
{
    Q_OBJECT
        
public:

    BasicPropertyDialog(QWidget *parent, const char *name="", WFlags f=0);
};

#endif //_VIZMO2_OBJ_PROPERTY_H_

