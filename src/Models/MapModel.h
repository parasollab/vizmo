#ifndef MAPMODEL_H_
#define MAPMODEL_H_

#include <Graph.h>
#include <GraphAlgo.h>

#include "CCModel.h"
#include "RobotModel.h"
#include "Utilities/IO.h"

struct EdgeAccess {
  typedef double value_type;
  template<typename Property>
    value_type get(Property& p) {return p.GetWeight();}

  template<typename Property>
    void put(Property& p, value_type _v) {p.GetWeight()=_v;}
};

template<typename, typename>
class CCModel;

template <class CFG, class WEIGHT>
class MapModel : public LoadableModel {

  public:
    typedef CCModel<CFG, WEIGHT> CCM;
    typedef stapl::sequential::graph<stapl::DIRECTED, stapl::NONMULTIEDGES, CFG, WEIGHT> Wg;
    typedef typename Wg::vertex_descriptor VID;
    typedef typename Wg::edge_descriptor EID;
    typedef stapl::sequential::vector_property_map<Wg, size_t> ColorMap;
    typedef stapl::sequential::edge_property_map<Wg, EdgeAccess> EdgeMap;
    typedef typename Wg::vertex_iterator VI;
    typedef typename CFG::Shape Shape;

    MapModel(RobotModel* _robotModel);
    //constructor only to grab header environment name
    MapModel(const string& _filename);
    MapModel(const string& _filename, RobotModel* _robotModel);
    virtual ~MapModel();

    //Access functions
    const string  GetEnvFileName() const{ return m_envFileName; }
    Wg* GetGraph(){ return m_graph; }
    CCM* GetCCModel(int _id){ return m_ccModels[_id]; }
    int NumberOfCC(){ return m_ccModels.size(); }

    //Load functions
    //Moving generic load functions to virtual in Model.h
    void WriteMapFile(const string& _filename);
    void ParseHeader(istream& _in);
    virtual void ParseFile();
    VID Cfg2VID(const CFG& _target);

    //Display fuctions
    virtual void SetRenderMode(RenderMode _mode); //Wire, solid, or invisible
    virtual void GetChildren(list<Model*>& _models);

    void BuildModels();
    void Select(GLuint* _index, vector<Model*>& _sel);
    void Draw(GLenum _mode);
    void DrawSelect() {}
    void Print(ostream& _os) const;

    //Modification functions
    void RandomizeCCColors();
    void RefreshMap();

  private:
    string  m_envFileName;
    string  m_fileDir;
    RobotModel* m_robot;
    vector<CCM*> m_ccModels;
    Wg* m_graph;
};

template <class CFG, class WEIGHT>
MapModel<CFG, WEIGHT>::MapModel(RobotModel* _robotModel) : LoadableModel("Map") {
  m_renderMode = INVISIBLE_MODE;
  m_robot = _robotModel;
  m_graph = new Wg();
  m_robot = NULL;
}

//constructor only to grab header environment name
template <class CFG, class WEIGHT>
MapModel<CFG, WEIGHT>::MapModel(const string& _filename) : LoadableModel("Map") {
  SetFilename(_filename);
  if(FileExists(_filename)) {
    ifstream ifs(_filename.c_str());
    ParseHeader(ifs);
  }

  m_graph = NULL;
  m_robot = NULL;
}

template <class CFG, class WEIGHT>
MapModel<CFG, WEIGHT>::MapModel(const string& _filename, RobotModel* _robotModel) : LoadableModel("Map") {
  SetFilename(_filename);
  m_renderMode = INVISIBLE_MODE;
  m_robot = _robotModel;

  ParseFile();
  BuildModels();
}

template <class CFG, class WEIGHT>
MapModel<CFG, WEIGHT>::~MapModel(){
  typedef typename vector<CCM*>::iterator CIT;
  for(CIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ic++)
    delete *ic;
  delete m_graph;
  m_graph = NULL;
}

///////////Load functions//////////

template <class CFG, class WEIGHT>
void
MapModel<CFG, WEIGHT>::ParseHeader(istream& _in){

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

template<class CFG, class WEIGHT>
void
MapModel<CFG,WEIGHT>::ParseFile(){
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

template <class CFG, class WEIGHT>
void
MapModel<CFG, WEIGHT>::WriteMapFile(const string& _filename){

  ofstream outfile(_filename.c_str());

  outfile << "#####ENVFILESTART##### \n";
  outfile << m_envFileName << "\n";
  outfile << "#####ENVFILESTOP##### \n";

  write_graph(*m_graph, outfile);
}

template <class CFG, class WEIGHT>
typename MapModel<CFG, WEIGHT>::VID
MapModel<CFG, WEIGHT>::Cfg2VID(const CFG& _target){
  for(VI vi = m_graph->begin(); vi != m_graph->end(); vi++)
    if(_target == vi->property())
      return vi->descriptor();
  return -1;
}

//////////Display functions//////////

template <class CFG, class WEIGHT>
void
MapModel<CFG, WEIGHT>::BuildModels() {

  typedef typename vector<CCM*>::iterator CCIT;
  for(CCIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ic++)
    delete (*ic);
  m_ccModels.clear();

  //Get CCs
  typedef typename vector< pair<size_t,VID> >::iterator CIT;
  vector<pair<size_t,VID> > ccs;
  ColorMap colorMap;
  get_cc_stats(*m_graph, colorMap, ccs);
  int CCSize = ccs.size();
  m_ccModels.reserve(CCSize);
  for(CIT ic = ccs.begin(); ic != ccs.end(); ic++){
    CCM* cc = new CCM(ic-ccs.begin());
    cc->SetRobotModel(m_robot);
    cc->BuildModels(ic->second, m_graph);
    m_ccModels.push_back(cc);
  }
}

template <class CFG, class WEIGHT>
void
MapModel<CFG, WEIGHT>::Draw(GLenum _mode){
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

template <class CFG, class WEIGHT>
void
MapModel<CFG, WEIGHT>::SetRenderMode(RenderMode _mode){
  m_renderMode = _mode;
  typedef typename vector<CCM*>::iterator CIT;//CC iterator
  for(CIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ic++)
    (*ic)->SetRenderMode(_mode);
}

template <class CFG, class WEIGHT>
void
MapModel<CFG, WEIGHT>::GetChildren(list<Model*>& _models){
  typedef typename vector<CCM*>::iterator CIT;
  for(CIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ic++)
    _models.push_back(*ic);
}

template <class CFG, class WEIGHT>
void
MapModel<CFG, WEIGHT>::Print(ostream& _os) const {
  _os << Name() << ": " << GetFilename() << endl
    << "Connected components: " << m_ccModels.size() << endl;
}

template <class CFG, class WEIGHT>
void
MapModel<CFG, WEIGHT>::Select(GLuint* _index, vector<Model*>& _sel){
  if(_index == NULL)
    return;
  m_ccModels[_index[0]]->Select(&_index[1],_sel);
}

template <class CFG, class WEIGHT>
void
MapModel<CFG, WEIGHT>::RandomizeCCColors() {
  typedef CCModel<CFG, WEIGHT> CC;
  typedef typename vector<CC*>::iterator CCIT;
  for(CCIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ++ic)
    (*ic)->SetColor(Color4(drand48(), drand48(), drand48(), 1));
}

template <class CFG, class WEIGHT>
void
MapModel<CFG, WEIGHT>::RefreshMap(){
  BuildModels();
  for(size_t i = 0; i < m_ccModels.size(); i++)
    m_ccModels[i]->SetRenderMode(m_renderMode);
}

#endif
