#ifndef CMultiBodyModel_H_
#define CMultiBodyModel_H_

#include "MultiBodyInfo.h"
#include "GLModel.h"
#include "PolyhedronModel.h"

namespace plum{
    
    class CMultiBodyModel : public CGLModel
    {
    public:
        //////////////////////////////////////////////////////////////////////
        // Cons/Des
        CMultiBodyModel(unsigned int index, const CMultiBodyInfo & MBInfo);
        virtual ~CMultiBodyModel();
        
        //////////////////////////////////////////////////////////////////////
        // Core
        //////////////////////////////////////////////////////////////////////
        virtual bool BuildModels();
        
        virtual void Select( unsigned int * index, vector<gliObj>& sel );
        
        //Draw
        virtual void Draw( GLenum mode );
        virtual void DrawSelect();
        
        //set wire/solid/hide
        virtual void SetRenderMode( int mode );
        
        virtual void SetColor(float r, float g, float b, float a);

        virtual const string GetName() const { return "MultiBody"; }

        virtual void GetChildren( list<CGLModel*>& models ){ 
            for(int i=0;i<m_PolySize;i++ )
                models.push_back(&m_pPoly[i]);
        }

        virtual list<string> GetInfo() const;

	virtual	void Scale(double x, double y, double z);

	double posX, posY, posZ;

        //////////////////////////////////////////////////////////////////////
        // Access
        //////////////////////////////////////////////////////////////////////
        void setAsFree( bool free=true ){ m_bFixed=!free; }
        
	double GetRadius() const { return m_R; }
        
	const Point3d& GetCOM() const { return m_COM; }
	
	CPolyhedronModel* GetPolyhedron()  {return m_pPoly;}

	list<CGLModel *> objlist; // to have access from glitransTool class

    private:
        CPolyhedronModel * m_pPoly;

        int m_PolySize;
        int m_index; //the index of this multibody
        const CMultiBodyInfo & m_MBInfo; //a reference to the CMultiBodyInfo
        bool m_bFixed; //is this multibody fixed. i.e obstacle
        
        double m_R; //Radius
        Point3d m_COM; // center of mass
        
    };
}//namespace plum

#endif //CMultiBodyModel_H_

