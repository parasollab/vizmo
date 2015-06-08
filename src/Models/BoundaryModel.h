#ifndef BOUNDARY_MODEL_H_
#define BOUNDARY_MODEL_H_

#include "Model.h"
#include <string>

////////////////////////////////////////////////////////////////////////////////
/// \brief Provides a base class for drawable environment boundaries.
////////////////////////////////////////////////////////////////////////////////
class BoundaryModel : public Model {

  public:

    // Construction
    BoundaryModel(const string& _name);
    virtual ~BoundaryModel();

    // Model functions
    virtual void Build() = 0;
    void Select(GLuint* _index, vector<Model*>& _sel);
    void DrawRender();
    void DrawSelect();
    void DrawSelected();
    virtual void Print(ostream& _os) const = 0;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get minimums and maximums for this environment in X, Y, Z.
    virtual vector<pair<double, double> > GetRanges() = 0;

    // IO functions
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Parse an input file opened in the input file stream.
    /// \param[in] _is The input file stream.
    virtual bool Parse(istream& _is) = 0;
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Output helper for printing boundary information to out streams.
    friend ostream& operator<<(ostream& _os, const BoundaryModel& _b) {
      _b.Write(_os);
      return _os;
    }

  protected:

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Print boundary information.
    virtual void Write(ostream& _os) const = 0;

  protected:

    size_t m_displayID; ///< This model's ID in the rendering call list.
    size_t m_linesID;   ///< This model's ID in the selection call list.
};

#endif
