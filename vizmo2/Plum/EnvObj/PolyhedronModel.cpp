// PolyhedronModel.cpp: implementation of the CPolyhedronModel class.
//
//////////////////////////////////////////////////////////////////////

#include "PolyhedronModel.h"
#include "EnvLoader.h"  //for CMultiBodyInfo
#include "Plum.h"
#include <math.h>

#include <ModelGraph.h>
using namespace modelgraph;

namespace plum{
    
    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////
        
    CPolyhedronModel::CPolyhedronModel()
    {
        m_SolidID=-1;
        m_WiredID=-1;
        m_R=0;
        SetColor(0.5,0.5,0.5,1);
    }
    
    CPolyhedronModel::~CPolyhedronModel()
    {
        if( m_WiredID!=-1 ) glDeleteLists(m_WiredID,1);
        if( m_SolidID!=-1 ) glDeleteLists(m_SolidID,1);
    }
    
    bool CPolyhedronModel::
        BuildModels()
    {
        CMovieBYULoader loader;
        loader.SetDataFileName(m_BodyInfo.m_strModelDataFileName);
        if( loader.ParseFile()==false ) return false;
        
        //compute center of mass
        const PtVector& points=loader.GetVertices();
        const TriVector& tris=loader.GetTriangles();
        Point3d com=COM(points); m_COM=com;
        m_R=Radius(com,points);

        //set point for opengl
        int psize=points.size();
        GLdouble * vertice=new GLdouble[3*psize];
        if( vertice==NULL ) return false;
        for(int iP=0;iP<psize;iP++){
            points[iP].get(vertice+iP*3);    // here are read the points of every *.g file
        }
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_DOUBLE, 0, vertice);
        
        //compute normals
        int tsize=tris.size();
        Vector3d * normal=new Vector3d[tsize];
        if( normal==NULL ) return false;
        for( int t=0;t<tsize;t++ ){
            const Tri & tri=tris[t];
            Vector3d v1=points[tri[1]]-points[tri[0]];
            Vector3d v2=points[tri[2]]-points[tri[0]];
            normal[t]=(v1%v2).normalize(); //cross product
        }
        
        //build solid model
        if( BuildGLModel_Solid(points,tris,com,normal)==false )
        {
            cerr<<"Build Model ("<<m_BodyInfo.m_strModelDataFileName<<") Error"<<endl;
            return false;
        }
        
        //build wired frame
        if( BuildGLModel_Wired(points,tris,com,normal)==false )
        {
            cerr<<"Build Model ("<<m_BodyInfo.m_strModelDataFileName<<") Error"<<endl;
            return false;
        }
        delete [] vertice; delete [] normal;

        //setup rotation and translation
        tx()=m_BodyInfo.m_X; ty()=m_BodyInfo.m_Y; tz()=m_BodyInfo.m_Z;
        double cx_2=cos(m_BodyInfo.m_Alpha/2); double sx_2=sin(m_BodyInfo.m_Alpha/2);
        double cy_2=cos(m_BodyInfo.m_Beta/2); double sy_2=sin(m_BodyInfo.m_Beta/2);
        double cz_2=cos(m_BodyInfo.m_Gamma/2); double sz_2=sin(m_BodyInfo.m_Gamma/2);

        Quaternion qx(cx_2,sx_2*Vector3d(1,0,0));
        Quaternion qy(cy_2,sy_2*Vector3d(0,1,0));
        Quaternion qz(cz_2,sz_2*Vector3d(0,0,1));
        Quaternion nq=qz*qy*qx; //new q
        this->q(nq.normalize()); //set new q

