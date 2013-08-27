#include <iostream>
using namespace std;

#include <glut.h>

#include <QApplication>

#include "GUI/MainWindow.h"

int
main(int _argc, char** _argv) {
  if(_argc > 1){
    cerr << "Error: vizmo++ doesn't take arguments." << endl;
    return 1;
  }
  //initialize gui, qapp, and main window
  glutInit(&_argc, _argv);

  QApplication::setColorSpec( QApplication::CustomColor );
  QApplication app(_argc, _argv);

  MainWindow window;
  if(!window.Init()){
    cerr << "Error: vizmo++ could not intialize main window." << endl;
    return 1;
  }

  //execute main window and application
  window.show();
  app.exec();
  return 0;
}
