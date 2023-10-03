#ifndef VIZMO_TRAITS_H_
#define VIZMO_TRAITS_H_

#include "ConfigurationSpace/Cfg.h"
#include "ConfigurationSpace/Weight.h"

/*
//distance metric includes
#include "DistanceMetrics/BinaryLPSweptDistance.h"
#include "DistanceMetrics/CenterOfMassDistance.h"
#include "DistanceMetrics/EuclideanDistance.h"
#include "DistanceMetrics/KnotTheoryDistance.h"
#include "DistanceMetrics/LPSweptDistance.h"
#include "DistanceMetrics/ManhattanDistance.h"
#include "DistanceMetrics/ReachableDistance.h"
#include "DistanceMetrics/RMSDDistance.h"
#include "DistanceMetrics/ScaledEuclideanDistance.h"

//validity checker includes
#include "AvoidRegionValidity.h"
#include "ValidityCheckers/AlwaysTrueValidity.h"
#include "ValidityCheckers/CollisionDetectionValidity.h"
#include "ValidityCheckers/ComposeValidity.h"
#include "ValidityCheckers/MedialAxisClearanceValidity.h"
#include "ValidityCheckers/NegateValidity.h"
#include "ValidityCheckers/NodeClearanceValidity.h"
#include "ValidityCheckers/ObstacleClearanceValidity.h"

//neighborhood finder includes
#include "NeighborhoodFinders/BandsNF.h"
#include "NeighborhoodFinders/BruteForceNF.h"
#include "NeighborhoodFinders/CGALNF.h"
#include "NeighborhoodFinders/DPESNF.h"
#include "NeighborhoodFinders/HierarchicalNF.h"
#include "NeighborhoodFinders/HopLimitNF.h"
#include "NeighborhoodFinders/OptimalNF.h"
#include "NeighborhoodFinders/RadiusNF.h"
//#include "NeighborhoodFinders/RandomNF.h" // graph-map problems

//sampler includes
#include "Samplers/BridgeTestSampler.h"
#include "Samplers/GaussianSampler.h"
#include "Samplers/GridSampler.h"
#include "Samplers/MedialAxisSampler.h"
#include "Samplers/MixSampler.h"
#include "Samplers/ObstacleBasedSampler.h"
#include "Samplers/SimilarStructureSampler.h"
#include "Samplers/UniformMedialAxisSampler.h"
#include "Samplers/UniformObstacleBasedSampler.h"
#include "Samplers/UniformRandomSampler.h"
#include "Samplers/WorkspaceImportanceSampler.h"

//local planner includes
#include "LocalPlanners/AStar.h"
#include "LocalPlanners/HierarchicalLP.h"
#include "LocalPlanners/MedialAxisLP.h"
#include "LocalPlanners/RotateAtS.h"
#include "LocalPlanners/StraightLine.h"
#include "LocalPlanners/ToggleLP.h"
#include "LocalPlanners/TransformAtS.h"
#include "LocalPlanners/ApproxSpheres.h"

//extenders includes
#include "Extenders/BasicExtender.h"
#include "Extenders/MedialAxisExtender.h"
#include "Extenders/MixExtender.h"
#include "Extenders/RandomObstacleVector.h"
#include "Extenders/RotationThenTranslation.h"
#include "Extenders/TraceCSpaceObstacle.h"
#include "Extenders/TraceMAPush.h"
#include "Extenders/TraceObstacle.h"

//path smoothing includes
#include "PathModifiers/CombinedPathModifier.h"
#include "PathModifiers/CRetractionPathModifier.h"
#include "PathModifiers/MedialAxisPathModifier.h"
#include "PathModifiers/ResamplePathModifier.h"
#include "PathModifiers/ShortcuttingPathModifier.h"

//connector includes
//#include "Connectors/AdaptiveConnector.h" // graph-map problems
#include "Connectors/CCExpansion.h"
#include "Connectors/CCsConnector.h"
#include "Connectors/ClosestVE.h"
#include "Connectors/NeighborhoodConnector.h"
//#include "Connectors/RewireConnector.h" // graph-map problems
#include "Connectors/RRTConnect.h"

//metric includes
#include "Metrics/CCDistanceMetric.h"
#include "Metrics/ConnectivityMetric.h"
#include "Metrics/CoverageDistanceMetric.h"
#include "Metrics/CoverageMetric.h"
#include "Metrics/DiameterMetric.h"
#include "Metrics/NumEdgesMetric.h"
#include "Metrics/NumNodesMetric.h"
#include "Metrics/RoadmapSet.h"
#include "Metrics/TimeMetric.h"
#include "Metrics/VectorSet.h"

//map evaluator includes
#include "MapEvaluators/ComposeEvaluator.h"
#include "MapEvaluators/ConditionalEvaluator.h"
#include "MapEvaluators/LazyQuery.h"
#include "MapEvaluators/LazyToggleQuery.h"
#include "MapEvaluators/NegateEvaluator.h"
#include "MapEvaluators/PrintMapEvaluation.h"
#include "MapEvaluators/PRMQuery.h"
#include "MapEvaluators/RRTQuery.h"
#include "MapEvaluators/TimeEvaluator.h"
#include "MapEvaluators/TrueEvaluation.h"

//mp strategies includes
#include "MPStrategies/AdaptiveRRT.h"
#include "MPStrategies/BasicPRM.h"
#include "MPStrategies/BasicRRTStrategy.h"
#include "MPStrategies/DynamicDomainRRT.h"
#include "MPStrategies/DynamicRegionRRT.h"
#include "MPStrategies/EvaluateMapStrategy.h"
#include "MPStrategies/HybridPRM.h"
#include "MPStrategies/LocalManeuveringStrategy.h"
#include "MPStrategies/LPCompare.h"
#include "MPStrategies/ModifyPath.h"
#include "MPStrategies/MultiStrategy.h"
#include "MPStrategies/PushQueryToMA.h"
#include "MPStrategies/SparkPRM.h"
#include "MPStrategies/SRTStrategy.h"
#include "MPStrategies/TogglePRMStrategy.h"
#include "MPStrategies/VisibilityBasedPRM.h"

//vizmo strateges includes
#include "CfgOracle.h"
#include "IRRTStrategy.h"
#include "PathOracle.h"
#include "PathStrategy.h"
#include "RegionOracle.h"
#include "RegionStrategy.h"
#include "RegionRRT.h"
#include "SparkRegion.h"
*/

