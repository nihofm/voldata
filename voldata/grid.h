#pragma once

#include <tuple>
#include <memory>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

namespace voldata {

class Grid {
public:
    virtual ~Grid() {}

    // grid interface
    virtual float lookup(const glm::ivec3& ipos) const = 0;                 // index-space grid lookup
    virtual std::tuple<float, float> minorant_majorant() const = 0;         // global minorant and majorant
    virtual glm::ivec3 index_extent() const = 0;                            // max of index space voxel AABB, origin always (0, 0, 0)
    virtual size_t num_voxels() const = 0;                                  // number of (active) voxels in this grid
    virtual size_t size_bytes() const = 0;                                  // required bytes to store this grid

    // convenience operators and functions
    virtual float operator[](const glm::ivec3& ipos) const;                 // index-space grid lookup (operator)
    virtual float operator[](const glm::vec3& wpos) const;                  // world-space grid lookup (operator)
    virtual float lookup_world(const glm::vec3& pos) const;                 // world-space grid lookup
    virtual std::tuple<glm::vec3, glm::vec3> world_aabb() const;            // world-space AABB
    virtual glm::vec3 world_extent() const;                                 // world-space extent
    virtual glm::vec4 to_world(const glm::vec4& index) const;               // index- to world-space transform
    virtual glm::vec4 to_index(const glm::vec4& world) const;               // world- to index-space transform
    virtual std::string to_string(const std::string& indent="") const;      // string representation

    // data
    glm::mat4 transform;                                                    // index- to world-space transform (i.e. model matrix)
};

std::ostream& operator<<(std::ostream& out, const Grid& grid);

}
