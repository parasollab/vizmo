#include "DebugModel.h"

DebugModel::DebugModel(const string& _filename) :
  LoadableModel("Debug"),
  m_index(-1),
  m_mapModel(new MapModel<CfgModel, EdgeModel>()),
  m_edgeNum(-1) {
    SetFilename(_filename);
    m_renderMode = INVISIBLE_MODE;
    ParseFile();
    Build();
  }

DebugModel::~DebugModel() {
  delete m_mapModel;
  typedef vector<Instruction*>::iterator IIT;
  for(IIT iit = m_instructions.begin(); iit != m_instructions.end(); ++iit)
    delete *iit;
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
    else if(name=="ClearAll")
      m_instructions.push_back(new ClearAll());
    else if(name=="ClearLastTemp")
      m_instructions.push_back(new ClearLastTemp());
    else if(name=="ClearComments")
      m_instructions.push_back(new ClearComments());
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
    else if(name=="Comment")
      m_instructions.push_back(new Comment(iss.str().substr(8, iss.str().length())));
    else if(name=="QueryInstruction"){
      CfgModel s, t;
      iss >> s >> t;
      m_instructions.push_back(new QueryInstruction(s, t));
    }
  }
}

void
DebugModel::Build(){
  m_prevIndex = 0;
  m_index = 0;
  m_edgeNum = 0;
  m_tempRay = NULL;

  m_mapModel->Build();
  m_mapModel->SetRenderMode(SOLID_MODE);
}

void
DebugModel::BuildForward() {
  typedef MapModel<CfgModel, EdgeModel> MM;
  typedef MM::VID VID;
  typedef MM::VI VI;
  typedef MM::EID EID;
  typedef MM::EI EI;
  typedef vector<VID>::iterator ITVID;
  typedef MM::ColorMap ColorMap;
  typedef MM::EdgeMap EdgeMap;

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
      vector<VID> nIdCC1; //node id in this cc
      vector<VID> nIdCC2; //node id in this cc
      vector<VID>* smallerCC;
      EdgeModel edge("", 1);

      VID svId = m_mapModel->Cfg2VID(ae->m_source);
      VID tvId = m_mapModel->Cfg2VID(ae->m_target);
      CfgModel& source = m_mapModel->GetGraph()->find_vertex(svId)->property();
      CfgModel& target = m_mapModel->GetGraph()->find_vertex(tvId)->property();

      //set properties of edge and increase the edge count
      edge.Set(m_edgeNum++, &source, &target);

      ColorMap cMap;
      cMap.reset();
      get_cc(*(m_mapModel->GetGraph()), cMap, tvId, nIdCC1);
      cMap.reset();
      get_cc(*(m_mapModel->GetGraph()), cMap, svId, nIdCC2);

      //store color of larger CC; will later color the smaller CC with this
      if(nIdCC1.size() > nIdCC2.size()){
        color = target.GetColor();
        smallerCC = &nIdCC2;
      }
      else{
        color = source.GetColor();
        smallerCC = &nIdCC1;
      }

      //store source and target colors in instruction;
      //will be restored when AddEdge is reversed in BuildBackward
      ae->m_targetColor = target.GetColor();
      ae->m_sourceColor = source.GetColor();

      //change color of smaller CC to that of larger CC
      for(ITVID it = smallerCC->begin(); it != smallerCC->end(); ++it) {
        VI vi = m_mapModel->GetGraph()->find_vertex(*it);
        CfgModel* cfgCC = &(vi->property());
        cfgCC->SetColor(color);

        for(EI ei = (*vi).begin(); ei != (*vi).end(); ++ei){
          (*ei).property().SetColor(color) ;
        }
      }

      //set color of new edge to that of larger cc
      edge.SetColor(color);

      //add edge
      m_mapModel->GetGraph()->add_edge(svId, tvId, edge);
      m_mapModel->GetGraph()->add_edge(tvId, svId, edge);
    }
    else if(ins->m_name == "AddTempCfg"){
      //add temporary cfg
      AddTempCfg* atc = static_cast<AddTempCfg*>(ins);
      m_tempCfgs.push_back(atc->m_cfg);
      m_tempCfgs.back().Set(0, NULL);
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
      EdgeModel e("", 1);
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
      VID xvId = m_mapModel->Cfg2VID(rn->m_cfg);
      m_mapModel->GetGraph()->delete_vertex(xvId);
    }
    else if(ins->m_name == "RemoveEdge"){
      //remove an existing edge
      RemoveEdge* re = static_cast<RemoveEdge*>(ins);
      VID svId = m_mapModel->Cfg2VID(re->m_source);
      VID tvId = m_mapModel->Cfg2VID(re->m_target);

      //store ID of edge being removed
      //to be restored in BuildBackward
      EID eid(svId, tvId);
      VI vi;
      EI ei;
      m_mapModel->GetGraph()->find_edge(eid,vi,ei);
      re->m_edgeNum = (*ei).property().GetID();

      m_mapModel->GetGraph()->delete_edge(svId, tvId);
      m_mapModel->GetGraph()->delete_edge(tvId, svId);
    }
    else if(ins->m_name == "Comment"){
      //add comment
      Comment* c = static_cast<Comment*>(ins);
      m_comments.push_back(c->m_comment);
    }
    else if(ins->m_name == "QueryInstruction"){
      //perform query
      QueryInstruction* q = static_cast<QueryInstruction*>(ins);
      vector<VID> path;

      VID svId = m_mapModel->Cfg2VID(q->m_source);
      VID tvId = m_mapModel->Cfg2VID(q->m_target);

      //clear existing query
      m_query.clear();

      //perform query using dijkstra's algorithm
      EdgeMap edgeMap(*(m_mapModel->GetGraph()));
      find_path_dijkstra(*(m_mapModel->GetGraph()), edgeMap, svId, tvId, path);

      //store edges of query
      CfgModel source = q->m_source;
      CfgModel target;
      if(path.size()>0){
        for(ITVID pit = path.begin()+1; pit != path.end(); pit++){
          target = m_mapModel->GetGraph()->find_vertex(*pit)->property();
          EdgeModel e("", 1);
          e.Set(1, &source, &target);
          m_query.push_back(e);
          source = target;
        }
      }
      q->m_query = m_query;
    }
  }

  //rebuild map model since graph may have changed
  m_mapModel->Build();
  m_mapModel->SetRenderMode(SOLID_MODE);
}

