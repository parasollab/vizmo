#ifndef MANAGER_H_
#define MANAGER_H_

#include "Vector.h"
using namespace mathtool;

#ifdef USE_HAPTICS
/*----------------------------- Real Version ---------------------------------*/

#include <vector>
using namespace std;

#include <HDU/hduVector.h>
#include <HL/hl.h>

////////////////////////////////////////////////////////////////////////////////
/// \brief Encapsulates all functions related to the use of a haptics device.
////////////////////////////////////////////////////////////////////////////////
namespace Haptics {

  class Manager;
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Accessor for the singleton phantom manager.
  Manager& GetManager();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Indicate whether the device is available.
  bool UsingPhantom();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Manager for a Phantom haptics device.
  /// \details Provides device management, state access, output control, and
  ///          rendering functions.
  //////////////////////////////////////////////////////////////////////////////
  class Manager {

    public:

      Manager();
      ~Manager();

      // Rendering functions
      void HapticRender();  ///< Render the haptic scene.
      void DrawRender();    ///< Render the haptic cursor in the OpenGL scene.
      //////////////////////////////////////////////////////////////////////////
      /// \brief Update the haptic viewpoint to match the OpenGL scene.
      void UpdateWorkspace();

      // State access functions
      //////////////////////////////////////////////////////////////////////////
      /// \brief Retrieve the input position in device coordinates.
      const hduVector3Dd& GetPos() const {return m_pos;}
      //////////////////////////////////////////////////////////////////////////
      /// \brief Retrieve the input rotation in device coordinates.
      const hduVector3Dd& GetRot() const {return m_rot;}
      //////////////////////////////////////////////////////////////////////////
      /// \brief Retrieve the input velocity in device coordinates.
      const hduVector3Dd& GetVel() const {return m_vel;}
      //////////////////////////////////////////////////////////////////////////
      /// \brief Retrieve the input force in device coordinates.
      const hduVector3Dd& GetForce() const {return m_force;}
      //////////////////////////////////////////////////////////////////////////
      /// \brief Retrieve the haptic cursor's position in world coordinates.
      Point3d GetWorldPos() const
          {return Point3d(m_worldPos[0], m_worldPos[1], m_worldPos[2]);}
      //////////////////////////////////////////////////////////////////////////
      /// \brief Retrieve the haptic cursor's rotation in world coordinates.
      /// \bug The cursor's world rotation is currently not used - support was
      ///      never completed during the initial implementation as only a
      ///      spherical cursor was needed. We should rectify this once we get
      ///      the Phantom back from Lydia's group.
      Point3d GetWorldRot() const
          {return Point3d(m_worldRot[0], m_worldRot[1], m_worldRot[2]);}
      //////////////////////////////////////////////////////////////////////////
      /// \brief Indicate whether the cursor is inside the boundary.
      bool IsInsideBBX();

      // Control functions
      //////////////////////////////////////////////////////////////////////////
      /// \brief Toggle force output on or off.
      void ToggleForceOutput();

    private:

      void Initialize();       ///< Initialize the haptics device.
      void Clean();            ///< Uninitialize the haptics device.

      void GetState();         ///< Get the current device state.

      void BoundaryRender();   ///< Render the boundary in the haptic scene.
      void ObstacleRender();   ///< Render the obstacles in the haptic scene.

      HHD   m_hhd;             ///< Haptic device handle.
      HHLRC m_hhlrc;           ///< HL rendering context.

      hduVector3Dd m_pos,      ///< Input position in device coordinates.
                   m_rot,      ///< Input rotation in device coordinates.
                   m_vel,      ///< Input velocity in device coordinates.
                   m_force;    ///< Input force in device coordinates.
      hduVector3Dd m_worldPos, ///< Haptic cursor position in world coordinates.
                   m_worldRot; ///< Haptic cursor rotation in world coordinates.
      vector<pair<double, double> > m_worldRange; ///< Environment bounds.

      HLuint m_boundaryId,     ///< Internal tracking of haptic boundary.
             m_obstaclesId;    ///< Internal tracking of haptic obstacles.
  };
}

#else
/*---------------------------- Dummy Version ---------------------------------*/


////////////////////////////////////////////////////////////////////////////////
/// \brief Encapsulates all functions related to the use of a haptics device.
///        This is the dummy version, which serves as a placeholder when no
///        device is in use.
////////////////////////////////////////////////////////////////////////////////
namespace Haptics {

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Indicate that no Phantom is in use.
  inline bool UsingPhantom() {return false;}

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Dummy manager for a Phantom haptics device. This is a placeholder
  ///        for when no Phantom is in use.
  //////////////////////////////////////////////////////////////////////////////
  class Manager {

    public:

      // Rendering functions
      void HapticRender() {}      ///< No-op when no device is in use.
      void DrawRender() {}        ///< No-op when no device is in use.
      void UpdateWorkspace() {}   ///< No-op when no device is in use.

      // State access functions
      Point3d GetWorldPos() {return Point3d();} ///< Returns the origin.
      Point3d GetWorldRot() {return Point3d();} ///< Returns the origin.

      // Control functions
      void ToggleForceOutput() {} ///< No-op when no device is in use.
  };
}

#endif
#endif
