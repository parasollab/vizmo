// BoundingBoxModel.h: interface for the CBoundingBoxModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BOUNDINGBOXMODEL_H__26E04623_BE82_4DD5_8A16_7B6C7D222149__INCLUDED_)
#define AFX_BOUNDINGBOXMODEL_H__26E04623_BE82_4DD5_8A16_7B6C7D222149__INCLUDED_

//////////////////////////////////////////////////////////////////////
//Plum Headers
#include <GLModel.h>
using namespace plum;

//////////////////////////////////////////////////////////////////////
//OBPRMView Headers
#include "BoundingBoxParser.h"

class CBoundingBoxModel : public CGLModel
{
public:
    //////////////////////////////////////////////////////////////////////
    // Constructor/Destructor
    //////////////////////////////////////////////////////////////////////
    CBoundingBoxModel(int i, vector<double> values);
    virtual ~CBoundingBoxModel();

    //////////////////////////////////////////////////////////////////////
    // GLModel functions
    //////////////////////////////////////////////////////////////////////
    virtual void Select( unsigned int * index, vector<gliObj>& sel );
    virtual bool BuildModels();
    virtual void Draw( GLenum mode );
    virtual void DrawSelect();
    void DrawLines(GLenum mode);
    virtual const string GetName() const;
    virtual list<string> GetInfo() const;
    //////////////////// find the max coord. in Z
    double returnMax( void );
    double minVal(double x, double y);
    double maxVal(double x, double y);
    vector<double>& getBBX(){return m_BBX;}

//////////////////////////////////////////////////////////////////////
// Private functions and data
//////////////////////////////////////////////////////////////////////
private:
    int m_DisplayID;//Display list index
    int m_BBXindex;
    vector<double> m_BBX;

   
    int m_LinesID;

};

#endif // !defined(AFX_BOUNDINGBOXMODEL_H__26E04623_BE82_4DD5_8A16_7B6C7D222149__INCLUDED_)
