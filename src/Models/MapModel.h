#ifndef MAPMODEL_H_
#define MAPMODEL_H_

#include <Graph.h>
#include <GraphAlgo.h>

#include "CCModel.h"
#include "CfgModel.h"
#include "EdgeModel.h"
#include "RobotModel.h"
#include "Utilities/IOUtils.h"

struct EdgeAccess {
  typedef double value_type;
  template<typename Property>
    value_type get(Property& p) {return p.GetWeight();}

  template<typename Property>
    void put(Property& p, value_type _v) {p.GetWeight()=_v;}
};

template<typename, typename>
class CCModel;

template <class CfgModel, class WEIGHT>
class MapModel : public LoadableModel {

  public:
    typedef CCModel<CfgModel, WEIGHT> CCM;
    typedef stapl::sequential::graph<stapl::DIRECTED, stapl::MULTIEDGES, CfgModel, WEIGHT> Wg;
    typedef typename Wg::vertex_descriptor VID;
    typedef typename Wg::edge_descriptor EID;
    typedef stapl::sequential::vector_property_map<Wg, size_t> ColorMap;
    typedef stapl::sequential::edge_property_map<Wg, EdgeAccess> EdgeMap;
    ColorMap m_colorMap;
    typedef typename Wg::vertex_iterator VI;
    typedef typename CfgModel::Shape Shape;

    MapModel(RobotModel* _robotModel);
    //constructor only to grab header environment name
    MapModel(const string& _filename);
    MapModel(const string& _filename, RobotModel* _robotModel);
    virtual ~MapModel();

    //Access functions
    const string  GetEnvFileName() const{ return m_envFileName; }
    const string  GetFileDir() const{ return m_fileDir; }
    Wg* GetGraph(){ return m_graph; }
    vector<CCM*>& GetCCModels(){ return m_ccModels; }
    //list<Model*>& GetNodeList(){ return m_nodes; }
    vector<Model*>& GetNodesToConnect(){ return m_nodesToConnect; }
    void SetMBEditModel(bool _setting){ m_editModel = _setting; }
    CCM* GetCCModel(int _id){ return m_ccModels[_id]; }
    int NumberOfCC(){ return m_ccModels.size(); }
    bool RobCfgOn() { return m_robCfgOn; }
    void SetEdgeThickness(double _thickness){ EdgeModel::m_edgeThickness = _thickness; }
    void SetNodeShape(Shape _s) { CfgModel::m_shape = _s; }
    void SetAddNode(bool _setting){ m_addNode = _setting; }
    void SetAddEdge(bool _setting){ m_addEdge = _setting; }


    //Load functions
    //Moving generic load functions to virtual in Model.h
    void InitGraph(){ m_graph = new Wg(); }
    void WriteMapFile(const string& _filename);
    void ParseHeader(istream& _in);
    virtual void ParseFile();
    VID Cfg2VID(const CfgModel& _target);
    void GenGraph();

    //Display fuctions
    virtual void SetRenderMode(RenderMode _mode); //Wire, solid, or invisible
    virtual void GetChildren(list<Model*>& _models);

    void BuildModels();
    void Select(GLuint* _index, vector<Model*>& _sel);
    void Draw(GLenum _mode);
    void DrawSelect() {}
    void Print(ostream& _os) const;

    bool AddCC(int _vid);
    void ScaleNodes(float _scale);

    //Modification functions
    void MoveNode(CfgModel* _node, vector<double>& _newCfg);
    void RefreshMap();

  private:
    string  m_envFileName;
    string  m_fileDir;
    bool m_robCfgOn;
    bool m_addNode;
    bool m_addEdge;
    RobotModel* m_robot;
    vector<CCM*> m_ccModels;
    vector<Model*> m_nodesToConnect; //nodes to be connected
    string m_cfgString, m_robCfgString; //s_cfg, s_robCfg
    bool m_editModel;
    bool m_noMap;

  protected:
    Wg* m_graph;
};

template <class CfgModel, class WEIGHT>
MapModel<CfgModel, WEIGHT>::MapModel(RobotModel* _robotModel) : LoadableModel("Map") {
  m_renderMode = INVISIBLE_MODE;
  m_robot = _robotModel;
  m_graph = NULL;
  m_editModel = false;
  m_addNode = false;
  m_addEdge = false;
  m_robCfgOn = false;
  m_robCfgString = "";
  m_noMap = false;
  m_robot = NULL;
}

//constructor only to grab header environment name
template <class CfgModel, class WEIGHT>
MapModel<CfgModel, WEIGHT>::MapModel(const string& _filename) : LoadableModel("Map") {
  SetFilename(_filename);
  if(FileExists(_filename)) {
    ifstream ifs(_filename.c_str());
    ParseHeader(ifs);
  }

  m_graph = NULL;
  m_editModel = false;
  m_addNode = false;
  m_addEdge = false;
  m_robCfgOn = false;
  m_robCfgString = "";
  m_noMap = false;
  m_robot = NULL;
}

template <class CfgModel, class WEIGHT>
MapModel<CfgModel, WEIGHT>::MapModel(const string& _filename, RobotModel* _robotModel) : LoadableModel("Map") {
  SetFilename(_filename);
  m_renderMode = INVISIBLE_MODE;
  m_robot = _robotModel;
  m_graph = NULL;
  m_editModel = false;
  m_addNode = false;
  m_addEdge = false;
  m_robCfgOn = false;
  m_robCfgString = "";
  m_noMap = false;

  ParseFile();
  BuildModels();
}

