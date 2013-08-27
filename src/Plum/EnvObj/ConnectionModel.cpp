#include "ConnectionModel.h"

#include "Utilities/Exceptions.h"
#include "Utilities/IOUtils.h"

size_t ConnectionModel::m_globalCounter = 0;

ConnectionModel::JointType
ConnectionModel::GetJointTypeFromTag(const string& _tag){
  if(_tag == "NONACTUATED")
    return ConnectionModel::NONACTUATED;
  else if(_tag == "REVOLUTE")
    return ConnectionModel::REVOLUTE;
  else if(_tag == "SMAJOINT")
    return ConnectionModel::SMAJOINT;
  else if (_tag == "SPHERICAL")
    return ConnectionModel::SPHERICAL;
  else
    throw ParseException(WHERE, "Failed parsing joint type. Choices are Revolute, SMA, or Spherical.");
}

string
ConnectionModel::GetTagFromJointType(const ConnectionModel::JointType& _joint){
  if(_joint == ConnectionModel::NONACTUATED)
    return "NONACTUATED";
  else if(_joint == ConnectionModel::REVOLUTE)
    return "REVOLUTE";
  else if(_joint == ConnectionModel::SMAJOINT)
    return "SMAJOINT";
  else if (_joint == ConnectionModel::SPHERICAL)
    return "SPHERICAL";
  else
    throw ParseException(WHERE, "Failed parsing joint type. Choices are Revolute, SMA, or Spherical.");
}

void
ConnectionModel::Draw(GLenum _mode) {
  if(m_jointType != SMAJOINT) return;
  if(m_renderMode == INVISIBLE_MODE) return;

  double ct(cos(m_theta/2)), st(sin(m_theta/2));
  double rl = 4.0*m_l/(3.0*PI);
  double fl = (m_theta == 0) ? m_l : 2*m_l/m_theta*st;
  double gl = fl - 2*rl*ct;

  glColor4fv(GetColor());
  glPushMatrix();
  glTransform();
  glScale();

  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(2.0, 2.0);
  glEnable(GL_NORMALIZE);

  GLfloat grid2x2[2][4][3] = {
    {{0, 0, 0},   {0, rl, 0},   {-gl*st, rl+gl*ct, 0},   {-fl*st, fl*ct, 0}},
    {{0, 0, m_w}, {0, rl, m_w}, {-gl*st, rl+gl*ct, m_w}, {-fl*st, fl*ct, m_w}}
  };

  glEnable(GL_MAP2_VERTEX_3);
  glMap2f(GL_MAP2_VERTEX_3,
      0.0, 1.0,  /* U ranges 0..1 */
      3,         /* U stride, 3 floats per coord */
      4,         /* U is 4th order, ie. cubic */
      0.0, 1.0,  /* V ranges 0..1 */
      4 * 3,     /* V stride, row is 4 coords, 3 floats per coord */
      2,         /* V is 2nd order, ie linear */
      *(*grid2x2));  /* control points */

  glMapGrid2f(
      15, 0.0, 1.0,
      1, 0.0, 1.0);

  if(m_renderMode == SOLID_MODE){
    glEvalMesh2(GL_FILL,
        0, 15,
        0, 1);
  }
  else{
    //just do the edges
    glEvalMesh2(GL_LINE,
        0, 0,
        0, 1);
    glEvalMesh2(GL_LINE,
        15, 15,
        0, 1);
    glEvalMesh2(GL_LINE,
        0, 15,
        0, 0);
    glEvalMesh2(GL_LINE,
        0, 15,
        1, 1);
  }

  glDisable(GL_NORMALIZE);
  glDisable(GL_POLYGON_OFFSET_FILL);
  glPopMatrix();
}

void
ConnectionModel::DrawSelect() {
  glLineWidth(2);
  glPushMatrix();
  glTransform();
  
  double ct(cos(m_theta/2)), st(sin(m_theta/2));
  double rl = 4.0*m_l/(3.0*PI);
  double fl = (m_theta == 0) ? m_l : 2*m_l/m_theta*st;
  double gl = fl - 2*rl*ct;

  GLfloat grid2x2[2][4][3] = {
    {{0, 0, 0},   {0, rl, 0},   {-gl*st, rl+gl*ct, 0},   {-fl*st, fl*ct, 0}},
    {{0, 0, m_w}, {0, rl, m_w}, {-gl*st, rl+gl*ct, m_w}, {-fl*st, fl*ct, m_w}}
  };

  glEnable(GL_MAP2_VERTEX_3);
  glMap2f(GL_MAP2_VERTEX_3,
      0.0, 1.0,  /* U ranges 0..1 */
      3,         /* U stride, 3 floats per coord */
      4,         /* U is 4th order, ie. cubic */
      0.0, 1.0,  /* V ranges 0..1 */
      4 * 3,     /* V stride, row is 4 coords, 3 floats per coord */
      2,         /* V is 2nd order, ie linear */
      *(*grid2x2));  /* control points */

  glMapGrid2f(
      15, 0.0, 1.0,
      1, 0.0, 1.0);

  //just do the edges
  glEvalMesh2(GL_LINE,
      0, 0,
      0, 1);
  glEvalMesh2(GL_LINE,
      15, 15,
      0, 1);
  glEvalMesh2(GL_LINE,
      0, 15,
      0, 0);
  glEvalMesh2(GL_LINE,
      0, 15,
      1, 1);

  glPopMatrix();
}

