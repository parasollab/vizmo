/*  This class sets up the tool tab and wraps up
 *  tab enable/disable functionality. It depends
 *  on the MainMenu to set up the option classes
 *  prior to its construction.
 */

#ifndef TOOLTABWIDGET_H_
#define TOOLTABWIDGET_H_

#include <QtGui>
#include <QTabWidget>

class MainWindow;

using namespace std;

class ToolTabWidget : public QTabWidget {

  Q_OBJECT

  public:
    ToolTabWidget(MainWindow* _mainWindow = 0);
    friend class ToolTabOptions;

  private slots:
    void ToggleTab();

  private:
    map<string, pair<string, QWidget*> > m_tabs;

};

#endif
