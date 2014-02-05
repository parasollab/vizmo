#ifndef REGIONMODEL_H_
#define REGIONMODEL_H_

#include "boost/shared_ptr.hpp"
using boost::shared_ptr;

#include "Utilities/Color.h"
#include "MPProblem/Boundary.h"

#include "Model.h"

class RegionModel : public Model {
  public:

    enum Type {ATTRACT, AVOID, NONCOMMIT};

    RegionModel(const string& _name) :
      Model(_name),
      m_successfulAttempts(0), m_failedAttempts(0), m_numCCs(0),
      m_type(NONCOMMIT), m_processed(false), m_created(-1) {
      SetColor(Color4(0, 0, 1, 0.8));
    }
    virtual ~RegionModel() {}

    Type GetType() const {return m_type;}
    void SetType(Type _t) {m_type = _t;}
    bool IsProcessed() const {return m_processed;}
    void Processed() {m_processed = true;}
    virtual shared_ptr<Boundary> GetBoundary() const = 0;

    //initialization of gl models
    virtual void Build() = 0;
    //determing if _index is this GL model
    virtual void Select(GLuint* _index, vector<Model*>& _sel) = 0;
    //draw is called for the scene.
    virtual void DrawRender() = 0;
    //draw is called for the scene.
    virtual void DrawSelect() = 0;
    //DrawSelect is only called if item is selected
    virtual void DrawSelected() = 0;
    //output model info
    virtual void Print(ostream& _os) const = 0;

    //density calculation
    virtual double WSpaceArea() const = 0;
    double FSpaceArea() const {return WSpaceArea() * m_successfulAttempts / (double)(m_successfulAttempts + m_failedAttempts);}
    double NodeDensity() const {return (m_successfulAttempts + m_failedAttempts) / WSpaceArea();}
    //double CCDensity() const {return WSpaceArea() / (double)m_numCCs;}
    //double CCDensity() const {return m_numCCs / WSpaceArea();}

    //successful attempts
    void IncreaseNodeCount(size_t _i) { m_successfulAttempts += _i; }
    void ClearNodeCount() { m_successfulAttempts = 0; }
    size_t GetNodeCount() { return m_successfulAttempts; }

    //failed attempts
    void IncreaseFACount(size_t _i) { m_failedAttempts += _i; }
    void ClearFACount() { m_failedAttempts = 0; }
    size_t GetFACount() { return m_failedAttempts; }

    //cc count
    //size_t GetCCCount() {return m_numCCs;}
    //void SetCCCount(size_t _i) { m_numCCs = _i; }

    size_t GetCreationIter() {return m_created;}
    void SetCreationIter(size_t _i) {m_created = _i;}

  protected:
    size_t m_successfulAttempts, m_failedAttempts, m_numCCs;

    Type m_type;
    bool m_processed; //check if rejection region has been processed or not

    size_t m_created;
};

#endif
