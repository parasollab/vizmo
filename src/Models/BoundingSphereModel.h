#ifndef BOUNDINGSPHEREMODEL_H_
#define BOUNDINGSPHEREMODEL_H_

#include "BoundaryModel.h"

class BoundingSphereModel : public BoundaryModel {
  public:
    BoundingSphereModel();

    virtual vector<pair<double, double> > GetRanges();
    virtual bool Parse(istream& _is);
    string GetCoord();
    virtual void BuildModels();
    virtual void Print(ostream& _os) const;

  private:
    Vector3d m_center;
    double m_radius;
};

#endif
