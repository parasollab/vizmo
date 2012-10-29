/*
#include "vizmo2.h"

///////////////////////////////////////////////////////////////////////////////
// QT Headhers
#include <q3mainwindow.h>
#include <qdialog.h>
//Added by qt3to4:
#include <QLabel>



class QLabel;

class AddObjDial : public QDialog
{
    Q_OBJECT
        
public:

    AddObjDial(QWidget* parent, const char* name, Qt::WFlags f=0);
    ~AddObjDial(){};

    bool create();

    void createMBInfo();

    void createMBModel(CMultiBodyInfo &mbi, CEnvModel* env);
    void commands();
    string getSubstr(string s, char c, int i);

private slots:

  void updateData();
    
private:

 CMultiBodyInfo *mbiTmp, *mbiNew;

 QString fn;
 bool r;
 int position; //used in getSubstr()
 string m_fileName;

};
*/ 
