#pragma once

#include <mcc/result.hpp>
#include <mcc/gl/voxel.hpp>

namespace mcc::data {
    Result<gl::Matrix, std::string> parse_qb(std::ifstream& ifs);
}