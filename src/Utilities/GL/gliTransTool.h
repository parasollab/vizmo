#ifndef _GLI_TANSFORM_TOOL_H_
#define _GLI_TANSFORM_TOOL_H_

#include<Vector.h>
#include<Quaternion.h>
using namespace mathtool;

#include <qgl.h>
#include "gliDataStructure.h"

///////////////////////////////////////////////////////////////////////////////
//
//  gliTToolBase : base class for all transformation tool
//
///////////////////////////////////////////////////////////////////////////////

class gliTToolBase
{
public:

    gliTToolBase(){}
    virtual ~gliTToolBase(){}

    ///////////////////////////////////////////////////////////////////////////
    // Core
    void Draw( void );
    virtual bool MP( QMouseEvent * e )=0; //mouse button pressed
    virtual bool MR( QMouseEvent * e )=0; //mouse button released
    virtual bool MM( QMouseEvent * e )=0; //mouse motion
    virtual void Enable()=0;   //called when this tool is activated
    virtual void Disable()=0;  //called when this tool is unactivated

    ///////////////////////////////////////////////////////////////////////////
    // Access
    virtual void setSObject( gliObj obj ){
        m_pSObj=obj;
        Project2Win();
    }

    void resetSelObj(){m_pSObj=NULL;}

    //need be updated when window size changed
    void setWinSize(int W, int H) {
        m_w=W; m_h=H;
        Project2Win();
    }

    void Project2Win();

protected:
    virtual void Draw(bool bSel)=0;

    static gliObj  m_pSObj;    //selected Objects
    static Point3d m_sObjPrj; //project(m_pSObj.pos and 3 axis), (win coord)
    static Point3d m_xPrj, m_yPrj, m_zPrj;
    static int m_w, m_h;          //window size
    static int m_hitX, m_hitY;    //mouse hit on m_hitX, m_hitY (win coord)
};

///////////////////////////////////////////////////////////////////////////////
//
//  gliMoveTool : Tool for translation
//
///////////////////////////////////////////////////////////////////////////////

class gliMoveTool : public gliTToolBase
{
    enum SelType{ NON, X_AXIS, Y_AXIS, Z_AXIS, VIEW_PLANE }; //move along
public:

    gliMoveTool():gliTToolBase(){ m_selType=NON; }
    virtual ~gliMoveTool() { /*do nothing*/ }

    ///////////////////////////////////////////////////////////////////////////
    // Core
    bool MP( QMouseEvent * e ); //mouse button pressed
    bool MR( QMouseEvent * e ); //mouse button released
    bool MM( QMouseEvent * e );  //mouse motion
    virtual void Enable(){}   //called when this tool is activated
    virtual void Disable(){}  //called when this tool is unactivated

protected:
    void Draw(bool bSel);
    bool Select(int x, int y);

private:
    Vector3d m_deltaDis;       //displacement caused by user
    SelType  m_selType;        //which axis is selected
    Point3d  m_hitUnPrj;       //unproject(m_w, m_h)
    Point3d  m_sObjPosC;       //catch for m_pSObj->pos
    Point3d  m_sObjPrjC;       //catch for m_sObjPrj
};

///////////////////////////////////////////////////////////////////////////////
//
//  gliScaleTool : Tool for salce
//
///////////////////////////////////////////////////////////////////////////////

class gliScaleTool : public gliTToolBase
{
    enum SelType{ NON, X_AXIS, Y_AXIS, Z_AXIS, VIEW_PLANE }; //move along
public:

    gliScaleTool():gliTToolBase(){ m_selType=NON; }
    virtual ~gliScaleTool() { /*do nothing*/ }

    ///////////////////////////////////////////////////////////////////////////
    // Core
    bool MP( QMouseEvent * e ); //mouse button pressed
    bool MR( QMouseEvent * e ); //mouse button released
    bool MM( QMouseEvent * e );  //mouse motion
    virtual void Enable(){}   //called when this tool is activated
    virtual void Disable(){}  //called when this tool is unactivated

protected:
    void Draw(bool bSel);
    bool Select(int x, int y);

private:
    Vector3d m_deltaDis;       //displacement caused by user
    SelType  m_selType;        //which axis is selected
    Point3d  m_hitUnPrj;       //unproject(m_w, m_h)
    Point3d  m_sObjPosC;       //catch for m_pSObj->pos
    Point3d  m_sObjPrjC;       //catch for m_sObjPrj
	double   m_osX,m_osY,m_osZ; //old scale
};

