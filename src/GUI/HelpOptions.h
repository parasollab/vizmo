#ifndef HELP_OPTIONS_H_
#define HELP_OPTIONS_H_

#include "OptionsBase.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief Provides the "What's This?" utility.
/// \details Help tips from other options classes are used in the WhatsThis
/// defined here.
////////////////////////////////////////////////////////////////////////////////
class HelpOptions : public OptionsBase {

    Q_OBJECT

    public:

      HelpOptions(QWidget* _parent);

    private:

      //gui management
      void CreateActions();
      void SetHelpTips() { }
      void SetUpToolbar();
      void Reset() { }

    private slots:

      //help functions
      void ShowAboutBox();    ///< Pop up the "About" window.
};

#endif
