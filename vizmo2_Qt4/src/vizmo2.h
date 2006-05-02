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
#include<stdio.h>
//////////////////////////////////////////////////////////////////////
// Include Plum headers
#include <Plum.h>
#include <EnvObj/MultiBodyModel.h>
using namespace plum;


#include <CollisionDetection.h>

#include <qstringlist.h> 


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
    PlumObject * m_BBox;    bool m_show_BBox;
    PlumObject * m_Qry;     bool m_show_Qry;  string m_QryFile;
    PlumObject * m_Path;    bool m_show_Path; string m_PathFile;
    PlumObject * m_Env;                       string m_EnvFile;
    PlumObject * m_Map;     bool m_show_Map;  string m_MapFile;
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
    void GetAccessFiles(const string& filename);
    
    /**
    * Create vizmo_obj.
    */
    bool InitVizmoObject();
    
    /**
    * Display OpenGL Scene
    */
    void Display(){ m_Plum.Draw();}
    
    /**
    * Select Objects in OpenGL Scene
    */
    void Select(const gliBox& box){ m_Plum.Select(box); }
    
    /**
    * Animate Robot motion.
    */
    void Animate( int frame );
    
    //////////////////////////////////////////////////////////////////////
    // Roadmap Related Function
    void ShowRoadMap( bool bShow=true );
    bool isRoadMapShown() const { return m_obj.m_show_Map; }
    void ChangeNodesSize(float s, string str);
    void ChangeNodesShape(string s);
    bool IsRoadMapLoaded(){ return m_obj.m_Map!=NULL; }
    void ChangeNodesColor(double r, double g, double b, string s);
    void ChangeNodesRandomColor();
    PlumObject * GetMap() const { return m_obj.m_Map; }

    void setMapObj(CMapLoader<CCfg,CSimpleEdge> *ml, CMapModel<CCfg,CSimpleEdge> * mm);
   
    //////////////////////////////////////////////////////////////////////
    // Path Related Function
    void ShowPathFrame( bool bShow=true );
    int GetPathSize();
    bool IsPathLoaded(){ return m_obj.m_Path!=NULL; }
    PlumObject * GetPath() const { return m_obj.m_Path; }

    //////////////////////////////////////////////////////////////////////
    // Query Related Function
    void ShowQueryFrame( bool bshow = true );   // to know if the Query has to be showed
    bool IsQueryLoaded(){ return m_obj.m_Qry!=NULL; }

    //functions and variables used to determine if Query has changed.
    // these are used by the autoQry() function in main_win 

    bool query_changed;
    void changeQryStatus(bool status);
    bool hasQryChanged() {return query_changed;}
    PlumObject * GetQry() const { return m_obj.m_Qry; }

    //////////////////////////////////////////////////////////////////////
    // Robot Related Functions
    void ShowRobot( bool bShow=true ){ m_bShowRobot=bShow; }
    void ResetRobot();

    // get the robot's cfg. to be drawn in the toolbar
    //getRoboCfg() called from scene_win::mouseMoveEvent()
    void getRoboCfg(); 
    list<string> info;
    //getRobCfgText() called from roadmap::printRobCfg()
    list<string> getRobCfgText(){return info;} 
    int getNumJoints();

    PlumObject * GetRobot() const {  return m_obj.m_Robot; }

    /////////////////////////////////////////////////////////////////////
    // Bounding Box Related Function
    void ShowBBox(bool bShow=true);
    
    /////////////////////////////////////////////////////////////////////
    // Environment Related Functions

    // Change the appearance of an object - Hidden/ Wire / Soid
    void ChangeAppearance(int );   
    void RefreshEnv();
    void envObjsRandomColor();
    bool SaveEnv(const char *filename); //save env. file
    bool SaveQry(const char *filename); //save query file
    void SaveQryStart(); //save start cfg
    //void SaveQryGoal(); //save goal cfg.
    void SaveQryCfg(char ch); //save start or goal cfg.
    void DeleteObject(CMultiBodyModel * mbl);
    //called form main_win::autoMkmp()
    bool env_changed;
    bool envChanged();
    bool isEnvChanged() {return env_changed;}
    PlumObject * GetEnv() const { return m_obj.m_Env; }

    /////////////////////////////////////////////////////////////////////
    // Collision Detection Related Functions

    CollisionDetection CD;
    void TurnOn_CD();
    bool IsCDOn();
    CCfg * m_cfg;
    bool is_collison; //used in roadmap.cpp:printRobCfg()
    bool getCD_value(){ return is_collison;}//used in roadmap.cpp:printRobCfg()

    //void Node_CD(double *cfg);
    void vizmo::Node_CD(CCfg * cfg);
    double * m_nodeCfg;
    bool m_IsNode;

    //Miscelaneous

    bool StringToInt(const string &s, int &i);
    bool oneColor; 

      
    //////////////////////////////////////////////////////////////////////
    // Access

    vector<gliObj>& GetSelectedItem(){ return m_Plum.GetSelectedItem();}
    
    vector<PlumObject*>& GetPlumObjects(){ return m_Plum.GetPlumObjects(); }

    void cleanSelectedItem(){ m_Plum.CleanSelectedItem();}
    void addSelectedItem(CGLModel *ml){ m_Plum.addSelectedItem(ml);}

    double GetEnvRadius();

    /////////////////////////////////////
    /// To get command line user set 
    /// this information will be read
    // from main_win.cpp : getCommLine()
    //////////////////////////////////////

    void setCommLine(QStringList comm);
    QStringList getCommLine() {return m_commandList;}
    QStringList m_commandList;

    ////////////////////////////////////////////////////////////////
    // Variables used to change color of objects in the environment.
    float mR, mG, mB;

    /////////////////////////////////////////////////////////////////////
    // Filenames
    const string& getMapFileName() const { return m_obj.m_MapFile; }
    const string& getEnvFileName() const { return m_obj.m_EnvFile; }
    const string& getPathFileName() const { return m_obj.m_PathFile; }
    const string& getQryFileName() const { return m_obj.m_QryFile; }

    void setMapFileName(const string& name){ m_obj.m_MapFile=name; }
    void setEnvFileName(const string& name){ m_obj.m_EnvFile=name; }
    void setPathFileName(const string& name) { m_obj.m_PathFile=name; }
    void setQryFileName(const string& name){ m_obj.m_QryFile=name; }

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
    * Put robot in start configuration if possible
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
    //string FindName(const string& ext, const vector<string>& names)const;
    
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





