
/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_ENVIRONMENTS_GRAPHVIZ_ENV_HPP
#define STAPL_SKELETONS_ENVIRONMENTS_GRAPHVIZ_ENV_HPP

#include <type_traits>
#include <ostream>
#include <iosfwd>
#include <sstream>
#include <fstream>
#include <memory>
#include <stapl/skeletons/flows/producer_info.hpp>
#include <stapl/skeletons/utility/printers.hpp>

namespace stapl {
namespace skeletons {

namespace debug_helpers {

//////////////////////////////////////////////////////////////////////
/// @brief This coloring is used in @c graphviz_env for coloring nodes
/// created by each location with a different color
///
/// @see graphviz_env
///
/// @ingroup skeletonsEnvironmentsInternal
//////////////////////////////////////////////////////////////////////
inline std::string get_color_name(std::size_t i)
{
  switch (i % 16) {
    case 0 : return "red";
    case 1 : return "blue";
    case 2 : return "green";
    case 3 : return "orange";
    case 4 : return "deeppink4";
    case 5 : return "hotpink";
    case 6 : return "khaki4";
    case 7 : return "navy";
    case 8 : return "purple1";
    case 9 : return "turquoise2";
    case 10 : return "tomato";
    case 11 : return "brown";
    case 12 : return "lavenderblush4";
    case 13 : return "gold";
    case 14 : return "chocolate4";
    case 15 : return "magenta2";
    default : return "purple";
  }
}

} // namespace debug_helpers

//////////////////////////////////////////////////////////////////////
/// @brief debug environment can be used along with an output stream
/// such as @c std::cout in order to dump a GraphViz representation of
/// the dependence graph in an output stream.
/// Remember, in the cases that a dynamic (conditional ) dependence
/// graphs such as @c do_while is used, you have to use this
/// along with a real execution environment such as @c taskgraph_env
///
/// @see do_while
/// @see taskgraph_env
///
/// @ingroup skeletonsEnvironments
//////////////////////////////////////////////////////////////////////
class graphviz_env
{
  std::shared_ptr<std::ofstream>   m_out;
  std::size_t                      m_num_PEs;
  runtime::location_id             m_PE_id;
public:
  graphviz_env(std::string name)
    : m_out(new std::ofstream()),
      m_num_PEs(-1),
      m_PE_id(-1)
  {
    if (m_out.unique()) {
      m_out->open(name + "." + std::to_string(get_location_id()) + ".dot");
    }
  }

