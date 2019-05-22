#ifndef STAPL_PROFILERS_MATRIX_INDEX_GENERATOR_HPP
#define STAPL_PROFILERS_MATRIX_INDEX_GENERATOR_HPP

#include <stapl/runtime.hpp>
#include <boost/tuple/tuple.hpp>

using namespace stapl;

////////////////////////////////////////////////////////////////////////////////
/// @brief Generates indices for use with a multidimensional container
////////////////////////////////////////////////////////////////////////////////
class matrix_index_generator {
public:
////////////////////////////////////////////////////////////////////////////////
/// @tparam V Vector/Array type for indices
/// @param indices Container for generated indices
/// @param firstn Starting index of n dimension
/// @param firstm Starting index of m dimension
/// @param n Final number of n indices
/// @param m Final number of m indices
/// @param p Percentage of remote indices
/// @param next_only If true, remote indices are only on direct neighbor
////////////////////////////////////////////////////////////////////////////////
  template <class V>
  static void generate(V& indices,
                       size_t firstn, size_t firstm, size_t n, size_t m,
                       size_t p, //percentage remote
                       size_t next_only)
  {
    typedef tuple<size_t, size_t> gid_type;

    //for next only the convention is
    //  if non local are on neighbours only (1)
    //      else on all Ps(0)
    size_t nLocs=get_num_locations();
    indices.resize(n*m);
    //generate first as if all are local
    for (size_t i=0; i<n*m; ++i) {
      gid_type ij(firstn + ( lrand48() % n), firstm + ( lrand48() % m));
      indices[i] = ij;
    }

    if (p==0) { //all indices local
      return;
    } else { //there is a remote percentage
      size_t nremote = (p*n*m) / 100;
      size_t bs = (n*m)/nremote;
      if (next_only == 1) { //remote only on neighbor
        bool left=false;
        for (size_t i=0;i<n*m;i+=bs) {
          if (!left) {
            gid_type ij((n + firstn + (lrand48() % n)) % (nLocs * n),
                        (m + firstm + (lrand48() % m)) % (nLocs * m));
            indices[i] = ij ;
          }
          else {
            if (firstn >= (lrand48() % n) && firstm >= (lrand48() % m)) {
              gid_type ij(firstn - (lrand48() % n), firstm - (lrand48() % m));
              indices[i] = ij;
            }
            else if (firstn >= (lrand48() % n)) {
              gid_type ij(firstn - (lrand48() % n),
                          (m + firstm + (lrand48() % m)) % (nLocs * m));
              indices[i] = ij;
            }
            else if (firstm >= ( lrand48() % m) ) {
              gid_type ij((n + firstn + (lrand48() % n)) % (nLocs * n),
                          firstm - (lrand48() % m));
              indices[i] = ij;
            }
          }
          left = ! left;
        }
      }
      else { //al over the place
        for (size_t i=0; i<n*m; i+=bs) {
          gid_type ij(lrand48() % (nLocs * n), lrand48() % (nLocs * m));
          indices[i] = ij;
        }
      }
    }
  }
};

#endif
