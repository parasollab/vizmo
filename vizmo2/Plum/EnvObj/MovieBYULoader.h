///////////////////////////////////////////////////////////////////////////////////////////
// This file defines a Movie.BYU format fileloader


#ifndef _MOVIEBYULOADER_H_
#define _MOVIEBYULOADER_H_

#include "ILoadable.h"

//////////////////////////////////////////////////////////////////////////////////////
///std header
#include <fstream>
#include <vector>
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////
//math tool header
#include <Vector.h>
#include <Point.h>
using namespace mathtool;

namespace plum{

typedef vector<Point3d> PtVector;  ///a vector of points
typedef Vector<int>     Tri;       ///a triangle has 3 index for 3 points
typedef vector<Tri> TriVector; ///a vector of trianlges

//this file loads Movie.BYU file.
class CMovieBYULoader : public I_Loadable
{
public:

    //////////////////////////////////////////////////////////////////////////////////////
    CMovieBYULoader()
    {
        m_PartsSize=m_VertexSize=m_PolygonSize=m_EdgeSize=0;
    }

    //////////////////////////////////////////////////////////////////////////////////////
    // Implemetation of ILoadable interface
    //////////////////////////////////////////////////////////////////////////////////////
    virtual bool ParseFile();

    //////////////////////////////////////////////////////////////////////////////////////
    // Implemetation of IPolygonal interface
    //////////////////////////////////////////////////////////////////////////////////////
    int GetVertices(PtVector& v) const{ v=points; return v.size(); }
    int GetTriangles(TriVector& v) const{ v=triangles; return v.size(); }
    const PtVector & GetVertices() const{ return points; }
    const TriVector & GetTriangles() const{ return triangles; }

////////////////////////////////////////////////////////////////////////////////////////////
//
//  Private Methods and data members
//
////////////////////////////////////////////////////////////////////////////////////////////
private:
    bool ParseSection1(ifstream & in);
    bool ParseSection2(ifstream & in);
    bool ParseSection3(ifstream & in);

    int m_PartsSize,m_VertexSize,m_PolygonSize,
        m_EdgeSize;

    vector< pair<int,int> > parts;
    PtVector points;
    TriVector triangles;
};

}// namespace plum

#endif

