#ifndef BOUNDARYMODEL_H_
#define BOUNDARYMODEL_H_

#include "Model.h"

class BoundaryModel : public Model {
  public:
    BoundaryModel(const string& _name);
    virtual ~BoundaryModel();

    virtual bool Parse(istream& _is) = 0;

    virtual void BuildModels() = 0;
    void Select(GLuint* _index, vector<Model*>& _sel);
    void Draw(GLenum _mode);
    void DrawSelect();
    virtual void Print(ostream& _os) const = 0;

  protected:
    size_t m_displayID, m_linesID;
};

#endif
