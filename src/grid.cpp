#include "grid.h"

float Grid::operator[](const glm::ivec3& ipos) const { return lookup(ipos); }

float Grid::operator[](const glm::vec3& wpos) const { return lookup_world(wpos); }

float Grid::lookup_world(const glm::vec3& pos) const {
    const glm::ivec3 ipos = glm::ivec3(to_index(glm::vec4(pos, 1.f)));
    return lookup(ipos);
}

std::tuple<glm::vec3, glm::vec3> Grid::world_aabb() const {
    const glm::vec3 wbb_min = glm::vec3(to_world(glm::vec4(0, 0, 0, 1)));
    const glm::vec3 wbb_max = glm::vec3(to_world(glm::vec4(glm::vec3(index_extent()), 1)));
    return { wbb_min, wbb_max };
}

glm::vec3 Grid::world_extent() const {
    const glm::vec3 wbb_min = glm::vec3(to_world(glm::vec4(0, 0, 0, 1)));
    const glm::vec3 wbb_max = glm::vec3(to_world(glm::vec4(glm::vec3(index_extent()), 1)));
    return wbb_max - wbb_min;
}

glm::vec4 Grid::to_world(const glm::vec4& index) const {
    return transform * index;
}

glm::vec4 Grid::to_index(const glm::vec4& world) const {
    return glm::inverse(transform) * world;
}

std::string Grid::to_string(const std::string& indent) const {
    std::stringstream out;
    const glm::ivec3 ibb_max = index_extent();
    out << indent << "AABB (index-space): " << glm::to_string(glm::ivec3(0)) << " / " << glm::to_string(ibb_max) << std::endl;
    const auto [min, maj] = minorant_majorant();
    out << indent << "minorant: " << min << ", majorant: " << maj << std::endl;
    const size_t active = num_voxels(), dense = size_t(ibb_max.x)*ibb_max.y*ibb_max.z;
    out << indent << "active voxels: " << active/1000 << "k / " << dense/1000 << "k (" << uint32_t(100 * active / float(dense)) << "%)" << std::endl;
    out << indent << "transform: " << glm::to_string(transform) << std::endl;
    out << indent << "memory: " << (size_bytes() / 100000) / 10.f << " MB";
    return out.str();
}

std::ostream& operator<<(std::ostream& out, const Grid& grid) { return out << grid.to_string(); }
