#ifndef BOUNDINGBOXMODEL_H_
#define BOUNDINGBOXMODEL_H_

using namespace std;

#include "BoundaryModel.h"

class BoundingBoxModel : public BoundaryModel {
  public:
    BoundingBoxModel();
    BoundingBoxModel(const pair<double, double>& _x,
        const pair<double, double>& _y,
        const pair<double, double>& _z = pair<double, double>(
          -numeric_limits<double>::max(),
          numeric_limits<double>::max())
        );

    virtual vector<pair<double, double> > GetRanges() { return m_bbx; }
    virtual bool Parse(istream& _is);
    virtual void Build();
    virtual void DrawHaptics();
    virtual void Print(ostream& _os) const;

  protected:
    virtual void Write(ostream& _os) const;

  private:
    vector<pair<double, double> > m_bbx;

    friend class BoundingBoxesModel;
};

#endif
