#include "grid_brick.h"

#include <iostream>
#include <numeric>
#include <algorithm>
#include <execution>

// ----------------------------------------------
// constants

static const uint32_t BRICK_SIZE = 8;
static const uint32_t BITS_PER_AXIS = 10;
static const uint32_t MAX_BRICKS = 1 << BITS_PER_AXIS;
static const uint32_t VOXELS_PER_BRICK = BRICK_SIZE * BRICK_SIZE * BRICK_SIZE;
static const float EPS = 1e-8f;

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

uint8_t encode_voxel(float value, float minorant, float majorant) {
    assert(value >= minorant && value <= majorant);
    return uint8_t(std::round(255 * (value - minorant) / (majorant - minorant)));
}

float decode_voxel(uint8_t data, float minorant, float majorant) {
    return data * (1.f / 255.f) * (majorant - minorant) + minorant;
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
                range[brick] = encode_range(0.f, 0.f);
                // compute brick range and save grid values
                float local_min = FLT_MAX, local_max = FLT_MIN;
                float brick_cache[BRICK_SIZE * BRICK_SIZE * BRICK_SIZE];
                for (size_t z = 0; z < BRICK_SIZE; ++z) {
                    for (size_t y = 0; y < BRICK_SIZE; ++y) {
                        for (size_t x = 0; x < BRICK_SIZE; ++x) {
                            const float value = grid.lookup(brick * BRICK_SIZE + glm::uvec3(x, y, z));
                            brick_cache[z * BRICK_SIZE * BRICK_SIZE + y * BRICK_SIZE + x] = value;
                            local_min = std::min(local_min, value);
                            local_max = std::max(local_max, value);
                        }
                    }
                }
                if (std::abs(local_max - local_min) < EPS) continue; // skip empty brick
                // allocate memory for brick
                const size_t id = brick_counter.fetch_add(1, std::memory_order_relaxed);
                const glm::uvec3 ptr = indirection.linear_coord(id);
                // store pointer (offset) and range
                indirection[brick] = encode_ptr(ptr);
                range[brick] = encode_range(local_min, local_max);
                // store brick data
                for (size_t z = 0; z < BRICK_SIZE; ++z)
                    for (size_t y = 0; y < BRICK_SIZE; ++y)
                        for (size_t x = 0; x < BRICK_SIZE; ++x)
                            atlas[ptr * BRICK_SIZE + glm::uvec3(x, y, z)] = encode_voxel(brick_cache[z * BRICK_SIZE * BRICK_SIZE + y * BRICK_SIZE + x], local_min, local_max);
            }
        }
    });

    // TODO prune atlas (run separate passes for range/ptr and data)
    const size_t atlas_slices = ceil(brick_counter / float(n_bricks.x * n_bricks.y));
    std::cout << "PRUNED ATLAS SLICES: " << atlas_slices << std::endl;
    //atlas.resize(glm::uvec3(n_bricks.x, n_bricks.y, atlas_slices));
}

BrickGrid::BrickGrid(const std::shared_ptr<Grid>& grid) : BrickGrid(*grid) {}

BrickGrid::~BrickGrid() {}

float BrickGrid::lookup(const glm::ivec3& ipos) const {
    const glm::uvec3 brick = ipos >> 3;
    const glm::uvec3 ptr = decode_ptr(indirection[brick]);
    const glm::vec2 minmax = decode_range(range[brick]);
    const glm::uvec3 voxel = ptr * BRICK_SIZE + glm::uvec3(ipos & 7);
    return decode_voxel(atlas[voxel], minmax.x, minmax.y);
}

std::tuple<float, float> BrickGrid::minorant_majorant() const { return min_maj; }

glm::ivec3 BrickGrid::index_extent() const { return n_bricks * BRICK_SIZE; }

size_t BrickGrid::num_voxels() const { return brick_counter * VOXELS_PER_BRICK; }

size_t BrickGrid::size_bytes() const {
    const size_t dense_bricks = n_bricks.x * n_bricks.y * n_bricks.z;
    const size_t size_indirection = sizeof(uint32_t) * dense_bricks;
    const size_t size_range = sizeof(uint32_t) * dense_bricks;
    const size_t size_atlas = sizeof(uint8_t) * brick_counter * VOXELS_PER_BRICK;
    return size_indirection + size_range + size_atlas;
}

std::string BrickGrid::to_string(const std::string& indent) const {
    // TODO print additional info?
    std::stringstream out;
    out << Grid::to_string(indent) << std::endl;
    out << indent << "voxel dim: " << glm::to_string(index_extent()) << std::endl;
    out << indent << "brick dim: " << glm::to_string(n_bricks) << std::endl;
    out << indent << "bricks in atlas: " << brick_counter << std::endl;
    out << indent << "atlas dim: " << glm::to_string(atlas.size()) << std::endl;
    return out.str();
}
