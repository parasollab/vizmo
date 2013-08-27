#include "DebugModel.h"

#include <algorithms/dijkstra.h>
using namespace stapl;

#include "EnvObj/RobotModel.h"
#include "GUI/SceneWin.h" //to call new drawText

struct EdgeAccess {
  typedef double value_type;
  template<typename Property>
    value_type get(Property& p) {return p.GetWeight();}

  template<typename Property>
    void put(Property& p, value_type _v) {p.GetWeight()=_v;}
};

DebugModel::DebugModel(const string& _filename, RobotModel* _robotModel) :
  m_robotModel(_robotModel), m_index(-1),
  m_mapModel(new MapModel<CfgModel, EdgeModel>(_robotModel)),
  m_edgeNum(-1) {
    SetFilename(_filename);
    m_renderMode = INVISIBLE_MODE;
    m_mapModel->InitGraph();
    ParseFile();
    BuildModels();
  }

DebugModel::~DebugModel() {
  delete m_mapModel;
  typedef vector<Instruction*>::iterator IIT;
  for(IIT iit = m_instructions.begin(); iit != m_instructions.end(); ++iit)
    delete *iit;
}

vector<string>
DebugModel::GetInfo() const {
  vector<string> info;
  info.push_back(GetFilename());
  ostringstream temp;
  temp << "There are " << m_instructions.size() << " debug frames";
  info.push_back(temp.str());
  return info;
}

vector<string>
DebugModel::GetComments(){
  return m_index == -1 ? vector<string>() : m_comments;
}

void
DebugModel::ParseFile() {
  //check if file exists
  if(!FileExists(GetFilename()))
    throw ParseException(WHERE, "'" + GetFilename() + "' does not exist");

  ifstream ifs(GetFilename().c_str());

  typedef vector<Instruction*>::iterator IIT;
  for(IIT iit = m_instructions.begin(); iit != m_instructions.end(); ++iit)
    delete *iit;
  m_instructions.clear();

  m_instructions.push_back(new Instruction());

  string line;
  while(getline(ifs,line)) {

    istringstream iss(line);
    string name;
    iss>>name;

    if(name=="AddNode"){
      CfgModel c;
      iss >> c;
      c.SetColor(Color4(drand48(), drand48(), drand48(), 1));
      m_instructions.push_back(new AddNode(c));
    }
    else if(name=="AddEdge"){
      CfgModel s, t;
      iss >> s >> t;
      m_instructions.push_back(new AddEdge(s, t));
    }
    else if(name=="AddTempCfg"){
      CfgModel c;
      bool valid;
      iss >> c >> valid;
      if(valid) c.SetColor(Color4(0, 0, 0, 0.25));
      else c.SetColor(Color4(1, 0, 0, 0.25));
      m_instructions.push_back(new AddTempCfg(c, valid));
    }
    else if(name=="AddTempRay"){
      CfgModel c;
      iss >> c;
      m_instructions.push_back(new AddTempRay(c));
    }
    else if(name=="AddTempEdge"){
      CfgModel s, t;
      iss >> s >> t;
      m_instructions.push_back(new AddTempEdge(s, t));
    }
    else if(name=="ClearAll"){
      m_instructions.push_back(new ClearAll());
    }
    else if(name=="ClearLastTemp"){
      m_instructions.push_back(new ClearLastTemp());
    }
    else if(name=="ClearComments"){
      m_instructions.push_back(new ClearComments());
    }
    else if(name=="RemoveNode"){
      CfgModel c;
      iss >> c;
      m_instructions.push_back(new RemoveNode(c));
    }
    else if(name=="RemoveEdge"){
      CfgModel s, t;
      iss >> s >> t;
      m_instructions.push_back(new RemoveEdge(s, t));
    }
    else if(name=="Comment"){
      m_instructions.push_back(new Comment(iss.str().substr(8, iss.str().length())));
    }
    else if(name=="Query"){
      CfgModel s, t;
      iss >> s >> t;
      m_instructions.push_back(new Query(s, t));
    }
  }
}

