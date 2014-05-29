#ifndef TEMP_OBJS_MODEL_H
#define TEMP_OBJS_MODEL_H

#include "Model.h"

class CfgModel;
class EdgeModel;

class TempObjsModel : public Model {
  public:
    TempObjsModel();
    ~TempObjsModel();

    //Temp object access
    vector<CfgModel*>& GetCfgs() {return m_tempCfgs;}
    vector<EdgeModel*>& GetEdges() {return m_tempEdges;}
    void AddCfg(CfgModel* _c) {m_tempCfgs.push_back(_c);}
    void AddEdge(EdgeModel* _e) {m_tempEdges.push_back(_e);}

    //Standard Model functions
    void Build() {}
    void Select(GLuint* _index, vector<Model*>& _sel) {}
    void DrawRender();
    void DrawSelect();
    void DrawSelected() {}
    void Print(ostream& _os) const;

    //mouse events not needed
    bool MousePressed(QMouseEvent* _e, Camera* _c) {return false;}
    bool MouseReleased(QMouseEvent* _e, Camera* _c) {return false;}
    bool MouseMotion(QMouseEvent* _e, Camera* _c) {return false;}
    bool PassiveMouseMotion(QMouseEvent* _e, Camera* _c) {return false;}

  private:
    vector<CfgModel*> m_tempCfgs;
    vector<EdgeModel*> m_tempEdges;
};

#endif
