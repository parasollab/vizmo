#ifndef BOUNDINGBOXMODEL_H_
#define BOUNDINGBOXMODEL_H_

#include "BoundaryModel.h"

class BoundingBoxModel : public BoundaryModel {
  public:
    BoundingBoxModel();

    bool Parse(istream& _is);

    void BuildModels();
    void Print(ostream& _os) const;

  private:
    pair<double, double> m_bbx[3];

    friend class BoundingBoxesModel;
};

#endif
