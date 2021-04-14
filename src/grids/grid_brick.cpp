#include "grid_brick.h"

#include <iostream>
#include <atomic>

// ----------------------------------------------
// constants

static const size_t BRICK_SIZE = 8;
static const size_t BITS_PER_AXIS = 10;
static const size_t MAX_BRICKS = 1 << BITS_PER_AXIS;
static const size_t VOXELS_PER_BRICK = BRICK_SIZE * BRICK_SIZE * BRICK_SIZE;
static const float EPS = 1e-8f;

// ----------------------------------------------
// encoding helpers

uint32_t encode_range(float x, float y) {
    return uint32_t(glm::detail::toFloat16(x)) | uint32_t(glm::detail::toFloat16(y)) << 16;
}

glm::vec2 decode_range(uint32_t data) {
    return glm::vec2(glm::detail::toFloat32(data & 0xFFFF), glm::detail::toFloat32(data >> 16));
}

uint32_t encode_offset(uint8_t x, uint8_t y, uint8_t z) {
    assert(x < MAX_BRICKS && y < MAX_BRICKS && z < MAX_BRICKS);
    return uint32_t(x) | (uint32_t(y) << BITS_PER_AXIS) | (uint32_t(z) << (2*BITS_PER_AXIS));
}

glm::uvec3 decode_offset(uint32_t data) {
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
    return (int)std::ceil((float)num / denom);
}

BrickGrid::BrickGrid(const Grid& grid) :
    Grid(grid),
    n_bricks(glm::ivec3(glm::ceil(glm::vec3(grid.index_extent()) / float(BRICK_SIZE)))),
    min_maj(grid.minorant_majorant())
{
    // allocate buffers
    if (glm::any(glm::greaterThanEqual(n_bricks, glm::ivec3(MAX_BRICKS))))
        throw std::runtime_error(std::string("exceeded max brick count of ") + std::to_string(MAX_BRICKS));
    const size_t N_BRICKS = size_t(n_bricks.x) * n_bricks.y * n_bricks.z;
    std::cout << "N_BRICKS: " << N_BRICKS << std::endl;
    offset_data.resize(N_BRICKS);
    range_data.resize(N_BRICKS);
    atlas_data.reserve(N_BRICKS * VOXELS_PER_BRICK);

    // construct brick grid
    std::atomic<size_t> counter(0);
    for (int bz = 0; bz < n_bricks.z; ++bz) {
        for (int by = 0; by < n_bricks.y; ++by) {
            for (int bx = 0; bx < n_bricks.x; ++bx) {
                // dummy data for empty brick
                const size_t idx = bz * n_bricks.x * n_bricks.y + by * n_bricks.x + bx;
                offset_data[idx] = 0;
                range_data[idx] = encode_range(0.f, 0.f);
                // compute brick
                float local_min = FLT_MAX, local_max = FLT_MIN;
                float brick[VOXELS_PER_BRICK];
                for (size_t z = 0; z < BRICK_SIZE; ++z) {
                    for (size_t y = 0; y < BRICK_SIZE; ++y) {
                        for (size_t x = 0; x < BRICK_SIZE; ++x) {
                            const float value = grid.lookup(int(BRICK_SIZE) * glm::ivec3(bx, by, bz) + glm::ivec3(x, y, z));
                            brick[z * BRICK_SIZE * BRICK_SIZE + y * BRICK_SIZE + x] = value;
                            local_min = std::min(local_min, value);
                            local_max = std::max(local_max, value);
                        }
                    }
                } // end compute brick
                if (std::abs(local_max - local_min) < EPS) continue; // skip empty brick
                const size_t offset = counter.fetch_add(1, std::memory_order_relaxed);
                // TODO check offset encoding
                offset_data[idx] = encode_offset(offset % n_bricks.x, (offset / n_bricks.x) % n_bricks.y, offset / (n_bricks.x * n_bricks.y));
                range_data[idx] = encode_range(local_min, local_max);
                for (size_t i = 0; i < VOXELS_PER_BRICK; ++i)
                    atlas_data.push_back(encode_voxel(brick[i], local_min, local_max));
            }
        }
    }
}

BrickGrid::BrickGrid(const std::shared_ptr<Grid>& grid) : BrickGrid(*grid) {}

BrickGrid::~BrickGrid() {}

float BrickGrid::lookup(const glm::ivec3& ipos) const {
    const glm::ivec3 brick = ipos >> 3;
    const size_t brick_idx = brick.z * n_bricks.x * n_bricks.y + brick.y * n_bricks.x + brick.x;
    const glm::vec2 range = decode_range(range_data[brick_idx]);
    std::cout << "range: " << glm::to_string(range) << std::endl;
    const glm::uvec3 offset = (decode_offset(offset_data[brick_idx]) * glm::uvec3(BRICK_SIZE)) + glm::uvec3(ipos & glm::ivec3(BRICK_SIZE-1));
    std::cout << "offset: " << glm::to_string(offset) << std::endl;
    // TODO atlas data is off/zero
    const size_t atlas_idx = offset.z * n_bricks.x * n_bricks.y + offset.y * n_bricks.x + offset.x;
    std::cout << "atlas data: " << int(atlas_data[atlas_idx]) << std::endl;
    return decode_voxel(atlas_data[atlas_idx], range.x, range.y);
}

std::tuple<float, float> BrickGrid::minorant_majorant() const { return min_maj; }

glm::ivec3 BrickGrid::index_extent() const { return n_bricks * glm::ivec3(BRICK_SIZE); }

size_t BrickGrid::num_voxels() const { return atlas_data.size(); }

size_t BrickGrid::size_bytes() const {
    const size_t size_offset = 4 * offset_data.size();
    const size_t size_range = 4 * range_data.size();
    const size_t size_atlas = 1 * atlas_data.size();
    return size_offset + size_range + size_atlas;
}

std::string BrickGrid::to_string(const std::string& indent) const {
    // TODO print additional info
    std::stringstream out(Grid::to_string(indent));
    out << Grid::to_string(indent) << std::endl;
    out << indent << "voxel dim: " << glm::to_string(index_extent()) << std::endl;
    out << indent << "brick dim: " << glm::to_string(n_bricks) << std::endl;
    const size_t atlas_slices = ceil(atlas_data.size() / float(VOXELS_PER_BRICK * n_bricks.x * n_bricks.y));
    out << indent << "atlas dim: " << glm::to_string(glm::ivec3(n_bricks.x, n_bricks.y, atlas_slices)) << std::endl;
    out << indent << "dense voxels: " << (n_bricks.x * n_bricks.y * n_bricks.z * VOXELS_PER_BRICK) / 1000 << "k" << std::endl;
    out << indent << "sparse voxels: " << atlas_data.size() / 1000 << "k" << std::endl;
    out << indent << "dense bricks: " << offset_data.size() / 1000 << "k" << std::endl;
    out << indent << "sparse bricks: " << atlas_data.size() / VOXELS_PER_BRICK / 1000 << "k";
    return out.str();
}
