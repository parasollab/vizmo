#ifndef BOUNDING_SPHERE_WIDGET_H_
#define BOUNDING_SPHERE_WIDGET_H_

#include <QtGui>

////////////////////////////////////////////////////////////////////////////////
/// \brief Helper widget for modifying a bounding sphere within a
///        ChangeBoundaryDialog.
////////////////////////////////////////////////////////////////////////////////
class BoundingSphereWidget : public QWidget {

  Q_OBJECT

  public:

    BoundingSphereWidget(QWidget* _parent);

  public slots:

    void SetBoundary();       ///< Accept input and modify the boundary.

  private:

    void ShowCurrentValues(); ///< Load current values into the edit boxes.

    QLineEdit* m_lineX;       ///< Edit box for the sphere's X coordinate.
    QLineEdit* m_lineY;       ///< Edit box for the sphere's Y coordinate.
    QLineEdit* m_lineZ;       ///< Edit box for the sphere's Z coordinate.
    QLineEdit* m_lineR;       ///< Edit box for the sphere's radius.
};

#endif