#include "MPProblem/MPProblem.h"
#include "MPLibrary/GoalTracker.h"
#include "MPLibrary/MPLibrary.h"
#include "MPLibrary/MPSolution.h"
#include "MPLibrary/MPTools/MPTools.h"

#include "ConfigurationSpace/LocalObstacleMap.h"
#include "ConfigurationSpace/GenericStateGraph.h"
#include "ConfigurationSpace/GroupCfg.h"
#include "ConfigurationSpace/GroupLocalPlan.h"
#include "ConfigurationSpace/GroupPath.h"
#include "ConfigurationSpace/GroupRoadmap.h"
#include "ConfigurationSpace/Path.h"
#include "ConfigurationSpace/Weight.h"
#include "ConfigurationSpace/GenericStateGraph.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief Defines the motion planning objects available in Vizmo.
////////////////////////////////////////////////////////////////////////////////
struct VizmoTraits {

  // Local types.
  typedef CfgModel  CfgType;                    ///< Configuration type.
  typedef EdgeModel WeightType;                 ///< Edge type.
  typedef CfgModel& CfgRef;                     ///< Configuration reference type.
  typedef MPProblem MPProblemType; ///< MPProblem type.

  typedef GenericStateGraph<CfgType, WeightType>         RoadmapType;
  typedef PathType<VizmoTraits>              Path;
  typedef MPLibraryType<VizmoTraits>         MPLibrary;
  typedef MPSolutionType<VizmoTraits>        MPSolution;
  typedef MPToolsType<VizmoTraits>           MPTools;
  typedef LocalObstacleMapType<VizmoTraits>  LocalObstacleMap;
  typedef GoalTrackerType<VizmoTraits>       GoalTracker;

  typedef GroupCfg<RoadmapType>                          GroupCfgType;
  typedef GroupLocalPlan<RoadmapType>                    GroupWeightType;
  typedef GroupRoadmap<GroupCfgType, GroupWeightType>    GroupRoadmapType;
  typedef GroupPath<VizmoTraits>                            GroupPathType;

