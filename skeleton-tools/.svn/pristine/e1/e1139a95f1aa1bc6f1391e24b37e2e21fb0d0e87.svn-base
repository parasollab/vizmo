#ifndef USER_PATH_MODEL_H_
#define USER_PATH_MODEL_H_

#include "Model.h"
#include "Vizmo.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief This class provides a tool for capturing approximate paths in two or
/// three dimensions.
/// \detail Paths can be created with mouse click-and-drag, camera tracking, or
/// haptics input. Collision detection is provided by checking a configuation at
/// the current path head. Currently, the path is considered to be valid if it
/// has no invalid configurations, and invalid otherwise.
////////////////////////////////////////////////////////////////////////////////
class UserPathModel : public Model {

  public:

    //the supported input types
    enum InputType {Mouse, CameraPath, Haptic};

    UserPathModel(InputType _t = Mouse);

    //standard model functions
    void Build() {}
    void Select(GLuint* _index, vector<Model*>& _sel);
    void DrawRender();
    void DrawSelect();
    void DrawSelected();
    void Print(ostream& _os) const;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Prints just the path positions.
    /// \details Use Print instead to also print the name and label.
    /// \param[in] _os The ostream to print to.
    void PrintPath(ostream& _os) const;

    //access functions
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the input device type.
    /// \return The input type used for this path.
    InputType GetInputType() {return m_type;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Check whether the user has finished creating the path.
    /// \return A \c bool indicating whether the path is finished.
    const bool IsFinished() const {return m_finished;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the current path head.
    /// \return The collision-checking configuration at the current path head.
    const CfgModel& GetNewPos() const {return m_newPos;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the previous path head.
    /// \return The collision-checking configuration at the previous path head.
    const CfgModel& GetOldPos() const {return m_oldPos;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the raw user path.
    /// \return The set of points on the user path.
    const vector<Point3d>& GetPath() {return m_userPath;}

    //path capture and control
    void RewindPos() {m_newPos = m_oldPos;} ///< Move the path head back one step.
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Send points to the user path.
    /// \param[in] _p The new point to append to the path.
    void SendToPath(const Point3d& _p);

    //planning functions
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create configurations from the user path points.
    /// \param[in] _randomize Randomize the orientation of the configurations.
    /// \return A set of configurations near to the user path.
    shared_ptr<vector<CfgModel> > GetCfgs(bool _randomize = true) const;

    //mouse events for selecting and drawing
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Start drawing a mouse path when the user clicks the left button.
    /// \param[in] _e The mouse-click event.
    /// \param[in] _c The current camera (not used).
    bool MousePressed(QMouseEvent* _e, Camera* _c);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Stop drawing a mouse path when the user unclicks the left button.
    /// \param[in] _e The mouse-click event.
    /// \param[in] _c The current camera (not used).
    bool MouseReleased(QMouseEvent* _e, Camera* _c);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Send the mouse position to the current path while drawing.
    /// \param[in] _e The mouse-click event.
    /// \param[in] _c The current camera (not used).
    bool MouseMotion(QMouseEvent* _e, Camera* _c);

    //key events for stopping input collection
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Terminate input collection for non-mouse input using the ESC key.
    /// \param[in] _e The key event.
    bool KeyPressed(QKeyEvent* _e);

    friend ostream& operator<<(ostream& _os, const UserPathModel& _upm);
    friend istream& operator>>(istream& _is, UserPathModel& _upm);

  private:

    //helper functions
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Update the path head to position \c _p.
    /// \param[in] _p The new path head location.
    void UpdatePositions(const Point3d& _p);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Update the whole-path validity based on the validity of the
    /// current path head.
    void UpdateValidity();
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the position of a configuration to the given point.
    /// \param[in/out] _cfg The configuration to position.
    /// \param[in] _p The desired position.
    void SetCfgPosition(CfgModel& _cfg, const Point3d& _p) const;

    //data objects
    InputType m_type;           ///< Indicates input device type.
    bool m_finished,            ///< Indicates whether the input is finished.
         m_valid;               ///< Validity of the whole path.
    CfgModel m_oldPos,          ///< The previous path head.
             m_newPos;          ///< The current path head.
    vector<Point3d> m_userPath; ///< The set of positions in the user path.
};

#endif
