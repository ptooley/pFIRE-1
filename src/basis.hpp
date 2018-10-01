#ifndef BASIS_HPP
#define BASIS_HPP

#include<stdexcept>
#include<iterator>
#include<algorithm>
#include<numeric>
#include<cmath>

#include<petscmat.h>

#include "types.hpp"
#include "indexing.hpp"
#include "iterator_routines.hpp"

Mat_unique build_basis_matrix(MPI_Comm comm, const intvector& src_shape, const intvector& tgt_shape, 
                              const intvector& scalings, const floatvector& offsets,
                              integer tile_dim);

template<class Input1, class Input2, class Rtype = typename std::iterator_traits<Input1>::value_type>
Rtype calculate_basis_coefficient(Input1 first1, Input1 last1, Input2 first2)
{
  Rtype res = 1;
  auto it1 = first1;
  auto it2 = first2;
  for(; it1 != last1;)
  {
    res *= 1 - std::abs(*it1++ - *it2++);
  }
  return res;
}

#endif