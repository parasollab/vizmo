#ifndef BOUNDING_BOX_MODEL_H_
#define BOUNDING_BOX_MODEL_H_

#include <memory>
using namespace std;

#include "BoundaryModel.h"

class WorkspaceBoundingBox;

class BoundingBoxModel : public BoundaryModel {

  public:

    // Construction
    BoundingBoxModel(WorkspaceBoundingBox* _b);
    BoundingBoxModel(const pair<double, double>& _x,
        const pair<double, double>& _y,
        const pair<double, double>& _z);

    // Model functions
    virtual void Build();
    virtual void DrawHaptics();
    virtual void Print(ostream& _os) const;

    // BoundaryModel functions
    // TODO::Update this to use Range class
    virtual vector<pair<double, double> > GetRanges() const;
    virtual double GetMaxDist();

  private:
    WorkspaceBoundingBox* m_boundingBox; ///< PMPL's BoundingBox
};

#endif
