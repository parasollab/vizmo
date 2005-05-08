#include "vizmoEditor.h"

#include <iostream>
using namespace std;

#include "icon/filing.xpm"
#include "icon/floppy2_green.xpm"
#include "icon/printer2.xpm"
#include "icon/computer_clock.xpm"

vizmoEditor::vizmoEditor(QWidget * parent, const char * name)
:QMainWindow( parent, name)

{
  QAction * fileNewAction;
  QAction * fileOpenAction;
  QAction * fileSaveAction, * fileSaveAsAction, * filePrintAction;
  QAction * fileQuitAction, * fileExecAction;

  fileNewAction = new QAction( "New", "&New", CTRL+Key_N, this );
  connect( fileNewAction, SIGNAL( activated() ) , this,
	   SLOT( newDoc() ) );

  fileOpenAction = new QAction( "Open", QPixmap( icon_filing ), "&Open...", 
				CTRL+Key_O, this);
  connect( fileOpenAction, SIGNAL( activated() ) , this, SLOT( choose() ) );

  const char * fileOpenText = "<p><img source=\"fileopen\"> "
    "Click this button to open a <em>new file</em>. <br>"
    "You can also select the <b>Open</b> command "
    "from the <b>File</b> menu.</p>";
  QMimeSourceFactory::defaultFactory()->setPixmap( "fileopen",
		       fileOpenAction->iconSet().pixmap() );
  fileOpenAction->setWhatsThis( fileOpenText );

  fileSaveAction = new QAction( "Save", QPixmap( icon_floppy ), "&Save",
                                CTRL+Key_S, this);
  connect( fileSaveAction, SIGNAL( activated() ) , this, SLOT( save() ) );

  QMimeSourceFactory::defaultFactory()->setPixmap( "save",
		       fileSaveAction->iconSet().pixmap() );

  const char * fileSaveText = "<p><img source=\"save\"> "
                     "Click this button to save the file you "
                     "are editing. You will be prompted for a file name.\n"
                     "You can also select the <b>Save</b> command "
                     "from the <b>File</b> menu.</p>";
  fileSaveAction->setWhatsThis( fileSaveText );

  fileSaveAsAction = new QAction( "Save File As", "Save &As...", 0,  this,
                                    "save as" );
  connect( fileSaveAsAction, SIGNAL( activated() ) , this,
	   SLOT( saveAs() ) );
  fileSaveAsAction->setWhatsThis( fileSaveText );

  filePrintAction = new QAction( "Print File", QPixmap( icon_printer ),
                                   "&Print...", CTRL+Key_P, this);
  connect( filePrintAction, SIGNAL( activated() ) , this,
             SLOT( print() ) );

  const char * filePrintText = "Click this button to print the file you "
                     "are editing.\n You can also select the Print "
                     "command from the File menu.";
  filePrintAction->setWhatsThis( filePrintText );

  fileExecAction = new QAction( "Execute", QPixmap( icon_computer_clock ), "&Execute",
                                CTRL+Key_E, this);
  connect( fileExecAction, SIGNAL( activated() ) , this, SLOT( execFile() ) );

  const char * fileExecText = "<p>Click this button to execute OBPRM \n";
                     
  fileExecAction->setWhatsThis( fileExecText );


  fileQuitAction = new QAction( "Quit", "&Quit", CTRL+Key_Q, this);
  connect( fileQuitAction, SIGNAL( activated() ) , this,
             SLOT( close() ) );

  // populate a tool bar with some actions

  QToolBar * fileTools = new QToolBar( this, "file operations" );
  fileTools->setLabel( "File Operations" );
  fileOpenAction->addTo( fileTools );
  fileSaveAction->addTo( fileTools );
  filePrintAction->addTo( fileTools );
  fileExecAction->addTo( fileTools );
  (void)QWhatsThis::whatsThisButton( fileTools );


    // populate menu

    QPopupMenu * file = new QPopupMenu( this );
    menuBar()->insertItem( "&File", file );
    fileNewAction->addTo( file );
    fileOpenAction->addTo( file );
    fileSaveAction->addTo( file );
    fileSaveAsAction->addTo( file );
    file->insertSeparator();
    filePrintAction->addTo( file );
    file->insertSeparator();
    fileQuitAction->addTo( file );


    menuBar()->insertSeparator();

    // add a help menu

    QPopupMenu * help = new QPopupMenu( this );
    menuBar()->insertItem( "&Help", help );
    help->insertSeparator();
    help->insertItem( "What's &This", this, SLOT(whatsThis()),
                      SHIFT+Key_F1 );


    // create and define the central widget

    e = new QTextEdit( this, "editor" );
    e->setFocus();
    setCentralWidget( e );
    statusBar()->message( "Ready", 2000 );
    
    resize( 450, 600 );

}

