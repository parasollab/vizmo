#ifndef CHANGEBOUNDARYDIALOG_H_
#define CHANGEBOUNDARYDIALOG_H_

#include <QtGui>

class BoundingBoxWidget;
class BoundingSphereWidget;

class ChangeBoundaryDialog : public QDialog {

  Q_OBJECT

  public:
    ChangeBoundaryDialog(QWidget* _parent);

  public slots:
    void SetBoundary();
    void ChangeToSphereDialog();
    void ChangeToBoxDialog();

  private:
    void ShowCurrentValues();

    bool m_isBox;
    QRadioButton* m_boxButton;
    QRadioButton* m_sphereButton;
    BoundingBoxWidget* m_boxWidget;
    BoundingSphereWidget* m_sphereWidget;
};

#endif

