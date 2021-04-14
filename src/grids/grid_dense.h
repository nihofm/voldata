#pragma once

#include "grid.h"

#include <vector>
#include <memory>

class DenseGrid : public Grid {
public:
    DenseGrid(size_t w, size_t h, size_t d, const uint8_t* data);
    DenseGrid(const Grid& grid);
    DenseGrid(const std::shared_ptr<Grid>& grid);
    virtual ~DenseGrid();

    float lookup(const glm::ivec3& ipos) const;
    std::tuple<float, float> minorant_majorant() const;
    glm::ivec3 index_extent() const;
    size_t num_voxels() const;
    size_t size_bytes() const;

    // data
    const glm::ivec3 n_voxels;
    float min_value, max_value;
    std::vector<uint8_t> voxel_data;
};
