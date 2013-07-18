/* PlumObject
 *
 * A PlumObject is composed of a GLModel
 */

#ifndef PLUMOBJECT_H_
#define PLUMOBJECT_H_

namespace plum {

  class GLModel;

  class PlumObject {
    public:
      PlumObject(GLModel* _model = NULL) : m_model(_model) {}
      GLModel* GetModel() const {return m_model;}

    protected:
      GLModel* m_model;
  };
}

#endif
