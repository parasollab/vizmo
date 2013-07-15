// PolyhedronModel.h: interface for the CPolyhedronModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MULTIPOLYHEDRONMODEL_H_)
#define _MULTIPOLYHEDRONMODEL_H_

#include "Plum/GLModel.h"
//#include "IModel.h"
//#include "MovieBYULoader.h" 
#include "MultiBodyInfo.h"
#include "Transformation.h"

#include <RAPID.H>

#include <Point.h>
using namespace mathtool; 

namespace plum{

  class CMultiBodyInfo;

  //a class construct a polyhedron body
  class CPolyhedronModel : public GLModel
  {
    public:
      typedef Vector<int, 3> Tri; 
      typedef vector<Point3d> PtVector; 
      typedef vector<Tri> TriVector; 
      //////////////////////////////////////////////////////////////////////
      // Constructor/Destructor
      //////////////////////////////////////////////////////////////////////
      CPolyhedronModel();
      virtual ~CPolyhedronModel( );

      //virtual ~CPolyhedronModel( delete rapidModel; );

      //////////////////////////////////////////////////////////////////////
      // Core
      //////////////////////////////////////////////////////////////////////
      bool BuildModels();
      void Draw( GLenum mode );
      virtual void DrawSelect();

      void Select( unsigned int * index, vector<gliObj>& sel ){}

      virtual const string GetName() const;
      virtual vector<string> GetInfo() const;

      //////////////////////////////////////////////////////////////////////
      // Access
      //////////////////////////////////////////////////////////////////////
      void SetBody( const CBodyInfo& bodyinfo ){ 
	m_BodyInfo=bodyinfo;
      }
      void SetSelected( bool bSel=true );
      double GetRadius() const { return m_R; }
      const Point3d& GetCOM() const { return m_COM; }

      ///////////////////////////////////////////////////////////////////
      // RAPID MODELS
      //////////////////////////////////////////////////////////////////

      RAPID_model * rapidModel;
      RAPID_model * GetRapidModel() {
	return rapidModel;
      }

      void ConvertToDouble(Vector3d vp1, Vector3d vp2, Vector3d vp3, 
	  double p1[], double p2[], double p3[]){
	p1[0] = vp1[0]; p1[1] = vp1[1]; p1[2] = vp1[2];
	p2[0] = vp2[0]; p2[1] = vp2[1]; p2[2] = vp2[2];
	p3[0] = vp3[0]; p3[1] = vp3[1]; p3[2] = vp3[2];
      }

    protected:
      //build model, given points and triangles
      bool BuildGLModel_Solid( const PtVector& points, const TriVector& tris, 
	  const Point3d& com, const Vector3d * n, bool isSurface );
      bool BuildGLModel_Wired( const PtVector& points, const TriVector& tris,
	  const Point3d& com, const Vector3d * n, bool isSurface );
      Point3d COM(const PtVector& points);
      double Radius(const Point3d& com,const PtVector& points);

    private:
      CBodyInfo m_BodyInfo;
      int m_SolidID; //the compiled model id for solid model
      int m_WiredID; //the compiled model id for wire frame

      double m_R; //radius
      Point3d m_COM; //Center of Mass

      Transformation finalTransform;
      vector<Transformation> transformVector;
  };
}//namespace plum

#endif // !defined(_MULTIPOLYHEDRONMODEL_H_)
