#pragma once

#include "grid.h"

#include <vector>
#include <memory>

class BrickGrid : public Grid {
public:
    BrickGrid(const Grid& grid);
    BrickGrid(const std::shared_ptr<Grid>& grid);
    virtual ~BrickGrid();

    float lookup(const glm::ivec3& ipos) const;
    std::tuple<float, float> minorant_majorant() const;
    glm::ivec3 index_extent() const;
    size_t num_voxels() const;
    size_t size_bytes() const;
    virtual std::string to_string(const std::string& indent="") const override;

    // data
    const glm::ivec3 n_bricks;
    const std::tuple<float, float> min_maj;
    std::vector<uint32_t> offset_data;      // 4x uint8_t: (ptr_x, ptr_y, ptr_z, unused)
    std::vector<uint32_t> range_data;       // 2x float16: (minorant, majorant)
    std::vector<uint8_t> atlas_data;        // 512x uint8_t: brick atlas data
};
