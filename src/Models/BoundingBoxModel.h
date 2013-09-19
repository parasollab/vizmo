#ifndef BOUNDINGBOXMODEL_H_
#define BOUNDINGBOXMODEL_H_

#include "BoundaryModel.h"

class BoundingBoxModel : public BoundaryModel {
  public:
    BoundingBoxModel();

    virtual const string GetName() const {return "BoundingBox";}
    virtual vector<string> GetInfo() const;

    virtual bool Parse(istream& _is);
    virtual void BuildModels();

  private:
    pair<double, double> m_bbx[3];

    friend class BoundingBoxesModel;
};

#endif
