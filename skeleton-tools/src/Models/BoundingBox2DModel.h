#ifndef BOUNDING_BOX_2D_MODEL_H_
#define BOUNDING_BOX_2D_MODEL_H_

#include <memory>
using namespace std;

#include "BoundaryModel.h"

class BoundingBox2D;

class BoundingBox2DModel : public BoundaryModel {

  public:

    // Construction
    BoundingBox2DModel(shared_ptr<BoundingBox2D> _b);
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
    shared_ptr<BoundingBox2D> m_boundingBox; ///< PMPL's BoundingBox
};

#endif
