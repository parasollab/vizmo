#ifndef BODY_MODEL_H_
#define BODY_MODEL_H_

#include <Transformation.h>
using namespace mathtool;

#include "Utilities/Color.h"
#include "Models/PolyhedronModel.h"

class ConnectionModel;

////////////////////////////////////////////////////////////////////////////////
/// \brief A representation for primative physical objects.
////////////////////////////////////////////////////////////////////////////////
class BodyModel : public TransformableModel {

  public:

    // Body properties
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Defines whether this object is a base and what type of base it
    ///        is.
    /// \arg <i>Planar</i>     This is a 2D base.
    /// \arg <i>Volumetric</i> This is a 3D base.
    /// \arg <i>Fixed</i>      This is an immobile base.
    /// \arg <i>Joint</i>      This is not a base.
    enum Base {Planar, Volumetric, Fixed, Joint};
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Describes the type of movement this object can perform.
    /// \arg <i>Rotational</i>    The object can rotate and translate.
    /// \arg <i>Translational</i> The object can translate but not rotate.
    enum BaseMovement {Rotational, Translational};

    // Static functions
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Convert a string tag into a Base type.
    /// \param[in] _tag The string to convert.
    /// \return         The corresponding Base object.
    static Base GetBaseFromTag(const string& _tag);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Convert a string tag into a BaseMovement type.
    /// \param[in] _tag The string to convert.
    /// \return         The corresponding BaseMovement object.
    static BaseMovement GetMovementFromTag(const string& _tag);

    // Construction
    BodyModel(bool _isSurface = false);
    BodyModel(const string& _modelDataDir, const string& _filename,
        const Transformation& _t);
    BodyModel(const BodyModel& _b);
    ~BodyModel();

    // Model properties
    void GetChildren(list<Model*>& _models);  ///< Get this model's children.
    void SetRenderMode(RenderMode _mode);     ///< Set the rendering mode.
    void SetSelectable(bool _s);              ///< Enable/disable selection.
    void ToggleNormals();                     ///< Toggle display of normals.

    // File information
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the directory of this object's input file.
    const string& GetDirectory() {return m_directory;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the filename of this object's input file.
    const string& GetFilename() const {return m_filename;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the relative path of this object's input file.
    const string& GetModelFilename() const {return m_modelFilename;}

    // PolyhedronModel info
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get a pointer to the drawable polyhedron model.
    PolyhedronModel* GetPolyhedronModel() const {return m_polyhedronModel;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get this object's center of mass.
    const Point3d& GetCOM() const {return m_polyhedronModel->GetCOM();}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get this object's radius.
    double GetRadius() const {return m_polyhedronModel->GetRadius();}

    // Base properties
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Indicates whether this is a surface object.
    bool IsSurface() const {return m_isSurface;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Indicates whether this is a base object.
    bool IsBase() const {return m_baseType != Joint;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Indicates whether this is an immobile base object.
    bool IsBaseFixed() const {return m_baseType == Fixed;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Indicates whether this is a 2D base object.
    bool IsBasePlanar() const {return m_baseType == Planar;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Indicates whether this is a 3D base object.
    bool IsBaseVolumetric() const {return m_baseType == Volumetric;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the base type for this object.
    Base GetBase() const {return m_baseType;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the base movement type for this object.
    BaseMovement GetBaseMovement() const {return m_baseMovementType;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Indicates whether this object can rotate.
    bool IsBaseRotational() const {return m_baseMovementType == Rotational;}

    //access to connections
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Connection iterator definition.
    typedef vector<ConnectionModel*>::const_iterator ConnectionIter;
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get an iterator to the first connection.
    ConnectionIter Begin() const {return m_connections.begin();}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get an end-iterator for this object's connections.
    ConnectionIter End() const {return m_connections.end();}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Add a connection to this object.
    /// \param[in] _c Points to the new connection.
    void AddConnection(ConnectionModel* _c) {m_connections.push_back(_c);}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Remove a connection to this object.
    /// \param[in] _c Points to the connection to be removed.
    void DeleteConnection(ConnectionModel* _c);

    //access to transformations
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the last computed transformation for this object.
    const Transformation& GetTransform() const {return m_currentTransform;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the current transformation for this object.
    void SetTransform(const Transformation& _t);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Determine whether transformation info is current.
    bool IsTransformDone() const {return m_transformDone;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the previous transformation info.
    void SetPrevTransform(const Transformation& _t) {m_prevTransform = _t;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Reset (zero) all transformation info.
    void ResetTransform() {
      m_currentTransform = m_prevTransform = Transformation();
      m_transformDone = false;
    }
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Update the object's transformation.
    /// \details Compute the actual (previous*current) transformation as
    ///          previous * TDH * dh * TBody2.
    /// \param[in] _body     The Body to which \c this is connected.
    /// \param[in] _nextBody The ID of \c this's connection to \c _body.
    void ComputeTransform(const BodyModel* _body, size_t _nextBody);

    // Model functions
    void Build() {}
    void Select(GLuint* _index, vector<Model*>& sel);
    void DrawRender();
    void DrawSelect();
    void DrawSelected();
    void DrawHaptics();
    void Print(ostream& _os) const;

    // File IO
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Parse the Body file opened in the input istream and produce an
    ///        active (movable) body.
    /// \param[in] _is           The input file stream.
    /// \param[in] _modelDataDir The directory where the file is located.
    /// \param[in] _color        The color for the body object.
    void ParseActiveBody(istream& _is, const string& _modelDataDir,
        const Color4 _color);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Parse the Body file opened in the input istream and produce a
    ///        passive (immobile) body.
    /// \param[in] _is           The input file stream.
    /// \param[in] _modelDataDir The directory where the file is located.
    /// \param[in] _color        The color for the body object.
    void ParseOtherBody(istream& _is, const string& _modelDataDir,
        const Color4 _color);
    friend ostream& operator<<(ostream& _os, const BodyModel& _b);

  private:

    string m_directory;                 ///< The file directory.
    string m_filename;                  ///< The filename.
    string m_modelFilename;             ///< The relative path.

    PolyhedronModel* m_polyhedronModel; ///< The drawable polyhedron model.

    bool m_isSurface;      ///< Indicates whether this is a surface object.
    Base m_baseType;                    ///< The Base type.
    BaseMovement m_baseMovementType;    ///< The BaseMovement type.

    vector<ConnectionModel*> m_connections; ///< Connections to this object.

    Transformation m_currentTransform;  ///< The current transformation.
    Transformation m_prevTransform;     ///< The previous transformation.
    bool m_transformDone; ///< Indicates whether transform info is current.
};

#endif
