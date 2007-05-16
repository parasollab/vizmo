#ifndef VIZMO2_FILE_LIST_DIALOG
#define VIZMO2_FILE_LIST_DIALOG

///////////////////////////////////////////////////////////////////////////////
// QT Headhers
#include <q3mainwindow.h>
#include <qnamespace.h>
#include <qdialog.h>
#include <q3toolbar.h>
//Added by qt3to4:
#include <QLabel>

class QLabel;

class FileListDialog : public QDialog
{
    Q_OBJECT
        
public:

    FileListDialog(QWidget *parent, const char *name, Qt::WFlags f=0);
    
private slots:
    
	void changeMap();
	void changeEnv();
	void changePath();
	void changeQry();
    
private:

    void storeAttributes();

    QLabel * Map_label;
	QLabel * Env_label;
	QLabel * Path_label;
	QLabel * Query_label;
};

#endif//VIZMO2_FILE_LIST_DIALOG

