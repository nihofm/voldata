#pragma once

#include <tuple>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

class Grid {
public:
    virtual ~Grid() {}

    virtual float fetch(const glm::ivec3& ipos) const = 0;
    float operator[](const glm::ivec3& ipos) const;

    virtual std::tuple<float, float> minorant_majorant() const = 0;
    virtual std::tuple<glm::ivec3, glm::ivec3> index_aabb() const = 0;
    virtual std::tuple<glm::vec3, glm::vec3> world_aabb() const = 0;

    virtual size_t num_voxels() const = 0;
    virtual bool is_empty() const { return num_voxels() == 0; }

    virtual std::string to_string(const std::string& indent="") const;

    // data
    glm::mat4 transform; // index- to world-space transform (i.e. model matrix)
};

std::ostream& operator<<(std::ostream& out, const Grid& grid);
