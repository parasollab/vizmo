#!/bin/sh

# Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
# component of the Texas A&M University System.
#
# All rights reserved.
#
# The information and source code contained herein is the exclusive
# property of TEES and may not be disclosed, examined or reproduced
# in whole or in part without explicit written authorization from TEES.

run_command=$1

# remove the output file from any previous runs.
rm -f graph.out

eval $run_command ./graph 100 30
if test $? != 0
then
    echo "ERROR:: while testing graph"
fi

eval $run_command ./dynamic_graph 100 30
if test $? != 0
then
    echo "ERROR:: while testing dynamic graph"
fi

eval $run_command ./csr 100 30
if test $? != 0
then
    echo "ERROR:: while testing csr graph"
fi

eval $run_command ./k_core 1234 35
if test $? != 0
then
    echo "ERROR:: while testing k_core"
    exit
fi

eval $run_command ./k_core_dynamic 1234 16 35
if test $? != 0
then
    echo "ERROR:: while testing k_core_dynamic"
fi

eval $run_command ./page_rank 12 16
if test $? != 0
then
    echo "ERROR:: while testing page_rank"
fi

eval $run_command ./breadth_first_search 1000 10
if test $? != 0
then
    echo "ERROR:: while testing breadth_first_search"
fi

eval $run_command ./sssp 100 20 --tuning 12
if test $? != 0
then
    echo "ERROR:: while testing sssp"
fi

eval $run_command ./mssp 50 20 --tuning 12 --num_sources 4
if test $? != 0
then
    echo "ERROR:: while testing mssp"
fi

eval $run_command ./connected_components 200
if test $? != 0
then
    echo "ERROR:: while testing connected_components"
fi


eval $run_command ./community_detection 30 20
if test $? != 0
then
    echo "ERROR:: while testing community_detection"
fi

eval $run_command ./pseudo_diameter 27 13 --tuning 17
if test $? != 0
then
    echo "ERROR:: while testing pseudo_diameter"
fi

eval $run_command ./random_walk 300 400 --path_length 100
if test $? != 0
then
    echo "ERROR:: while testing random_walk"
fi

eval $run_command ./triangle_count 3000 700 --num_triangles 567
if test $? != 0
then
    echo "ERROR:: while testing triangle_count"
fi

eval $run_command ./link_prediction 22 33
if test $? != 0
then
    echo "ERROR:: while testing link_prediction"
fi

eval $run_command ./closeness_centrality 30 40  --tuning 40
if test $? != 0
then
    echo "ERROR:: while testing closeness_centrality"
fi

eval $run_command ./topological_sort 1 1000 23
if test $? != 0
then
    echo "ERROR:: while testing topological_sort"
fi

eval $run_command ./create_level 43 41
if test $? != 0
then
    echo "ERROR:: while testing create_level"
fi

eval $run_command ./create_level_partial_info 431
if test $? != 0
then
    echo "ERROR:: while testing create_level_partial_info"
fi

eval $run_command ./create_hierarchy 43 41
if test $? != 0
then
    echo "ERROR:: while testing create_hierarchy"
fi

eval $run_command ./graph_coloring 16 16
if test $? != 0
then
    echo "ERROR:: while testing graph_coloring"
fi

eval $run_command ./maximal_bipartite_matching 1234 9999 --group_size 349
if test $? != 0
then
    echo "ERROR:: while testing maximal_bipartite_matching"
fi

eval $run_command ./boruvka 200
if test $? != 0
then
    echo "ERROR:: while testing boruvka"
fi

eval $run_command ./hierarchical_view 10 4
if test $? != 0
then
    echo "ERROR:: while testing hierarchical_view"
fi

eval $run_command ./hierarchical_graph 100 30
if test $? != 0
then
    echo "ERROR:: while testing hierarchical graph"
fi

eval $run_command ./hgraph_view 10 4
if test $? != 0
then
    echo "ERROR:: while testing hgraph_view"
fi

eval $run_command ./graph_metrics 100 40
if test $? != 0
then
    echo "ERROR:: while testing graph_metrics"
fi

eval $run_command ./rebalance_global 1000 16
if test $? != 0
then
    echo "ERROR:: while testing rebalance_global"
fi

eval $run_command ./rebalance_diffusive 256 4
if test $? != 0
then
    echo "ERROR:: while testing rebalance_diffusive"
fi

eval $run_command ./test_balance_partitioner_example 1000 11
if test $? != 0
then
    echo "ERROR:: while testing balance_partitioner_example"
fi

eval $run_command ./weighted_balanced_repartitioner 300 300
if test $? != 0
then
    echo "ERROR:: while testing weighted_balanced_repartitioner"
fi

if [ -n "${METIS_ROOT}" ]
then
    eval $run_command ./multilevel_partitioner 4 5 5 4 100 100
    if test $? != 0
    then
        echo "ERROR:: while testing multilevel_partitioner"
    fi
fi

eval $run_command ./test_implicit_regular_mesh 12 10 9
if test $? != 0
then
    echo "ERROR:: while testing implicit regular mesh"
fi

eval $run_command ./test_regular_spatial_decomposition 12 10 9 4 2
if test $? != 0
then
    echo "ERROR:: while testing regular spatial decomposition"
fi

if [ -n "${HDF5_ROOT}" ] && [ -n "${SILO_ROOT}" ]
then
  eval $run_command ./test_arbitrary_mesh_decomposition_2D inputs/tri_800.root 2 2
  if test $? != 0
  then
      echo "ERROR:: while testing arbitrary mesh decomposition 2D"
  fi

  eval $run_command ./test_arbitrary_mesh_decomposition_3D inputs/tet10500.root 2 2 2
  if test $? != 0
  then
      echo "ERROR:: while testing arbitrary mesh decomposition 3D"
  fi
fi

eval $run_command ./cut_conductance 250 12 2 0.1
if test $? != 0
then
    echo "ERROR:: while testing ./cut_conductance"
fi

eval $run_command ./pscc 0 400 10
if test $? != 0
then
    echo "ERROR:: while testing ./test_pscc on SCCMulti"
fi

eval $run_command ./pscc 1 400 10
if test $? != 0
then
    echo "ERROR:: while testing ./pscc on DCSC"
fi

eval $run_command ./pscc 2 400 2
if test $? != 0
then
    echo "ERROR:: while testing ./pscc on Schudy's MultiPivot"
fi

eval $run_command ./graph_test
if test $? != 0
then
    echo "ERROR:: while testing const correctness in graph_test"
fi

eval $run_command ./graph_redistribution
if test $? != 0
then
    echo "ERROR:: while testing redistribution of graph"
fi

eval $run_command ./graph_viewbased_dist
if test $? != 0
then
    echo "ERROR:: while testing graph viewbased distribution"
fi

eval $run_command ./dynamic_graph_viewbased_dist
if test $? != 0
then
    echo "ERROR:: while testing dynamic graph viewbased distribution"
fi

eval $run_command ./hub_avoiding_kla 1159
if test $? != 0
then
    echo "ERROR:: while testing hub avoiding KLA"
fi

graph_out_error=`wc -l graph.out | sed 's/ graph.out//'`
if test ${graph_out_error} != 121
then
    echo "ERROR:: while verifying the write_graph output"
else
    echo "verifying the write_graph output...             [PASSED]"
fi

eval $run_command ./view_based_dynamic_graph
if test $? != 0
then
    echo "ERROR:: while testing dynamic graph viewbased distribution adds"
fi

