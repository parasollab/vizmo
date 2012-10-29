/*#ifndef VIZMO2_EDITOR
#define VIZMO2_EDITOR

///////////////////////////////////////////////////////////////////////////////
// QT Headhers
#include <q3mainwindow.h>

#include <qimage.h>
#include <qpixmap.h>
#include <q3toolbar.h>
#include <qtoolbutton.h>
#include <q3popupmenu.h>
#include <qmenubar.h>
#include <q3textedit.h>
#include <qfile.h>
#include <q3filedialog.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <qapplication.h>
#include <q3accel.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <q3paintdevicemetrics.h>
#include <q3whatsthis.h>
#include <qaction.h>
#include <q3simplerichtext.h>
//Added by qt3to4:
#include <QCloseEvent>

class Q3TextEdit;


class vizmoEditor : public Q3MainWindow
{
    Q_OBJECT
        
public:

    vizmoEditor(QWidget *parent, const char *name);
    ~vizmoEditor(){};

    Q3TextEdit *e;


protected:
    void closeEvent( QCloseEvent* );

private slots:
    void newDoc();
    void choose();
    void load( const QString &fileName );
    void save();
    void saveAs();
    void print();
    void execFile();

private:
    QPrinter *printer;
    QString filename;
};
    


#endif//VIZMO2_EDITOR
*/ 