template <class CfgModel, class WEIGHT>
MapModel<CfgModel, WEIGHT>::~MapModel(){

  typedef typename vector<CCM*>::iterator CIT;
  for(CIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ic++)
    delete *ic;
  delete m_graph;
  m_graph = NULL;
}

///////////Load functions//////////

template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::ParseHeader(istream& _in){

  m_fileDir = GetPathName(GetFilename());

  //Open file for reading data
  string s;

  //Get env file name info
  GoToNext(_in);
  getline(_in, s);

  //See if we need to add directory
  if(s[0] != '/')
    m_envFileName = m_fileDir + s;
  else
    m_envFileName = s;
}

template<class CfgModel, class WEIGHT>
void
MapModel<CfgModel,WEIGHT>::ParseFile(){
  if(!FileExists(GetFilename()))
    throw ParseException(WHERE, "'" + GetFilename() + "' does not exist");

  ifstream ifs(GetFilename().c_str());

  ParseHeader(ifs);

  //Get Graph Data
  string s;
  getline(ifs, s);
  m_graph  = new Wg();
  read_graph(*m_graph, ifs);
}

template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::WriteMapFile(const string& _filename){

  ofstream outfile(_filename.c_str());

  outfile << "#####ENVFILESTART##### \n";
  outfile << m_envFileName << "\n";
  outfile << "#####ENVFILESTOP##### \n";

  write_graph(*m_graph, outfile);
}

template <class CfgModel, class WEIGHT>
//VID
typename MapModel<CfgModel, WEIGHT>::VID
MapModel<CfgModel, WEIGHT>::Cfg2VID(const CfgModel& _target){

  VI vi;
  //typename VID tvid = -1;
  VID tvid = -1;
  for(vi = m_graph->begin(); vi != m_graph->end(); vi++){
    if(_target == (*vi).property()){
      tvid=(*vi).descriptor();
      break;
    }
  }
  return tvid;
}

template<class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::GenGraph(){
  m_graph  = new Wg();
}

//////////Display functions//////////

template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::BuildModels() {

  typedef typename vector<CCM*>::iterator CCIT;
  for(CCIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ic++)
    delete (*ic);
  m_ccModels.clear();

  //Get CCs
  typedef typename vector< pair<size_t,VID> >::iterator CIT;
  vector<pair<size_t,VID> > ccs;
  m_colorMap.reset();
  get_cc_stats(*m_graph, m_colorMap, ccs);
  int CCSize = ccs.size();
  m_ccModels.reserve(CCSize);
  for(CIT ic = ccs.begin(); ic != ccs.end(); ic++){
    CCM* cc = new CCM(ic-ccs.begin());
    cc->SetRobotModel(m_robot);
    cc->BuildModels(ic->second, m_graph);
    m_ccModels.push_back(cc);
  }
}

template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::Draw(GLenum _mode){
  if(m_renderMode == INVISIBLE_MODE)
    return;

  //Draw each CC
  typedef typename vector<CCM*>::iterator CIT;//CC iterator
  for(CIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ic++){
    if(_mode == GL_SELECT)
      glPushName((*ic)->GetID());
    (*ic)->Draw(_mode);
    if(_mode == GL_SELECT)
      glPopName();
  }
}

template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::SetRenderMode(RenderMode _mode){
  m_renderMode = _mode;
  typedef typename vector<CCM*>::iterator CIT;//CC iterator
  for(CIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ic++)
    (*ic)->SetRenderMode(_mode);
}


template <class CfgModel, class WEIGHT>
bool
MapModel<CfgModel, WEIGHT>::AddCC(int _vid){

  if(m_graph == NULL)
    return false;

  CCM* cc = new CCM(m_ccModels.size());
  cc->RobotModel(m_robot);
  cc->BuildModels(_vid, m_graph);

  float size;
  vector<float> color;
  if(m_ccModels[m_ccModels.size()-1]->getShape() == 0)
    size = m_ccModels[m_ccModels.size()-1]->getRobotSize();
  else if(m_ccModels[m_ccModels.size()-1]->getShape() == 1)
    size = m_ccModels[m_ccModels.size()-1]->getBoxSize();
  else
    size = 0.0;

  color =  m_ccModels[m_ccModels.size()-1]->getColor();
  cc->change_properties(m_ccModels[m_ccModels.size()-1]->getShape(),size,
      color, true);

  m_ccModels.push_back(cc);
  return true;
}

template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::GetChildren(list<Model*>& _models){
  typedef typename vector<CCM*>::iterator CIT;
  for(CIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ic++)
    _models.push_back(*ic);
}

template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::Print(ostream& _os) const {
  _os << Name() << ": " << GetFilename() << endl
    << "Connected components: " << m_ccModels.size() << endl;
}

template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::ScaleNodes(float _scale){

  CfgModel::Scale(_scale);
}

template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::Select(GLuint* _index, vector<Model*>& _sel){
  if(_index == NULL)
    return;
  m_ccModels[_index[0]]->Select(&_index[1],_sel);
}

template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::MoveNode(CfgModel* _node, vector<double>& _newCfg){

  //Actualllly....no
}

template <class CfgModel, class WEIGHT>
void
MapModel<CfgModel, WEIGHT>::RefreshMap(){

  BuildModels();

  for(size_t i = 0; i < m_ccModels.size(); i++){
    m_ccModels[i]->SetRenderMode(m_renderMode);
  }
}

#endif
