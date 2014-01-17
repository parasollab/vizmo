#ifndef REGIONMODEL_H_
#define REGIONMODEL_H_

#include "boost/shared_ptr.hpp"
using boost::shared_ptr;

#include "Utilities/Color.h"
#include "MPProblem/Boundary.h"

#include "Model.h"

class RegionModel : public Model {
  public:
    RegionModel(const string& _name) : Model(_name), m_numVertices(0), m_failedAttempts(0) {
      SetColor(Color4(.5, .5, 0., 1.));
    }
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

    //density calculation
    virtual double Density() const = 0;

    //access for node count
    void IncreaseNodeCount(size_t _i) { m_numVertices += _i; }
    void ClearNodeCount() { m_numVertices = 0; }
    size_t GetNodeCount() { return m_numVertices; }
    void IncreaseFACount(size_t _i) { m_failedAttempts += _i; }
    void ClearFACount() { m_failedAttempts = 0; }
    size_t GetFACount() { return m_failedAttempts; }

  protected:
    size_t m_numVertices;
    size_t m_failedAttempts;
};

#endif
