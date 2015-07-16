// Traits class for PMPL MPProblem.

#ifndef VIZMO_STATE_TRAITS_H_
#define VIZMO_STATE_TRAITS_H_

//#include "Cfg/State.h"
//#include "Edges/StateEdge.h"

//distance metric includes
#include "DistanceMetrics/EuclideanDistance.h"
#include "DistanceMetrics/WeightedEuclideanDistance.h"

//validity checker includes
#include "AvoidRegionValidity.h"
#include "ValidityCheckers/CollisionDetectionValidity.h"
#include "ValidityCheckers/ComposeValidity.h"

//neighborhood finder includes
#include "NeighborhoodFinders/BruteForceNF.h"
#include "NeighborhoodFinders/RadiusNF.h"

//sampler includes
#include "Samplers/UniformRandomSampler.h"

//local planner includes
#include "LocalPlanners/StraightLine.h"

//extenders includes
#include "Extenders/KinodynamicExtender.h"

//path smoothing includes
#include "PathModifiers/ShortcuttingPathModifier.h"

//connector includes
#include "Connectors/NeighborhoodConnector.h"

//metric includes
#include "Metrics/NumEdgesMetric.h"
#include "Metrics/NumNodesMetric.h"

//map evaluator includes
#include "MapEvaluators/ComposeEvaluator.h"
#include "MapEvaluators/ConditionalEvaluator.h"
#include "MapEvaluators/PrintMapEvaluation.h"
//#include "MapEvaluators/Query.h"

//mp strategies includes
//#include "MPStrategies/KinodynamicRRTStrategy.h"
#include "KinodynamicRegionRRT.h"

#include "MPProblem/MPProblem.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief Defines the motion planning objects available in Vizmo.
////////////////////////////////////////////////////////////////////////////////
struct VizmoTraits {

  // Local types.
  typedef CfgModel  CfgType;      ///< Vizmo's configuration type.
  typedef EdgeModel WeightType;   ///< Vizmo's edge type.
  typedef CfgModel& CfgRef;       ///< Vizmo's configuration reference type.
  typedef MPProblem<VizmoTraits> MPProblemType; ///< Vizmo's MPProblem type.

  //types of distance metrics available in our world
  typedef boost::mpl::list<
    EuclideanDistance<VizmoTraits>,
    WeightedEuclideanDistance<VizmoTraits>
    > DistanceMetricMethodList;   ///< The available distance metrics.

  //types of validity checkers available in our world
  typedef boost::mpl::list<
    AvoidRegionValidity<VizmoTraits>,
    CollisionDetectionValidity<VizmoTraits>,
    ComposeValidity<VizmoTraits>
    > ValidityCheckerMethodList; ///< The available validity checkers.

  //types of neighborhood finders available in our world
  typedef boost::mpl::list<
    BruteForceNF<VizmoTraits>,
    RadiusNF<VizmoTraits>
    > NeighborhoodFinderMethodList; ///< The available neighborhood finders.

  //types of samplers available in our world
  typedef boost::mpl::list<
    UniformRandomSampler<VizmoTraits>
    > SamplerMethodList;  ///< The available samplers.

  //types of local planners available in our world
  typedef boost::mpl::list<
    StraightLine<VizmoTraits>
    > LocalPlannerMethodList;  ///< The available local planners.

  //types of extenders avaible in our world
  typedef boost::mpl::list<
    KinodynamicExtender<VizmoTraits>
    > ExtenderMethodList; ///< The available extenders.

  //types of path smoothing available in our world
  typedef boost::mpl::list<
    ShortcuttingPathModifier<VizmoTraits>
    > PathModifierMethodList; ///< The available path modifiers.

  //types of connectors available in our world
  typedef boost::mpl::list<
    NeighborhoodConnector<VizmoTraits>
    > ConnectorMethodList; ///< The available connectors.

  //types of metrics available in our world
  typedef boost::mpl::list<
    NumEdgesMetric<VizmoTraits>,
    NumNodesMetric<VizmoTraits>
    > MetricMethodList; ///< The available metrics.

  //types of map evaluators available in our world
  typedef boost::mpl::list<
    ComposeEvaluator<VizmoTraits>,
    ConditionalEvaluator<VizmoTraits>,
    PrintMapEvaluation<VizmoTraits>//,
    //Query<VizmoTraits>
    > MapEvaluatorMethodList; ///< The available evaluators.

  //types of motion planning strategies available in our world
  typedef boost::mpl::list<
    KinodynamicRegionRRT<VizmoTraits>
    > MPStrategyMethodList; ///< The available strategies.

};

typedef MPProblem<VizmoTraits> VizmoProblem; ///< Vizmo's MPProblem type.

#endif
