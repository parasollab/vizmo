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
    
    //////////////////////////////////////////////////////////////////////
    // Roadmap Related Function
    void ShowRoadMap( bool bShow=true );
    void ChangeNodesSize(float s, string str);
    void ChangeNodesShape(string s);
    bool IsRoadMapLoaded(){ return m_obj.m_Map!=NULL; }
    void ChangeNodesColor(double r, double g, double b, string s);
    
    //////////////////////////////////////////////////////////////////////
    // Path Related Function
    void ShowPathFrame( bool bShow=true );
    int GetPathSize();

    //////////////////////////////////////////////////////////////////////
    // Query Related Function
    void ShowQueryFrame( bool bshow = true );   // to know if the Query has to be showed
    bool IsQueryLoaded(){ return m_obj.m_Qry!=NULL; }

    //////////////////////////////////////////////////////////////////////
    // Robot Related Functions
    void ShowRobot( bool bShow=true ){ m_bShowRobot=bShow; }
    
    /////////////////////////////////////////////////////////////////////
    // Bounding Box Related Function
    void ShowBBox(bool bShow=true);
    
    /////////////////////////////////////////////////////////////////////
    // Environment Related Functions

    // Change the appearance of an object - Hidden/ Wire / Soid
    void ChangeAppearance(int );   
    void RefreshEnv();
    void envObjsRandomColor();
    
  //Miscelaneous

    bool StringToInt(const string &s, int &i);
    bool oneColor;   
    //////////////////////////////////////////////////////////////////////
    // Access

    vector<gliObj>& GetSelectedItem(){ return m_Plum.GetSelectedItem();}
    
    vector<PlumObject*>& GetPlumObjects(){ return m_Plum.GetPlumObjects(); }

    double GetEnvRadius();

    ////////////////////////////////////////////////////////////////
    // Variables used to change color of objects in the environment.
    float mR, mG, mB;
    
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
    
    /**
    * Put robot in start configuration is possible
    */
    void PlaceRobot();
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





