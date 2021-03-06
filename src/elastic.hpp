//
//   Copyright 2019 University of Sheffield
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#ifndef ELASTIC_HPP
#define ELASTIC_HPP

#include <algorithm>
#include <iostream>

#include <petscdmda.h>
#include <petscmat.h>

#include "types.hpp"
#include "baseconfiguration.hpp"
#include "image.hpp"
#include "map.hpp"
#include "workspace.hpp"

class Elastic {
public:
  Elastic(const Image& fixed, const Image& moved, const floatvector nodespacing,
          const ConfigurationBase& configuration);

  void autoregister();

  std::shared_ptr<Image> registered() const
  {
    return m_p_registered;
  }

  //  protected:

  integer m_max_iter = 50;
  floating m_convergence_thres = 0.1;

  // Straightforward initialize-by-copy
  MPI_Comm m_comm;
  const ConfigurationBase& configuration;
  integer m_imgdims;
  integer m_mapdims;
  integer m_size;
  integer m_iternum;
  const Image& m_fixed;
  const Image& m_moved;

  // Other class data, default initialize then populate in c'tor
  floatvector2d m_v_nodespacings;
  floatvector m_v_final_nodespacing;
  std::shared_ptr<Image> m_p_registered;
  std::unique_ptr<Map> m_p_map;
  std::shared_ptr<WorkSpace> m_workspace;
  Mat_unique normmat;

  void save_debug_frame(std::string prefix, integer ocount, integer icount);
  void innerloop(integer outer_count);
  void innerstep(floating lambda, integer inum);

  void block_precondition();
  void calculate_node_spacings();
  void calculate_tmat(integer inum);
};

#endif
