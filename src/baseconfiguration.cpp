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

#include "baseconfiguration.hpp"

#include <sstream>

#include "infix_iterator.hpp"

const config_map ConfigurationBase::default_config = {{"verbose", "false"},
                                                      {"registered", "registered.xdmf:/registered"},
                                                      {"map", "map.xdmf:/map"},
                                                      {"debug_frames", "false"},
                                                      {"debug_frames_prefix", "debug"}};

const std::vector<std::string> ConfigurationBase::required_options = {"fixed", "moved",
                                                                      "nodespacing"};

const std::vector<std::string> ConfigurationBase::arg_options = {
    "fixed", "moved", "mask", "nodespacing", "registered", "map", "debug_frames_prefix"};

const std::vector<std::string> ConfigurationBase::bool_options = {"verbose", "debug_frames"};

ConfigurationBase::ConfigurationBase(const int &argc, char const *const *argv)
  : config(default_config), arguments(argv + 1, argv + argc),
    invocation_name(get_invocation_name(argv[0]))
{
}

void ConfigurationBase::validate_config()
{
  std::list<std::string> missing;
  for (auto &req_it : ConfigurationBase::required_options)
  {
    if (config.find(req_it) == config.cend())
    {
      missing.push_back(req_it);
    }
  }
  if (missing.size() > 0)
  {
    std::ostringstream errmsg;
    errmsg << "Missing required argument(s) \"";
    std::copy(missing.cbegin(), missing.cend(), infix_ostream_iterator<std::string>(errmsg, ", "));
    errmsg << "\"";
    throw std::runtime_error(errmsg.str());
  }
}

std::string ConfigurationBase::get_invocation_name(const std::string &argzero)
{
  bf::path invpath(argzero);
  return (invpath.filename().string());
}
