#ifndef REGIONMODEL_H_
#define REGIONMODEL_H_

#include "boost/shared_ptr.hpp"
using boost::shared_ptr;

#include "MPProblem/Boundary.h"

#include "Model.h"

class RegionModel : public Model {
  public:
    RegionModel(const string& _name) : Model(_name) {}
    virtual ~RegionModel() {}

    virtual shared_ptr<Boundary> GetBoundary() const = 0;

    //initialization of gl models
    virtual void BuildModels() = 0;
    //determing if _index is this GL model
    virtual void Select(GLuint* _index, vector<Model*>& _sel) = 0;
    //draw is called for the scene.
    virtual void Draw() = 0;
    //DrawSelect is only called if item is selected
    virtual void DrawSelect() = 0;
    //output model info
    virtual void Print(ostream& _os) const = 0;
};

#endif
