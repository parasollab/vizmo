#ifndef RV_OPTIONS_H_
#define RV_OPTIONS_H_

#include "OptionsBase.h"


////////////////////////////////////////////////////////////////////////////////
/// \brief Provides the "RV" submenu and associated action buttons.
////////////////////////////////////////////////////////////////////////////////
class RVOptions : public OptionsBase {

  Q_OBJECT

  public:

    RVOptions(QWidget* _parent);

  private slots:

    //query functions
    void ShowHideRV();   ///< Toggle query display on or off.
    void NextRV();   ///< Display next RV
    void PrevRV();   ///< Display previous RV

  private:

    //gui management
    void CreateActions();
    void SetHelpTips();
    void Reset();
};

#endif