  ~graphviz_env()
  {
    if (m_out.unique()) {
      m_out->close();
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Every @c spawn_element is converted to a node in the
  /// GraphViz output. The label for this node is in the format of
  /// <b>tid(num_succs)</b>. There is also a color representation which
  /// is associated with each task. This color changes based on the
  /// location id of each task. This is useful when the load balancing
  /// of an algorithm should be checked. Remember that this shows the
  /// load of each processor during the task creation process and does
  /// not necessarily represent the execution load balance.
  ///
  /// @param tid      the unique id assigned to the node in the graph
  /// @param wf       the workfunction is discarded in here
  /// @param no_succs the exact number of successors for this element.
  ///                 Remember that in some cases this value is set to
  ///                 @c stapl::defer_specs (when the @c spawner is in
  ///                 @c SET_HOLD mode). This number is shown in
  ///                 parenthesis in the output graph.
  /// @param in       the producer information for the node's input
  ///                 arguments that will be translated as edge in the
  ///                 output graph
  //////////////////////////////////////////////////////////////////////
  template <bool isResult, typename WF, typename ...In>
  void spawn_element(std::size_t tid, WF&& wf, std::size_t no_succs,
                     In&&... in) const
  {
    spawn_element<isResult>(
      tid, std::vector<std::size_t>(),
      std::forward<WF>(wf), no_succs,
      std::forward<In>(in)...);
  }

  template <bool isResult, typename WF, typename ...In>
  void spawn_element(std::size_t tid,
                     std::vector<std::size_t> const& notifications,
                     WF&& wf, std::size_t no_succs,
                     In&&... in) const
  {
    std::stringstream o;
    o << "a"<< tid << "[" << (isResult ? "shape=diamond," : "")
      << "label=\"{";
#ifdef SKELETONS_FULL_NODE_NAME
    std::stringstream s;
    skeletons::show(wf, s);
    std::string name = s.str();
    skeletons::replace_string(name, "<", "&lt;");
    skeletons::replace_string(name, ">", "&gt;");
    o << name << "|";
#endif
    o << "id=" << tid << "|succ=" << no_succs
      << "}\", fillcolor=white, style=filled, color=\""
      << debug_helpers::get_color_name(get_location_id())
      << "\"];\n";
    (*m_out) << o.str();

    print_nodes_and_edges(*m_out, tid, std::forward<In>(in)...);
    print_notifications_dependencies(tid, notifications, *m_out);
    m_out->flush();
  }

  template <typename Skeleton, typename... Arg>
  void pre_spawn(Skeleton&& skeleton,
                std::size_t lid_offset,
                Arg&&...) const
  {
    std::stringstream o;
    o << "\nsubgraph cluster_";
    std::stringstream s;
    skeletons::show(skeleton, s);
    std::size_t cluster_id = std::hash<std::string>()(
                               s.str() + std::to_string(lid_offset));
    o << std::to_string(cluster_id);
    o << " {\n";
    o << "style=filled;\n";
    o << "color="
      << std::to_string(std::hash<std::string>()(s.str()) % 5 + 1)
      << ";\n";
    o << "label=\"";
    skeletons::show(skeleton, o);
    o << "\";\n";
    (*m_out) << o.str();
  }

  template <typename Skeleton, typename... Args>
  void post_spawn(Skeleton&& skeleton, Args&&... args) const
  {
    (*m_out) << "\n}\n";
  }

  void set_num_succs(std::size_t tid, std::size_t no_succs) const
  { }

  void init_location_info(std::size_t num_PEs, runtime::location_id PE_id)
  {
    m_num_PEs = num_PEs;
    m_PE_id = PE_id;
  }

  std::size_t get_num_PEs() const
  {
    return m_num_PEs;
  }

  runtime::location_id get_PE_id() const
  {
    return m_PE_id;
  }

private:
  template <typename C>
  std::string get_cluster_name(C const& container, std::true_type) const
  {
    std::stringstream o;
    o << container.get_rmi_handle().get_uid();
    std::string s(o.str());
    replace_string(s, "&", "");
    return s;
  }

  template <typename C>
  std::string get_cluster_name(C const& container, std::false_type) const
  {
    return "FunctorView";
  }

  template <typename V, typename CP>
  std::string get_cluster_name(
    view_coarsen_impl::coarsen_container<V, CP> const& c) const
  {
    return get_cluster_name(c.container().container(),
                            is_p_object<typename V::view_container_type>());
  }

  template <typename C>
  std::string get_cluster_name(C const& container) const
  {
    return get_cluster_name(container, is_p_object<C>());
  }

  template <typename P>
  void
  print_edge_name(P const& producer,
                  std::ostream& o) const
  { }

  template <typename View>
  void
  print_edge_name(flows::view_producer_info<View> const& producer,
                  std::ostream& o) const
  {
    o << "[label=\"";
    show_value(producer.get_index(), o);
    o << "\","
      << "color=\"green\"]";
  }

  template <typename View>
  void
  print_node_name(flows::view_producer_info<View> const& producer,
                  std::ostream& o) const
  {
    std::string name = get_cluster_name(producer.get_element().container());
    o << "v" <<  name << "[shape=\"box\",style=filled,label=\"";
    skeletons::show(producer.get_element(), o);
    o << "\" ,fillcolor=ivory3,color=black];\n";
    o << "v" << name;
  }

  template <typename Element, bool isRef>
  void
  print_node_name(
    flows::reflexive_producer_info<Element, isRef> const& producer,
    std::ostream& o) const
  {
    skeletons::show(producer.get_element(), o);
  }

  template <typename T>
  void
  print_node_name(flows::constant_producer_info<T> const& producer,
                  std::ostream& o) const
  {
    o << "C" << producer.get_element();
  }

  template <typename Producer>
  void
  print_node_name(Producer const& producer,
                  std::ostream& o) const
  {
    o << "a" << producer.get_index();
  }

  template <typename In0>
  void print_nodes_and_edges(std::ostream& o,
                             std::size_t tid, In0 const& in0) const
  {
    print_node_name(in0, o);
    o << " -> a" << tid;
    print_edge_name(in0, o);
    o << ";\n";
  }

  template <typename V, typename I, typename F>
  void print_nodes_and_edges(
    std::ostream& o, std::size_t tid,
    flows::indexed_producers_info<V, I, F> const& producer) const
  {
    for (auto&& i : producer.get_indices()) {
      o << "a" << i << " -> a" << tid << ";\n";
    }
  }

  template <typename In0, typename ...In>
  void print_nodes_and_edges(std::ostream& o,
                             std::size_t tid,
                             In0 const& in0, In&&... in) const
  {
    print_nodes_and_edges(o, tid, in0);
    print_nodes_and_edges(o, tid, in...);
  }

  void print_notifications_dependencies(
         std::size_t id,
         std::vector<std::size_t> const& notifications,
         std::ostream& o) const
  {
    for (std::size_t i = 0; i < notifications.size(); ++i) {
      o << "a" << notifications[i] << " -> a" << id;
      o << "[label=\"no-data\",style=\"dashed\"]";
      o << ";\n";
    }
  }
};


}
} // namespace stapl

#endif // STAPL_SKELETONS_ENVIRONMENTS_GRAPHVIZ_ENV_HPP
