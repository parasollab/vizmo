#ifndef PLUM_H_
#define PLUM_H_

#include <vector>
using namespace std;

#include "GLModel.h"

namespace plum {

  class Plum {
    public:

      Plum() {}

      void AddGLModel(GLModel* _model){if(_model) m_glModels.push_back(_model);}
      vector<GLModel*>& GetGLModels() {return m_glModels;}

      vector<GLModel*>& GetSelectedItems() {return m_selectedItems;}

      //clear both the gl models vector and selected items vector
      void Clean();

      //parse data for gl models
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
      vector<GLModel*> m_glModels;
      vector<GLModel*> m_selectedItems;
  };

}

#endif
