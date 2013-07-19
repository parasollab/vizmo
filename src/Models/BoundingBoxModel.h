#ifndef BOUNDINGBOXMODEL_H_
#define BOUNDINGBOXMODEL_H_

#include "BoundaryModel.h"

class BoundingBoxModel : public BoundaryModel {
  public:
    BoundingBoxModel();

    virtual void BuildModels();

    virtual const string GetName() const;
    virtual vector<string> GetInfo() const;

    const pair<double, double>* getBBX(){return m_bbx;}

    virtual bool Parse(istream& _is);

  private:
    pair<double, double> m_bbx[3];
};

#endif