        return true;
    }
    
    //Build solid mode
    bool CPolyhedronModel::BuildGLModel_Solid
        (const PtVector& points, const TriVector& tris, 
        const Point3d& com, const Vector3d * n)
    {
        m_SolidID=glGenLists(1);
        glNewList(m_SolidID,GL_COMPILE);
        
        glEnable(GL_LIGHTING);
        glMatrixMode( GL_MODELVIEW );
        glPushMatrix();     
        glTranslated(-com[0],-com[1],-com[2]);
        
        //draw
        double value[3];
        TriVector::const_iterator t;
        for( t=tris.begin();t!=tris.end();t++ ){
            const Tri & tri=*t;
            n[t-tris.begin()].get(value);
            glNormal3dv(value);
            GLuint id[3]={tri[0],tri[1],tri[2]};
            glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_INT, id );
        }
        
        glPopMatrix();
        glEndList();
        
        return true;
    }
    
    //build wire frame
    bool CPolyhedronModel::BuildGLModel_Wired
    (const PtVector& points, const TriVector& tris, 
     const Point3d& com, const Vector3d * n)
    {   
        CModelGraph mg;
        if( mg.doInit(points,tris)==false ) return false;
        
        //build model
        m_WiredID=glGenLists(1);
        glNewList(m_WiredID,GL_COMPILE);

        glDisable(GL_LIGHTING);
        glMatrixMode( GL_MODELVIEW );
        glPushMatrix();     
        glTranslated(-com[0],-com[1],-com[2]);
        
        const CModelEdge * edge=mg.getEdges();
        while(edge!=NULL){
            int tri_l=edge->getLeftTri();
            int tri_r=edge->getRightTri();
            
            bool draw=false;
            if( tri_l==-1 || tri_r==-1 ) draw=true;
            else if( (1-fabs(n[tri_l]*n[tri_r]))>1e-3 ) draw=true;
            
            if( draw ){
                GLuint id[2]={edge->getStartPt(),edge->getEndPt()};
                glDrawElements( GL_LINES, 2, GL_UNSIGNED_INT, id );
            }
            edge=edge->getNext();
        }
        
        glPopMatrix();
        glEndList();
        
        return true;
    }

    void CPolyhedronModel::Draw( GLenum mode )
    {
        if( m_SolidID==-1 ) return;
        if( m_RenderMode==CPlumState::MV_INVISIBLE_MODE ) return;
        
        ////////////////////////////////////////////////////////////////////////////////
        glColor4fv(m_RGBA);
        glPushMatrix();
        glTransform();
        if( m_RenderMode==CPlumState::MV_SOLID_MODE ){           
            glEnable( GL_POLYGON_OFFSET_FILL );
            glPolygonOffset( 2.0, 2.0 );
            glCallList(m_SolidID);          
            glDisable( GL_POLYGON_OFFSET_FILL ); 
        }
        else
			glCallList(m_WiredID);
        glPopMatrix();
    }

    void CPolyhedronModel::DrawSelect()
    {
        ////////////////////////////////////////////////////////////////////////////////
        glLineWidth(2);
        glPushMatrix();
        glTransform();
        glColor3d(1,1,0);
        glCallList(m_WiredID);
        glPopMatrix();
    }

    //compute center of mass
    Point3d CPolyhedronModel::COM(const PtVector& points)
    {
        Point3d com; //center of mass
        PtVector::const_iterator i=points.begin();
        for( ;i!=points.end();i++ )
        {
            const Point3d tmp=*i;
            com[0]+=tmp[0];
            com[1]+=tmp[1];
            com[2]+=tmp[2];
        }
        
        int size=points.size();
        com[0]/=size; com[1]/=size; com[2]/=size;
        return com;
    }

    double CPolyhedronModel::Radius(const Point3d& com,const PtVector& points)
    {
        double R=0;
        PtVector::const_iterator i=points.begin();
        for( ;i!=points.end();i++ )
        {
            double d=(*i-com).normsqr();
            if( d>R ) R=d;
        }
        return sqrt(R);
    }    

	const string CPolyhedronModel::GetName() const { 
		string sep="/";
#ifdef WIN32
		sep="\\";
#endif
		string filename=m_BodyInfo.m_strModelDataFileName;
		int pos=0;
		if( (pos=filename.find(sep))!=string::npos )
			filename=filename.substr(pos+1);
		return filename; 
	}

}//namespace plum

