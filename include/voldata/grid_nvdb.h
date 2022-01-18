#pragma once

#include <voldata/grid.h>

#include <string>
#include <filesystem>
namespace fs = std::filesystem;
#include <nanovdb/NanoVDB.h>
#include <nanovdb/util/GridHandle.h>

namespace voldata {

class NanoVDBGrid : public Grid {
public:
    NanoVDBGrid(const fs::path& path, const std::string& gridname = "density");
    NanoVDBGrid(const Grid& grid);
    NanoVDBGrid(const std::shared_ptr<Grid>& grid);
    virtual ~NanoVDBGrid();

    float lookup(const glm::uvec3& ipos) const;
    std::pair<float, float> minorant_majorant() const;
    glm::uvec3 index_extent() const;
    size_t num_voxels() const;
    size_t size_bytes() const;

    // write to nvdb file on disk
    void write(const fs::path& path) const;

    // data
    nanovdb::GridHandle<nanovdb::HostBuffer> handle;
    nanovdb::NanoGrid<float>* grid;
    glm::ivec3 ibb_min;
    glm::uvec3 extent;
    float minorant, majorant;
};

}
