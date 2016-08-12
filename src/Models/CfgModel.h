#ifndef CFG_MODEL_H_
#define CFG_MODEL_H_

#include <iostream>
#include <string>
#include <mutex>
using namespace std;

#include <GL/gl.h>

#ifdef PMPCfg
#include "Cfg/Cfg.h"
typedef Cfg CfgType;
#elif defined(PMPState)
#include "Cfg/State.h"
typedef State CfgType;
#endif

#include "EdgeModel.h"
#include "Model.h"

template<typename, typename> class CCModel;
class EdgeModel;

////////////////////////////////////////////////////////////////////////////////
/// \brief A drawable configuration model.
////////////////////////////////////////////////////////////////////////////////
class CfgModel : public Model, public CfgType {

  public:

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Supported rendering modes for configurations.
    ////////////////////////////////////////////////////////////////////////////
    enum Shape {
      Robot, ///< Robot rendered at Cfg
      Point  ///< Point rendered at center of robot's base
    };

    // Construction
    explicit CfgModel(size_t _index = 0);
    explicit CfgModel(Vector3d _vec);
    CfgModel(const CfgType& _c);
    CfgModel(const CfgModel& _c);

    // DOF and validity info
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set new values for each DOF.
    void SetCfg(const vector<double>& _newCfg);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the data for this configuration.
    vector<double> GetDataCfg() {return m_v;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the center of the robot's base.
    Point3d GetPoint() const;
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Label this cfg as valid or invalid.
    void SetValidity(bool _validity) {m_isValid = _validity;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the last known collision status for this cfg.
    bool IsValid() const {return m_isValid;}

    // Name, query, index, and CC info
    void SetName();                           ///< Set the name for this cfg.
    void SetIsQuery() {m_isQuery = true;}     ///< Make this a query cfg.
    bool IsQuery() {return m_isQuery;} ///< Indicates whether this is a query cfg.
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set this configuration's VID.
    void SetIndex(size_t _i) {m_index = _i; SetName();}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get this configuration's VID.
    size_t GetIndex() const {return m_index;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set this configuration's CC.
    void SetCCModel(CCModel<CfgModel, EdgeModel>* _cc) {m_cc = _cc;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the CCModel to which this cfg belongs.
    CCModel<CfgModel, EdgeModel>* GetCC() const {return m_cc;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set this configuration's VID and CC.
    void Set(size_t _index, CCModel<CfgModel, EdgeModel>* _cc);

    // Class functions - rendering
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the current cfg rendering mode.
    static void SetShape(Shape _shape) {m_shape = _shape;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the current cfg rendering mode.
    static Shape GetShape() {return m_shape;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the current scaling factor for point mode.
    static float GetPointSize() {return m_pointScale;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Modify the current scaling factor for point mode.
    /// \param[in] _scale Multiplier for the current scale factor.
    static void Scale(float _scale);

    // Model functions
    void Build() {}
    void Select(GLuint* _index, vector<Model*>& _sel) {}
    void DrawRender();
    void DrawSelect();
    void DrawSelected();
    void Print(ostream& _os) const;

    void DrawPathRobot();

    void Lock() {
      m_mutex->lock();
    }

    void UnLock() {
      m_mutex->unlock();
    }

  protected:

    bool m_isValid{true}; ///< Was the last collision check valid?

  private:

    // Class data
    static Shape m_shape;            ///< The current CfgModel display mode.
    static float m_pointScale;       ///< The size for point mode display.

    // Local data
    bool m_isQuery{false};        ///< Is this configuration is part of a query?
    size_t m_index{(size_t)-1};   ///< This configuration's VID in the roadmap.
    CCModel<CfgModel, EdgeModel>* m_cc{nullptr}; ///< This configuration's CC.

    shared_ptr<mutex> m_mutex;
};

#endif
