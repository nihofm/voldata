#include "grid_brick.h"

#include <iostream>
#include <numeric>
#include <algorithm>
#include <execution>

namespace voldata {

// ----------------------------------------------
// constants

static const uint32_t BRICK_SIZE = 8;
static const uint32_t BITS_PER_AXIS = 8; // TODO 10/10/10/2 format with 10 bits per axis?
static const uint32_t MAX_BRICKS = 1 << BITS_PER_AXIS;
static const uint32_t VOXELS_PER_BRICK = BRICK_SIZE * BRICK_SIZE * BRICK_SIZE;

// ----------------------------------------------
// encoding helpers

uint32_t encode_range(float x, float y) {
    return uint32_t(glm::detail::toFloat16(x)) | uint32_t(glm::detail::toFloat16(y)) << 16;
}

glm::vec2 decode_range(uint32_t data) {
    return glm::vec2(glm::detail::toFloat32(data & 0xFFFF), glm::detail::toFloat32(data >> 16));
}

uint32_t encode_ptr(const glm::uvec3& ptr) {
    assert(ptr.x < MAX_BRICKS && ptr.y < MAX_BRICKS && ptr.z < MAX_BRICKS);
    return ptr.x | (ptr.y << BITS_PER_AXIS) | (ptr.z << (2*BITS_PER_AXIS));
}

glm::uvec3 decode_ptr(uint32_t data) {
    return glm::uvec3(data & (MAX_BRICKS-1), (data >> BITS_PER_AXIS) & (MAX_BRICKS-1), (data >> (2*BITS_PER_AXIS)) & (MAX_BRICKS-1));
}

uint8_t encode_voxel(float value, const glm::vec2& range) {
    assert(value >= minorant && value <= majorant);
    const float value_norm = glm::clamp((value - range.x) / (range.y - range.x), 0.f, 1.f);
    return uint8_t(std::round(255 * value_norm));
}

float decode_voxel(uint8_t data, const glm::vec2& range) {
    return range.x + data * (1.f / 255.f) * (range.y - range.x);
}

inline int div_round_up(int num, int denom) {
    return std::round(std::ceil((float)num / denom));
}

BrickGrid::BrickGrid(const Grid& grid) :
    Grid(grid),
    n_bricks(glm::ceil(glm::vec3(grid.index_extent()) / float(BRICK_SIZE))),
    min_maj(grid.minorant_majorant())
{
    // allocate buffers
    if (glm::any(glm::greaterThanEqual(n_bricks, glm::uvec3(MAX_BRICKS))))
        throw std::runtime_error(std::string("exceeded max brick count of ") + std::to_string(MAX_BRICKS));
    indirection.resize(n_bricks);
    range.resize(n_bricks);
    atlas.resize(n_bricks * BRICK_SIZE);

    // construct brick grid
    brick_counter = 0;
    std::vector<int> slices(n_bricks.z);
    std::iota(slices.begin(), slices.end(), 0);
    std::for_each(std::execution::par_unseq, slices.begin(), slices.end(),
    [&](int bz)
    {
        for (size_t by = 0; by < n_bricks.y; ++by) {
            for (size_t bx = 0; bx < n_bricks.x; ++bx) {
                // store empty brick
                const glm::uvec3 brick = glm::uvec3(bx, by, bz);
                indirection[brick] = 0;
                // compute local range over dilated brick
                float local_min = FLT_MAX, local_max = -FLT_MAX;
                for (int z = -1; z < int(BRICK_SIZE) + 1; ++z) {
                    for (int y = -1; y < int(BRICK_SIZE) + 1; ++y) {
                        for (int x = -1; x < int(BRICK_SIZE) + 1; ++x) {
                            const float value = grid.lookup(glm::uvec3(glm::ivec3(brick * BRICK_SIZE) + glm::ivec3(x, y, z)));
                            local_min = std::min(local_min, value);
                            local_max = std::max(local_max, value);
                        }
                    }
                }
                range[brick] = encode_range(local_min, local_max);
                if (local_max == local_min) continue;
                // allocate memory for brick
                const size_t id = brick_counter.fetch_add(1, std::memory_order_relaxed);
                const glm::uvec3 ptr = indirection.linear_coord(id);
                // store pointer (offset) and range
                indirection[brick] = encode_ptr(ptr);
                // store brick data
                for (size_t z = 0; z < BRICK_SIZE; ++z)
                    for (size_t y = 0; y < BRICK_SIZE; ++y)
                        for (size_t x = 0; x < BRICK_SIZE; ++x)
                            atlas[ptr * BRICK_SIZE + glm::uvec3(x, y, z)] = encode_voxel(grid.lookup(brick * BRICK_SIZE + glm::uvec3(x, y, z)), decode_range(range[brick]));
            }
        }
    });

    // prune atlas in z dimension
    atlas.prune(BRICK_SIZE * std::round(std::ceil(brick_counter / float(n_bricks.x * n_bricks.y))));
}

BrickGrid::BrickGrid(const std::shared_ptr<Grid>& grid) : BrickGrid(*grid) {}

BrickGrid::~BrickGrid() {}

float BrickGrid::lookup(const glm::uvec3& ipos) const {
    const glm::uvec3 brick = ipos >> 3u;
    const glm::uvec3 ptr = decode_ptr(indirection[brick]);
    const glm::vec2 minmax = decode_range(range[brick]);
    const glm::uvec3 voxel = (ptr << 3u) + glm::uvec3(ipos & 7u);
    return decode_voxel(atlas[voxel], minmax);
}

std::tuple<float, float> BrickGrid::minorant_majorant() const { return min_maj; }

glm::uvec3 BrickGrid::index_extent() const { return n_bricks * BRICK_SIZE; }

size_t BrickGrid::num_voxels() const { return brick_counter * VOXELS_PER_BRICK; }

size_t BrickGrid::size_bytes() const {
    const size_t dense_bricks = n_bricks.x * n_bricks.y * n_bricks.z;
    const size_t size_indirection = sizeof(uint32_t) * dense_bricks;
    const size_t size_range = sizeof(uint32_t) * dense_bricks;
    const size_t size_atlas = sizeof(uint8_t) * brick_counter * VOXELS_PER_BRICK;
    return size_indirection + size_range + size_atlas;
}

std::string BrickGrid::to_string(const std::string& indent) const {
    std::stringstream out;
    out << Grid::to_string(indent) << std::endl;
    out << indent << "voxel dim: " << glm::to_string(index_extent()) << std::endl;
    out << indent << "brick dim: " << glm::to_string(n_bricks) << std::endl;
    const size_t bricks_allocd = brick_counter, bricks_capacity = atlas.size().x * atlas.size().y * atlas.size().z / VOXELS_PER_BRICK;
    out << indent << "bricks in atlas: " << bricks_allocd << " / " << bricks_capacity << " (" << uint32_t(std::round(100 * bricks_allocd / float(bricks_capacity))) << "%)" << std::endl;
    out << indent << "atlas dim: " << glm::to_string(atlas.size()) << std::endl;
    return out.str();
}

}
