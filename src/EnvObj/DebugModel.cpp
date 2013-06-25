// DebugModel.cpp: implementation of the DebugModel class.
//
//////////////////////////////////////////////////////////////////////

#include "DebugModel.h"
#include "Robot.h"
#include "GUI/SceneWin.h" //to call new drawText 
#include <algorithms/dijkstra.h>

//////////////////////////////////////////////////////////////////////
// Include Plum headers
#include "Plum/Plum.h"
#include "Utilities/GL/gliFont.h"
using namespace stapl;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

struct my_edge_access_functor{

  typedef double value_type;
  template<typename Property>
  value_type get(Property p) {return p.GetWeight();}

  template<typename Property>
  void put(Property p, value_type _v) {p.GetWeight()=_v; }
  
};

DebugModel::DebugModel(){
  m_index=-1;
  m_debugLoader=NULL;
  m_mapLoader = new CMapLoader<Cfg, Edge>();
  m_mapLoader->InitGraph();
  m_robot=NULL;
  m_renderMode = INVISIBLE_MODE;
  m_mapModel=NULL;
  m_edgeNum=-1;
}

DebugModel::~DebugModel() {
  delete m_mapLoader;
  delete m_mapModel;
}

//////////////////////////////////////////////////////////////////////
// Action Methods
//////////////////////////////////////////////////////////////////////
bool
DebugModel::BuildModels(){
  //can't build model without model loader
  if( m_robot == NULL || m_debugLoader == NULL)
    return false;
  
  m_prevIndex = -1;
  m_index = 0;
  m_edgeNum = 0;
  m_tempRay = NULL;

  m_mapModel = new CMapModel<Cfg, Edge>();
  m_mapModel->SetMapLoader(m_mapLoader);
  m_mapModel->SetRobotModel(m_robot);
  m_mapModel->BuildModels();
  m_mapModel->SetRenderMode(SOLID_MODE);

  return true;
}

