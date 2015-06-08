#ifndef GL_UTILS_H_
#define GL_UTILS_H_

#include "Vector.h"
using namespace mathtool;


////////////////////////////////////////////////////////////////////////////////
/// \brief Provides utility functions for working with the OpenGL scene.
////////////////////////////////////////////////////////////////////////////////
namespace GLUtils {

  extern int windowWidth;   ///< OpenGL scene window width.
  extern int windowHeight;  ///< OpenGL scene window height.


  // Projections between window and world coordinates
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Project a single world point to the window, returning its
  ///        representation in window coordinates.
  /// \param[in] _pt The world point to project.
  /// \return        The same point in window coordinates.
  Point3d ProjectToWindow(const Point3d& _pt);
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Project a set of world points to the window. Modifies the points in
  ///        place.
  /// \param[in/out] _pts  The set of world points to convert to window
  ///                      coordinates.
  /// \param[in]     _size The number of points to convert.
  void ProjectToWindow(Point3d* _pts, size_t _size);
  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Project a point in window coordinates (<i>x</i>, <i>y</i>) to
  ///          world coordinates.
  /// \details The projected point is defined as the intersection of a ray shot
  ///          from (<i>x</i>, <i>y</i>) along the screen-in direction and the
  ///          plane defined by the point \ref _ref and normal \ref _n.
  /// \param[in] _x   The input window-x coordinate.
  /// \param[in] _y   The input window-y coordinate.
  /// \param[in] _ref The reference point in world coordinates. Default is
  ///                 the origin.
  /// \param[in] _n   The reference normal in world coordinates. Default is the
  ///                 screen-out direction (0, 0, 1).
  /// \return         The projected point in world coordinates.
  Point3d ProjectToWorld(double _x, double _y, const Point3d& _ref = Point3d(),
      const Vector3d& _n = Vector3d(0, 0, 1.));

  // Drawing helpers
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Draw a circle in the OpenGL scene.
  /// \param[in] _r    The radius of the circle.
  /// \param[in] _fill A bool indicating a filled circle (true) or outline
  ///                  only (false).
  void DrawCircle(double _r, bool _fill);
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Draw an arc in the OpenGL scene.
  /// \param[in] _r  The arc radius.
  /// \param[in] _s  The starting angle in radians.
  /// \param[in] _e  The ending angle in radians.
  /// \param[in] _v1 The drawing plane x-direction.
  /// \param[in] _v2 The drawing plane y-direction.
  void DrawArc(double _r, double _s, double _e, const Vector3d& _v1,
      const Vector3d& _v2);

}
#endif
