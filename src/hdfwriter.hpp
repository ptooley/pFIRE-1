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

#ifndef HDFWRITER_HPP
#define HDFWRITER_HPP

#include <string>

#include <hdf5.h>
#include <mpi.h>

#include "types.hpp"
#include "basewriter.hpp"

class HDFWriter: public BaseWriter {
public:
  HDFWriter(const std::string& filename, const MPI_Comm& comm);
  ~HDFWriter();

  void write_image(const Image& image);
  void write_map(const Map& map);

  static const std::string writer_name;
  static const std::vector<std::string> extensions;

protected:
  std::string h5_filename;
  std::string h5_groupname;

private:
  hid_t _file_h;

  void open_or_create_h5();
};

#endif // HDFWRITER_HPP
