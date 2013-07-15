#ifndef BOUNDARYMODEL_H_
#define BOUNDARYMODEL_H_

#include "Plum/GLModel.h" 
using namespace plum;

class BoundaryModel : public GLModel {
  public:
    BoundaryModel();
    virtual ~BoundaryModel() {}

    virtual void Select(unsigned int* _index, vector<gliObj>& _sel);
    virtual bool BuildModels() = 0; 
    virtual void Draw(GLenum mode);
    virtual void DrawSelect();

    virtual const string GetName() const {return "Wrong object";}
    virtual vector<string> GetInfo() const = 0;

    virtual bool Parse(istream& _is) = 0;

  protected:
    int m_displayID, m_linesID;
};

#endif
