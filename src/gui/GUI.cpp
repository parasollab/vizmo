#include "GUI.h"
#include "MainWin.h"
#include <glut.h>
#include <iostream>
///////////////////////////////////////////////////////////////////////////////
// Qt Headers
#include <qapplication.h>

VizmoMainWin* CreatWindow(int _argc, char** _argv) {
   VizmoMainWin* win=new VizmoMainWin();
   //check file name
   vector<string> args; args.reserve(_argc-1);
   for( int i=1;i<_argc;i++ )
      args.push_back(_argv[i]);
   if( win->Init()==false ){ delete win; return NULL; }
   win->SetArgs(args);
   return win;
}

///Initialize gui objects, such as creating windows, toolbars, ...etc.
bool VizGUI_Init(int _argc, char** _argv)
{
   glutInit(&_argc,_argv);
   QApplication::setColorSpec( QApplication::CustomColor );
   QApplication app(_argc, _argv);

   VizmoMainWin* win=CreatWindow(_argc, _argv);
   if( win==NULL ) return false;
   win->show();
   app.exec();
   return true;
}
