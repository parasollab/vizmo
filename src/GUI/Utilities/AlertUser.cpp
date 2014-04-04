#include "AlertUser.h"

#include <QtGui>

void
AlertUser(const string& _s) {
  QMessageBox m;
  m.setText(_s.c_str());
  m.exec();
}
