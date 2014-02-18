#include <iostream>
using namespace std;

#include <glut.h>

#include <QApplication>

#include "GUI/MainWindow.h"
#include "Models/Vizmo.h"

int
main(int _argc, char** _argv) {
  //parse command line args
  long seed = 0;
  char arg;
  opterr = 0;
  while((arg = getopt(_argc, _argv, "s:")) != -1) {
    switch(arg) {
      case 's':
        seed = atol(optarg);
        break;
      default:
        cerr << "\nUnknown commandline argument." << endl;
        exit(1);
    }
  }

  GetVizmo().SetSeed(seed);

  //initialize gui, qapp, and main window
  glutInit(&_argc, _argv);

  QApplication::setColorSpec(QApplication::CustomColor);
  QApplication app(_argc, _argv);

  MainWindow window;
  if(!window.Init()) {
    cerr << "Error: vizmo++ could not intialize main window." << endl;
    return 1;
  }

  //execute main window and application
  window.show();
  app.exec();
  return 0;
}
