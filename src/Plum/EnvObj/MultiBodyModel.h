#ifndef MULTIBODYMODEL_H_
#define MULTIBODYMODEL_H_

#include "MultiBodyInfo.h"
#include "Plum/GLModel.h"
#include "Models/PolyhedronModel.h"

class MultiBodyModel : public GLModel{
  public:
    //////////////////////////////////////////////////////////////////////
    // Cons/Des
    MultiBodyModel(const MultiBodyInfo& m_mbInfo);

    //////////////////////////////////////////////////////////////////////
    // Core
    //////////////////////////////////////////////////////////////////////
    virtual void BuildModels();
    virtual void Select(unsigned int* _index, vector<GLModel*>& sel);

    //Draw
    virtual void Draw(GLenum _mode);
    virtual void DrawSelect();

    //set wire/solid/hide
    virtual void SetRenderMode(RenderMode _mode);
    virtual void SetColor(const Color4& _c) {
      GLModel::SetColor(_c);
      for(size_t i=0; i<m_poly.size(); i++)
        m_poly[i].SetColor(_c);
    }

    virtual const string GetName() const{return "MultiBody";}

    virtual void GetChildren(list<GLModel*>& _models){
      for(size_t i=0; i<m_poly.size(); i++)
        _models.push_back(&m_poly[i]);
    }

    virtual vector<string> GetInfo() const;
    //used to print the confg. of the MultiBody
    void SetCfg(vector<double>& _cfg);
    virtual void Scale(double x, double y, double z);

    //////////////////////////////////////////////////////////////////////
    // Access
    //////////////////////////////////////////////////////////////////////
    void SetAsFree(bool free=true){m_fixed = !free;}

    double GetRadius() const{return m_radius;}
    const Point3d& GetCOM() const{return m_com;}
    vector<PolyhedronModel>& GetPolyhedron(){return m_poly;}
    const MultiBodyInfo& GetMBinfo(){return m_mbInfo;}
    bool IsFixed() const{return m_fixed;}

    //public variables
    double m_posX, posY, posZ;
    list<GLModel*> m_objlist; // to have access from glitransTool class

  private:
    const MultiBodyInfo& m_mbInfo; //a reference to the MultiBodyInfo
    vector<PolyhedronModel> m_poly;

    bool m_fixed; //is this multibody fixed. i.e obstacle
    double m_radius; //Radius
    Point3d m_com; // center of mass
    vector<double> m_cfg;
};

#endif
