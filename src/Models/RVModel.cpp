#include "RVModel.h"

#include "Utilities/IOUtils.h"

RVModel::
RVModel(const string& _filename) : LoadableModel("RV"), m_glRVIndex(0) {
  SetFilename(_filename);
  m_renderMode = INVISIBLE_MODE;

  ParseFile();
  Build();

  //Print(cout);
}


RVModel::
~RVModel() {
  glDeleteLists(m_glRVIndex, 1);
}


void
RVModel::
ParseFile() {
  m_rv.clear();
  m_rv_duration.clear();
  m_rv_placement.clear();

  //check input filename
  if(!FileExists(GetFilename()))
    throw ParseException(WHERE, "'" + GetFilename() + "' does not exist");

  ifstream ifs(GetFilename().c_str());

  //Throw out the first line (comment)
  string garbage;
  getline(ifs, garbage);

  size_t numRVSets;
  ifs >> numRVSets >> m_voxel_dimensions;
  getline(ifs, garbage); //throw out rest of line
  m_rv.resize(numRVSets);

  for(size_t i=0; i < numRVSets && ifs; ++i) {
    //Throw out next line (comment)
    getline(ifs, garbage);

    size_t numVoxels;
    size_t duration;
    mathtool::Vector3d placement;
    ifs >> numVoxels >> duration >> placement;
    getline(ifs, garbage); //throw out rest of line
    m_rv_duration.push_back(duration);
    m_rv_placement.push_back(placement);    

    //Throw out next line (comment)
    getline(ifs, garbage);

    for(size_t j=0; j<numVoxels && ifs; ++j) {
      mathtool::Vector3d voxel;
      ifs >> voxel;
      m_rv[i].push_back(voxel);
    }
  }
}

void
RVModel::
Build() {
  size_t rvSetIndex = 0; //later update this to include them all (make multiple GenLists and not 1 etc)
  
  glMatrixMode(GL_MODELVIEW);
 
  //clear old display list, create a new display list 
  if(m_glRVIndex != 0)
    glDeleteLists(m_glRVIndex, 1);
  m_glRVIndex = glGenLists(1);
  glNewList(m_glRVIndex, GL_COMPILE);
  glDisable(GL_LIGHTING);

  //set color
  glColor4f(61./255., 127./255., 191./255., 0.4);

  /*  
  //add rv points to display list
  glPointSize(9);
  glBegin(GL_POINTS);
  for(size_t i=0; i<m_rv[rvSetIndex].size(); ++i) 
    glVertex3dv(m_rv[rvSetIndex][i]);
  glEnd();
  */

  //add rv voxels to display list
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glPolygonMode(GL_FRONT, GL_FILL);
  glEnableClientState(GL_VERTEX_ARRAY);

  //Face index
  GLubyte id1[] = { 3, 2, 1, 0 }; //bottom
  GLubyte id2[] = { 4, 5, 6, 7 }; //top
  GLubyte id3[] = { 2, 6, 5, 1 }; //left
  GLubyte id4[] = { 0, 4, 7, 3 }; //right
  GLubyte id5[] = { 1, 5, 4, 0 }; //back
  GLubyte id6[] = { 7, 6, 2, 3 }; //front

  for(size_t i=0; i<m_rv[rvSetIndex].size(); ++i) { 
    Vector3d center = m_rv[rvSetIndex][i];

    vector< pair<double, double> > bbx;
    bbx.push_back(make_pair(center[0] - m_voxel_dimensions[0]/2., center[0] + m_voxel_dimensions[0]/2.));
    bbx.push_back(make_pair(center[1] - m_voxel_dimensions[1]/2., center[1] + m_voxel_dimensions[1]/2.));
    bbx.push_back(make_pair(center[2] - m_voxel_dimensions[2]/2., center[2] + m_voxel_dimensions[2]/2.));

    GLdouble vertices[] = {
      bbx[0].first,  bbx[1].first, bbx[2].first,
      bbx[0].second, bbx[1].first, bbx[2].first,
      bbx[0].second, bbx[1].first, bbx[2].second,
      bbx[0].first,  bbx[1].first, bbx[2].second,
      bbx[0].first,  bbx[1].second, bbx[2].first,
      bbx[0].second, bbx[1].second, bbx[2].first,
      bbx[0].second, bbx[1].second, bbx[2].second,
      bbx[0].first,  bbx[1].second, bbx[2].second
    };

    glVertexPointer(3, GL_DOUBLE, 0, vertices);
    glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, id1);
    glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, id2);
    glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, id3);
    glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, id4);
    glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, id5);
    glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, id6);
  }
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisable(GL_POLYGON_OFFSET_FILL);
  glDisable(GL_BLEND);

  glEnable(GL_LIGHTING);
  glEndList();
}

void
RVModel::
DrawRender() {
  if(m_renderMode == INVISIBLE_MODE)
    return; //not draw any thing else

  glCallList(m_glRVIndex);
}

void
RVModel::
DrawSelect() {
  if(m_renderMode == INVISIBLE_MODE)
    return; //not draw any thing else

  glCallList(m_glRVIndex);
}

void
RVModel::
Print(ostream& _os) const {
  _os << Name() << ": " << GetFilename() << endl;
  _os << "\t" << m_rv.size() << " rv sets" << endl;
  _os << "\tVoxel Dimensions: " << m_voxel_dimensions << endl;
  for(size_t i=0; i<m_rv.size(); ++i) {
    _os << "\t\t" << m_rv[i].size() 
        << " rv voxels with base at [" << m_rv_placement[i] 
        << "] for " << m_rv_duration[i] << " frames" 
        << endl;
  }
}

