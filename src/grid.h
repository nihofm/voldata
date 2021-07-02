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
    virtual float lookup(const glm::uvec3& ipos) const = 0;                 // index-space grid lookup
    virtual std::pair<float, float> minorant_majorant() const = 0;          // global minorant and majorant
    virtual glm::uvec3 index_extent() const = 0;                            // max of index space voxel AABB, origin always (0, 0, 0)
    virtual size_t num_voxels() const = 0;                                  // number of (active) voxels in this grid
    virtual size_t size_bytes() const = 0;                                  // required bytes to store this grid

    // convenience operators and functions
    virtual std::string to_string(const std::string& indent="") const;      // string representation
    inline float operator[](const glm::uvec3& ipos) const { return lookup(ipos); };

    // data
    glm::mat4 transform;                                                    // grid transformation (i.e. model matrix)
};

std::ostream& operator<<(std::ostream& out, const Grid& grid);

}