  //types of distance metrics available in our world
  typedef boost::mpl::list<
  /*
    BinaryLPSweptDistance<VizmoTraits>,
    CenterOfMassDistance<VizmoTraits>,
    EuclideanDistance<VizmoTraits>,
    KnotTheoryDistance<VizmoTraits>,
    LPSweptDistance<VizmoTraits>,
    ManhattanDistance<VizmoTraits>,
    MinkowskiDistance<VizmoTraits>,
    RMSDDistance<VizmoTraits>,
    ScaledEuclideanDistance<VizmoTraits>
    */
      > DistanceMetricMethodList;

  //types of validity checkers available in our world
  typedef boost::mpl::list<
  /*
    AlwaysTrueValidity<VizmoTraits>,
    AvoidRegionValidity<VizmoTraits>,
    CollisionDetectionValidity<VizmoTraits>,
    ComposeValidity<VizmoTraits>,
    MedialAxisClearanceValidity<VizmoTraits>,
    NegateValidity<VizmoTraits>,
    NodeClearanceValidity<VizmoTraits>,
    ObstacleClearanceValidity<VizmoTraits>
    */
      > ValidityCheckerMethodList;

  //types of neighborhood finders available in our world
  typedef boost::mpl::list<
  /*
    BandsNF<VizmoTraits>,
    BruteForceNF<VizmoTraits>,
    CGALNF<VizmoTraits>,
    DPESNF<VizmoTraits>,
    HierarchicalNF<VizmoTraits>,
    HopLimitNF<VizmoTraits>,
    //MPNNNF<VizmoTraits>,
    OptimalNF<VizmoTraits>,
    RadiusNF<VizmoTraits>//,
    //RandomNF<VizmoTraits>
    */
    > NeighborhoodFinderMethodList;

  //types of samplers available in our world
  typedef boost::mpl::list<
  /*
    BridgeTestSampler<VizmoTraits>,
    GaussianSampler<VizmoTraits>,
    GridSampler<VizmoTraits>,
    MedialAxisSampler<VizmoTraits>,
    MixSampler<VizmoTraits>,
    ObstacleBasedSampler<VizmoTraits>,
    SimilarStructureSampler<VizmoTraits>,
    UniformMedialAxisSampler<VizmoTraits>,
    UniformObstacleBasedSampler<VizmoTraits>,
    UniformRandomSampler<VizmoTraits>,
    WorkspaceImportanceSampler<VizmoTraits>
    */
      > SamplerMethodList;

  //types of local planners available in our world
  typedef boost::mpl::list<
  /*
    AStarClearance<VizmoTraits>,
    AStarDistance<VizmoTraits>,
    HierarchicalLP<VizmoTraits>,
    MedialAxisLP<VizmoTraits>,
    RotateAtS<VizmoTraits>,
    StraightLine<VizmoTraits>,
    ToggleLP<VizmoTraits>,
    TransformAtS<VizmoTraits>,
    ApproxSpheres<VizmoTraits>
    */
      > LocalPlannerMethodList;

  //types of extenders avaible in our world
  typedef boost::mpl::list<
  /*
    BasicExtender<VizmoTraits>,
    MedialAxisExtender<VizmoTraits>,
    MixExtender<VizmoTraits>,
    RandomObstacleVector<VizmoTraits>,
    RotationThenTranslation<VizmoTraits>,
    TraceCSpaceObstacle<VizmoTraits>,
    TraceMAPush<VizmoTraits>,
    TraceObstacle<VizmoTraits>
    */
      > ExtenderMethodList;

  //types of path smoothing available in our world
  typedef boost::mpl::list<
  /*
    CRetractionPathModifier<VizmoTraits>,
    CombinedPathModifier<VizmoTraits>,
    MedialAxisPathModifier<VizmoTraits>,
    ResamplePathModifier<VizmoTraits>,
    ShortcuttingPathModifier<VizmoTraits>
    */
      > PathModifierMethodList;