///////////////////////////////////////////////////////////////////////////////
//
//  gliRotateTool : Tool for rotation
//
///////////////////////////////////////////////////////////////////////////////

class gliRotateTool : public gliTToolBase
{
    enum SelType{ NON, X_AXIS, Y_AXIS, Z_AXIS, OUTLINE ,CENTER }; //rotate around
public:

    gliRotateTool():gliTToolBase(),m_r(50){ m_selType=NON; }
    virtual ~gliRotateTool(){ /*do nothing*/ }

    ///////////////////////////////////////////////////////////////////////////
    // Core
    bool MP( QMouseEvent * e ); //mouse button pressed
    bool MR( QMouseEvent * e ); //mouse button released
    bool MM( QMouseEvent * e );  //mouse motion

    virtual void setSObject( gliObj obj ){
        gliTToolBase::setSObject(obj);
        ComputLocalAxis();
        ComputAngles();
    }

    virtual void Enable(){//called when this tool is activated
        ComputLocalAxis();
        ComputAngles();
    }
    virtual void Disable(){}  //called when this tool is unactivated
    void ComputAngles();      //compute values for m_arc, called when view changed

protected:

    void Draw(bool bSel);
    bool Select(int x, int y);
    void ComputAngles
    (double angle[2],Vector3d& n,Vector3d& v1,Vector3d& v2,Vector3d& view);
    void ComputLocalAxis();
    double ComputAngle(Vector3d& s, Vector3d& v1,Vector3d& v2){
        return atan2(s*v2,s*v1);
    }
    Point3d UnProj2World(const Point3d& ref,const Vector3d& n,int x, int y);

private:
    Vector3d m_lA[3];          //axis of sel object
    Vector3d m_lAC[3];         //catch for m_lA[3]
    double m_arc[3][2];        //start/end of each arc
    SelType  m_selType;        //which axis is selected
    double m_hitAngle;         //the angle when mouse clikced.
    double m_curAngle;         //current angle of mouse point
    double m_r;                //Radius of tool
    Point3d  m_sObjPosC;       //catch for m_pSObj->pos
    Quaternion m_sObjQC;       //catch for m_pSObj->q
};

///////////////////////////////////////////////////////////////////////////////
//
//  gliTransformTool
//
///////////////////////////////////////////////////////////////////////////////
class gliBox;
class gliTransformTool
{
public:

    gliTransformTool(){ m_pTool=NULL; }

    ///////////////////////////////////////////////////////////////////////////
    // Core
    void Draw(void);
    bool MP( QMouseEvent * e ); //mouse button pressed
    bool MR( QMouseEvent * e ); //mouse button released
    bool MM( QMouseEvent * e ); //mouse motion
    bool KEY( QKeyEvent * e ); //Key
    void CheckSelectObject();
    void CM(){ //camera move event
        m_mT.Project2Win();
	m_sT.Project2Win();
        if( &m_rT==m_pTool ) m_rT.ComputAngles(); //view angle changed...
    }

    ///////////////////////////////////////////////////////////////////////////
    // Access
    void setWinSize(int W, int H) {
		m_mT.setWinSize(W,H);
		m_sT.setWinSize(W,H);
	}

    void resetObj() {m_pTool->resetSelObj();}
private:
    //current tool, m_mT or m_rT or NULL
    gliTToolBase * m_pTool;
    gliMoveTool    m_mT;
    gliRotateTool  m_rT;
	gliScaleTool   m_sT;
};

///////////////////////////////////////////////////////////////////////////////
//Singleton for gliTransformTool
gliTransformTool& gliGetTransformTool();

#endif //_GLI_TANSFORM_TOOL_H_

