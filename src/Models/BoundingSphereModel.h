#ifndef BOUNDINGSPHEREMODEL_H_
#define BOUNDINGSPHEREMODEL_H_

#include "BoundaryModel.h"

class BoundingSphereModel : public BoundaryModel {
  public:
    BoundingSphereModel();

    virtual bool Parse(istream& _is);

    virtual void BuildModels();
    virtual void Print(ostream& _os) const;

  private:
    Vector3d m_center;
    double m_radius;
};

#endif
