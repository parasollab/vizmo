#ifndef STAPL_PROFILERS_INDEX_GENERATOR_HPP
#define STAPL_PROFILERS_INDEX_GENERATOR_HPP

#include <stapl/runtime.hpp>
#include <boost/tuple/tuple.hpp>

using namespace stapl;

////////////////////////////////////////////////////////////////////////////////
/// @brief Generates indices for use with a container
////////////////////////////////////////////////////////////////////////////////

class index_generator {
public:
////////////////////////////////////////////////////////////////////////////////
/// @tparam V Vector/Array type for indices
/// @param indices Container for generated indices
/// @param first Starting index
/// @param sz Final size of index container
/// @param premote Percentage of remote indices
/// @param next_only If true, remote indices are only on direct neighbor
////////////////////////////////////////////////////////////////////////////////
  template <class V>
  static void generate(V& indices, size_t first, size_t sz,
                size_t premote, size_t next_only)
  {
    //for next only the convention is
    //  if non local are on neighbours only (1)
    //      else on all Ps(0)
    size_t nLocs=get_num_locations();
    indices.resize(sz);
    //generate first as if all are local
    for (size_t i=0; i<sz; ++i)
      indices[i] = first + ( lrand48() % sz);

    if (premote==0) { //all indices local
      return;
    } else { //there is a remote percentage
      size_t nremote = (premote*sz) / 100;
      size_t bs = sz/nremote;
      if (next_only == 1) { //remote only on neighbor
        bool left=false;
        for (size_t i=0;i<sz;i+=bs) {
          if (!left) {
            indices[i] = (sz + first + ( lrand48() % sz)) % (nLocs * sz) ;
          }
          else {
            if (first >= ( lrand48() % sz) )
              indices[i] = first - ( lrand48() % sz);
          }
          left = ! left;
        }
      }
      else { //al over the place
        for (size_t i=0; i<sz; i+=bs)
          indices[i] = lrand48() % (nLocs * sz) ;
      }
    }
  }
};

#endif