void 
DebugModel::BuildForward(){
  typedef graph<DIRECTED,MULTIEDGES,Cfg,Edge> WG;
  typedef WG::vertex_iterator VI;
  typedef WG::vertex_descriptor VID;
  typedef WG::adj_edge_iterator EI;
  typedef WG::edge_descriptor EID;
  typedef vector<VID>::iterator ITVID;
  typedef vector_property_map<WG, size_t> color_map_t;
  typedef edge_property_map<WG, my_edge_access_functor> edge_map_t;
  
  for(size_t i = m_prevIndex; i < (size_t)m_index; i++){
    Instruction* ins = m_debugLoader->ConfigureFrame(i);
    if(ins->name == "default"){
      //do nothing
    }
    else if(ins->name == "AddNode"){
      //add vertex specified by instruction to the graph
      AddNode* an = dynamic_cast<AddNode*>(ins);
      m_mapLoader->GetGraph()->add_vertex(an->cfg);
    }
    else if(ins->name == "AddEdge"){
      AddEdge* ae = dynamic_cast<AddEdge*>(ins);
      //add edge to the graph

      float color[3] = {0,0,0};
      vector<VID> ccnid1; //node id in this cc     
      vector<VID> ccnid2; //node id in this cc 
      vector<VID>* smaller_cc;
      Edge edge(1);
      
      VID svid = m_mapLoader->Cfg2VID(ae->source); 
      VID tvid = m_mapLoader->Cfg2VID(ae->target); 
      Cfg& source = m_mapLoader->GetGraph()->find_vertex(svid)->property();
      Cfg& target = m_mapLoader->GetGraph()->find_vertex(tvid)->property();
      
      //set properties of edge and increase the edge count
      edge.Set(m_edgeNum++, &source, &target);
      
      color_map_t cmap;
      cmap.reset();
      get_cc(*(m_mapLoader->GetGraph()), cmap, tvid, ccnid1);
      cmap.reset();
      get_cc(*(m_mapLoader->GetGraph()), cmap, svid, ccnid2);
     
      //store color of larger CC; will later color the smaller CC with this
      if(ccnid1.size() > ccnid2.size()){
        color[0] = target.GetColor()[0];
        color[1] = target.GetColor()[1];
        color[2] = target.GetColor()[2];
        smaller_cc = &ccnid2;
      }
      else{
        color[0] = source.GetColor()[0];
        color[1] = source.GetColor()[1];
        color[2] = source.GetColor()[2];
        smaller_cc = &ccnid1;
      }

      //store source and target colors in instruction;
      //will be restored when AddEdge is reversed in BuildBackward 
      ae->target_color[0] = target.GetColor()[0];
      ae->target_color[1] = target.GetColor()[1];
      ae->target_color[2] = target.GetColor()[2];

      ae->source_color[0] = source.GetColor()[0];
      ae->source_color[1] = source.GetColor()[1];
      ae->source_color[2] = source.GetColor()[2];

      //change color of smaller CC to that of larger CC
      for(ITVID it = (*smaller_cc).begin(); it != (*smaller_cc).end(); ++it) {
        VI vi = m_mapLoader->GetGraph()->find_vertex(*it);
        Cfg* cfgcc = &(vi->property());
        cfgcc->SetColor(color[0],color[1],color[2],1);
        
        for(EI ei = (*vi).begin(); ei != (*vi).end(); ++ei){
          (*ei).property().SetColor(color[0],color[1],color[2],1) ;
        }
      }
      
      //set color of new edge to that of larger cc
      edge.SetColor(color[0],color[1],color[2],1);

      //add edge
      m_mapLoader->GetGraph()->add_edge(svid, tvid,edge);
      m_mapLoader->GetGraph()->add_edge(tvid, svid,edge);
    }
    else if(ins->name == "AddTempCfg"){
      //add temporary cfg
      AddTempCfg* atc = dynamic_cast<AddTempCfg*>(ins);
      m_tempCfgs.push_back(atc->cfg);
      m_tempCfgs.back().Set(0,m_robot,NULL);
      m_tempCfgs.back().SetShape(Cfg::Robot);
      if(!atc->valid)
        m_tempCfgs.back().SetColor(1, 0, 0, 1);
    }
    else if(ins->name == "AddTempRay"){
      //add temporary ray
      AddTempRay* atr = dynamic_cast<AddTempRay*>(ins);
      m_tempRay = &atr->cfg;
    }
    else if(ins->name == "AddTempEdge"){
      //add temporary edge
      AddTempEdge* ate = dynamic_cast<AddTempEdge*>(ins);
      Edge e(1);
      e.Set(1, &ate->source, &ate->target);
      m_tempEdges.push_back(e);
    }
    else if(ins->name == "ClearLastTemp"){
      //clear last temporary cfg, edge, ray
      //and store them in the instruction to be restored in BuildBackward
      ClearLastTemp* clt = dynamic_cast<ClearLastTemp*>(ins);
      
      clt->tempRay = m_tempRay;
      m_tempRay = NULL;
      
      if(m_tempCfgs.size()>0) {
        clt->lastTempCfgs.push_back(m_tempCfgs.back());
        m_tempCfgs.pop_back();
      }
      if(m_tempEdges.size()>0) {
        clt->lastTempEdges.push_back(m_tempEdges.back());
        m_tempEdges.pop_back();
      }
    }
    else if(ins->name == "ClearAll"){
      //clear all temporary cfgs, edges, rays, comments, queries
      ClearAll* ca = dynamic_cast<ClearAll*>(ins);
      
      ca->tempRay = m_tempRay;
      m_tempRay = NULL;

      ca->tempCfgs = m_tempCfgs;
      m_tempCfgs.clear();

      ca->tempEdges = m_tempEdges;
      m_tempEdges.clear();

      ca->query = m_query;
      m_query.clear();

      ca->comments = m_comments;
      m_comments.clear();
    }
    else if(ins->name == "ClearComments"){
      //clear all comments
      ClearComments* cc = dynamic_cast<ClearComments*>(ins);
      cc->comments = m_comments;
      m_comments.clear();
    }
    else if(ins->name == "RemoveNode"){
      //remove an existing node
      RemoveNode* rn = dynamic_cast<RemoveNode*>(ins);
      VID xvid = m_mapLoader->Cfg2VID(rn->cfg); 
      m_mapLoader->GetGraph()->delete_vertex(xvid);
    }
    else if(ins->name == "RemoveEdge"){
      //remove an existing edge
      RemoveEdge* re = dynamic_cast<RemoveEdge*>(ins);
      VID svid = m_mapLoader->Cfg2VID(re->source); 
      VID tvid = m_mapLoader->Cfg2VID(re->target); 

      //store ID of edge being removed
      //to be restored in BuildBackward
      EID eid(svid, tvid);
      VI vi;
      EI ei;
      m_mapLoader->GetGraph()->find_edge(eid,vi,ei);
      re->edgeNum = (*ei).property().GetID();

      m_mapLoader->GetGraph()->delete_edge(svid, tvid);
      m_mapLoader->GetGraph()->delete_edge(tvid, svid);
    }
    else if(ins->name == "Comment"){
      //add comment
      Comment* c = dynamic_cast<Comment*>(ins);
      m_comments.push_back(c->comment);
    }
    else if(ins->name == "Query"){
      //perform query
      Query* q = dynamic_cast<Query*>(ins);
      vector<VID> path;

      VID svid = m_mapLoader->Cfg2VID(q->source);
      VID tvid = m_mapLoader->Cfg2VID(q->target);
      
      //clear existing query
      m_query.clear();

      //perform query using dijkstra's algorithm
      edge_map_t edge_map(*(m_mapLoader->GetGraph()));
      find_path_dijkstra(*(m_mapLoader->GetGraph()), edge_map, svid,tvid, path);
     
      //store edges of query
      Cfg source = q->source;
      Cfg target;
      if(path.size()>0){
        for(ITVID pit = path.begin()+1; pit != path.end(); pit++){
          target = m_mapLoader->GetGraph()->find_vertex(*pit)->property();
          Edge e(1);
          e.Set(1, &source, &target);
          m_query.push_back(e);
          source = target;
        }
      }
      q->query = m_query;
    }
  }

  //rebuild map model since graph may have changed
  m_mapModel->BuildModels();
  m_mapModel->SetRenderMode(SOLID_MODE);

}