void
DebugModel::BuildModels(){
  //can't build model without robot
  if(!m_robotModel)
    throw BuildException(WHERE, "RobotModel is null.");

  m_prevIndex = 0;
  m_index = 0;
  m_edgeNum = 0;
  m_tempRay = NULL;

  m_mapModel->BuildModels();
  m_mapModel->SetRenderMode(SOLID_MODE);
}

void
DebugModel::BuildForward() {
  typedef graph<DIRECTED,MULTIEDGES,CfgModel,EdgeModel> WG;
  typedef WG::vertex_iterator VI;
  typedef WG::vertex_descriptor VID;
  typedef WG::adj_edge_iterator EI;
  typedef WG::edge_descriptor EID;
  typedef vector<VID>::iterator ITVID;
  typedef vector_property_map<WG, size_t> ColorMap;
  typedef edge_property_map<WG, EdgeAccess> EdgeMap;

  for(int i = m_prevIndex; i < m_index; i++){
    Instruction* ins = m_instructions[i];
    if(ins->m_name == "AddNode") {
      //add vertex specified by instruction to the graph
      AddNode* an = static_cast<AddNode*>(ins);
      m_mapModel->GetGraph()->add_vertex(an->m_cfg);
    }
    else if(ins->m_name == "AddEdge"){
      AddEdge* ae = static_cast<AddEdge*>(ins);
      //add edge to the graph

      Color4 color;
      vector<VID> ccnid1; //node id in this cc
      vector<VID> ccnid2; //node id in this cc
      vector<VID>* smallerCC;
      EdgeModel edge(1);

      VID svid = m_mapModel->Cfg2VID(ae->m_source);
      VID tvid = m_mapModel->Cfg2VID(ae->m_target);
      CfgModel& source = m_mapModel->GetGraph()->find_vertex(svid)->property();
      CfgModel& target = m_mapModel->GetGraph()->find_vertex(tvid)->property();

      //set properties of edge and increase the edge count
      edge.Set(m_edgeNum++, &source, &target);

      ColorMap cmap;
      cmap.reset();
      get_cc(*(m_mapModel->GetGraph()), cmap, tvid, ccnid1);
      cmap.reset();
      get_cc(*(m_mapModel->GetGraph()), cmap, svid, ccnid2);

      //store color of larger CC; will later color the smaller CC with this
      if(ccnid1.size() > ccnid2.size()){
        color = target.GetColor();
        smallerCC = &ccnid2;
      }
      else{
        color = source.GetColor();
        smallerCC = &ccnid1;
      }

      //store source and target colors in instruction;
      //will be restored when AddEdge is reversed in BuildBackward
      ae->m_targetColor = target.GetColor();
      ae->m_sourceColor = source.GetColor();

      //change color of smaller CC to that of larger CC
      for(ITVID it = smallerCC->begin(); it != smallerCC->end(); ++it) {
        VI vi = m_mapModel->GetGraph()->find_vertex(*it);
        CfgModel* cfgcc = &(vi->property());
        cfgcc->SetColor(color);

        for(EI ei = (*vi).begin(); ei != (*vi).end(); ++ei){
          (*ei).property().SetColor(color) ;
        }
      }

      //set color of new edge to that of larger cc
      edge.SetColor(color);

      //add edge
      m_mapModel->GetGraph()->add_edge(svid, tvid, edge);
      m_mapModel->GetGraph()->add_edge(tvid, svid, edge);
    }
    else if(ins->m_name == "AddTempCfg"){
      //add temporary cfg
      AddTempCfg* atc = static_cast<AddTempCfg*>(ins);
      m_tempCfgs.push_back(atc->m_cfg);
      m_tempCfgs.back().Set(0, m_robotModel, NULL);
      m_tempCfgs.back().SetShape(CfgModel::Robot);
      if(!atc->m_valid)
        m_tempCfgs.back().SetColor(Color4(1, 0, 0, 1));
    }
    else if(ins->m_name == "AddTempRay"){
      //add temporary ray
      AddTempRay* atr = static_cast<AddTempRay*>(ins);
      m_tempRay = &atr->m_cfg;
    }
    else if(ins->m_name == "AddTempEdge"){
      //add temporary edge
      AddTempEdge* ate = static_cast<AddTempEdge*>(ins);
      EdgeModel e(1);
      e.Set(1, &ate->m_source, &ate->m_target);
      m_tempEdges.push_back(e);
    }
    else if(ins->m_name == "ClearLastTemp"){
      //clear last temporary cfg, edge, ray
      //and store them in the instruction to be restored in BuildBackward
      ClearLastTemp* clt = static_cast<ClearLastTemp*>(ins);

      clt->m_tempRay = m_tempRay;
      m_tempRay = NULL;

      if(m_tempCfgs.size()>0) {
        clt->m_lastTempCfgs.push_back(m_tempCfgs.back());
        m_tempCfgs.pop_back();
      }
      if(m_tempEdges.size()>0) {
        clt->m_lastTempEdges.push_back(m_tempEdges.back());
        m_tempEdges.pop_back();
      }
    }
    else if(ins->m_name == "ClearAll"){
      //clear all temporary cfgs, edges, rays, comments, queries
      ClearAll* ca = static_cast<ClearAll*>(ins);

      ca->m_tempRay = m_tempRay;
      m_tempRay = NULL;

      ca->m_tempCfgs = m_tempCfgs;
      m_tempCfgs.clear();

      ca->m_tempEdges = m_tempEdges;
      m_tempEdges.clear();

      ca->m_query = m_query;
      m_query.clear();

      ca->m_comments = m_comments;
      m_comments.clear();
    }
    else if(ins->m_name == "ClearComments"){
      //clear all comments
      ClearComments* cc = static_cast<ClearComments*>(ins);
      cc->m_comments = m_comments;
      m_comments.clear();
    }
    else if(ins->m_name == "RemoveNode"){
      //remove an existing node
      RemoveNode* rn = static_cast<RemoveNode*>(ins);
      VID xvid = m_mapModel->Cfg2VID(rn->m_cfg);
      m_mapModel->GetGraph()->delete_vertex(xvid);
    }
    else if(ins->m_name == "RemoveEdge"){
      //remove an existing edge
      RemoveEdge* re = static_cast<RemoveEdge*>(ins);
      VID svid = m_mapModel->Cfg2VID(re->m_source);
      VID tvid = m_mapModel->Cfg2VID(re->m_target);

      //store ID of edge being removed
      //to be restored in BuildBackward
      EID eid(svid, tvid);
      VI vi;
      EI ei;
      m_mapModel->GetGraph()->find_edge(eid,vi,ei);
      re->m_edgeNum = (*ei).property().GetID();

      m_mapModel->GetGraph()->delete_edge(svid, tvid);
      m_mapModel->GetGraph()->delete_edge(tvid, svid);
    }
    else if(ins->m_name == "Comment"){
      //add comment
      Comment* c = static_cast<Comment*>(ins);
      m_comments.push_back(c->m_comment);
    }
    else if(ins->m_name == "Query"){
      //perform query
      Query* q = static_cast<Query*>(ins);
      vector<VID> path;

      VID svid = m_mapModel->Cfg2VID(q->m_source);
      VID tvid = m_mapModel->Cfg2VID(q->m_target);

      //clear existing query
      m_query.clear();

      //perform query using dijkstra's algorithm
      EdgeMap edgeMap(*(m_mapModel->GetGraph()));
      find_path_dijkstra(*(m_mapModel->GetGraph()), edgeMap, svid, tvid, path);

      //store edges of query
      CfgModel source = q->m_source;
      CfgModel target;
      if(path.size()>0){
        for(ITVID pit = path.begin()+1; pit != path.end(); pit++){
          target = m_mapModel->GetGraph()->find_vertex(*pit)->property();
          EdgeModel e(1);
          e.Set(1, &source, &target);
          m_query.push_back(e);
          source = target;
        }
      }
      q->m_query = m_query;
    }
  }

  //rebuild map model since graph may have changed
  m_mapModel->BuildModels();
  m_mapModel->SetRenderMode(SOLID_MODE);
}

