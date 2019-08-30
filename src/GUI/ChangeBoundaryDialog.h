#ifndef CHANGE_BOUNDARY_DIALOG_H_
#define CHANGE_BOUNDARY_DIALOG_H_

#include <QtGui>
#include <QDialog>

class BoundingBoxWidget;
class BoundingSphereWidget;
class MainWindow;

////////////////////////////////////////////////////////////////////////////////
/// \brief A dialog for changing the boundary type and properties.
////////////////////////////////////////////////////////////////////////////////
class ChangeBoundaryDialog : public QDialog {

  Q_OBJECT

  public:

    ChangeBoundaryDialog(MainWindow* _mainWindow);

  public slots:

    void SetBoundary();           ///< Accept input and modify the boundary.
    void ChangeToSphereDialog();  ///< Load the bounding sphere widget.
    void ChangeToBoxDialog();     ///< Load the bounding box widget.

  private:

    void ShowCurrentValues();     ///< Load current values into the edit boxes.

    bool m_isBox;
    BoundingBoxWidget*    m_boxWidget;    ///< Helper for modifying boxes.
    BoundingSphereWidget* m_sphereWidget; ///< Helper for modifying spheres.
};

#endif
