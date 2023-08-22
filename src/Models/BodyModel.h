#ifndef BODY_MODEL_H_
#define BODY_MODEL_H_

#include <memory>
using namespace std;

#include <Transformation.h>
using namespace mathtool;

#include "Utilities/Color.h"
#include "Models/PolyhedronModel.h"

class Body;
class ConnectionModel;

////////////////////////////////////////////////////////////////////////////////
/// \brief A representation for primative physical objects.
////////////////////////////////////////////////////////////////////////////////
class BodyModel : public TransformableModel {
  public:

    // Construction
    BodyModel(Body* _b);
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

    //access to transformations
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the last computed transformation for this object.
    const Transformation& GetTransform() const {return m_currentTransform;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the current transformation for this object.
    void SetTransform(const Transformation& _t);

    // Model functions
    void Build();
    void Select(GLuint* _index, vector<Model*>& sel);
    void DrawRender();
    void DrawSelect();
    void DrawSelected();
    void DrawHaptics();
    void Print(ostream& _os) const;

    // File IO
    friend ostream& operator<<(ostream& _os, const BodyModel& _b);

  private:

    shared_ptr<Body> m_body;            ///< PMPL Body

    string m_directory;                 ///< The file directory.
    string m_filename;                  ///< The filename.
    string m_modelFilename;             ///< The relative path.

    PolyhedronModel* m_polyhedronModel; ///< The drawable polyhedron model.

    Transformation m_currentTransform;  ///< The current transformation.

    GLuint m_textureID;                 ///< GL context's ID for texture
};

#endif
