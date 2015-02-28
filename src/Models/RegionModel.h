#ifndef REGION_MODEL_H_
#define REGION_MODEL_H_

#include <memory>

#include "Utilities/Color.h"
#include "MPProblem/Boundary.h"
#include "Utilities/GLUtils.h"

#include "Model.h"

class RegionModel : public Model {
  public:
    enum Type {ATTRACT, AVOID, NONCOMMIT};
    enum Shape {DEFAULT, BOX, BOX2D, SPHERE, SPHERE2D}; // DEFAULT is currently used only for vizmo debug

    RegionModel(const string& _name, const Shape _shape) :
      Model(_name), m_successfulAttempts(0),
      m_failedAttempts(0), m_numCCs(0), m_type(NONCOMMIT),
      m_shape(_shape), m_sampler(""), m_processed(false), m_created(-1) {
        SetColor(Color4(0, 0, 1, 0.8));
      }

    virtual ~RegionModel() {}

    Type GetType() const {return m_type;}
    void SetType(Type _t) {m_type = _t;}
    bool IsProcessed() const {return m_processed;}
    void Processed() {m_processed = true;}
    virtual shared_ptr<Boundary> GetBoundary() const = 0;
    const Point3d& GetCenter() const {return m_center;}

    Shape GetShape() const {return m_shape;}
    string GetSampler() const {return m_sampler;}
    virtual void ApplyOffset(const Vector3d& _v) = 0;
    virtual double GetShortLength() const = 0;
    virtual double GetLongLength() const = 0;

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
    //output debug info
    virtual void OutputDebugInfo(ostream& _os) const = 0;

    // Operators
    virtual bool operator==(const RegionModel& _other) const = 0;

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

    void ChangeColor() {
      if(m_type == NONCOMMIT)
        SetColor(Color4(0, 0, 1, 0.8));
      else if(m_type == ATTRACT)
        SetColor(Color4(0, 1, 0, 0.8));
      else if(m_type == AVOID)
        SetColor(Color4(0.5, 0.5, 0.5, 0.8));
    }

    void SetSampler(string _s) {m_sampler = _s;}

  protected:
    size_t m_successfulAttempts, m_failedAttempts, m_numCCs;

    Type m_type;
    Shape m_shape;
    string m_sampler;
    bool m_processed; //check if rejection region has been processed or not

    size_t m_created;
    Point3d m_center;

    // Functions
    void SetShape(Shape _s) {m_shape = _s;}
};

#endif
