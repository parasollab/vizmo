// Plum.h: interface for the CPlum class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_PLUM_H_)
#define _PLUM_H_

#ifdef WIN32
#pragma warning( disable : 4244 4786 )
#endif 

#include <vector>
#include <string>
using namespace std;

#include "MapObj/MapModel.h"
#include "MapObj/MapLoader.h"
#include "EnvObj/CmdParser.h"
#include "EnvObj/EnvModel.h"
#include "EnvObj/EnvLoader.h"

#include "PlumState.h"
#include "PlumObject.h"

namespace plum{

    class CPlum  
    {
    private:
        
        //////////////////////////////////////////////////////////////////////
        // public data
        //////////////////////////////////////////////////////////////////////
    public:
        
        //////////////////////////////////////////////////////////////////////
        //
        // Constructor/Destructor
        //
        //////////////////////////////////////////////////////////////////////
        
        CPlum();
        virtual ~CPlum();
        
        //////////////////////////////////////////////////////////////////////
        // Core function
        //////////////////////////////////////////////////////////////////////
        int ParseFile();
        int BuildModels();
        
        //////////////////////////////////////////////////////////////////////
        // Access functions
        //////////////////////////////////////////////////////////////////////
        void AddPlumObject( PlumObject * obj ){ 
            if( obj!=NULL ) m_ObjList.push_back(obj); 
        }
        
        vector<gliObj>& GetSelectedItem(){ return m_SelectedItem; }
		vector<PlumObject *>& GetPlumObjects(){ return m_ObjList; }

        //////////////////////////////////////////////////////////////////////
        // Action functions
        //////////////////////////////////////////////////////////////////////
        
        //dump message to std output about selected object
        //void DumpSelected();
        
        //select object by given x, y screen coordinate.
        void Select( const gliBox& box  );
        
        //draw/redraw
        void Draw();

        //clean all objects stored in this plum.
        void Clean();
        
    protected:

        /**
         * Parse the Hit Buffer.
         * Store selected obj into m_SelectedItem.
         * @param all If true, all obj select will be put into #m_SelectedItem,
         *        otherwise only the closest will be selected.
         * @param buffer the hit buffer
         * @param hit number of hit by this selection
         */
        virtual void SearchSelectedItem(int hit, void * buffer, bool all);
        
        ////////////////////////////////////////////////////////////////////////////
        //
        //      Private Methods and data members
        //
        ////////////////////////////////////////////////////////////////////////////
    private:
        
        vector<PlumObject *> m_ObjList;
        vector<gliObj> m_SelectedItem;
    };

}//namespace plum

#endif // !defined(_PLUM_H_)
