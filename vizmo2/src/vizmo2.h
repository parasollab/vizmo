// OBPRMView.h: interface for the vizmo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_VIZMO2_H_)
#define _VIZMO2_H_

//////////////////////////////////////////////////////////////////////
// Include std headers
#include <vector>
#include <string>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Include Plum headers
#include <Plum.h>
using namespace plum;

//Define singleton
class vizmo;
vizmo& GetVizmo();

/**
 * This class holds all vizmo objects.
 * When an item is NULL, it means this item is not availiable for 
 * current session.
 * Only vizmo can create this object.
 */
class vizmo_obj{
private:
    friend class vizmo;

    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    vizmo_obj(){m_Robot=m_BBox=m_Qry=m_Path=m_Env=m_Map=NULL;}
    ~vizmo_obj(){Clean();}
    void Clean();

    PlumObject * m_Robot;
    PlumObject * m_BBox;
    PlumObject * m_Qry;
    PlumObject * m_Path;
    PlumObject * m_Env;
    PlumObject * m_Map;
};

/**
 * vizmo is the main class that handles event from gui and
 * contains objects (vizmo_obj) for data loaded in.
 */
class vizmo  
{
public:
    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////
    
    vizmo();
    virtual ~vizmo();
    
    //////////////////////////////////////////////////////////////////////
    // Core Function
    //////////////////////////////////////////////////////////////////////

    /**
     * This file locates all possible filename related to OBPRM.
     * For example, *.path, *.env, *.query...etc.
     */
    vector<string> GetAccessFiles(const string& filename);

    /**
     * Create vizmo_obj.
     */
    bool InitVizmoObject( const vector<string>& filenames );

    /**
     * Display OpenGL Scene
     */
    void Display(){ m_Plum.Draw();}
 
    /**
     * Display OpenGL Scene
     */
    void Select(const gliBox& box){ m_Plum.Select(box);}

    /**
     * Animate Robot motion.
     */
    void Animate( int frame );
	int GetPathSize();

    /*
    void GetConfiguration(int);

    int GetCurrentCfg();


    //return the number of frames
    int getTimer();
     */

     /* functions for the tree view display */
      int GetNoEnvObjects();
      const CMultiBodyInfo * GetMultiBodyInfo(string &) const;
      

    //////////////////////////////////////////////////////////////////////
    // Roadmap Related Function
    void ShowRoadMap( bool bShow=true );
    //nov. 2002 Aimee
    void ChangeNodesSize(float s);
    //jan. 2003 Aimee
    void ChangeNodesShape(string s);

    //////////////////////////////////////////////////////////////////////
    // Path Related Function
    void ShowPathFrame( bool bShow=true );

    //////////////////////////////////////////////////////////////////////
    // Query Related Function
    void ShowQueryFrame( bool bshow = true );   // to know if the Query has to be showed

    //////////////////////////////////////////////////////////////////////
    // Robot Related Functions
    void ShowRobot( bool bShow=true ){ m_bShowRobot=bShow; }


    /////////////////////////////////////////////////////////////////////
    // Bounding Box Related Function
    void ShowBBox(bool bShow=true);

    // Change the appearance of an object - Hidden/ Wire / Soid
    void ChangeAppearance(int );
    
    void RefreshEnv();
    

    //////////////////////////////////////////////////////////////////////
    // Access
    vector<gliObj>& GetSelectedItem(){return m_Plum.GetSelectedItem();}

    double GetEnvRadius(){ 
        if(m_obj.m_Env!=NULL ){
            CEnvModel* env=(CEnvModel*)m_obj.m_Env->getModel();
            return env->GetRadius();
        }
        return 200;
    }

    //////////////////////////////////////////////////////////////////////
    // Protected Function
    //////////////////////////////////////////////////////////////////////
protected:

    /**
     * Create Environment Plum Object.
     */
    bool CreateEnvObj( vizmo_obj& obj, const string& fname );

    /**
     * Create Map Plum Object.
     */
    bool CreateMapObj( vizmo_obj& obj, const string& fname );

    /**
     * Create Path Plum Object.
     */
    bool CreatePathObj( vizmo_obj& obj, const string& fname );

    /**
     * Create Query Plum Object.
     */
    bool CreateQueryObj( vizmo_obj& obj, const string& fname );
    
    /**
     * Create BBX Plum Object.
     */
    bool CreateBBoxObj( vizmo_obj& obj );

    /**
     * Create Robot Plum Object.
     */
    bool CreateRobotObj( vizmo_obj& obj );

    //////////////////////////////////////////////////////////////////////
    // Private Function
    //////////////////////////////////////////////////////////////////////
private:

    /** 
     * Find filename in name with certain extension.
     * @param ext extention
     * @param names filenames
     */
    string FindName(const string& ext, const vector<string>& names)const;

    /**
     * Check if given filename exists.
     * @return ture if file is found.
     */
    bool FileExits(const string& filename) const;
    bool m_bShowRobot;

    // list<vizmo_obj> m_objs; //for future reference

    vizmo_obj m_obj;

    CPlum m_Plum;
};

#endif // !defined(_VIZMO2_H_)





