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
    CBoundingBoxModel();
    virtual ~CBoundingBoxModel();

    //////////////////////////////////////////////////////////////////////
    // Action functions 
    //////////////////////////////////////////////////////////////////////
    void SetBBXParser( CBoundingBoxParser * pParser ){ m_pBBXParser=pParser; }
    
    //////////////////////////////////////////////////////////////////////
    // GLModel functions
    //////////////////////////////////////////////////////////////////////
    //virtual void Select( unsigned int * index );
    virtual bool BuildModels();
    virtual void Draw( GLenum mode );
    virtual const string GetName() const { return "Bounding Box"; }
	virtual list<string> GetInfo() const { 
		list<string> info; 
		info.push_back(string("I am Bounding Box"));		
		return info;
	}
    //////////////////// find the max coord. in Z
    double returnMax( void );
    double minVal(double x, double y);
    double maxVal(double x, double y);

//////////////////////////////////////////////////////////////////////
// Private functions and data
//////////////////////////////////////////////////////////////////////
private:
    int m_DisplayID;//Display list index
    CBoundingBoxParser * m_pBBXParser;
    const double * m_BBX;
};

#endif // !defined(AFX_BOUNDINGBOXMODEL_H__26E04623_BE82_4DD5_8A16_7B6C7D222149__INCLUDED_)
