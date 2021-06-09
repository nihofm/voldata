#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "grid.h"

namespace voldata {

class Volume {
public:
    Volume();
    Volume(const std::string& filepath);
    Volume(size_t w, size_t h, size_t d, const uint8_t* data);
    Volume(size_t w, size_t h, size_t d, const float* data);
    virtual ~Volume();

    // grids
    void clear();
    void load_grid(const std::string& filepath);
    std::shared_ptr<Grid> current_grid() const;

    // transformation
    glm::mat4 get_transform() const;                                    // index- to world-space transformation matrix
    glm::vec4 to_world(const glm::vec4& index) const;                   // transform from index- to world-space
    glm::vec4 to_index(const glm::vec4& world) const;                   // transform from world- to index-space

    // AABB (world-space)
    std::tuple<glm::vec3, glm::vec3> AABB() const;                      // world-space AABB
    // minorant and majorant of the current grid
    std::tuple<float, float> minorant_majorant() const;                 // minimum and maximal density values in the current grid

    virtual std::string to_string(const std::string& indent="") const;  // string representation

    // data
    glm::mat4 model;
    glm::vec3 albedo;
    float phase;
    float density_scale;
    size_t grid_frame;
    // TODO grid slots (density/emission/...)
    std::vector<std::shared_ptr<Grid>> grids;
};

std::ostream& operator<<(std::ostream& out, const Volume& volume);

}