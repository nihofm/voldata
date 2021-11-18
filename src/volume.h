#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "grid.h"
#include "grid_dense.h"
#include "grid_brick.h"
#include "grid_vdb.h"

namespace voldata {

class Volume {
    using GridPtr = std::shared_ptr<Grid>;
    using DenseGridPtr = std::shared_ptr<DenseGrid>;
    using BrickGridPtr = std::shared_ptr<BrickGrid>;
    using OpenVDBGridPtr = std::shared_ptr<OpenVDBGrid>;
    using GridFrame = std::map<std::string, GridPtr>;

public:
    Volume();
    Volume(const std::string& filename, const std::string& gridname = "density");
    Volume(size_t w, size_t h, size_t d, const uint8_t* data, const std::string& gridname = "density");
    Volume(size_t w, size_t h, size_t d, const float* data, const std::string& gridname = "density");
    virtual ~Volume();

    // grid management
    void clear();
    void add_grid_frame(const GridFrame& frame);
    void add_grid_to_new_frame(const GridPtr& grid, const std::string& gridname = "density");
    void update_current_grid(const GridPtr& grid, const std::string& gridname = "density");
    size_t n_grid_frames() const;
    bool has_grid(const std::string& gridname) const;

    // conveniently access the current grid frame
    GridFrame current_grid_frame() const;                                               // return current grid frame
    GridPtr current_grid(const std::string& gridname = "density") const;                // return grid from current frame
    DenseGridPtr current_grid_dense(const std::string& gridname = "density") const;     // return grid from current frame as BrickGrid, convert if necessary
    BrickGridPtr current_grid_brick(const std::string& gridname = "density") const;     // return grid from current frame as BrickGrid, convert if necessary
    OpenVDBGridPtr current_grid_vdb(const std::string& gridname = "density") const;     // return grid from current frame as OpenVDBGrid, convert if necessary

    // transformation, AABB (world space) and extrema of the current grid
    glm::mat4 get_transform() const;                                                    // index- to world-space transformation matrix
    glm::vec4 to_world(const glm::vec4& index) const;                                   // transform from index- to world-space
    glm::vec4 to_index(const glm::vec4& world) const;                                   // transform from world- to index-space
    std::pair<glm::vec3, glm::vec3> AABB() const;                                       // world-space AABB
    std::pair<float, float> minorant_majorant() const;                                  // minimum and maximal density values in the current grid
    std::string to_string(const std::string& indent="") const;                          // string representation

    // static grid management helpers
    static GridPtr load_grid(const std::string& filename, const std::string& gridname = "density");
    static DenseGridPtr to_dense_grid(const GridPtr& grid);
    static BrickGridPtr to_brick_grid(const GridPtr& grid);
    static OpenVDBGridPtr to_vdb_grid(const GridPtr& grid);

    // data
    size_t grid_frame_counter;          // current grid frame
    std::vector<GridFrame> grids;       // grid frames
    glm::mat4 model;                    // global model matrix
    glm::vec3 albedo;                   // global volume albedo
    float phase;                        // global volume phase (henyey-greenstein g parameter)
    float density_scale;                // global density scaling factor
    float emission_scale;               // global emission scaling factor
};

inline std::ostream& operator<<(std::ostream& out, const Volume& volume) { return out << volume.to_string(); }

}
