#include <voldata/grid_dense.h>

#include <numeric>
#include <algorithm>
#include <execution>
#include <glm/gtx/string_cast.hpp>

namespace voldata {

DenseGrid::DenseGrid() : Grid(), n_voxels(0), min_value(0), max_value(0) {}

DenseGrid::DenseGrid(const Grid& grid) :
    Grid(grid),
    n_voxels(grid.index_extent()),
    min_value(std::get<0>(grid.minorant_majorant())),
    max_value(std::get<1>(grid.minorant_majorant()))
{
    std::vector<uint32_t> slices(n_voxels.z);
    std::iota(slices.begin(), slices.end(), 0);
    // encode dense grid data with 8bit per voxel
    voxel_data.resize(n_voxels.x * n_voxels.y * n_voxels.z);
    std::for_each(std::execution::par_unseq, slices.begin(), slices.end(),
    [&](uint32_t z)
    {
        for (uint32_t y = 0; y < n_voxels.y; ++y)
            for (uint32_t x = 0; x < n_voxels.x; ++x) {
                const float value = grid.lookup(glm::ivec3(x, y, z));
                const size_t idx = z * n_voxels.x * n_voxels.y + y * n_voxels.x + x;
                voxel_data[idx] = uint8_t(std::round(255 * (value - min_value) / (max_value - min_value)));
            }
    });
}

DenseGrid::DenseGrid(const std::shared_ptr<Grid>& grid) : DenseGrid(*grid) {}

DenseGrid::DenseGrid(size_t w, size_t h, size_t d, const uint8_t* data) :
    Grid(),
    n_voxels(w, h, d), 
    min_value(0),
    max_value(1)
{
    // parallel copy voxel data
    std::vector<uint32_t> slices(n_voxels.z);
    std::iota(slices.begin(), slices.end(), 0);
    voxel_data.resize(n_voxels.x * n_voxels.y * n_voxels.z);
    std::for_each(std::execution::par_unseq, slices.begin(), slices.end(),
    [&](uint32_t z)
    {
        for (uint32_t y = 0; y < n_voxels.y; ++y)
            for (uint32_t x = 0; x < n_voxels.x; ++x) {
                const size_t idx = z * n_voxels.x * n_voxels.y + y * n_voxels.x + x;
                voxel_data[idx] = data[idx];
            }
    });
}

DenseGrid::DenseGrid(size_t w, size_t h, size_t d, const float* data) :
    Grid(),
    n_voxels(w, h, d), 
    min_value(FLT_MAX),
    max_value(FLT_MIN)
{
    // prepare slices
    std::vector<uint32_t> slices(n_voxels.z);
    std::iota(slices.begin(), slices.end(), 0);
    // pass to find global minorant and majorant
    std::vector<float> minima(n_voxels.z, FLT_MAX);
    std::vector<float> maxima(n_voxels.z, FLT_MIN);
    std::for_each(std::execution::par_unseq, slices.begin(), slices.end(),
    [&](uint32_t z)
    {
        for (uint32_t y = 0; y < n_voxels.y; ++y)
            for (uint32_t x = 0; x < n_voxels.x; ++x) {
                const float value = data[z * n_voxels.x * n_voxels.y + y * n_voxels.x + x];
                minima[z] = std::min(minima[z], value);
                maxima[z] = std::max(maxima[z], value);
            }
    });
    // reduce
    for (uint32_t z = 0; z < n_voxels.z; ++z) {
        min_value = std::min(min_value, minima[z]);
        max_value = std::max(max_value, maxima[z]);
    }
    // encode dense grid data with 8bit per voxel
    voxel_data.resize(n_voxels.x * n_voxels.y * n_voxels.z);
    std::for_each(std::execution::par_unseq, slices.begin(), slices.end(),
    [&](uint32_t z)
    {
        for (uint32_t y = 0; y < n_voxels.y; ++y)
            for (uint32_t x = 0; x < n_voxels.x; ++x) {
                const size_t idx = z * n_voxels.x * n_voxels.y + y * n_voxels.x + x;
                voxel_data[idx] = uint8_t(std::round(255 * (data[idx] - min_value) / (max_value - min_value)));
            }
    });
}

DenseGrid::~DenseGrid() {}

float DenseGrid::lookup(const glm::uvec3& ipos) const {
    if (glm::any(glm::greaterThanEqual(ipos, n_voxels))) return 0.f;
    const size_t idx = ipos.z * n_voxels.x * n_voxels.y + ipos.y * n_voxels.x + ipos.x;
    return min_value + (voxel_data[idx] / 255.f) * (max_value - min_value);
}

std::pair<float, float> DenseGrid::minorant_majorant() const { return { min_value, max_value }; }

glm::uvec3 DenseGrid::index_extent() const { return n_voxels; }

size_t DenseGrid::num_voxels() const { return size_t(n_voxels.x) * n_voxels.y * n_voxels.z; }

size_t DenseGrid::size_bytes() const { return size_t(n_voxels.x) * n_voxels.y * n_voxels.z; }

}
