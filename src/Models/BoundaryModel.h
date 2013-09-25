#ifndef BOUNDARYMODEL_H_
#define BOUNDARYMODEL_H_

#include "Model.h"

class BoundaryModel : public Model {
  public:
    BoundaryModel();
    virtual ~BoundaryModel();

    virtual void Select(unsigned int* _index, vector<Model*>& _sel);
    virtual void BuildModels() = 0;
    virtual void Draw(GLenum mode);
    virtual void DrawSelect();

    virtual const string GetName() const {return "Wrong object";}
    virtual vector<string> GetInfo() const = 0;

    virtual bool Parse(istream& _is) = 0;

  protected:
    size_t m_displayID, m_linesID;
};

#endif
