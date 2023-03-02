#include "grid_nvdb.h"
#include <nanovdb/util/IO.h>
#include <nanovdb/util/GridBuilder.h>

namespace voldata {

NanoVDBGrid::NanoVDBGrid(const fs::path& path, const std::string& gridname) {
    handle = nanovdb::io::readGrid<nanovdb::HostBuffer>(path.string(), gridname);
    grid = handle.grid<float>();
    if (!grid || !grid->isValid() || !grid->isFogVolume())
        throw std::runtime_error("Empty or invalid NanoVDB grid!");
    // compute index bounding box
    const nanovdb::CoordBBox ibb = grid->indexBBox();
    ibb_min = grid->isEmpty() ? glm::vec3(0) : glm::vec3(ibb.min()[0], ibb.min()[1], ibb.min()[2]);
    extent = grid->isEmpty() ? glm::vec3(0) : glm::vec3(ibb.max()[0] - ibb.min()[0] + 1, ibb.max()[1] - ibb.min()[1] + 1, ibb.max()[2] - ibb.min()[2] + 1);
    minorant = grid->tree().root().minimum();
    majorant = grid->tree().root().maximum();
    // extract transform
    transform = glm::mat4(1);
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j)
            transform[i][j] = grid->map().mMatF[i * 3 + j];
        transform[3][i] = grid->map().mVecF[i];
    }
    // translate by ibb_min in world space to align grid
    transform[3] += transform * glm::vec4(ibb_min.x, ibb_min.y, ibb_min.z, 0);
}

NanoVDBGrid::NanoVDBGrid(const Grid& other) : Grid(other) {
    const auto [min, maj] = other.minorant_majorant();
    const auto isize = other.index_extent();
    // create fog volume grid
    auto builder = nanovdb::GridBuilder<float>(0.f, nanovdb::GridClass::FogVolume);
    auto acc = builder.getAccessor();
    for (uint32_t z = 0; z < isize.z; ++z) {
        for (uint32_t y = 0; y < isize.y; ++y) {
            for (uint32_t x = 0; x < isize.x; ++x) {
                const float value = other.lookup(glm::uvec3(x, y, z));
                if (value > min)
                    acc.setValue(nanovdb::Coord(x, y, z), value);
            }
        }
    }
    handle = builder.getHandle<>();
    grid = handle.grid<float>();
    if (!grid || !grid->isValid() || !grid->isFogVolume())
        throw std::runtime_error("Empty or invalid NanoVDB grid!");
    // compute index bounding box
    const nanovdb::CoordBBox ibb = grid->indexBBox();
    ibb_min = grid->isEmpty() ? glm::vec3(0) : glm::vec3(ibb.min()[0], ibb.min()[1], ibb.min()[2]);
    extent = grid->isEmpty() ? glm::vec3(0) : glm::vec3(ibb.max()[0] - ibb.min()[0] + 1, ibb.max()[1] - ibb.min()[1] + 1, ibb.max()[2] - ibb.min()[2] + 1);
    minorant = grid->tree().root().minimum();
    majorant = grid->tree().root().maximum();
    // TODO: set transform
    // grid->map().set(transform, glm::inverse(transform), 0.f);
    // translate by ibb_min in world space to align grid
    transform[3] += transform * glm::vec4(ibb_min.x, ibb_min.y, ibb_min.z, 0);
}

NanoVDBGrid::NanoVDBGrid(const std::shared_ptr<Grid>& other) : NanoVDBGrid(*other) {}

NanoVDBGrid::~NanoVDBGrid() {}

float NanoVDBGrid::lookup(const glm::uvec3& ipos) const {
    const auto acc = grid->getAccessor();
    return acc.getValue(nanovdb::Coord(ipos.x + ibb_min.x, ipos.y + ibb_min.y, ipos.z + ibb_min.z));
}

std::pair<float, float> NanoVDBGrid::minorant_majorant() const {
    return { minorant, majorant };
}

glm::uvec3 NanoVDBGrid::index_extent() const {
    return extent;
}

size_t NanoVDBGrid::num_voxels() const {
    return size_t(grid->activeVoxelCount());
}

size_t NanoVDBGrid::size_bytes() const {
    return handle.size();
}

void NanoVDBGrid::write(const fs::path& path) const {
    fs::path p = path; p.replace_extension(".nvdb");
    nanovdb::io::writeGrid<nanovdb::HostBuffer>(p.string(), handle);
    std::cout << p << " written." << std::endl;
}

}
