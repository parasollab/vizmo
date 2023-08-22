#ifndef BOUNDING_BOX_2D_MODEL_H_
#define BOUNDING_BOX_2D_MODEL_H_

#include <memory>
using namespace std;

#include "BoundaryModel.h"

class WorkspaceBoundingBox;

class BoundingBox2DModel : public BoundaryModel {

  public:

    // Construction
    BoundingBox2DModel(shared_ptr<WorkspaceBoundingBox> _b);
    BoundingBox2DModel(const pair<double, double>& _x,
        const pair<double, double>& _y);

    // Model functions
    virtual void Build();
    virtual void DrawHaptics();
    virtual void Print(ostream& _os) const;

    // BoundaryModel functions
    virtual vector<pair<double, double> > GetRanges() const;
    virtual double GetMaxDist();

  private:
    shared_ptr<WorkspaceBoundingBox> m_boundingBox; ///< PMPL's BoundingBox
};

#endif