void
DebugModel::BuildBackward(){
  typedef graph<DIRECTED,MULTIEDGES,CfgModel,EdgeModel> WG;
  typedef WG::vertex_iterator VI;
  typedef WG::vertex_descriptor VID;
  typedef WG::edge_descriptor EID;
  typedef WG::adj_edge_iterator EI;
  typedef vector_property_map<WG, size_t> ColorMap;
  typedef vector<VID>::iterator ITVID;

  //traverse list of instructions BACKWARDS
  //perform reverse of the forward operation
  //e.g. for AddNode, remove the specified node instead of adding it
  for(size_t i = m_prevIndex; i > (size_t)m_index; i--){
    Instruction* ins = m_instructions[i-1];
    if(ins->m_name == "AddNode"){
      //undo addition of specified node
      AddNode* an = static_cast<AddNode*>(ins);
      VID xvid = m_mapModel->Cfg2VID(an->m_cfg);
      m_mapModel->GetGraph()->delete_vertex(xvid);
    }
    else if(ins->m_name == "AddEdge"){
      //undo addition of edge and restore original colors of CCs
      AddEdge* ae = static_cast<AddEdge*>(ins);
      VID svid = m_mapModel->Cfg2VID(ae->m_source);
      VID tvid = m_mapModel->Cfg2VID(ae->m_target);
      Color4 color;

      //remove both the forward and reverse edge
      m_mapModel->GetGraph()->delete_edge(svid, tvid);
      m_mapModel->GetGraph()->delete_edge(tvid, svid);

      vector<VID> ccnid1; //node id in this cc
      vector<VID> ccnid2; //node id in this cc
      ColorMap cmap;
      cmap.reset();
      get_cc(*(m_mapModel->GetGraph()), cmap, svid, ccnid2);
      cmap.reset();
      get_cc(*(m_mapModel->GetGraph()), cmap, tvid, ccnid1);

      //get color of the target cfg's CC
      //which was stored in the instruction
      color = ae->m_targetColor;

      //restore color of target's CC
      for(ITVID it = ccnid1.begin(); it != ccnid1.end(); ++it){
        VI vi = m_mapModel->GetGraph()->find_vertex(*it);
        CfgModel* cfgcc = &(vi->property());
        cfgcc->SetColor(color);

        for(EI ei = (*vi).begin(); ei != (*vi).end(); ++ei){
          (*ei).property().SetColor(color) ;
        }
      }

      //get color of source cfg's CC
      //which was stored in the instruction
      color = ae->m_sourceColor;

      //restore color of source's CC
      for(ITVID it = ccnid2.begin(); it != ccnid2.end(); ++it){
        VI vi = m_mapModel->GetGraph()->find_vertex(*it);
        CfgModel* cfgcc = &(vi->property());
        cfgcc->SetColor(color);

        for(EI ei = (*vi).begin(); ei != (*vi).end(); ++ei){
          (*ei).property().SetColor(color) ;
        }
      }

      //undo increment of edge number
      m_edgeNum--;
    }
    else if(ins->m_name == "AddTempCfg"){
      //undo addition of temp cfg
      m_tempCfgs.pop_back();
    }
    else if(ins->m_name == "AddTempRay"){
      //undo addition of temp ray
      m_tempRay = NULL;
    }
    else if(ins->m_name == "AddTempEdge"){
      //undo addiion of temp edge
      m_tempEdges.pop_back();
    }
    else if(ins->m_name == "ClearLastTemp"){
      //undo clearing of last temp cfg, edge, ray
      //by restoring them from the instruction where they were stored
      ClearLastTemp* clt = static_cast<ClearLastTemp*>(ins);
      m_tempRay = clt->m_tempRay;

      while(clt->m_lastTempCfgs.size()>0) {
        m_tempCfgs.push_back(clt->m_lastTempCfgs.back());
        clt->m_lastTempCfgs.pop_back();
      }

      while(clt->m_lastTempEdges.size()>0) {
        m_tempEdges.push_back(clt->m_lastTempEdges.back());
        clt->m_lastTempEdges.pop_back();
      }
    }
    else if(ins->m_name == "ClearAll"){
      //undo clearing of all temp cfgs, edges, queries, comments
      //by restoring them from the instruction where they were stored
      ClearAll* ca = static_cast<ClearAll*>(ins);

      m_tempRay = ca->m_tempRay;
      ca->m_tempRay = NULL;

      m_tempCfgs = ca->m_tempCfgs;
      ca->m_tempCfgs.clear();

      m_tempEdges = ca->m_tempEdges;
      ca->m_tempEdges.clear();

      m_query = ca->m_query;
      ca->m_query.clear();

      m_comments = ca->m_comments;
      ca->m_comments.clear();
    }
    else if(ins->m_name == "ClearComments"){
      //undo clearing of commments
      ClearComments* cc =static_cast<ClearComments*>(ins);
      m_comments = cc->m_comments;
      cc->m_comments.clear();
    }
    else if(ins->m_name == "RemoveNode"){
      //undo removal of node
      RemoveNode* rn = static_cast<RemoveNode*>(ins);
      m_mapModel->GetGraph()->add_vertex(rn->m_cfg);
    }
    else if(ins->m_name == "RemoveEdge"){
      //undo removal of edge
      RemoveEdge* re = static_cast<RemoveEdge*>(ins);
      EdgeModel edge(1);
      VID svid = m_mapModel->Cfg2VID(re->m_source);
      VID tvid = m_mapModel->Cfg2VID(re->m_target);

      //restore edge number of edge
      edge.Set(re->m_edgeNum, &re->m_source, &re->m_target);
      m_mapModel->GetGraph()->add_edge(svid, tvid, edge);
      m_mapModel->GetGraph()->add_edge(tvid, svid, edge);
    }
    else if(ins->m_name == "Comment"){
      //undo addition of comment
      m_comments.pop_back();
    }
    else if(ins->m_name == "Query"){
      //undo addition of query and restore the previous one if any
      Query* q = static_cast<Query*>(ins);
      m_query = q->m_query;
      q->m_query.clear();
    }
  }

  //rebuild map model since graph may have changed
  m_mapModel->BuildModels();
  m_mapModel->SetRenderMode(SOLID_MODE);
}

