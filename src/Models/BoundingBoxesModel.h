#ifndef BOUNDINGBOXESMODEL_H_
#define BOUNDINGBOXESMODEL_H_

#include "Model.h"

class BoundingBoxModel;

class BoundingBoxesModel : public Model {
  public:
    BoundingBoxesModel();
    ~BoundingBoxesModel();

    virtual const string GetName() const {return "BoundingBoxes";}
    virtual vector<string> GetInfo() const;
    virtual void GetChildren(list<Model*>& _models);

    virtual void BuildModels();
    virtual void Draw(GLenum _mode);
    virtual void Select(unsigned int* _index, vector<Model*>& _sel);

  private:
    enum OverlapType {O, EN, EP, D};

    void BuildOverlapModel(BoundingBoxModel* _a, BoundingBoxModel* _b);
    OverlapType ClassifyOverlap(double _min1, double _max1, double _min2, double _max2);
    vector<double> Overlap(BoundingBoxModel* _a, BoundingBoxModel* _b);

    vector<BoundingBoxModel*> m_bbxModels;
    vector<BoundingBoxModel*> m_bbxOverlaps;
};

#endif