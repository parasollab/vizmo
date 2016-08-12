#ifndef TEMP_OBJS_MODEL_H
#define TEMP_OBJS_MODEL_H

#include <mutex>

#include "Model.h"

class CfgModel;
class EdgeModel;

////////////////////////////////////////////////////////////////////////////////
/// This class provides a container for rendering temporary Models. Populate it
/// by dynamically creating a model and adding it with AddModel. All added
/// models become owned by this container and will be de-allocated when it is
/// deleted.
////////////////////////////////////////////////////////////////////////////////
class TempObjsModel : public Model {

  public:

    ///\name Construction
    ///@{

    TempObjsModel();
    ~TempObjsModel();

    ///@}
    ///\name Interface
    ///@{

    vector<Model*>::iterator begin() {return m_models.begin();}
    vector<Model*>::iterator end() {return m_models.end();}
    vector<Model*>::const_iterator begin() const {return m_models.begin();}
    vector<Model*>::const_iterator end() const {return m_models.end();}

    ////////////////////////////////////////////////////////////////////////////
    /// Get a copy of the temporary model list.
    vector<Model*> GetModels() {return m_models;}

    ////////////////////////////////////////////////////////////////////////////
    /// Add a temporary model.
    /// \param[in] _m The model to add.
    /// \param[in] _col The model display color.
    void AddModel(Model* _m, const Color4& _col = Color4(0., 1., 0., 0.));

    ////////////////////////////////////////////////////////////////////////////
    /// Remove a temporary model.
    /// \param[in] _m The model to remove.
    void RemoveModel(Model* _m);

    ///@}
    ///\name Model Overrides
    ///@{

    virtual void Build() override {}
    virtual void Select(GLuint* _index, vector<Model*>& _sel) override {}
    virtual void DrawRender() override;
    virtual void DrawSelect() override {}
    virtual void DrawSelected() override {}
    virtual void Print(ostream& _os) const override {_os << Name() << endl;}

    ///@}

  private:

    ///@}
    ///\name Internal State
    ///@{

    vector<Model*> m_models; ///< Owned models.
    mutable mutex m_lock;    ///< Lock for thread-safety.

    ///@}
};

#endif
