#include "grid_dense.h"

#include <numeric>
#include <algorithm>
#include <execution>

DenseGrid::DenseGrid(size_t w, size_t h, size_t d, const uint8_t* data) :
    Grid(),
    n_voxels(w, h, d), 
    min_value(FLT_MAX),
    max_value(FLT_MIN)
{
    voxel_data.resize(n_voxels.x * n_voxels.y * n_voxels.z);
    std::vector<int> slices(n_voxels.z);
    std::iota(slices.begin(), slices.end(), 0);
    std::for_each(std::execution::par_unseq, slices.begin(), slices.end(),
    [&](int z)
    {
        for (int y = 0; y < n_voxels.y; ++y)
            for (int x = 0; x < n_voxels.x; ++x) {
                const size_t idx = z * n_voxels.x * n_voxels.y + y * n_voxels.x + x;
                voxel_data[idx] = data[idx];
                min_value = std::min(min_value, data[idx] / 255.f);
                max_value = std::max(max_value, data[idx] / 255.f);
            }
    });
}

DenseGrid::DenseGrid(const Grid& grid) :
    Grid(grid),
    n_voxels(grid.index_extent()),
    min_value(std::get<0>(grid.minorant_majorant())),
    max_value(std::get<1>(grid.minorant_majorant()))
{
    voxel_data.resize(n_voxels.x * n_voxels.y * n_voxels.z);
    std::vector<int> slices(n_voxels.z);
    std::iota(slices.begin(), slices.end(), 0);
    // build dense 3d grid (8bit per voxel, range [0, 1])
    std::for_each(std::execution::par_unseq, slices.begin(), slices.end(),
    [&](int z)
    {
        for (int y = 0; y < n_voxels.y; ++y)
            for (int x = 0; x < n_voxels.x; ++x) {
                const float value = glm::clamp(grid.lookup(glm::ivec3(x, y, z)), 0.f, 1.f); // TODO encode
                min_value = std::min(min_value, value);
                max_value = std::max(max_value, value);
                const size_t idx = z * n_voxels.x * n_voxels.y + y * n_voxels.x + x;
                voxel_data[idx] = uint8_t(std::round(255 * value)); 
            }
    });
    assert(min_value >= 0 && max_value <= 255);
}

DenseGrid::DenseGrid(const std::shared_ptr<Grid>& grid) : DenseGrid(*grid) {}

DenseGrid::~DenseGrid() {}

float DenseGrid::lookup(const glm::ivec3& ipos) const {
    // TODO: bounds checks or ignore?
    const size_t idx = ipos.z * n_voxels.x * n_voxels.y + ipos.y * n_voxels.x + ipos.x;
    return voxel_data[idx] / 255.f;
}

std::tuple<float, float> DenseGrid::minorant_majorant() const { return { min_value, max_value }; }

glm::ivec3 DenseGrid::index_extent() const { return n_voxels; }

size_t DenseGrid::num_voxels() const { return size_t(n_voxels.x) * n_voxels.y * n_voxels.z; }

size_t DenseGrid::size_bytes() const { return size_t(n_voxels.x) * n_voxels.y * n_voxels.z; }
