#ifndef TEMP_OBJS_MODEL_H
#define TEMP_OBJS_MODEL_H

#include "Model.h"

class CfgModel;
class EdgeModel;


////////////////////////////////////////////////////////////////////////////////
/// This class provides a container for temporary CfgModels and EdgeModels.
/// These temporary objects should be created dynamically and then added to
/// this container with AddCfg/Edge. TempObjsModel will then handle their
/// drawing (via the EnvironmentModel) and eventual deletion. Valid temporary
/// objects are displayed green, while invalid objects are colored magenta.
////////////////////////////////////////////////////////////////////////////////
class TempObjsModel : public Model {

  public:

    TempObjsModel();
    ~TempObjsModel();

    //Temp object access
    vector<CfgModel*>& GetCfgs() {return m_tempCfgs;}
    vector<EdgeModel*>& GetEdges() {return m_tempEdges;}
    vector<Model*>& GetOthers() {return m_tempModels;}

    //Add new temporary cfgs/edges. These will be deleted on destruction of the
    //TempObjsModel.
    void AddCfg(CfgModel* _c) {m_tempCfgs.push_back(_c);}
    void AddEdge(EdgeModel* _e) {m_tempEdges.push_back(_e);}
    void AddOther(Model* _m) {m_tempModels.push_back(_m);}
    void RemoveOther(Model* _m) {
      m_tempModels.erase(find(m_tempModels.begin(), m_tempModels.end(), _m));
      delete _m;
      _m = nullptr;
    }

    //Standard Model functions
    void Build() {}
    void Select(GLuint* _index, vector<Model*>& _sel) {}
    void DrawRender();
    void DrawSelect();
    void DrawSelected() {}
    void Print(ostream& _os) const;

  private:

    vector<CfgModel*> m_tempCfgs;
    vector<EdgeModel*> m_tempEdges;
    vector<Model*> m_tempModels;

};

#endif
