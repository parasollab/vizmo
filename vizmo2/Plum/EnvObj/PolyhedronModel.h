// PolyhedronModel.h: interface for the CPolyhedronModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MULTIPOLYHEDRONMODEL_H_)
#define _MULTIPOLYHEDRONMODEL_H_

#include "GLModel.h"
#include "MovieBYULoader.h"
#include "MultiBodyInfo.h"

namespace plum{
    
    class CMultiBodyInfo;
    
    //a class construct a polyhedron body
    class CPolyhedronModel : public CGLModel
    {
    public:
        //////////////////////////////////////////////////////////////////////
        // Constructor/Destructor
        //////////////////////////////////////////////////////////////////////
        CPolyhedronModel();
        virtual ~CPolyhedronModel();
        
        //////////////////////////////////////////////////////////////////////
        // Core
        //////////////////////////////////////////////////////////////////////
        bool BuildModels();
        void Draw( GLenum mode );
        virtual void DrawSelect();
        
        void Select( unsigned int * index, vector<gliObj>& sel ){
        }
        
        virtual const string GetName() const;
        virtual list<string> GetInfo() const;
        
        //////////////////////////////////////////////////////////////////////
        // Access
        //////////////////////////////////////////////////////////////////////
        void SetBody( const CBodyInfo& bodyinfo ){ m_BodyInfo=bodyinfo; }
        void SetSelected( bool bSel=true );
        double GetRadius() const { return m_R; }
        const Point3d& GetCOM() const { return m_COM; }
        
    protected:
        //build model, given points and triangles
        bool BuildGLModel_Solid( const PtVector& points, const TriVector& tris, 
                                 const Point3d& com, const Vector3d * n );
        bool BuildGLModel_Wired( const PtVector& points, const TriVector& tris,
                                 const Point3d& com, const Vector3d * n );
        Point3d COM(const PtVector& points);
        double Radius(const Point3d& com,const PtVector& points);
        
    private:
        CBodyInfo m_BodyInfo;
        int m_SolidID; //the compiled model id for solid model
        int m_WiredID; //the compiled model id for wire frame
        
        double m_R; //radius
        Point3d m_COM; //Center of Mass
    };
}//namespace plum

#endif // !defined(_MULTIPOLYHEDRONMODEL_H_)