void vizmoEditor::newDoc()
{
    vizmoEditor *ed = new vizmoEditor(this, "Vizmo++ Editor");
    ed->show();
}

void vizmoEditor::choose()
{
    QString fn = QFileDialog::getOpenFileName( QString::null, QString::null,
                                               this);
    if ( !fn.isEmpty() )
        load( fn );
    else
        statusBar()->message( "Loading aborted", 2000 );
}


void vizmoEditor::load( const QString &fileName )
{
    QFile f( fileName );
    if ( !f.open( IO_ReadOnly ) )
        return;

    QTextStream ts( &f );
    e->setText( ts.read() );
    e->setModified( FALSE );
    setCaption( fileName );
    statusBar()->message( "Loaded document " + fileName, 2000 );
}


void vizmoEditor::save()
{
    if ( filename.isEmpty() ) {
        saveAs();
        return;
    }

    QString text = e->text();
    QFile f( filename );
    if ( !f.open( IO_WriteOnly ) ) {
        statusBar()->message( QString("Could not write to %1").arg(filename),
                              2000 );
        return;
    }

    QTextStream t( &f );
    t << text;
    f.close();

    e->setModified( FALSE );

    setCaption( filename );

    statusBar()->message( QString( "File %1 saved" ).arg( filename ), 2000 );

    int i = system("chmod u+x "+filename);

}


void vizmoEditor::saveAs()
{
    QString fn = QFileDialog::getSaveFileName( QString::null, QString::null,
                                               this );
    if ( !fn.isEmpty() ) {
        filename = fn;
        save();
    } else {
        statusBar()->message( "Saving aborted", 2000 );
    }
}


void vizmoEditor::print()
{
  printer = new QPrinter(QPrinter::PrinterResolution);

    printer->setFullPage( TRUE );
    if ( printer->setup(this) ) {               // printer dialog
        statusBar()->message( "Printing..." );
        QPainter p;
        if( !p.begin( printer ) ) {               // paint on printer
            statusBar()->message( "Printing aborted", 2000 );
            return;
        }

        QPaintDeviceMetrics metrics( p.device() );
        int dpiy = metrics.logicalDpiY();
        int margin = (int) ( (2/2.54)*dpiy ); // 2 cm margins
        QRect view( margin, margin, metrics.width() - 2*margin, metrics.height() - 2*margin );
        QSimpleRichText richText( QStyleSheet::convertFromPlainText(e->text()),
                                  QFont(),
                                  e->context(),
                                  e->styleSheet(),
                                  e->mimeSourceFactory(),
                                  view.height() );
        richText.setWidth( &p, view.width() );
        int page = 1;
        do {
            richText.draw( &p, margin, margin, view, colorGroup() );
            view.moveBy( 0, view.height() );
            p.translate( 0 , -view.height() );
            p.drawText( view.right() - p.fontMetrics().width( QString::number( page ) ),
                        view.bottom() + p.fontMetrics().ascent() + 5, QString::number( page ) );
            if ( view.top() - margin >= richText.height() )
                break;
            printer->newPage();
            page++;
        } while (TRUE);

        statusBar()->message( "Printing completed", 2000 );
    } else {
        statusBar()->message( "Printing aborted", 2000 );
    }
}

void vizmoEditor::execFile(){

  QFileInfo fi(filename);

  int i;

  //i = system (fi.dirPath (TRUE)+"scripTest");
  i = system (filename);
  if (i==-1) puts ("Error executing FILE");
  else puts ("Commands successfully executed");


}


void vizmoEditor::closeEvent( QCloseEvent* ce )
{
    if ( !e->isModified() ) {
        ce->accept();
        return;
    }

    switch( QMessageBox::information( this, "Message",
                                      "The document has been changed since "
                                      "the last save.",
                                      "Save Now", "Cancel", "Leave Anyway",
                                      0, 1 ) ) {
    case 0:
        save();
        ce->accept();
        break;
    case 1:
    default:
        ce->ignore();
        break;
    case 2:
        ce->accept();
        break;
    }
}
