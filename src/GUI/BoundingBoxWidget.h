#ifndef BOUNDING_BOX_WIDGET_H_
#define BOUNDING_BOX_WIDGET_H_

#include <QtGui>

////////////////////////////////////////////////////////////////////////////////
/// \brief Helper widget for modifying a bounding box within a
///        ChangeBoundaryDialog.
////////////////////////////////////////////////////////////////////////////////
class BoundingBoxWidget : public QWidget {

  Q_OBJECT

  public:

    BoundingBoxWidget(QWidget* _parent);

  public slots:

    void Toggle2D();          ///< Switch between 2D and 3D bounding box.
    void SetBoundary();       ///< Accept input and modify the boundary.

  private:

    void ShowCurrentValues(); ///< Load current values into the edit boxes.

    bool m_is2D;              ///< Indicates whether the box is 2D or 3D.
    QCheckBox* m_checkIs2D;   ///< Sets m_is2D.
    QLineEdit* m_lineXMin;    ///< Edit box for the box's X lower bound.
    QLineEdit* m_lineXMax;    ///< Edit box for the box's X upper bound.
    QLineEdit* m_lineYMin;    ///< Edit box for the box's Y lower bound.
    QLineEdit* m_lineYMax;    ///< Edit box for the box's Y upper bound.
    QLineEdit* m_lineZMin;    ///< Edit box for the box's Z lower bound.
    QLineEdit* m_lineZMax;    ///< Edit box for the box's Z upper bound.
};

#endif
