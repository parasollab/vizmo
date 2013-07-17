#ifndef BOUNDINGSPHEREMODEL_H_
#define BOUNDINGSPHEREMODEL_H_

#include "BoundaryModel.h"

class BoundingSphereModel : public BoundaryModel {
  public:
    BoundingSphereModel();

    virtual bool BuildModels();

    virtual const string GetName() const {return "Bounding Sphere";}
    virtual vector<string> GetInfo() const;

    virtual bool Parse(istream& _is);

  private:
    Vector3d m_center;
    double m_radius;
};

#endif
