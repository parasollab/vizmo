#ifndef CHANGEBOUNDARYDIALOG_H_
#define CHANGEBOUNDARYDIALOG_H_

#include <QDialog>

using namespace std;

class QPushButton;
class QRadioButton;
class QLineEdit;
class QLabel;
class QValidator;
class QCheckBox;

class ChangeBoundaryDialog : public QDialog {

  Q_OBJECT

  public:
    ChangeBoundaryDialog(QWidget* _parent);

  public slots:
    void SetBoundary();
    void ChangeToSphereDialog();
    void ChangeToBoxDialog();
    void ChangeToSurface();

  private:
    //Functions
    void SetUpLayout();
    void ShowCurrentValues();
    void DisplaySphereItems(bool _i);
    void DisplayBoxItems(bool _i);
    bool m_isBox;
    bool m_isSurface;
    //Qt Variables
    QPushButton* m_cancelButton;
    QPushButton* m_loadButton;
    QRadioButton* m_boxButton;
    QRadioButton* m_sphereButton;
    QLineEdit* m_lineBoxX1;
    QLineEdit* m_lineBoxX2;
    QLineEdit* m_lineBoxY1;
    QLineEdit* m_lineBoxY2;
    QLineEdit* m_lineBoxZ1;
    QLineEdit* m_lineBoxZ2;
    QLineEdit* m_lineSphereX;
    QLineEdit* m_lineSphereY;
    QLineEdit* m_lineSphereZ;
    QLineEdit* m_lineSphereR;
    QLabel* m_labelBoxX;
    QLabel* m_labelBoxY;
    QLabel* m_labelBoxZ;
    QLabel* m_labelBoxSep1;
    QLabel* m_labelBoxSep2;
    QLabel* m_labelBoxSep3;
    QLabel* m_labelSphereX;
    QLabel* m_labelSphereY;
    QLabel* m_labelSphereZ;
    QLabel* m_labelSphereR;
    QCheckBox* m_checkIsSurface;
    QValidator* m_validator;
};

#endif

