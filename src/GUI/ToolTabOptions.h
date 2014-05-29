#ifndef TOOLTABOPTIONS_H_
#define TOOLTABOPTIONS_H_

#include <QtGui>
#include "OptionsBase.h"

class MainWindow;
class ToolTabWidget;

class ToolTabOptions : public OptionsBase {

  Q_OBJECT

  public:
    ToolTabOptions(QWidget* _parent, MainWindow* _mainWindow = 0);
    void Init() {CreateActions();}
    void Reset() {}

  private:
    void CreateActions();
    void SetUpToolbar() {}
    void SetHelpTips();
};

#endif
