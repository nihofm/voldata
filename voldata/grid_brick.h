#pragma once

#include "grid.h"
#include "buf3d.h"

#include <vector>
#include <memory>
#include <atomic>

namespace voldata {

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
    const glm::uvec3 n_bricks;
    const std::tuple<float, float> min_maj;
    std::atomic<size_t> brick_counter;
    Buf3D<uint32_t> indirection;            // 3x 10bits uint: (ptr_x, ptr_y, ptr_z, unused)
    Buf3D<uint32_t> range;                  // 2x float16: (minorant, majorant)
    Buf3D<uint8_t> atlas;                   // 512x uint8_t: 8x8x8 normalized brick data
};

}
