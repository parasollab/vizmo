#include "MultiBodyModel.h"
#include "PlumState.h"

namespace plum{
    
    //////////////////////////////////////////////////////////////////////
    // Cons/Des
    CMultiBodyModel::CMultiBodyModel
        (unsigned int index, const CMultiBodyInfo & MBInfo)
        :m_MBInfo(MBInfo)
    {
        m_index=index;
        m_PolySize=0;
        m_pPoly=NULL;
        SetColor(0.4f,0.4f,0.4f,1);
        m_bFixed=true;
        m_R=0;
    }
    
    CMultiBodyModel::~CMultiBodyModel()
    {
        delete [] m_pPoly;
        m_pPoly=NULL;
    }
    
    //////////////////////////////////////////////////////////////////////
    // Core
    //////////////////////////////////////////////////////////////////////
    bool CMultiBodyModel::BuildModels()
    {
        m_PolySize=m_MBInfo.m_cNumberOfBody;
        if( (m_pPoly=new CPolyhedronModel[m_PolySize])==NULL )
            return false; //Out of memory
        int iM=0;
        //build for each body and compute com
        for( iM=0; iM<m_PolySize; iM++ ){
            CBodyInfo & info=m_MBInfo.m_pBodyInfo[iM];
            //only build fixed, free body will not be build (when m_bFixed is set)
            if( !info.m_bIsFixed && m_bFixed==true )
                continue;
            
            m_pPoly[iM].SetBody(info);
            if( m_pPoly[iM].BuildModels()==false )
                return false;
            m_COM[0]+=info.m_X;
            m_COM[1]+=info.m_Y;
            m_COM[2]+=info.m_Z;
        }
        for( int id=0;id<3;id++ ) m_COM[id]/=m_PolySize;
        //set position of multi-body as com
        tx()=m_COM[0]; ty()=m_COM[1]; tz()=m_COM[2];

        //compute radius
        m_R=0; //set radius to 0 and compute it later
        for( iM=0; iM<m_PolySize; iM++ ){
            CBodyInfo & info=m_MBInfo.m_pBodyInfo[iM];
            //only build fixed, free body will not be build (when m_bFixed is set)
            if( !info.m_bIsFixed && m_bFixed==true )
                continue;
            double dist=(Point3d(info.m_X,info.m_Y,info.m_Z)-m_COM).norm()
                        +m_pPoly[iM].GetRadius();
            if( m_R<dist ) m_R=dist;
            //change to local coorindate of multibody
            m_pPoly[iM].tx()-=tx(); m_pPoly[iM].ty()-=ty(); m_pPoly[iM].tz()-=tz();
        }

        return true;
    }
    
    void CMultiBodyModel::Select( unsigned int * index, vector<gliObj>& sel )
    {
        if(index!=NULL)
            sel.push_back(this);
    }
    
    //Draw
    void CMultiBodyModel::Draw( GLenum mode )
    {
        glColor4fv(m_RGBA);
        glPushMatrix();
        glTransform();
        for( int i=0;i<m_PolySize;i++ )
            m_pPoly[i].Draw( mode );
        glPopMatrix();
    }

    void CMultiBodyModel::DrawSelect()
    {
        glPushMatrix();
        glTransform();
        for( int i=0;i<m_PolySize;i++ )
            m_pPoly[i].DrawSelect();
        glPopMatrix();
    }

    void CMultiBodyModel::SetRenderMode(int mode)
    {
    m_RenderMode=mode;
        for( int i=0;i<m_PolySize;i++ )
            m_pPoly[i].SetRenderMode(mode);
    }

    void CMultiBodyModel::SetColor(float r, float g, float b, float a)
    {
        CGLModel::SetColor(r,g,b,a);
        for( int i=0;i<m_PolySize;i++ )
            m_pPoly[i].SetColor(r,g,b,a);
    }
    
    list<string> CMultiBodyModel::GetInfo() const 
    { 
        list<string> info; 
        if( m_bFixed ) info.push_back(string("Obstacle"));
        else info.push_back(string("Robot"));
        {
            ostringstream temp;
            temp<<"There are "<<m_PolySize<<" bodies";
            info.push_back(temp.str());
        }
        return info;
    }
    
}//namespace plum