void
DebugModel::BuildBackward(){
  typedef graph<DIRECTED,MULTIEDGES,Cfg,Edge> WG;
  typedef WG::vertex_iterator VI;
  typedef WG::vertex_descriptor VID;
  typedef WG::edge_descriptor EID;
  typedef WG::adj_edge_iterator EI;
  typedef vector_property_map<WG, size_t> color_map_t;
  typedef vector<VID>::iterator ITVID; 

  //traverse list of instructions BACKWARDS
  //perform reverse of the forward operation
  //e.g. for AddNode, remove the specified node instead of adding it
  for(size_t i = m_prevIndex; i > (size_t)m_index; i--){
    Instruction* ins = m_debugLoader->ConfigureFrame(i-1);
    if(ins->name == "default"){
      //do nothing
    }
    else if(ins->name == "AddNode"){
      //undo addition of specified node
      AddNode* an = dynamic_cast<AddNode*>(ins);
      VID xvid = m_mapLoader->Cfg2VID(an->cfg); 
      m_mapLoader->GetGraph()->delete_vertex(xvid);   
    }
    else if(ins->name == "AddEdge"){
      //undo addition of edge and restore original colors of CCs
      AddEdge* ae = dynamic_cast<AddEdge*>(ins);
      VID svid = m_mapLoader->Cfg2VID(ae->source); 
      VID tvid = m_mapLoader->Cfg2VID(ae->target); 
      float color[3] = {0,0,0};
     
      //remove both the forward and reverse edge
      m_mapLoader->GetGraph()->delete_edge(svid, tvid);
      m_mapLoader->GetGraph()->delete_edge(tvid, svid);

      vector<VID> ccnid1; //node id in this cc     
      vector<VID> ccnid2; //node id in this cc 
      color_map_t cmap;
      cmap.reset();
      get_cc(*(m_mapLoader->GetGraph()), cmap, svid, ccnid2);
      cmap.reset();
      get_cc(*(m_mapLoader->GetGraph()), cmap, tvid, ccnid1);
    
      //get color of the target cfg's CC
      //which was stored in the instruction
      color[0] = ae->target_color[0];
      color[1] = ae->target_color[1];
      color[2] = ae->target_color[2];
      
      //restore color of target's CC
      for(ITVID it = ccnid1.begin(); it != ccnid1.end(); ++it){
        VI vi = m_mapLoader->GetGraph()->find_vertex(*it);
        Cfg* cfgcc = &(vi->property());
        cfgcc->SetColor(color[0],color[1],color[2],1);

        for(EI ei = (*vi).begin(); ei != (*vi).end(); ++ei){
          (*ei).property().SetColor(color[0],color[1],color[2],1) ;
        }
      }
     
      //get color of source cfg's CC
      //which was stored in the instruction
      color[0] = ae->source_color[0];
      color[1] = ae->source_color[1];
      color[2] = ae->source_color[2];
     
      //restore color of source's CC
      for(ITVID it = ccnid2.begin(); it != ccnid2.end(); ++it){
        VI vi = m_mapLoader->GetGraph()->find_vertex(*it);
        Cfg* cfgcc = &(vi->property());
        cfgcc->SetColor(color[0],color[1],color[2],1);
        
        for(EI ei = (*vi).begin(); ei != (*vi).end(); ++ei){
          (*ei).property().SetColor(color[0],color[1],color[2],1) ;
        }
      }

      //undo increment of edge number
      m_edgeNum--;
    }
    else if(ins->name == "AddTempCfg"){
      //undo addition of temp cfg
      m_tempCfgs.pop_back();
    }
    else if(ins->name == "AddTempRay"){
      //undo addition of temp ray
      m_tempRay = NULL;
    }
    else if(ins->name == "AddTempEdge"){
      //undo addiion of temp edge
      m_tempEdges.pop_back();
    }
    else if(ins->name == "ClearLastTemp"){
      //undo clearing of last temp cfg, edge, ray
      //by restoring them from the instruction where they were stored
      ClearLastTemp* clt = dynamic_cast<ClearLastTemp*>(ins);
      m_tempRay = clt->tempRay;
      
      while(clt->lastTempCfgs.size()>0) {
        m_tempCfgs.push_back(clt->lastTempCfgs.back());
        clt->lastTempCfgs.pop_back();
      }
      
      while(clt->lastTempEdges.size()>0) {
        m_tempEdges.push_back(clt->lastTempEdges.back());
        clt->lastTempEdges.pop_back();
      }
    }
    else if(ins->name == "ClearAll"){
      //undo clearing of all temp cfgs, edges, queries, comments
      //by restoring them from the instruction where they were stored
      ClearAll* ca = dynamic_cast<ClearAll*>(ins);
      
      m_tempRay = ca->tempRay;
      ca->tempRay = NULL;
      
      m_tempCfgs = ca->tempCfgs;
      ca->tempCfgs.clear();
      
      m_tempEdges = ca->tempEdges;
      ca->tempEdges.clear();
      
      m_query = ca->query;
      ca->query.clear();
      
      m_comments = ca->comments;
      ca->comments.clear();
    }
    else if(ins->name == "ClearComments"){
      //undo clearing of commments
      ClearComments* cc =dynamic_cast<ClearComments*>(ins);
      m_comments = cc->comments;
      cc->comments.clear();
    }
    else if(ins->name == "RemoveNode"){
      //undo removal of node
      RemoveNode* rn = dynamic_cast<RemoveNode*>(ins);
      m_mapLoader->GetGraph()->add_vertex(rn->cfg);
    }
    else if(ins->name == "RemoveEdge"){
      //undo removal of edge
      RemoveEdge* re = dynamic_cast<RemoveEdge*>(ins);
      Edge edge(1);
      VID svid = m_mapLoader->Cfg2VID(re->source); 
      VID tvid = m_mapLoader->Cfg2VID(re->target);

      //restore edge number of edge
      edge.Set(re->edgeNum, &re->source, &re->target);
      m_mapLoader->GetGraph()->add_edge(svid, tvid,edge);
      m_mapLoader->GetGraph()->add_edge(tvid, svid,edge);
    }
    else if(ins->name == "Comment"){
      //undo addition of comment
      m_comments.pop_back();
    }
    else if(ins->name == "Query"){
      //undo addition of query and restore the previous one if any
      Query* q = dynamic_cast<Query*>(ins);
      m_query = q->query;
      q->query.clear();
    }
  }

  //rebuild map model since graph may have changed
  m_mapModel->BuildModels();
  m_mapModel->SetRenderMode(SOLID_MODE);

}

