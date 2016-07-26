#ifdef USE_SPACEMOUSE

#include "Cursor3d.h"

#include "GUI/GLWidget.h"
#include "GUI/MainMenu.h"
#include "GUI/MainWindow.h"
#include "GUI/ModelSelectionWidget.h"
#include "GUI/PlanningOptions.h"

#include "Models/EnvModel.h"
#include "Models/RegionSphereModel.h"
#include "Models/Vizmo.h"

#include "Utilities/Camera.h"
#include "Utilities/GLUtils.h"

#include "SpaceMouse/SpaceMouseManager.h"

Cursor3d::
Cursor3d(const Point3d& _p) : QObject(GetMainWindow()->GetGLWidget()), m_pos(_p),
    m_crosshair(&m_pos) {
  m_crosshair.Enable();
}


void
Cursor3d::
Draw() {
  if(m_enable) {
    // Draw crosshair
    m_crosshair.DrawRender();

    // Draw sphere
    glPushMatrix();
    glTranslatef(m_pos[0], m_pos[1], m_pos[2]);
    glColor4fv(m_currentRegion ? Color4(1, 1, 1, 1) - m_color : m_color);
    GLUtils::DrawSphere(m_radius);
    glPopMatrix();
  }
}


void
Cursor3d::
Toggle() {
  // Check that the space mouse is active.
  SpaceMouseManager* smm = GetVizmo().GetSpaceMouseManager();
  if(!smm->IsEnabled())
    return;

  // Toggle the display and space mouse feed.
  if(smm->IsEnabledCursor()) {
    m_enable = false;
    smm->EnableCamera();
  }
  else {
    m_enable = true;
    smm->EnableCursor();
  }
}


void
Cursor3d::
Reset() {
  m_enable = false;
  m_radius = .02 * GetVizmo().GetEnv()->GetRadius();

  // Check that the space mouse is active.
  SpaceMouseManager* smm = GetVizmo().GetSpaceMouseManager();
  if(smm->IsEnabled() && smm->IsEnabledCursor())
    smm->DisableCursor();
}


void
Cursor3d::
Translate(const Vector3d& _v) {
  Camera* camera = GetMainWindow()->GetGLWidget()->GetCurrentCamera();
  Point3d temp = m_pos + camera->ProjectToWorld(_v);
  if(!GetVizmo().GetEnv()->InBoundary(temp))
    PushIntoBoundary(temp);
  swap(m_pos, temp);

  // Drag current region if grabbed
  if(m_grabbed)
    m_currentRegion->ApplyOffset(m_pos - temp);
  // Resize current region if drawing
  else if(m_drawing) {
    double dist = (m_pos - m_currentRegion->GetCenter()).norm();
    static_cast<RegionSphereModel*>(m_currentRegion.get())->SetRadius(dist);
  }
  // If neither grabbed nor drawing, check for next current region
  else
    CheckRegions();
}


void
Cursor3d::
ButtonHandler(int _button, bool _pressed) {
  /// The button number scheme:
  /// \arg \c 0 is the left button, which grabs regions.
  /// \arg \c 1 is the right button, which edits (draws and resizes) regions.
  // Only one event per press/release: handle releases only.
  if(_pressed)
    return;
  switch(_button) {
    case 0: // Grab region button
      if(!m_drawing && m_currentRegion)
        m_grabbed = !m_grabbed;
      break;
    case 1: // Edit region button
      if(m_grabbed)
        break;
      if(m_drawing) { // Stop drawing
        m_drawing = false;
        m_grabbed = true;
      }
      else if(!m_currentRegion) { // Draw new region
        m_drawing = true;
        static_cast<PlanningOptions*>(GetMainWindow()->m_mainMenu->
            m_planningOptions)->StartPreInputTimer();
        m_currentRegion = shared_ptr<RegionModel>(new RegionSphereModel(m_pos, 0,
            false));
        GetVizmo().GetEnv()->AddNonCommitRegion(m_currentRegion);
        GetMainWindow()->GetModelSelectionWidget()->ResetLists();
        SelectCurrentRegion();
      }
      else { // Edit current region
        m_drawing = true;
      }
  }
}


/*----------------------- Private Helpers ------------------------------------*/


void
Cursor3d::
PushIntoBoundary(Point3d& _p) const {
  const vector<pair<double, double>>& ranges = GetVizmo().GetEnv()->
      GetBoundary()->GetRanges();
  for(size_t i = 0; i < 3; ++i) {
    _p[i] = min(ranges[i].second, _p[i]);
    _p[i] = max(ranges[i].first , _p[i]);
  }
}


void
Cursor3d::
CheckRegions() {
  // First check if still inside current region
  if(m_currentRegion && m_currentRegion->InBoundary(m_pos))
    return;
  // If not in current region, mark it for deselection later
  bool deselect = m_currentRegion != nullptr;
  // Check for cursor inside non commit regions
  for(const auto& region : GetVizmo().GetEnv()->GetNonCommitRegions()) {
    if(region->InBoundary(m_pos)) {
      m_currentRegion = region;
      SelectCurrentRegion();
      return;
    }
  }
  // Check for cursor inside attract regions
  for(const auto& region : GetVizmo().GetEnv()->GetAttractRegions()) {
    if(region->InBoundary(m_pos)) {
      m_currentRegion = region;
      SelectCurrentRegion();
      return;
    }
  }
  // If cursor was in a region but isn't any more, deselect current region
  if(deselect) {
    m_currentRegion = nullptr;
    GetVizmo().GetSelectedModels().clear();
    GetMainWindow()->GetModelSelectionWidget()->Select();
    GetMainWindow()->GetGLWidget()->SetCurrentRegion();
  }
}


void
Cursor3d::
SelectCurrentRegion() {
  GetVizmo().GetSelectedModels().clear();
  GetVizmo().GetSelectedModels().push_back(m_currentRegion.get());
  GetMainWindow()->GetModelSelectionWidget()->Select();
  GetMainWindow()->GetGLWidget()->SetCurrentRegion(m_currentRegion);
}

#endif
