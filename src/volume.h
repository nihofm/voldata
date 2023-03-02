#pragma once

#include "grid.h"
#include "grid_dense.h"
#include "grid_brick.h"
#include "grid_vdb.h"
#include "grid_nvdb.h"

#include <glm/glm.hpp>

#include <map>
#include <vector>
#include <memory>
#include <string>

namespace voldata {

class Volume {
public:
    using GridPtr = std::shared_ptr<Grid>;
    using DenseGridPtr = std::shared_ptr<DenseGrid>;
    using BrickGridPtr = std::shared_ptr<BrickGrid>;
#ifdef VOLDATA_WITH_OPENVDB
    using OpenVDBGridPtr = std::shared_ptr<OpenVDBGrid>;
#endif
    using NanoVDBGridPtr = std::shared_ptr<NanoVDBGrid>;
    using GridFrame = std::map<std::string, GridPtr>;
    using VolumePtr = std::shared_ptr<Volume>;

    Volume();
    Volume(const GridPtr& grid, const std::string& gridname = "density");
    Volume(const std::string& filename, const std::string& gridname = "density");
    Volume(size_t w, size_t h, size_t d, const uint8_t* data, const std::string& gridname = "density");
    Volume(size_t w, size_t h, size_t d, const float* data, const std::string& gridname = "density");
    virtual ~Volume();

    // grid management
    void clear();
    void add_grid_frame(const GridFrame& frame = GridFrame());
    void update_grid_frame(const size_t i, const GridPtr& grid, const std::string& gridname = "density");
    bool has_grid(const size_t i, const std::string& gridname) const;
    size_t n_grid_frames() const;

    // conveniently access the current grid frame
    GridFrame current_grid_frame() const;                                                       // return current grid frame
    GridPtr current_grid(const std::string& gridname = "density") const;                        // return grid from current frame
    DenseGridPtr current_grid_dense(const std::string& gridname = "density") const;             // return grid from current frame as BrickGrid, convert if necessary
    BrickGridPtr current_grid_brick(const std::string& gridname = "density") const;             // return grid from current frame as BrickGrid, convert if necessary
#ifdef VOLDATA_WITH_OPENVDB
    OpenVDBGridPtr current_grid_vdb(const std::string& gridname = "density") const;             // return grid from current frame as OpenVDBGrid, convert if necessary
#endif
    NanoVDBGridPtr current_grid_nvdb(const std::string& gridname = "density") const;             // return grid from current frame as NanoVDBGrid, convert if necessary

    // transformation, AABB (world space) and extrema of the current grid
    glm::mat4 get_transform(const std::string& gridname = "density") const;                     // index- to world-space transformation matrix
    glm::vec4 to_world(const glm::vec4& index, const std::string& gridname = "density") const;  // transform from index- to world-space
    glm::vec4 to_index(const glm::vec4& world, const std::string& gridname = "density") const;  // transform from world- to index-space
    std::pair<glm::vec3, glm::vec3> AABB(const std::string& gridname = "density") const;        // world-space AABB
    std::pair<float, float> minorant_majorant(const std::string& gridname = "density") const;   // minimum and maximal density values in the current grid
    std::string to_string(const std::string& indent="") const;                                  // string representation

    // static grid management helpers
    static GridPtr load_grid(const std::string& filename, const std::string& gridname = "density");
    static DenseGridPtr to_dense_grid(const GridPtr& grid);
    static BrickGridPtr to_brick_grid(const GridPtr& grid);
#ifdef VOLDATA_WITH_OPENVDB
    static OpenVDBGridPtr to_vdb_grid(const GridPtr& grid);
#endif
    static NanoVDBGridPtr to_nvdb_grid(const GridPtr& grid);
    static VolumePtr load_folder(const std::string& path, std::vector<std::string> gridnames = { "density" });

    // data
    size_t grid_frame_counter;          // current grid frame
    std::vector<GridFrame> grids;       // grid frames
    glm::mat4 transform;                // transformation matrix
};

inline std::ostream& operator<<(std::ostream& out, const Volume& volume) { return out << volume.to_string(); }

}
