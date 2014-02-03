#ifndef MAPMODEL_H_
#define MAPMODEL_H_

#include "boost/shared_ptr.hpp"
using boost::shared_ptr;
#include "MPProblem/RoadmapGraph.h"

#include <QMutex>
#include <QMutexLocker>

#include "CCModel.h"
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
    typedef typename vector<CCM*>::iterator CCIT;
    typedef RoadmapGraph<CFG, WEIGHT> RGraph;
    typedef stapl::sequential::graph<stapl::DIRECTED, stapl::NONMULTIEDGES, CFG, WEIGHT> Graph;
    typedef typename Graph::vertex_descriptor VID;
    typedef typename Graph::vertex_iterator VI;
    typedef typename Graph::edge_descriptor EID;
    typedef typename Graph::adj_edge_iterator EI;
    typedef stapl::sequential::vector_property_map<Graph, size_t> ColorMap;
    typedef stapl::sequential::edge_property_map<Graph, EdgeAccess> EdgeMap;

    MapModel();
    MapModel(RGraph* _g);
    MapModel(const string& _filename);
    ~MapModel();

    //Access functions
    const string& GetEnvFileName() const {return m_envFileName;}
    RGraph* GetGraph() {return m_graph;}
    vector<CFG*>& GetTempCfgs() {return m_tempCfgs;}

    VID Cfg2VID(const CFG& _target);

    //Load functions
    //Moving generic load functions to virtual in Model.h
    void Write(const string& _filename);
    virtual void ParseFile();

    //Display fuctions
    virtual void SetRenderMode(RenderMode _mode); //Wire, solid, or invisible
    virtual void GetChildren(list<Model*>& _models);

    void BuildModels();
    void Select(GLuint* _index, vector<Model*>& _sel);
    void Draw();
    void DrawSelect() {}
    void Print(ostream& _os) const;
    void SetColor(const Color4& _c);

    //Modification functions
    void RandomizeCCColors();
    void RefreshMap();
    void ClearTempCfgs();

    QMutex& AcquireMutex() {return m_lock;}

  private:
    string m_envFileName;

    vector<CCM*> m_ccModels;
    RGraph* m_graph;

    bool m_delGraph;

    mutable QMutex m_lock;
    QMutexLocker* m_locker; //for acquire/release around get children

    //Currently for preview in MergeNodes
    vector<CFG*> m_tempCfgs;
};

template <class CFG, class WEIGHT>
MapModel<CFG, WEIGHT>::MapModel() : LoadableModel("Map"), m_delGraph(true) {
  m_renderMode = INVISIBLE_MODE;
  m_graph = new RGraph();
}

template <class CFG, class WEIGHT>
MapModel<CFG, WEIGHT>::MapModel(RGraph* _g) : LoadableModel("Map"), m_graph(_g), m_delGraph(false) {
  m_renderMode = SOLID_MODE;
  BuildModels();
}

//constructor only to grab header environment name
template <class CFG, class WEIGHT>
MapModel<CFG, WEIGHT>::MapModel(const string& _filename) : LoadableModel("Map"), m_delGraph(true) {
  SetFilename(_filename);
  m_renderMode = INVISIBLE_MODE;
  m_graph = new RGraph();

  ParseFile();
  BuildModels();
}

template <class CFG, class WEIGHT>
MapModel<CFG, WEIGHT>::~MapModel() {
  for(CCIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ic++)
    delete *ic;
  if(m_delGraph)
    delete m_graph;
}

///////////Load functions//////////

template<class CFG, class WEIGHT>
void
MapModel<CFG,WEIGHT>::ParseFile(){
  if(!FileExists(GetFilename()))
    throw ParseException(WHERE, "File '" + GetFilename() + "' does not exist");

  ifstream ifs(GetFilename().c_str());

  //parse env filename
  string s;
  GoToNext(ifs);
  getline(ifs, m_envFileName);
  getline(ifs, s);

  //Get Graph Data
  read_graph(*m_graph, ifs);
}

