#pragma once

#include "grid.h"

#include <filesystem>
namespace fs = std::filesystem;
#include <openvdb/openvdb.h>

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
    virtual std::string to_string(const std::string& indent="") const override;

    // TODO write to vdb file

    // data
    openvdb::FloatGrid::Ptr grid;
    glm::ivec3 ibb_min, ibb_max;
    float minorant, majorant;
};

}
