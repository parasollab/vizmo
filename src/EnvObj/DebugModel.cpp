// PathModel.cpp: implementation of the CDebugModel class.
//
//////////////////////////////////////////////////////////////////////

#include "DebugModel.h"
#include "Robot.h"

//////////////////////////////////////////////////////////////////////
// Include Plum headers
#include <Plum.h>
#include "GL/gliFont.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDebugModel::CDebugModel()
{
    m_Index=-1;
    m_pDebugLoader=NULL;
    m_mapLoader = new CMapLoader<CCfg, CSimpleEdge>();
    m_mapLoader->InitGraph();
    m_pRobot=NULL;
    m_RenderMode=CPlumState::MV_INVISIBLE_MODE;
}

CDebugModel::~CDebugModel()
{
    m_Index=-1;
    m_pDebugLoader=NULL;
    m_pRobot=NULL;
    delete m_mapLoader;
    for(size_t i = 0; i<m_mapModels.size(); i++){
      delete m_mapModels[i].mapModel;
    }
    m_mapModels.clear();
}

//////////////////////////////////////////////////////////////////////
// Action Methods
//////////////////////////////////////////////////////////////////////
bool CDebugModel::BuildModels(){
  //can't build model without model loader
  if( m_pRobot==NULL || m_pDebugLoader==NULL ) return false;


  ///////////////////////////////////////////////////////
  //Build Path Model

  size_t iDebugSize = m_pDebugLoader->GetDebugSize();
  int edgeNum = 0;
  vector<CCfg> tempCfgs;
  vector<CSimpleEdge> tempEdges, query;
  vector<string> comments;
  for( size_t iP=0; iP<iDebugSize; iP++ ){
    CCfg* tempRay = NULL;
    Instruction* ins = m_pDebugLoader->ConfigureFrame(iP);
    if(ins->name == "default"){
    }
    else if(ins->name == "AddNode"){
      AddNode* an = dynamic_cast<AddNode*>(ins);
      m_mapLoader->GetGraph()->AddVertex(an->cfg);
    }
    else if(ins->name == "AddEdge"){
      AddEdge* ae = dynamic_cast<AddEdge*>(ins);
      CSimpleEdge edge(1);
      int tvid = m_mapLoader->GetGraph()->GetVID(ae->target);
      int svid = m_mapLoader->GetGraph()->GetVID(ae->source);
      CCfg* target = m_mapLoader->GetGraph()->GetReferenceofData(tvid);
      CCfg* source = m_mapLoader->GetGraph()->GetReferenceofData(svid);
      edge.Set(edgeNum++, &ae->source, &ae->target);
      float color[3] = {0,0,0};
      vector<int> ccnid; //node id in this cc
      vector<int> ccnid1; //node id in this cc
      vector<int> ccnid2; //node id in this cc
      GetCC(*(m_mapLoader->GetGraph()),tvid,ccnid1);
      GetCC(*(m_mapLoader->GetGraph()),svid,ccnid2);
      if(ccnid1.size()>ccnid2.size()){
        color[0] = target->GetColor()[0];
        color[1] = target->GetColor()[1];
        color[2] = target->GetColor()[2];
        ccnid = ccnid2;
      }
      else{
        color[0] = source->GetColor()[0];
        color[1] = source->GetColor()[1];
        color[2] = source->GetColor()[2];
        ccnid = ccnid1;
      }
      for(size_t i = 0; i<ccnid.size(); i++){
        CCfg* cfgcc = m_mapLoader->GetGraph()->GetReferenceofData(ccnid[i]);
        cfgcc->SetColor(color[0],color[1],color[2],1);
        vector<pair<pair<int,int>, CSimpleEdge> > incidentEdges;
        m_mapLoader->GetGraph()->GetIncidentEdges(ccnid[i],incidentEdges);
        for(size_t j = 0; j<incidentEdges.size(); j++){
          incidentEdges[j].second.SetColor(color[0],color[1],color[2],1);
          m_mapLoader->GetGraph()->ChangeEdgeWeight(incidentEdges[j].first.first, 
                                                    incidentEdges[j].first.second,
                                                    incidentEdges[j].second);
        }
      }
      edge.SetColor(color[0],color[1],color[2],1);
      m_mapLoader->GetGraph()->AddEdge(svid, tvid,edge);
    }
    else if(ins->name == "AddTempCfg"){
      AddTempCfg* atc = dynamic_cast<AddTempCfg*>(ins);
      tempCfgs.push_back(atc->cfg);
      tempCfgs[tempCfgs.size()-1].Set(0,m_pRobot,NULL);
      tempCfgs[tempCfgs.size()-1].SetShape(CCfg::Robot);
    }
    else if(ins->name == "AddTempRay"){
      AddTempRay* atr = dynamic_cast<AddTempRay*>(ins);
      tempRay = &atr->cfg;
    }
    else if(ins->name == "AddTempEdge"){
      AddTempEdge* ate = dynamic_cast<AddTempEdge*>(ins);
      CSimpleEdge e(1);
      e.Set(1, &ate->source, &ate->target);
      tempEdges.push_back(e);
    }
    else if(ins->name == "ClearLastTemp"){
      tempRay = NULL;
      if(tempCfgs.size()>0)
        tempCfgs.pop_back();
      if(tempEdges.size()>0)
        tempEdges.pop_back();
    }
    else if(ins->name == "ClearAll"){
      tempRay = NULL;
      tempCfgs.clear();
      tempEdges.clear();
      query.clear();
      comments.clear();
    }
    else if(ins->name == "ClearComments"){
      comments.clear();
    }
    else if(ins->name == "RemoveNode"){
      RemoveNode* rn = dynamic_cast<RemoveNode*>(ins);
      m_mapLoader->GetGraph()->DeleteVertex(rn->cfg);
    }
    else if(ins->name == "RemoveEdge"){
      RemoveEdge* re = dynamic_cast<RemoveEdge*>(ins);
      m_mapLoader->GetGraph()->DeleteEdge(re->source, re->target);
    }
    else if(ins->name == "Comment"){
      Comment* c = dynamic_cast<Comment*>(ins);
      comments.push_back(c->comment);
    }
    else if(ins->name == "Query"){
      Query* q = dynamic_cast<Query*>(ins);
      typedef vector<pair<CCfg, CSimpleEdge> > PATH;
      PATH path;
      FindPathDijkstra(*(m_mapLoader->GetGraph()), q->source, q->target, path);
      CCfg source = q->source, target;
      for(PATH::iterator pit = path.begin()+1; pit!=path.end(); pit++){
        target = pit->first;
        CSimpleEdge e(1);
        e.Set(1, &source, &target);
        query.push_back(e);
        source = target;
      }
    }
    CMapModel<CCfg, CSimpleEdge>* mapModel = new CMapModel<CCfg, CSimpleEdge>();
    mapModel->SetMapLoader(m_mapLoader);
    mapModel->SetRobotModel(m_pRobot);
    mapModel->BuildModels();
    mapModel->SetRenderMode(CPlumState::MV_SOLID_MODE);
    m_mapModels.push_back(Model(mapModel, tempCfgs, tempEdges, query, tempRay, comments));
  }

  return true;
}

