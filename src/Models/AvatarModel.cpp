#include "AvatarModel.h"

#include "Models/ActiveMultiBodyModel.h"
#include "Models/EnvModel.h"
#include "Models/Vizmo.h"

#include "GUI/GLWidget.h"
#include "GUI/MainWindow.h"

#include "Utilities/GLUtils.h"

#include <QCursor>
#include <QWidget>


AvatarModel::
AvatarModel(InputType _t, const CfgModel& _initialCfg) : CfgModel(_initialCfg),
    m_input(_t), m_tracking(false) {
  this->m_color = Color4(0., 1., 0., .8);
}


void
AvatarModel::
DrawRender() {
  if(m_tracking) {
    shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobotModel(m_robotIndex);

    if(this->m_isValid) {
      glColor4fv(m_color);
      robot->SetColor(m_color);
    }
    else {
      Color4 ic(1.0-m_color[0], 1.0-m_color[1], 1.0-m_color[2], 0.0);
      glColor4fv(ic);
      robot->SetColor(ic); //Invert colors
    }

    robot->SetRenderMode(WIRE_MODE);
    robot->ConfigureRender(m_dofs);
    robot->DrawRender();
  }
}


void
AvatarModel::
Print(ostream& _os) const {
  _os << "RobotAvatar data: input type = " << m_input << endl;
  CfgModel::Print(_os);
}


bool
AvatarModel::
PassiveMouseMotion(QMouseEvent* _e, Camera* _c) {
  if(m_input == Mouse && m_tracking) {
    Point3d worldPos = GLUtils::ProjectToWorld(_e->pos().x(),
        GLUtils::windowHeight - _e->pos().y());
    UpdatePosition(worldPos);
    return true;
  }
  return false;
}


void
AvatarModel::
UpdatePosition(Point3d _p) {
  vector<double> data = this->GetDataCfg();
  if(GetVizmo().GetEnv()->IsPlanar())
    copy(_p.begin(), _p.end() - 1, data.begin());
  else
    copy(_p.begin(), _p.end(), data.begin());
  this->SetCfg(data);
}
