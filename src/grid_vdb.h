#pragma once

#include "grid.h"
#include <openvdb/openvdb.h>

#include <filesystem>
namespace fs = std::filesystem;

namespace voldata {

class OpenVDBGrid : public Grid {
public:
    OpenVDBGrid(const fs::path& filename, const std::string& gridname = "density");
    OpenVDBGrid(const Grid& grid);
    OpenVDBGrid(const std::shared_ptr<Grid>& grid);
    virtual ~OpenVDBGrid();

    float lookup(const glm::uvec3& ipos) const;
    std::pair<float, float> minorant_majorant() const;
    glm::uvec3 index_extent() const;
    size_t num_voxels() const;
    size_t size_bytes() const;

    // write to vdb file on disk
    void write(const fs::path& path) const;

    // data
    openvdb::FloatGrid::Ptr grid;
    glm::ivec3 ibb_min;
    glm::uvec3 extent;
    float minorant, majorant;
};

}
