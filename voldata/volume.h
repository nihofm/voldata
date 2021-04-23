#pragma once

#include <vector>
#include <filesystem>
namespace fs = std::filesystem;
#include <glm/glm.hpp>

#include "dict.h"
#include "grid.h"

namespace VOLDATA_NAMESPACE { // default: voldata

class Volume {
public:
    Volume();
    // TODO volme/grid factory
    Volume(const fs::path& path);
    Volume(size_t w, size_t h, size_t d, const uint8_t* data);
    Volume(size_t w, size_t h, size_t d, const float* data);
    virtual ~Volume();

    void clear();
    void load_grid(const fs::path& path);

    inline std::shared_ptr<Grid> current_grid() const { return grids[grid_frame]; }

    inline void set_albedo(const glm::vec3& albedo) { params.set("albedo", albedo); }
    inline glm::vec3 get_albedo() { return params.get<glm::vec3>("albedo"); }

    inline void set_phase(float phase) { params.set("phase", phase); }
    inline float get_phase() { return params.get<float>("phase"); }

    inline void set_density_scale(float density_scale) { params.set("density_scale", density_scale); }
    inline float get_density_scale() { return params.get<float>("density_scale"); }

    // data
    /*
    const std::string name;
    glm::mat4 model;
    glm::vec3 albedo;
    float phase_g;
    glm::vec3 slice_thickness;
    Texture3D texture;
    float majorant;
    float density_scale;
    */

    Dictionary params;
    size_t grid_frame;
    // TODO grid slots (density/emission/...)
    std::vector<std::shared_ptr<Grid>> grids;
};

}