  //types of connectors available in our world
  typedef boost::mpl::list<
  /*
    //AdaptiveConnector<VizmoTraits>,
    CCExpansion<VizmoTraits>,
    CCsConnector<VizmoTraits>,
    ClosestVE<VizmoTraits>,
    NeighborhoodConnector<VizmoTraits>//,
    //PreferentialAttachment<VizmoTraits>,
    //RewireConnector<VizmoTraits>,
    //RRTConnect<VizmoTraits>
    */
      > ConnectorMethodList;

  /*typedef ConnectivityMetric<VizmoTraits, RoadmapSet<VizmoTraits>>
      ConnectivityMetricRoadmapSet;
  typedef CoverageDistanceMetric<VizmoTraits, RoadmapSet<VizmoTraits>>
      CoverageDistanceMetricRoadmapSet;
  typedef CoverageMetric<VizmoTraits, RoadmapSet<VizmoTraits>>
      CoverageMetricRoadmapSet;

  typedef ConnectivityMetric<VizmoTraits, VectorSet<VizmoTraits>>
      ConnectivityMetricVectorSet;
  typedef CoverageDistanceMetric<VizmoTraits, VectorSet<VizmoTraits>>
      CoverageDistanceMetricVectorSet;
  typedef CoverageMetric<VizmoTraits, VectorSet<VizmoTraits>>
      CoverageMetricVectorSet;
  */

  //types of metrics available in our world
  typedef boost::mpl::list<
  /*
    CCDistanceMetric<VizmoTraits>,
    ConnectivityMetricRoadmapSet,
    CoverageDistanceMetricRoadmapSet,
    CoverageMetricRoadmapSet,
    ConnectivityMetricVectorSet,
    CoverageDistanceMetricVectorSet,
    CoverageMetricVectorSet,
    DiameterMetric<VizmoTraits>,
    NumEdgesMetric<VizmoTraits>,
    NumNodesMetric<VizmoTraits>,
    TimeMetric<VizmoTraits>
    */
      > MetricMethodList;

  //types of map evaluators available in our world
  typedef boost::mpl::list<
  /*
    ComposeEvaluator<VizmoTraits>,
    ConditionalEvaluator<VizmoTraits>,
    LazyQuery<VizmoTraits>,
    LazyToggleQuery<VizmoTraits>,
    NegateEvaluator<VizmoTraits>,
    PrintMapEvaluation<VizmoTraits>,
    PRMQuery<VizmoTraits>,
    RRTQuery<VizmoTraits>,
    TimeEvaluator<VizmoTraits>,
    TrueEvaluation<VizmoTraits>
    */
      > MapEvaluatorMethodList;

  //types of motion planning strategies available in our world
  typedef boost::mpl::list<
  /*
    AdaptiveRRT<VizmoTraits>,
    BasicPRM<VizmoTraits>,
    BasicRRTStrategy<VizmoTraits>,
    DynamicDomainRRT<VizmoTraits>,
    DynamicRegionRRT<VizmoTraits>,
    EvaluateMapStrategy<VizmoTraits>,
    HybridPRM<VizmoTraits>,
    LPCompare<VizmoTraits>,
    ModifyPath<VizmoTraits>,
    MultiStrategy<VizmoTraits>,
    PushQueryToMA<VizmoTraits>,
    SparkPRM<VizmoTraits, BasicPRM>,
    SparkPRM<VizmoTraits, TogglePRMStrategy>,
    SRTStrategy<VizmoTraits>,
    TogglePRMStrategy<VizmoTraits>,
    VisibilityBasedPRM<VizmoTraits>,
    // Vizmo-only strategies:
    CfgOracle<VizmoTraits>,
    IRRTStrategy<VizmoTraits>,
    PathOracle<VizmoTraits>,
    PathStrategy<VizmoTraits>,
    RegionOracle<VizmoTraits>,
    RegionRRT<VizmoTraits>,
    RegionStrategy<VizmoTraits>,
    SparkRegion<VizmoTraits>,
    SparkPRM<VizmoTraits, SparkRegion>
    */
    > MPStrategyMethodList;

  typedef boost::mpl::list<
      > EdgeValidityCheckerMethodList;
};

typedef MPProblem VizmoProblem; ///< Vizmo's MPProblem type.

#endif
