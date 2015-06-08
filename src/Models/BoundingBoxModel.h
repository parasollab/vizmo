#ifndef BOUNDING_BOX_MODEL_H_
#define BOUNDING_BOX_MODEL_H_

using namespace std;

#include "BoundaryModel.h"

class BoundingBoxModel : public BoundaryModel {

  public:

    // Construction
    BoundingBoxModel();
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
    virtual vector<pair<double, double> > GetRanges() { return m_bbx; }
    virtual bool Parse(istream& _is);

  protected:

    virtual void Write(ostream& _os) const;

  private:

    vector<pair<double, double> > m_bbx; ///< The min/max values in X, Y, Z.
};

#endif
