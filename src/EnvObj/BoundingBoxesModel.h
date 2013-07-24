// BoundingBoxesModel.h: interface for the CBoundingBoxesModel class.
//
//////////////////////////////////////////////////////////////////////

#ifndef BOUNDINGBOXESMODEL_H_
#define BOUNDINGBOXESMODEL_H_

//////////////////////////////////////////////////////////////////////
//Plum Headers
#include "Plum/GLModel.h"

//////////////////////////////////////////////////////////////////////
//OBPRMView Headers
#include "BoundingBoxParser.h"
#include "Models/BoundingBoxModel.h"

class CBoundingBoxesModel : public GLModel
{
   public:
      //////////////////////////////////////////////////////////////////////
      // Constructor/Destructor
      //////////////////////////////////////////////////////////////////////
      CBoundingBoxesModel();
      virtual ~CBoundingBoxesModel();

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
      virtual vector<string> GetInfo() const;
      virtual void Select( unsigned int * index, vector<GLModel*>& sel );
      virtual void GetChildren( list<GLModel*>& models ){
         typedef vector<BoundingBoxModel*>::iterator BIT;
         for(BIT i=m_bbxModels.begin();i!=m_bbxModels.end();i++)
            models.push_back(*i);
      }

      //////////////////////////////////////////////////////////////////////
      // Private functions and data
      //////////////////////////////////////////////////////////////////////
   private:
      CBoundingBoxParser * m_pBBXParser;
      vector<BoundingBoxModel*> m_bbxModels;

      enum overlapType {O,EN,EP,D};
      vector<BoundingBoxModel*> m_bbxOverlaps;

      //overlapType classify(double min1, double max1, double min2, double max2);
      //vector<double> overlap(vector<double>& a, vector<double>& b);
      //void BuildOverlapModel(vector<double>& a, vector<double>& b);
};

#endif
