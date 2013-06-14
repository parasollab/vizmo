/* PlumObject
 *
 * A PlumObject is composed of both a Loadable and a CGLModel
 */

#ifndef PLUMOBJECT_H_
#define PLUMOBJECT_H_

namespace plum {

  class CGLModel;
  class Loadable;

  class PlumObject {
    public:
      PlumObject(CGLModel* _model = NULL, Loadable* _loader = NULL) : 
        m_model(_model), m_loader(_loader) {}

      CGLModel* GetModel() const {return m_model;}
      Loadable* GetLoader() const {return m_loader;}

    protected:
      CGLModel* m_model;
      Loadable* m_loader;
  };
}

#endif