template <class CFG, class WEIGHT>
void
MapModel<CFG, WEIGHT>::Write(const string& _filename){
  QMutexLocker lock(&m_lock);
  ofstream outfile(_filename.c_str());

  outfile << "#####ENVFILESTART##### \n";
  outfile << m_envFileName << "\n";
  outfile << "#####ENVFILESTOP##### \n";

  write_graph(*m_graph, outfile);
}

template <class CFG, class WEIGHT>
typename MapModel<CFG, WEIGHT>::VID
MapModel<CFG, WEIGHT>::Cfg2VID(const CFG& _target){
  QMutexLocker lock(&m_lock);
  for(VI vi = m_graph->begin(); vi != m_graph->end(); vi++)
    if(_target == vi->property())
      return vi->descriptor();
  return -1;
}

//////////Display functions//////////

template <class CFG, class WEIGHT>
void
MapModel<CFG, WEIGHT>::BuildModels() {
  QMutexLocker lock(&m_lock);

  for(CCIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ic++)
    delete *ic;
  m_ccModels.clear();

  //Get CCs
  typedef typename vector<pair<size_t, VID> >::iterator CIT;
  vector<pair<size_t, VID> > ccs;
  ColorMap colorMap;
  get_cc_stats(*m_graph, colorMap, ccs);
  m_ccModels.reserve(ccs.size());
  for(CIT ic = ccs.begin(); ic != ccs.end(); ic++)
    m_ccModels.push_back(new CCM(ic-ccs.begin(), ic->second, m_graph));
}

template <class CFG, class WEIGHT>
void
MapModel<CFG, WEIGHT>::Draw(){
  QMutexLocker lock(&m_lock);
  if(m_renderMode == INVISIBLE_MODE)
    return;

  //Draw each CC
  for(CCIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ic++){
    glPushName((*ic)->GetID());
    (*ic)->Draw();
    glPopName();
  }

  typedef typename vector<CFG*>::iterator NIT;
  for(NIT nit = m_tempCfgs.begin(); nit != m_tempCfgs.end(); nit++)
    (*nit)->Draw();
}

template <class CFG, class WEIGHT>
void
MapModel<CFG, WEIGHT>::SetRenderMode(RenderMode _mode){
  QMutexLocker lock(&m_lock);
  m_renderMode = _mode;
  for(CCIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ic++)
    (*ic)->SetRenderMode(_mode);
}

template <class CFG, class WEIGHT>
void
MapModel<CFG, WEIGHT>::GetChildren(list<Model*>& _models) {
  //QMutexLocker lock(&m_lock);
  for(CCIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ic++)
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
  QMutexLocker lock(&m_lock);
  if(!m_selectable || _index == NULL)
    return;
  m_ccModels[_index[0]]->Select(&_index[1], _sel);
}

template <class CFG, class WEIGHT>
void
MapModel<CFG, WEIGHT>::SetColor(const Color4& _c) {
  QMutexLocker lock(&m_lock);
  Model::SetColor(_c);
  for(CCIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ++ic)
    (*ic)->SetColor(_c);
}

template <class CFG, class WEIGHT>
void
MapModel<CFG, WEIGHT>::RandomizeCCColors() {
  QMutexLocker lock(&m_lock);
  for(CCIT ic = m_ccModels.begin(); ic != m_ccModels.end(); ++ic)
    (*ic)->SetColor(Color4(drand48(), drand48(), drand48(), 1));
}

template <class CFG, class WEIGHT>
void
MapModel<CFG, WEIGHT>::RefreshMap(){
  BuildModels();
  SetRenderMode(m_renderMode);
}

template <class CFG, class WEIGHT>
void
MapModel<CFG, WEIGHT>::ClearTempCfgs(){

  m_tempCfgs.clear();
}

#endif
