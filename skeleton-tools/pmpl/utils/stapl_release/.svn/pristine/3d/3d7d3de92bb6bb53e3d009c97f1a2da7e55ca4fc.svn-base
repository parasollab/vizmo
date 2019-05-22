#ifndef KRIPKE_COMMON_KERNEL_FUNCTIONS_H__
#define KRIPKE_COMMON_KERNEL_FUNCTIONS_H__

// The result from each execution of the direction loop body is a set of planes
// that are partially populated with the results for a direction. This work
// function merges those results to form the final result for the zoneset.
template<typename ResultValue>
struct reduce_planes
{
  typedef ResultValue result_type;

  template<typename PlaneSet0, typename PlaneSet1>
  result_type operator()(PlaneSet0 const& p0, PlaneSet1 const& p1)
  {
    result_type result;

    result[0] = p0[0];
    std::transform(result[0].begin(), result[0].end(), p1[0].cbegin(),
      result[0].begin(), stapl::plus<double>());

    result[1] = p0[1];
    std::transform(result[1].begin(), result[1].end(), p1[1].cbegin(),
      result[1].begin(), stapl::plus<double>());

    result[2] = p0[2];
    std::transform(result[2].begin(), result[2].end(), p1[2].cbegin(),
      result[2].begin(), stapl::plus<double>());

    return result;
  }
};


// The original extent is the bounds of the spatial domain on a location.
// We need two extents.  The first is used to index into Grid_Data structures
// that use the extents of the location. The second is used to index into the
// cellset-based variables and uses local indexing from 0 instead of the
// location-based extent indexing.
inline std::pair<Grid_Sweep_Block, Grid_Sweep_Block>
adjust_extent(Grid_Sweep_Block const& extent,
  Grid_Data_Base* grid_data, std::tuple<size_t, size_t, size_t> cellset_id)
{
  std::pair<Grid_Sweep_Block, Grid_Sweep_Block> cellset_extents;

  // increments will not be changed.
  cellset_extents.first.inc_i = extent.inc_i;
  cellset_extents.first.inc_j = extent.inc_j;
  cellset_extents.first.inc_k = extent.inc_k;
  cellset_extents.second.inc_i = extent.inc_i;
  cellset_extents.second.inc_j = extent.inc_j;
  cellset_extents.second.inc_k = extent.inc_k;

  // Size of zoneset in each dimension
  int zones_x = grid_data->ax();
  int zones_y = grid_data->ay();
  int zones_z = grid_data->az();

  // Set the extents of the local indexing extent
  if (cellset_extents.second.inc_i > 0) {
    cellset_extents.second.start_i = 0;
    cellset_extents.second.end_i = zones_x;
  } else {
    cellset_extents.second.start_i = zones_x-1;
    cellset_extents.second.end_i = -1;
  }
  if (cellset_extents.second.inc_j > 0) {
    cellset_extents.second.start_j = 0;
    cellset_extents.second.end_j = zones_y;
  } else {
    cellset_extents.second.start_j = zones_y-1;
    cellset_extents.second.end_j = -1;
  }
  if (cellset_extents.second.inc_k > 0) {
    cellset_extents.second.start_k = 0;
    cellset_extents.second.end_k = zones_z;
  } else {
    cellset_extents.second.start_k = zones_z-1;
    cellset_extents.second.end_k = -1;
  }

  // Global-indexed extent is the local extent + the offset for the number of
  // zonesets between the current zoneset and the origin.
  int offset_x = zones_x * std::get<0>(cellset_id);
  int offset_y = zones_y * std::get<1>(cellset_id);
  int offset_z = zones_z * std::get<2>(cellset_id);

  cellset_extents.first.start_i = cellset_extents.second.start_i + offset_x;
  cellset_extents.first.end_i   = cellset_extents.second.end_i + offset_x;
  cellset_extents.first.start_j = cellset_extents.second.start_j + offset_y;
  cellset_extents.first.end_j   = cellset_extents.second.end_j + offset_y;
  cellset_extents.first.start_k = cellset_extents.second.start_k + offset_z;
  cellset_extents.first.end_k   = cellset_extents.second.end_k + offset_z;

  return cellset_extents;
}

#endif
