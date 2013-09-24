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

//EnvModel.h includes MultiBodyModel.h
#include "Models/EnvModel.h"
#include "Models/RobotModel.h"

#include <RAPID.H>

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

  /*bool IsInCollision(CEnvLoader* envLoader,
  		     MultiBodyModel* robot,
		     MultiBodyModel* obstacle);

  bool IsInCollision(int numMB, CEnvModel* env,
		     CEnvLoader* envLoader,
		     MultiBodyModel * robotModel,
		     RobotModel * robotObj);*/

  bool IsInCollision(EnvModel* _envModel,
  		     MultiBodyModel* _robot,
		     MultiBodyModel* _obstacle);

  bool IsInCollision(int _numMB, EnvModel* _envModel,
		     MultiBodyModel* _robotModel,
		     RobotModel* _robotObj);

  void CopyNodeCfg(vector<double>& cfg, int dof);
  vector<double> nodeCfg;
  bool TestNode;
  //  CollisionDetectionMethod cdMethod;

  RAPID_model* m_rapid;

  Rapid* rapid;

};

class CollisionDetectionMethod {

 public:
  CollisionDetectionMethod(){}
  virtual  ~CollisionDetectionMethod(){}

  virtual string GetName() const = 0;
  virtual bool IsInCollision(MultiBodyModel* robot,
			     RobotModel * robotObj, int dof,
			     MultiBodyModel* obstacle) = 0;
};

class Rapid: public CollisionDetectionMethod {
 public:

  Rapid() : DoF(0) {test_node = false;}
  virtual ~Rapid(){}

  virtual string GetName() const {return "RAPID";}

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
  //  virtual bool IsInCollision(MultiBodyModel* robot,
  //			     MultiBodyModel* obstacle);

  virtual bool IsInCollision(MultiBodyModel * robot,
			     RobotModel * robotObj, int dof,
  			     MultiBodyModel * obstacle);

  void RCopyNodeCfg(vector<double>& n_cfg, int dof);

  ///////////////////////////////////////
  // Variables
  //////////////////////////////////////
 public:
  int DoF;
  vector<double> nodeCfg; //to keep node cfg.
  bool test_node; //to know whether this is a CD for a node
};
