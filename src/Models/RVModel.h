#ifndef RV_MODEL_H_
#define RV_MODEL_H_

#include <Vector.h>

#include "Model.h"

class RVModel : public LoadableModel {
  public:
    RVModel(const string& _filename);
    ~RVModel();

    void ParseFile();
    void Build();

    void Select(GLuint* _index, vector<Model*>& _sel) {}
    void DrawRender();
    void DrawSelect();
    void DrawSelected() {}

    void Print(ostream& _os) const;
    void IncrementIndex();
    void DecrementIndex();
  
  private:
    mathtool::Vector3d m_voxel_dimensions;     ///< how large the voxels are
    vector<size_t> m_rv_duration;              ///< how many pathsteps each rv lasts
    vector<mathtool::Vector3d> m_rv_placement; ///< placement of robot base for which rv applies
    vector<vector<mathtool::Vector3d> > m_rv;  ///< rv storage: sets of rvs

    size_t m_glRVIndex;                        ///< Display list index.
    size_t m_currentIndex;                        ///< current Index
};

#endif
