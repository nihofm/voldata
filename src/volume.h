#pragma once

#include <vector>
#include <filesystem>
namespace fs = std::filesystem;
#include <glm/glm.hpp>

#include "dict.h"
#include "grid.h"

namespace voldata {

class Volume {
public:
    Volume();
    Volume(const fs::path& path);
    Volume(size_t w, size_t h, size_t d, const uint8_t* data);
    Volume(size_t w, size_t h, size_t d, const float* data);
    virtual ~Volume();

    // grids
    void clear();
    void load_grid(const fs::path& path);
    std::shared_ptr<Grid> current_grid() const;

    // transformation
    glm::mat4 get_transform() const;                                    // index- to world-space transformation matrix
    glm::vec4 to_world(const glm::vec4& index) const;                   // transform from index- to world-space
    glm::vec4 to_index(const glm::vec4& world) const;                   // transform from world- to index-space

    // AABB (world-space)
    std::tuple<glm::vec3, glm::vec3> AABB() const;                      // world-space AABB

    // some parameter convenience functions
    inline void set_albedo(const glm::vec3& albedo) { params.set("albedo", albedo); }
    inline glm::vec3& get_albedo() { return params.get<glm::vec3&>("albedo"); }

    inline void set_phase(float phase) { params.set("phase", phase); }
    inline float& get_phase() { return params.get<float&>("phase"); }

    inline void set_density_scale(float density_scale) { params.set("density_scale", density_scale); }
    inline float& get_density_scale() { return params.get<float&>("density_scale"); }

    virtual std::string to_string(const std::string& indent="") const;  // string representation

    // data
    glm::mat4 model;
    Dictionary params;
    size_t grid_frame;
    // TODO grid slots (density/emission/...)
    std::vector<std::shared_ptr<Grid>> grids;
};

std::ostream& operator<<(std::ostream& out, const Volume& volume);

}
