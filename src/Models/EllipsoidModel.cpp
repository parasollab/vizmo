#include "EllipsoidModel.h"

#include <limits>
#include <sstream>


#include "Utilities/GLUtils.h"

/*------------------------------ Construction --------------------------------*/

EllipsoidModel::
EllipsoidModel() : BoundaryModel("Bounding Ellipse", NULL),
  m_center(vector<double>({0,0,0})),
  m_axes(vector<pair<double, double> >(3, make_pair(0,0))) {}

EllipsoidModel::
EllipsoidModel(const size_t _n, const std::vector<double>& _r,
    const EulerAngle& _e) : BoundaryModel("Bounding Ellipse", NULL),
    m_center(_n, 0), m_radii(_r), m_orientation(_e) {
	if(_r.empty())
		m_radii.assign(_n,std::numeric_limits<double>::max());
        Build();
}


EllipsoidModel::
EllipsoidModel(const std::vector<double>& _c, const std::vector<double>& _r,
    const EulerAngle& _e) : BoundaryModel("Bounding Ellipse", NULL),
    m_center(_c), m_radii(_r), m_orientation(_e) {
	if(_r.empty())
		m_radii.assign(_c.size(),std::numeric_limits<double>::max());
        Build();
}


/*------------------------------- Accessors ----------------------------------*/

void
EllipsoidModel::
Build() {

  if(m_radii.size() == 2) {

    m_displayID = glGenLists(1);
    glNewList(m_displayID, GL_COMPILE);
    glLineWidth(2);
    glColor3d(0.0, 0.0, 0.0);
		glPushMatrix();
    glTranslatef(m_center.at(0), m_center.at(1), m_center.at(2)); 
    glRotatef(m_orientation.alpha() * 180 / PI, 0, 0, 1);
    GLUtils::DrawEllipse(m_radii[0], m_radii[1], false);
		glPopMatrix();
    glEndList();
  }
  if(m_radii.size() == 3) {
    m_displayID = glGenLists(1);
    glNewList(m_displayID, GL_COMPILE);
    glLineWidth(2);
    glColor3d(0.0, 0.0, 0.0);
		glPushMatrix();
    glTranslatef(m_center.at(0), m_center.at(1), m_center.at(2));
    glRotatef(m_orientation.alpha()*180/PI, 0, 0, 1);
    glRotatef(m_orientation.beta()*180/PI, 0, 1, 0);
    GLUtils::DrawEllipsoid(m_radii[0], m_radii[1], m_radii[2]);
		glPopMatrix();
    glEndList();
  }
  /*
  m_linesID = glGenLists(1);
  glNewList(m_linesID, GL_COMPILE);
  glTranslatef(center[0], center[1], center[2]);
  GLUtils::DrawCircle(radius, false);
  glEndList();
  */
}

void
EllipsoidModel::
Print(ostream& _os) const {
}

double
EllipsoidModel::
GetMaxDist() {
  return 0;
}
