/* Traits class for PMPL MPProblem.
*/

#ifndef VIZMOTRAITS_H_
#define VIZMOTRAITS_H_

#include "Cfg/Cfg.h"
#include "MPProblem/Weight.h"

//distance metric includes
#include "DistanceMetrics/EuclideanDistance.h"

//validity checker includes
#include "ValidityCheckers/AlwaysTrueValidity.h"
#include "AvoidRegionValidity.h"
#include "ValidityCheckers/CollisionDetectionValidity.h"
#include "ValidityCheckers/ComposeValidity.h"

//neighborhood finder includes
#include "NeighborhoodFinders/BruteForceNF.h"
#include "NeighborhoodFinders/RadiusNF.h"

//sampler includes
#include "Samplers/UniformRandomSampler.h"
#include "Samplers/ObstacleBasedSampler.h"
#include "Samplers/GaussianSampler.h"
#include "Samplers/UniformObstacleBasedSampler.h"

//local planner includes
#include "LocalPlanners/StraightLine.h"

//extenders includes
#include "Extenders/BasicExtender.h"

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
#include "MapEvaluators/Query.h"

//mp strategies includes
#include "MPStrategies/BasicRRTStrategy.h"
#include "MPStrategies/BasicPRM.h"
#include "MPStrategies/SparkPRM.h"
#include "IRRTStrategy.h"
#include "PathStrategy.h"
#include "RegionStrategy.h"
#include "RegionRRT.h"
#include "SparkRegion.h"

#include "MPProblem/MPProblem.h"

struct VizmoTraits {
  typedef CfgModel CfgType;
  typedef EdgeModel WeightType;
  typedef CfgModel& CfgRef;

  typedef MPProblem<VizmoTraits> MPProblemType;

  //types of distance metrics available in our world
  typedef boost::mpl::list<
    EuclideanDistance<VizmoTraits>
    > DistanceMetricMethodList;

  //types of validity checkers available in our world
  typedef boost::mpl::list<
    AvoidRegionValidity<VizmoTraits>,
    CollisionDetectionValidity<VizmoTraits>,
    ComposeValidity<VizmoTraits>
      > ValidityCheckerMethodList;

  //types of neighborhood finders available in our world
  typedef boost::mpl::list<
    BruteForceNF<VizmoTraits>,
    RadiusNF<VizmoTraits>
      > NeighborhoodFinderMethodList;

  //types of samplers available in our world
  typedef boost::mpl::list<
    UniformRandomSampler<VizmoTraits>,
    ObstacleBasedSampler<VizmoTraits>,
    GaussianSampler<VizmoTraits>,
    UniformObstacleBasedSampler<VizmoTraits>
      > SamplerMethodList;

  //types of local planners available in our world
  typedef boost::mpl::list<
    StraightLine<VizmoTraits>
    > LocalPlannerMethodList;

  //types of extenders avaible in our world
  typedef boost::mpl::list<
    BasicExtender<VizmoTraits>
    > ExtenderMethodList;

  //types of path smoothing available in our world
  typedef boost::mpl::list<
    ShortcuttingPathModifier<VizmoTraits>
    > PathModifierMethodList;

  //types of connectors available in our world
  typedef boost::mpl::list<
    NeighborhoodConnector<VizmoTraits>
    > ConnectorMethodList;

  //types of metrics available in our world
  typedef boost::mpl::list<
    NumEdgesMetric<VizmoTraits>,
    NumNodesMetric<VizmoTraits>
      > MetricMethodList;

  //types of map evaluators available in our world
  typedef boost::mpl::list<
    ComposeEvaluator<VizmoTraits>,
    ConditionalEvaluator<VizmoTraits>,
    PrintMapEvaluation<VizmoTraits>,
    Query<VizmoTraits>
      > MapEvaluatorMethodList;

  //types of motion planning strategies available in our world
  typedef boost::mpl::list<
    BasicPRM<VizmoTraits>,
    BasicRRTStrategy<VizmoTraits>,
    IRRTStrategy<VizmoTraits>,
    PathStrategy<VizmoTraits>,
    RegionRRT<VizmoTraits>,
    RegionStrategy<VizmoTraits>,
    SparkRegion<VizmoTraits>,
    SparkPRM<VizmoTraits, SparkRegion>
    > MPStrategyMethodList;

};

typedef MPProblem<VizmoTraits> VizmoProblem;

#endif
