#ifndef CHANGE_BOUNDARY_DIALOG_H_
#define CHANGE_BOUNDARY_DIALOG_H_

#include <QtGui>

class BoundingBoxWidget;
class BoundingSphereWidget;
class MainWindow;

class ChangeBoundaryDialog : public QDialog {

  Q_OBJECT

  public:
    ChangeBoundaryDialog(MainWindow* _mainWindow);

  public slots:
    void SetBoundary();
    void ChangeToSphereDialog();
    void ChangeToBoxDialog();

  private:
    void ShowCurrentValues();

    bool m_isBox;
    BoundingBoxWidget* m_boxWidget;
    BoundingSphereWidget* m_sphereWidget;
    MainWindow* m_mainWindow;
};

#endif
