#ifndef VIZMO2_EDITOR
#define VIZMO2_EDITOR

///////////////////////////////////////////////////////////////////////////////
// QT Headhers
#include <qmainwindow.h>

#include <qimage.h>
#include <qpixmap.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qtextedit.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <qapplication.h>
#include <qaccel.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qsimplerichtext.h>

class QTextEdit;


class vizmoEditor : public QMainWindow
{
    Q_OBJECT
        
public:

    vizmoEditor(QWidget *parent, const char *name);
    ~vizmoEditor(){};

    QTextEdit *e;


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

