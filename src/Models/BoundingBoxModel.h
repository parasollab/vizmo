#ifndef BOUNDING_BOX_MODEL_H_
#define BOUNDING_BOX_MODEL_H_

#include <memory>
using namespace std;

#include "BoundaryModel.h"

class BoundingBox;

class BoundingBoxModel : public BoundaryModel {

  public:

    // Construction
    BoundingBoxModel(shared_ptr<BoundingBox> _b);
    BoundingBoxModel(const pair<double, double>& _x,
        const pair<double, double>& _y,
        const pair<double, double>& _z = pair<double, double>(
          -numeric_limits<double>::max(),
          numeric_limits<double>::max())
        );

    // Model functions
    virtual void Build();
    virtual void DrawHaptics();
    virtual void Print(ostream& _os) const;

    // BoundaryModel functions
    virtual vector<pair<double, double> > GetRanges() const;
    virtual double GetMaxDist();

    virtual size_t GetNumVertices() const override;
    virtual size_t GetNumFacets() const override;
    virtual void AddToTetGen(tetgenio* _tetModel, size_t _pOff, size_t _fOff) const override;

  private:
    shared_ptr<BoundingBox> m_boundingBox; ///< PMPL's BoundingBox
};

#endif