void
ConnectionModel::ComputeTransformFromPrev(const Transformation& _prevTransform) {
  m_currentTransform = _prevTransform * m_toDHFrame;
}

Transformation
ConnectionModel::DHTransform() const {
  if(m_jointType == SMAJOINT) {
    //transform just like a regular joint, except only half of theta
    double ca(cos(m_alpha)), sa(sin(m_alpha)), ct(cos(m_theta/2)), st(sin(m_theta/2));
    Vector3d pos1(m_a, -sa*m_d, ca*m_d);
    Matrix3x3 rot1;
    getMatrix3x3(rot1,
        ct, -st, 0.0,
        st*ca, ct*ca, -sa,
        st*sa, ct*sa, ca);
    //then, translate the correct distance along y and rotate the rest of theta
    double l = (m_theta == 0) ? m_l : 2*m_l/m_theta*st;
    Vector3d pos2(0, l, 0);
    Matrix3x3 rot2;
    getMatrix3x3(rot2,
        ct, -st, 0,
        st, ct, 0,
        0, 0, 1);
    return Transformation(pos1, Orientation(rot1))*Transformation(pos2, Orientation(rot2));
  } else {
    double ca(cos(m_alpha)), sa(sin(m_alpha)), ct(cos(m_theta)), st(sin(m_theta));
    Vector3d pos(m_a, -sa*m_d, ca*m_d);
    Matrix3x3 rot;
    getMatrix3x3(rot,
        ct, -st, 0.0,
        st*ca, ct*ca, -sa,
        st*sa, ct*sa, ca);
    return Transformation(pos, Orientation(rot));
  }
}

istream&
operator>>(istream& _is, ConnectionModel& _c) {
  //body indices
  _c.m_previousIndex = ReadField<size_t>(_is, WHERE, "Failed reading previous body index");
  _c.m_nextIndex = ReadField<size_t>(_is, WHERE, "Failed reading next body index");

  //Grab the joint type
  string connectionTypeTag = ReadFieldString(_is, WHERE, "Failed reading connection type.");
  _c.m_jointType = ConnectionModel::GetJointTypeFromTag(connectionTypeTag);

  //Grab the joint limits for revolute and spherical joints
  if(_c.m_jointType != ConnectionModel::NONACTUATED){
    _c.m_jointLimits[0].first = _c.m_jointLimits[1].first = -1;
    _c.m_jointLimits[0].second = _c.m_jointLimits[1].second = 1;
    size_t numRange = (_c.m_jointType == ConnectionModel::SPHERICAL) ? 2 : 1;
    for(size_t i = 0; i<numRange; i++) {
      string tok;
      if(_is >> tok) {
        size_t del = tok.find(":");
        if(del == string::npos)
          throw ParseException(WHERE, "Failed reading joint range. Should be delimited by ':'.");

        istringstream minv(tok.substr(0,del)), maxv(tok.substr(del+1, tok.length()));
        if(!(minv >> _c.m_jointLimits[i].first && maxv >> _c.m_jointLimits[i].second))
          throw ParseException(WHERE, "Failed reading joint range.");
      }
      else if(numRange > i) //error. too few tokens provided.
        throw ParseException(WHERE, "Failed reading joint ranges. Not enough provided.");
    }
  }

  //Transformations and DH parameters
  _c.m_toDHFrame = ReadField<Transformation>(_is, WHERE, "Failed reading transformation to DHFrame");

  Vector4d dhparameters = ReadField<Vector4d>(_is, WHERE, "Failed reading DH Parameters.");
  _c.m_alpha = dhparameters[0];
  _c.m_a = dhparameters[1];
  _c.m_d = dhparameters[2];
  _c.m_theta = dhparameters[3];

  if(_c.m_jointType == ConnectionModel::SMAJOINT) {
    _c.m_l = ReadField<double>(_is, WHERE, "Failed to read SMA length parameter.");
    _c.m_w = ReadField<double>(_is, WHERE, "Failed to read SMA width parameter.");
  }
  else
    _c.m_l = _c.m_w = 0;

  _c.m_toBody2 = ReadField<Transformation>(_is, WHERE, "Failed reading transformation to body 2");

  return _is;
}

ostream&
operator<<(ostream& _os, const ConnectionModel& _c){
  _os << _c.m_previousIndex << " " << _c.m_nextIndex << " ";
  _os << ConnectionModel::GetTagFromJointType(_c.m_jointType) << " ";

  if(_c.m_jointType == ConnectionModel::NONACTUATED)
    _os << endl;
  else if(_c.m_jointType == ConnectionModel::REVOLUTE
      || _c.m_jointType == ConnectionModel::SMAJOINT)
    _os << _c.m_jointLimits[0].first << ":" << _c.m_jointLimits[0].second << endl;
  else if(_c.m_jointType == ConnectionModel::SPHERICAL)
    _os << _c.m_jointLimits[0].first << ":" << _c.m_jointLimits[0].second << " "
      << _c.m_jointLimits[1].first << ":" << _c.m_jointLimits[1].second << endl;

  _os << _c.m_toDHFrame << "\t"
    << _c.m_alpha << " " << _c.m_a << " " << _c.m_d << " " << _c.m_theta;
  if(_c.m_jointType == ConnectionModel::SMAJOINT)
    _os << " " << _c.m_l << " " << _c.m_w; //SMA joints have extra parameters
  _os << "\t" << _c.m_toBody2;
  
  return _os;
}
