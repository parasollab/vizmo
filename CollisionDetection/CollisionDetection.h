////////////////////////////////////////////////////////////
/**@file CollisionDetection.h
   @date Nov. 2004
*/
// 
// 
// The following classes are defined:
//     * CollisonDetection
//     * CollisonDetectionMethod
//     * Rapid
//
// We will implement RAPID first, based on the 
// CollisionDetection class from OBPRM.
// This structure will allow the addition 
// of more CD methods
// 
///////////////////////////////////////////////////////////

//include math files
#include<Vector.h>
#include<Matrix.h>

//EnvModel.h includes MultiBodyModel.h and EnvLoader.h
#include "Plum/EnvObj/EnvModel.h"
#include "Plum/EnvObj/Transformation.h"
#include "Plum/EnvObj/PolyhedronModel.h"
using namespace plum;

#include "src/EnvObj/Robot.h"

#include <RAPID.H>


/* class CollisionDetectionMethod; */
/* class CEnvLoader;*/
/* class CEnvModel; */
/*class CMultiBodyModel;*/

//class Rapid;

#ifndef Rapid
class Rapid;
#endif

class CollisionDetection{

 public:
  CollisionDetection();
  ~CollisionDetection();
  
  /**Check collision for Robot with all obstacles.
   *@return true if Robot collides with Obstacle(s).
   */
  
  bool IsInCollision(plum::CEnvLoader* envLoader, 
		     plum::CMultiBodyModel* robot, 
		     plum::CMultiBodyModel* obstacle);

  bool IsInCollision(int numMB, plum::CEnvModel* env, 
		     plum::CEnvLoader* envLoader,
		     plum::CMultiBodyModel * robotModel,
		     OBPRMView_Robot * robotObj);

  void CopyNodeCfg(double * cfg, int dof);
  double *nodeCfg;
  bool TestNode;
  //  CollisionDetectionMethod cdMethod;

  RAPID_model * m_rapid;

  Rapid *rapid;

};

class CollisionDetectionMethod {

 public:
  CollisionDetectionMethod(){}
  virtual  ~CollisionDetectionMethod(){}

  virtual const char* GetName() const = 0;
  virtual bool IsInCollision(plum::CMultiBodyModel* robot, 
			     OBPRMView_Robot * robotObj, int dof,
			     plum::CMultiBodyModel* obstacle) = 0;
};

class Rapid: public CollisionDetectionMethod {
 public:

  Rapid(){test_node = false;}
  virtual ~Rapid(){}

  virtual const char* GetName() const { string s = "RAPID"; return s.c_str();}
  
  /**RAPID to check collision between two MultiBody.
   *Collision is checked at body level between two MultiBody objects,
   *if any Robot's polyhedron collides with any obstacle,
   *true will be returned.
   *
   *@note if RAPID_Collide, the RAPID method, return false, process will 
   *be terminated.
   *@note collision between two ajacent links will be ignored.
   *@return true if Collision found, false otherwise.
   */
  //  virtual bool IsInCollision(plum::CMultiBodyModel* robot, 
  //			     plum::CMultiBodyModel* obstacle);

  virtual bool IsInCollision(CMultiBodyModel * robot, 
			     OBPRMView_Robot * robotObj, int dof,
  			     CMultiBodyModel * obstacle);

  void RCopyNodeCfg(double * n_cfg, int dof);

  ///////////////////////////////////////
  // Variables
  //////////////////////////////////////
 public:
  int DoF;
  double * nodeCfg; //to keep node cfg.
  bool test_node; //to know whether this is a CD for a node
};
