#ifndef PLUM_H_
#define PLUM_H_

#include <vector>

#include "GLModel.h"
#include "Utilities/GL/gliDataStructure.h"

using namespace std;

namespace plum {

  class PlumObject;

  class Plum {
    public:

      Plum() {}

      void AddPlumObject(PlumObject* obj){if( obj!=NULL ) m_plumObjects.push_back(obj);}
      vector<PlumObject*>& GetPlumObjects() {return m_plumObjects;}
      void CleanPlumObjects() {m_plumObjects.clear();}

      void AddSelectedItem(GLModel* l) {m_selectedItems.push_back(l);}
      vector<gliObj>& GetSelectedItems() {return m_selectedItems;}
      void CleanSelectedItems() {m_selectedItems.clear();}

      //clear both the plum objects vector and selected items vector
      void Clean();

      //parse data for plum object
      bool ParseFile();

      //build models to draw
      BuildState BuildModels();

      //draw/redraw
      void Draw();

      //select object by given x, y screen coordinate.
      void Select(const gliBox& box);

    protected:

      /**
       * Parse the Hit Buffer.
       * Store selected obj into m_selectedItems.
       *
       * hit is the number of hit by this selection
       * buffer is the hit buffer
       * if all, all obj select will be put into m_selectedItems,
       *  otherwise only the closest will be selected.
       */
      void SearchSelectedItems(int hit, void * buffer, bool all);

    private:
      vector<PlumObject*> m_plumObjects;
      vector<gliObj> m_selectedItems;
  };

}

#endif
