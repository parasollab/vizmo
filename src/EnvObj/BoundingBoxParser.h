// BoundingBoxParser.h: interface for the CBoundingBoxParser class.
//
//////////////////////////////////////////////////////////////////////

#ifndef BOUNDINGBOXPARSER_H_
#define BOUNDINGBOXPARSER_H_

#include <vector>
#include <string>
using namespace std;

class CBoundingBoxParser
{
public:

    //////////////////////////////////////////////////////////////////////
    // Constructor
    //////////////////////////////////////////////////////////////////////
    CBoundingBoxParser(string _filename);

    //////////////////////////////////////////////////////////////////////
    // Core
    //////////////////////////////////////////////////////////////////////
    virtual bool ParseFile();
    int getNumBBXs(){return (int)m_bbxValues.size();}
    vector<vector<double> >& getBBXValues() { return m_bbxValues; }
    vector<double>& getBBXValue(int i) {return m_bbxValues[i];}
    double getScale(){return m_scale;}

//////////////////////////////////////////////////////////////////////
// Private Stuff
//////////////////////////////////////////////////////////////////////
private:
    string m_filename;
    vector<vector<double> > m_bbxValues;
    double m_scale;
};

#endif // !defined(AFX_BOUNDINGBOXPARSER_H__FF5525B3_FF61_4863_BEF1_6B872B99E3E7__INCLUDED_)
