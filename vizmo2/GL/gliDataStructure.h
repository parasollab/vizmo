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
		m_Scale[0]=m_Scale[1]=m_Scale[2]=1;
    }
    
    void glTransform();
    void Euiler2Quaternion();

    //Access
    
    ///Translation
    ///@{
    double& tx(){ return m_Pos[0]; }
    double& ty(){ return m_Pos[1]; }
    double& tz(){ return m_Pos[2]; }
    const double& tx() const { return m_Pos[0]; }
    const double& ty() const { return m_Pos[1]; }
    const double& tz() const { return m_Pos[2]; }
    ///@}
    
    ///Sacle
    ///@{
    double& sx(){ return m_Scale[0]; }
    double& sy(){ return m_Scale[1]; }
    double& sz(){ return m_Scale[2]; }
    const double& sx() const { return m_Scale[0]; }
    const double& sy() const { return m_Scale[1]; }
    const double& sz() const { return m_Scale[2]; }
    ///@}

    ///Rotation
    ///@{
    double& rx(){ return m_Rot[0]; }
    double& ry(){ return m_Rot[1]; }
    double& rz(){ return m_Rot[2]; }
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
	double m_Scale[3];       //Sacle
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