void
DebugModel::BuildBackward(){
  typedef MapModel<CfgModel, EdgeModel> MM;
  typedef MM::VID VID;
  typedef MM::VI VI;
  typedef MM::EID EID;
  typedef MM::EI EI;
  typedef MM::ColorMap ColorMap;
  typedef vector<VID>::iterator ITVID;

  //traverse list of instructions BACKWARDS
  //perform reverse of the forward operation
  //e.g. for AddNode, remove the specified node instead of adding it
  for(size_t i = m_prevIndex; i > (size_t)m_index; i--){
    Instruction* ins = m_instructions[i-1];
    if(ins->m_name == "AddNode"){
      //undo addition of specified node
      AddNode* an = static_cast<AddNode*>(ins);
      VID xvId = m_mapModel->Cfg2VID(an->m_cfg);
      m_mapModel->GetGraph()->delete_vertex(xvId);
    }
    else if(ins->m_name == "AddEdge"){
      //undo addition of edge and restore original colors of CCs
      AddEdge* ae = static_cast<AddEdge*>(ins);
      VID svId = m_mapModel->Cfg2VID(ae->m_source);
      VID tvId = m_mapModel->Cfg2VID(ae->m_target);
      Color4 color;

      //remove both the forward and reverse edge
      m_mapModel->GetGraph()->delete_edge(svId, tvId);
      m_mapModel->GetGraph()->delete_edge(tvId, svId);

      vector<VID> nIdCC1; //node id in this cc
      vector<VID> nIdCC2; //node id in this cc
      ColorMap cMap;
      cMap.reset();
      get_cc(*(m_mapModel->GetGraph()), cMap, svId, nIdCC2);
      cMap.reset();
      get_cc(*(m_mapModel->GetGraph()), cMap, tvId, nIdCC1);

      //get color of the target cfg's CC
      //which was stored in the instruction
      color = ae->m_targetColor;

      //restore color of target's CC
      for(ITVID it = nIdCC1.begin(); it != nIdCC1.end(); ++it){
        VI vi = m_mapModel->GetGraph()->find_vertex(*it);
        CfgModel* cfgCC = &(vi->property());
        cfgCC->SetColor(color);

        for(EI ei = (*vi).begin(); ei != (*vi).end(); ++ei){
          (*ei).property().SetColor(color) ;
        }
      }

      //get color of source cfg's CC
      //which was stored in the instruction
      color = ae->m_sourceColor;

      //restore color of source's CC
      for(ITVID it = nIdCC2.begin(); it != nIdCC2.end(); ++it){
        VI vi = m_mapModel->GetGraph()->find_vertex(*it);
        CfgModel* cfgCC = &(vi->property());
        cfgCC->SetColor(color);

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
      EdgeModel edge("", 1);
      VID svId = m_mapModel->Cfg2VID(re->m_source);
      VID tvId = m_mapModel->Cfg2VID(re->m_target);

      //restore edge number of edge
      edge.Set(re->m_edgeNum, &re->m_source, &re->m_target);
      m_mapModel->GetGraph()->add_edge(svId, tvId, edge);
      m_mapModel->GetGraph()->add_edge(tvId, svId, edge);
    }
    else if(ins->m_name == "Comment"){
      //undo addition of comment
      m_comments.pop_back();
    }
    else if(ins->m_name == "QueryInstruction"){
      //undo addition of query and restore the previous one if any
      QueryInstruction* q = static_cast<QueryInstruction*>(ins);
      m_query = q->m_query;
      q->m_query.clear();
    }
  }

  //rebuild map model since graph may have changed
  m_mapModel->Build();
  m_mapModel->SetRenderMode(SOLID_MODE);
}

void
DebugModel::DrawRender(){

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
    m_mapModel->DrawRender();
    for(CIT cit = m_tempCfgs.begin(); cit!=m_tempCfgs.end(); cit++)
      cit->DrawRender();
    for(EIT eit = m_tempEdges.begin(); eit!=m_tempEdges.end(); eit++)
      eit->DrawRender();
    for(EIT eit = m_query.begin(); eit!=m_query.end(); eit++) {
      glLineWidth(32);
      eit->SetColor(Color4(1, 1, 0, 1));
      eit->DrawRender();
    }
    if(m_tempRay != NULL && m_tempCfgs.size() > 0) {
      EdgeModel edge;
      CfgModel ray = *m_tempRay;
      CfgModel* tmp = &(m_tempCfgs.back());
      for(int i=0; i<3; i++)
        ray[i]+= (*tmp)[i];
      edge.Set(0, tmp, &ray);
      edge.SetColor(Color4(1, 1, 0, 1));
      glLineWidth(8);
      edge.DrawRender();
    }
  }
}

void
DebugModel::Print(ostream& _os) const {
  _os << Name() << ": " << GetFilename() << endl
    << m_instructions.size() << " debug frames" << endl;
}

void
DebugModel::ConfigureFrame(int _frame){
  m_index = _frame+1;
}
