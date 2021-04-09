#include "grid.h"

float Grid::operator[](const glm::ivec3& ipos) const { return fetch(ipos); }

std::string Grid::to_string(const std::string& indent) const {
    std::stringstream out;
    const auto [ibb_min, ibb_max] = index_aabb();
    const glm::ivec3 size = ibb_max - ibb_min + 1; // inclusive bounds
    out << indent << "AABB (index-space): " << glm::to_string(ibb_min) << " / " << glm::to_string(ibb_max) << std::endl;
    out << indent << "size: " << glm::to_string(size) << std::endl;
    const auto [min, maj] = minorant_majorant();
    out << indent << "minorant: " << min << ", majorant: " << maj << std::endl;
    const size_t active = num_voxels(), dense = size_t(size.x)*size.y*size.z;
    out << indent << "active voxels: " << active/1000 << "k / " << dense/1000 << "k (" << uint32_t(100 * active / float(dense)) << "%)" << std::endl;
    out << indent << "transform: " << glm::to_string(transform);
    return out.str();
}

std::ostream& operator<<(std::ostream& out, const Grid& grid) { return out << grid.to_string(); }