void CDebugModel::Draw( GLenum mode ){
  if(m_Index<m_mapModels.size()){
    Model& m = m_mapModels[m_Index];
    m.mapModel->Draw(mode);
    typedef vector<CCfg>::iterator CIT;
    typedef vector<CSimpleEdge>::iterator EIT;
    for(CIT cit = m.tempCfgs.begin(); cit!=m.tempCfgs.end(); cit++){
      cit->Draw(mode);
    }
    for(EIT eit = m.tempEdges.begin(); eit!=m.tempEdges.end(); eit++){
      eit->Draw(mode);
    }
    for(EIT eit = m.query.begin(); eit!=m.query.end(); eit++){
      glLineWidth(6);
      eit->SetColor(1,1,0,1);
      eit->Draw(mode);
    }
    if(m.tempRay!=NULL && m.tempCfgs.size()>0){
      CSimpleEdge edge;
      CCfg ray = *m.tempRay;
      vector<double> cfg = ray.GetDataCfg(); 
      CCfg* tmp = &m.tempCfgs[m.tempCfgs.size()-1];
      cfg[0]+=tmp->tx();
      cfg[1]+=tmp->ty();
      cfg[2]+=tmp->tz();
      ray.setCfg(cfg);
      edge.Set(0, tmp, &ray);
      edge.SetColor(1,1,0,1);
      glLineWidth(4);
      edge.Draw(mode);
    }
    glPushAttrib(GL_CURRENT_BIT);

    //draw reference axis
    glMatrixMode(GL_PROJECTION); //change to Ortho view
    glPushMatrix(); 
    glLoadIdentity();
    gluOrtho2D(0,20,0,20);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    //Draw text
    glTranslated(0.25,19.75,0);
    setfont("times roman", 24);
    typedef vector<string>::iterator SIT;
    for(SIT i=m.comments.begin();i!=m.comments.end();i++){
      //////////////////////////////////////////////
      glTranslated(0,-0.75,0);
      glColor3f(0,0,0);
      drawstr(0,0,0,i->c_str());
    }
    glPopMatrix();
    //pop GL_PROJECTION
    glMatrixMode(GL_PROJECTION); //change to Pers view
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopAttrib();
    setfont("helvetica", 12);
  }
}

void CDebugModel::ConfigureFrame(int f){
  m_Index = f;
}

list<string> CDebugModel::GetInfo() const 
{ 
  list<string> info; 
  info.push_back(m_pDebugLoader->GetFileName());
  {
    ostringstream temp;
    temp<<"There are "<<m_pDebugLoader->GetDebugSize()<<" debug frames";
    info.push_back(temp.str());
  }	
  return info;
}

