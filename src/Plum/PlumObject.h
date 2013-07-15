/* PlumObject
 *
 * A PlumObject is composed of both a Loadable and a GLModel
 */

#ifndef PLUMOBJECT_H_
#define PLUMOBJECT_H_

namespace plum {

  class GLModel;
  class Loadable;

  class PlumObject {
    public:
      PlumObject(GLModel* _model = NULL, Loadable* _loader = NULL) : 
        m_model(_model), m_loader(_loader) {}

      GLModel* GetModel() const {return m_model;}
      Loadable* GetLoader() const {return m_loader;}

    protected:    
      GLModel* m_model;
      Loadable* m_loader;
  };
}

#endif
