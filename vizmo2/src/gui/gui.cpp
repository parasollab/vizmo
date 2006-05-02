#include "gui.h"
#include "main_win.h"

///////////////////////////////////////////////////////////////////////////////
// Qt Headers
#include <qapplication.h>

VizmoMainWin * CreatWindow(int argc, char ** argv)
{
    VizmoMainWin * win=new VizmoMainWin();
    //check file name
    vector<string> args; args.reserve(argc-1);
    for( int i=1;i<argc;i++ )
        args.push_back(argv[i]);
    if( win->Init()==false ){ delete win; return NULL; }
    win->SetArgs(args);
    return win;
}

///Initialize gui objects, such as creating windows, toolbars, ...etc.
bool VizGUI_Init(int argc, char ** argv)
{
    QApplication::setColorSpec( QApplication::CustomColor );
    QApplication app(argc, argv);
    
    VizmoMainWin * win=CreatWindow(argc, argv);
    if( win==NULL ) return false;
    app.setMainWidget(win);
    win->show();
    app.exec();
    return true;
}
