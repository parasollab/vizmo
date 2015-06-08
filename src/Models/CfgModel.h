#ifndef CFG_MODEL_H_
#define CFG_MODEL_H_

#include <iostream>
#include <string>
using namespace std;

#include <GL/gl.h>
#include <GL/glut.h>

#include "Cfg/Cfg.h"

#include "EdgeModel.h"
#include "Model.h"

template<typename, typename> class CCModel;
class EdgeModel;

////////////////////////////////////////////////////////////////////////////////
/// \brief A drawable configuration model.
////////////////////////////////////////////////////////////////////////////////
class CfgModel : public Model, public Cfg {

  public:

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Supported rendering modes for configurations.
    /// \arg <i>Robot</i> The entire robot is rendered.
    /// \arg <i>Point</i> A point is rendered at the center of the robot's base.
    enum Shape {Robot, Point};

    // Construction
    CfgModel();
    CfgModel(const Cfg& _c);

    // DOF and validity info
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the number of degrees of freedom for this configuration.
    /// \bug   The implementation in its current state doesn't make sense. Need
    ///        to confirm with the cfg multi-robot team what is going on here.
    void SetDOF(size_t _d) {
      /*m_dof[m_robotIndex] = _d;*/
      //Temporary fix for PMPL Compile to avoid seg fault for regular vizmo use.
      m_dof.push_back(_d);
    }
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set new values for each DOF.
    void SetCfg(const vector<double>& _newCfg);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the data for this configuration.
    vector<double> GetDataCfg() {return m_v;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the center of the robot's base.
    Point3d GetPoint() const {
      return Point3d(m_v[0], m_v[1], m_isVolumetricRobot ? m_v[2] : 0);
    }
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

    // Class functions - robot information
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Label this robot as 2D.
    static void SetIsPlanarRobot(bool _b) {m_isPlanarRobot = _b;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Label this robot as 3D.
    static void SetIsVolumetricRobot(bool _b) {m_isVolumetricRobot = _b;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Indicates whether this is a 3D robot.
    static bool GetIsVolumetricRobot() {return m_isVolumetricRobot;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Label this robot as rotatable.
    static void SetIsRotationalRobot(bool _b) {m_isRotationalRobot = _b;}

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

  protected:

    bool m_isValid; ///< Indicates whether last collision check was valid.

  private:

    // Class data
    static double m_defaultDOF;      ///< The default DOFs for this problem.
    static bool m_isPlanarRobot;     ///< Indicates whether the robot is 2D.
    static bool m_isVolumetricRobot; ///< Indicates whether the robot is 3D.
    static bool m_isRotationalRobot; ///< Indicates whether the robot can rotate.
    static Shape m_shape;            ///< The current CfgModel display mode.
    static float m_pointScale;       ///< The size for point mode display.

    // Local data
    bool m_isQuery; ///< Indicates whether this configuration is part of a query.
    size_t m_index; ///< Indicates this configuration's VID in the graph.
    CCModel<CfgModel, EdgeModel>* m_cc; ///< Points to this configuration's CC.
};

#endif
