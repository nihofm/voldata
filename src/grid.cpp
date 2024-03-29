#include "grid.h"

#include <sstream>
#include <glm/gtx/string_cast.hpp>

namespace voldata {

Grid::Grid() : transform(glm::mat4(1)) {}

std::string Grid::to_string(const std::string& indent) const {
    std::stringstream out;
    const glm::uvec3 ibb_max = index_extent();
    out << indent << "AABB (index-space): " << glm::to_string(glm::uvec3(0)) << " / " << glm::to_string(ibb_max) << std::endl;
    const auto [min, maj] = minorant_majorant();
    out << indent << "minorant: " << min << ", majorant: " << maj << std::endl;
    const size_t active = num_voxels(), dense = size_t(ibb_max.x)*ibb_max.y*ibb_max.z;
    out << indent << "active voxels: " << active/1000 << "k / " << dense/1000 << "k (" << uint32_t(std::round(100 * active / float(dense))) << "%)" << std::endl;
    out << indent << "transform: " << std::endl;
    for (int i = 0; i < 4; ++i)
        out << indent << "    " << std::fixed << transform[0][i] << ", " << transform[1][i] << ", " << transform[2][i] << ", " << transform[3][i] << std::endl;
    out << indent << "memory: " << (size_bytes() / 100000) / 10.f << " MB";
    return out.str();
}

std::ostream& operator<<(std::ostream& out, const Grid& grid) { return out << grid.to_string(); }

}
