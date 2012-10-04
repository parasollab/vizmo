//////////////////////////////////////////////////////////////////////////////////////
// MapModel.h: interface for the CMapModel class.
// Many things that used to be in Roadmap.h/.cpp are now implemented directly in here
//////////////////////////////////////////////////////////////////////////////////////

#if !defined(_MAPMODEL_H_)
#define _MAPMODEL_H_

#include <math.h>
#include "Gauss.h"
#include "src/EnvObj/Robot.h"
#include "Plum/PlumObject.h" 

#include "MapLoader.h"
#include "CCModel.h"
#include "PlumState.h"
//#include "src/vizmo2.h" 
#include <algorithms/graph_algo_util.h>

using namespace stapl;
using namespace std;

namespace plum {

  template <class Cfg, class WEIGHT>
  class CMapModel : public CGLModel {
      
    private:
      typedef CCModel<Cfg,WEIGHT> myCCModel;
      typedef CMapLoader<Cfg,WEIGHT> Loader;
      typedef graph<DIRECTED,MULTIEDGES,Cfg,WEIGHT> Weg;
      typedef typename Weg::vertex_descriptor VID;
      typedef vector_property_map<Weg, size_t> color_map_t;
      color_map_t cmap;
    
    public:
      //////////////////////////////////////////////////////////////////////
      // Constructor/Destructor
      //////////////////////////////////////////////////////////////////////
      CMapModel();
      virtual ~CMapModel();

      //////////////////////////////////////////////////////////////////////
      // Access functions
      //////////////////////////////////////////////////////////////////////
      void SetMapLoader(Loader* mapLoader){m_mapLoader = mapLoader;}
      void SetRobotModel(OBPRMView_Robot* pRobot){m_pRobot = pRobot;}     
      vector<myCCModel*>& GetCCModels() {return m_CCModels;}
      list<CGLModel*>& GetNodeList() {return m_nodes;} 
      vector<CGLModel*>& GetNodesToConnect() {return m_nodesToConnect;} //prev. Node_Edge  
      //(Some other class besides QLabel*) GetCfgLabel() {return m_cfgLabel;}
      //("") GetRobCfgLabel() {return m_robCfgLabel;}   
      //("") GetMessageLabel() {return m_messageLabel;}
      //("") GetIconLabel() {return m_iconLabel;}   
      void SetMBEditModel(bool _setting) {m_bEditModel = _setting;} 
      void SetSize(double _size) {m_size = _size;}

      myCCModel* GetCCModel(int id) {return m_CCModels[id]; }
      int number_of_CC(){return m_CCModels.size();}

      bool 
      AddCC(int vid){
        
        //get graph
        if(m_mapLoader==NULL) 
          return false;
        typename Loader::Wg * graph = m_mapLoader->GetGraph();
        if(graph==NULL) 
          return false;
        myCCModel * cc=new myCCModel(m_CCModels.size());
        cc->RobotModel(m_pRobot);  
        cc->BuildModels(vid,graph); 
        float size;
        vector<float> color; 
        if(m_CCModels[m_CCModels.size()-1]->getShape() == 0)
          size = m_CCModels[m_CCModels.size()-1]->getRobotSize();
        else if(m_CCModels[m_CCModels.size()-1]->getShape() == 1)
          size = m_CCModels[m_CCModels.size()-1]->getBoxSize();
        else
          size = 0.0;

        color =  m_CCModels[m_CCModels.size()-1]->getColor();
        cc->change_properties(m_CCModels[m_CCModels.size()-1]->getShape(),size,
          color, true);
        m_CCModels.push_back(cc);

        return true;
      }

      //////////////////////////////////////////////////////////////////////
      // Action functions
      //////////////////////////////////////////////////////////////////////

      virtual bool BuildModels();
      virtual void Draw(GLenum mode);
      void Select(unsigned int* index, vector<gliObj>& sel);
      //set wire/solid/hide
      virtual void SetRenderMode(int mode);
      virtual const string GetName() const {return "Map";}

      virtual void 
      GetChildren(list<CGLModel*>& models){

        typedef typename vector<myCCModel*>::iterator CIT;
        for(CIT ic=m_CCModels.begin();ic!=m_CCModels.end();ic++ )
          models.push_back(*ic);
      }  

      void 
      SetProperties(typename myCCModel::Shape s, float size, 
        vector<float> color, bool isNew){
        
        typedef typename vector<myCCModel*>::iterator CIT;//CC iterator
        for(CIT ic=m_CCModels.begin(); ic!=m_CCModels.end(); ic++)
          (*ic)->change_properties(s, size, color, isNew);
      }

      virtual vector<string> GetInfo() const;
      //  void HandleSelect(); ORIGINALLY IN ROADMAP.H/.CPP  
      //  void HandleAddEdge();
      //  void HandleAddNode();
      //  void HandleEditMap();
      //  void MoveNode(); 

      bool m_robCfgOn; //Move to private...hopefully 
      bool m_addNode; 
      bool m_addEdge;

      double* m_cfg; 
      int m_dof; 

    private:

      OBPRMView_Robot* m_pRobot;
      vector<myCCModel*> m_CCModels;
      list<CGLModel*> m_nodes; //moved from obsolete Roadmap.h! 
      vector<CGLModel*> m_nodesToConnect; //nodes to be connected
      Loader* m_mapLoader;
      double m_oldT[3], m_oldR[3];  //old_T 
      double m_size;                //check purpose of this variable  
      //    bool m_robCfgOn;  ALSO FROM ROADMAP 
      //  (Some other class besides QLabel*) m_cfgLabel; //previously l_cfg 
      //  ("") m_robCfgLabel;  //l_robCfg
      //  ("") m_messageLabel;  //l_message or something
      //  ("") m_iconLabel; //l_icon
      string m_cfgString, m_robCfgString; //s_cfg, s_robCfg  
      bool m_bEditModel;
      bool m_noMap; //noMap

    protected:
      //virtual void DumpNode();
      //virtual void SelectNode( bool bSel );
  };

}//namespace plum

#endif // !defined(_MAPMODEL_H_)

