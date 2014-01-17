/****************************************************************
 * Class for the "What's This?" utility.
 * A QWhatsThis is added as an action to the single-button
 * "menubar" in this class.
 * **************************************************************/

#ifndef HELPOPTIONS_H_
#define HELPOPTIONS_H_

#include "OptionsBase.h"

class HelpOptions : public OptionsBase {

    Q_OBJECT

    public:
      HelpOptions(QWidget* _parent = 0, MainWindow* _mainWindow = 0);

    private:
      void CreateActions();
      void SetUpToolbar();
      void Reset();           //Just a placeholder in this case
      void SetHelpTips();     //Of course, this is NOT implemented here

    private slots:
      void ShowAboutBox(); //Pop up the "About" window
};

#endif