void
DebugModel::Draw(GLenum _mode){

  typedef vector<Cfg>::iterator CIT;
  typedef vector<Edge>::iterator EIT;
  
  //update the map model in either the forward or backward direction
  //depending on the indices of the previous and current frames
  //(do nothing if the indices are the same)
  if(m_index > m_prevIndex)
    BuildForward();
  else if(m_index < m_prevIndex)
    BuildBackward();

  //update the index of the previous frame
  m_prevIndex = m_index;

  if(m_index){
    
    m_mapModel->Draw(_mode);
    
    for(CIT cit = m_tempCfgs.begin(); cit!=m_tempCfgs.end(); cit++){
      cit->Draw(_mode);
    }
    for(EIT eit = m_tempEdges.begin(); eit!=m_tempEdges.end(); eit++){
      eit->Draw(_mode);
    }
    for(EIT eit = m_query.begin(); eit!=m_query.end(); eit++){
      glLineWidth(32);
      eit->SetColor(1,1,0,1);
      eit->Draw(_mode);
    }
    if(m_tempRay!=NULL && m_tempCfgs.size()>0){
      Edge edge;
      Cfg ray = *m_tempRay;
      vector<double> cfg = ray.GetDataCfg(); 
      Cfg* tmp = &(m_tempCfgs.back());
      cfg[0]+=tmp->tx();
      cfg[1]+=tmp->ty();
      cfg[2]+=tmp->tz();
      ray.SetCfg(cfg);
      edge.Set(0, tmp, &ray);
      edge.SetColor(1,1,0,1);
      glLineWidth(8);
      edge.Draw(_mode);
    }
  }
}

void 
DebugModel::ConfigureFrame(int _frame){
  m_index = _frame+1;
}

vector<string> 
DebugModel::GetInfo() const { 
  vector<string> info; 
  info.push_back(m_debugLoader->GetFilename());
  {
    ostringstream temp;
    temp<<"There are "<<m_debugLoader->GetDebugSize()<<" debug frames";
    info.push_back(temp.str());
  } 
  return info;
}

vector<string>    
DebugModel::GetComments(){
  if(m_index != -1)
    return m_comments;
  else{
    vector<string> tmp;
    return tmp;
  }
} 

