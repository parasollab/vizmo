// CfgLoader.h: interface for the CCmdParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CMDPARSER_H_)
#define _CMDPARSER_H_

#include <string>
using namespace std;

namespace plum{

    class CCmdParser  
    {
    public:
            CCmdParser();

            bool ParseCmd( const string & dir, const string & cmd );
            const string GetModelDataDir() const{ return m_strModelDataDir; }
    protected:
            bool FindModelDataDir( string cmd );
    private:
            string m_strModelDataDir;
    };

}//namespace plum

#endif // !defined(_CMDPARSER_H_)
