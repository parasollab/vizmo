#ifdef USE_SPACEMOUSE

#ifndef CURSOR_3D_H_
#define CURSOR_3D_H_

#include "Models/CrosshairModel.h"
#include "Models/RegionModel.h"

#include "Utilities/Color.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief A 3d cursor for vizmo.
///
/// The cursor is represented as a small sphere with a crosshair to aid in depth
/// perception. The cursor is bounded within the environment, and can be toggled
/// on/off through GLWidgetOptions.
///
/// \warning This tool requires a space mouse at present, but can be easily
///          modified to use any input device that can deliver 3 DOFs to the
///          Translate method.
////////////////////////////////////////////////////////////////////////////////
class Cursor3d : public QObject {

  Q_OBJECT

  // Cursor data
  Point3d m_pos;                  ///< The cursor position.
  CrosshairModel m_crosshair;     ///< The crosshair model.
  double m_radius;                ///< The sphere radius.
  Color4 m_color{.4, .6, .9, .3}; ///< The sphere color.
  bool m_enable{false};           ///< Marks whether display is enabled.

  // Region data
  shared_ptr<RegionModel> m_currentRegion{nullptr};
  bool m_grabbed{false};///< Indicates whether the cursor is controlling a region.
  bool m_drawing{false};///< Indicates whether the cursor is drawing a new region.

  public:

    Cursor3d(const Point3d& _p = Point3d());
    ~Cursor3d() {Reset();}

    // Control interface
    void Reset();               ///< Reset state.
    void Draw();                ///< Render the cursor.
    void Toggle();              ///< Enable/disable display.
    const Point3d& GetPos() const {return m_pos;} ///< Get the position.

  public slots:

    // Input event handler
    void Translate(const Vector3d&);          ///< Translate the cursor position.
    void ButtonHandler(int, bool);    ///< Grab/release the current region.

  private:

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Push a point into the environment boundary.
    void PushIntoBoundary(Point3d&) const;
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Check if the cursor is inside an attract or non-commit region.
    void CheckRegions();
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the current region as selected.
    void SelectCurrentRegion();
};

#endif
#endif