void
DebugModel::Draw(GLenum _mode){

  typedef vector<CfgModel>::iterator CIT;
  typedef vector<EdgeModel>::iterator EIT;

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
    for(CIT cit = m_tempCfgs.begin(); cit!=m_tempCfgs.end(); cit++)
      cit->Draw(_mode);
    for(EIT eit = m_tempEdges.begin(); eit!=m_tempEdges.end(); eit++)
      eit->Draw(_mode);
    for(EIT eit = m_query.begin(); eit!=m_query.end(); eit++) {
      glLineWidth(32);
      eit->SetColor(Color4(1, 1, 0, 1));
      eit->Draw(_mode);
    }
    if(m_tempRay != NULL && m_tempCfgs.size() > 0) {
      EdgeModel edge;
      CfgModel ray = *m_tempRay;
      vector<double> cfg = ray.GetDataCfg();
      CfgModel* tmp = &(m_tempCfgs.back());
      cfg[0]+=tmp->Translation()[0];
      cfg[1]+=tmp->Translation()[1];
      cfg[2]+=tmp->Translation()[2];
      ray.SetCfg(cfg);
      edge.Set(0, tmp, &ray);
      edge.SetColor(Color4(1, 1, 0, 1));
      glLineWidth(8);
      edge.Draw(_mode);
    }
  }
}

void
DebugModel::ConfigureFrame(int _frame){
  m_index = _frame+1;
}
