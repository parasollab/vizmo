#ifndef _GLI_DATASTRUCTURE_H_
#define _GLI_DATASTRUCTURE_H_

#include <vector>
using namespace std;

#include <Quaternion.h>
using namespace mathtool;

/**
* Defines function and data for tansfromation.
*/
class gliTransform {
public:
    
    gliTransform(){ 
        m_Pos[0]=m_Pos[1]=m_Pos[2]=
            m_Rot[0]=m_Rot[1]=m_Rot[2]=0;
    }
    
    gliTransform(const gliTransform& other){
        memcpy(m_Pos,other.m_Pos,3*sizeof(double));   
        memcpy(m_Rot,other.m_Rot,3*sizeof(double));
        m_q=other.m_q;
    }
    
    void glTransform();
    
    //Access
    
    ///Translation
    ///@{
    double& tx(){ return m_Pos[0]; }
    double& ty(){ return m_Pos[1]; }
    double& tz(){ return m_Pos[2]; }
    ///@}
    
    ///Rotation : Read Only
    ///@{
    const double& rx() const { return m_Rot[0]; }
    const double& ry() const { return m_Rot[1]; }
    const double& rz() const { return m_Rot[2]; }
    ///@}
    
    //Get&Set Quaternion
    ///@{
    const Quaternion& q() const {return m_q;}
    void q(const Quaternion& q){ m_q=q; }
    ///@}
    
protected:
    double m_Pos[3];         //Position
    double m_Rot[3];         //Rotation
    Quaternion m_q;          //Rotation
};

typedef gliTransform* gliObj;

//definition for a 2D box
class gliBox{
public:
    gliBox(){ l=r=t=b=0; }
    double l,r,t,b; //left, right,top,bottom
};

///////////////////////////////////////////////////////////////////////////////
vector<gliObj>& gliGetPickedSceneObjs();

#endif //_GLI_DATASTRUCTURE_H_

