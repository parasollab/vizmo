#ifndef THREAD_SAFE_SPHERE_MODEL_H_
#define THREAD_SAFE_SPHERE_MODEL_H_

#include <mutex>
#include "Model.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief Sphere model for use from the planning thread.
////////////////////////////////////////////////////////////////////////////////
class ThreadSafeSphereModel : public Model {

  public:

    // Construction
    ThreadSafeSphereModel(const Point3d& _p, double _r) :
        Model("ThreadSafeSphereModel"), m_center(_p), m_radius(_r) {
      m_selectable = false;
    }
    virtual ~ThreadSafeSphereModel() = default;

    ///\name Accessor
    ///@{

    void MoveTo(const Point3d& _p);

    ///@}
    ///@\name Model Overrides
    ///@{

    virtual void DrawRender() override;

    ///@}
    ///\name Unused but required overrides
    ///@{

    virtual void Build() override { }
    virtual void Select(GLuint* _index, vector<Model*>& _sel) override { }
    virtual void DrawSelect() override { }
    virtual void DrawSelected() override { }
    virtual void DrawHaptics() override { }
    virtual void Print(ostream& _os) const override { }

    ///@}

  protected:

    Point3d m_center;
    double m_radius;
    std::mutex m_lock;

};

#endif
