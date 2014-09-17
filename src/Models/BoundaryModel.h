#ifndef BOUNDARY_MODEL_H_
#define BOUNDARY_MODEL_H_

#include "Model.h"
#include <string>

class BoundaryModel : public Model {
  public:
    BoundaryModel(const string& _name);
    virtual ~BoundaryModel();

    virtual vector<pair<double, double> > GetRanges() = 0;
    virtual bool Parse(istream& _is) = 0;
    virtual void Build() = 0;
    void Select(GLuint* _index, vector<Model*>& _sel);
    void DrawRender();
    void DrawSelect();
    void DrawSelected();
    virtual void Print(ostream& _os) const = 0;

    friend ostream& operator<<(ostream& _os, const BoundaryModel& _b) {_b.Write(_os); return _os;}

  protected:
    virtual void Write(ostream& _os) const = 0;

  protected:
    size_t m_displayID, m_linesID;
};

#endif
