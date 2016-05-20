#include <iostream>
using namespace std;

#include <glut.h>

#include <QApplication>

#include "GUI/MainWindow.h"
#include "Models/Vizmo.h"
#include "Vector.h"

int
main(int _argc, char** _argv) {
  //parse command line args
  long seed = 0;
  vector<string> filename;
  char arg;
  bool noXML = false;
  opterr = 0;
  while((arg = getopt(_argc, _argv, "x:f:s:")) != -1) {
    if(arg == 'x') {
      if(noXML) {
        cerr << "XML cannot be passed as argument with other files" << endl;
        exit(1);
      }
      filename.push_back(optarg);
      break;
    }
    else {
      noXML = true;
      switch(arg) {
        case 'f':
          filename.push_back(optarg);
          break;
        case 's':
          seed = atol(optarg);
          break;
        default:
          cerr << "\nUnknown commandline argument." << endl;
          exit(1);
      }
    }
  }

  GetVizmo().SetSeed(seed);

  //initialize gui, qapp, and main window
  glutInit(&_argc, _argv);

  QApplication::setColorSpec(QApplication::CustomColor);
  QApplication app(_argc, _argv);
  qRegisterMetaType<mathtool::Vector3d>("Vector3d");

  MainWindow*& window = GetMainWindow();
  window = new MainWindow(filename);
  if(!window->Init()) {
    cerr << "Error: vizmo++ could not intialize main window." << endl;
    return 1;
  }

  //execute main window and application
  window->show();
  app.exec();
  return 0;
}
