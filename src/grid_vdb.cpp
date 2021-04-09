#include "grid_vdb.h"

GridOpenVDB::GridOpenVDB(const fs::path& filename, const std::string& gridname) {
    // open file
    openvdb::initialize();
    openvdb::io::File vdb_file(filename.string());
    vdb_file.open();
    // load grid
    openvdb::GridBase::Ptr baseGrid = 0;
    for (openvdb::io::File::NameIterator nameIter = vdb_file.beginName(); nameIter != vdb_file.endName(); ++nameIter) {
        if (nameIter.gridName() == gridname) {
            baseGrid = vdb_file.readGrid(nameIter.gridName());
            break;
        }
    }
    vdb_file.close();
    // cast to FloatGrid
    if (!baseGrid) throw std::runtime_error("No OpenVDB grid with name \"" + gridname + "\" found in " + filename.string());
    grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
    // extract transform
    if (!grid->transform().isLinear()) throw std::runtime_error("Only linear transformations supported!");
    const openvdb::Mat4R mat4 = grid->transform().baseMap()->getAffineMap()->getMat4();
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            transform[i][j] = mat4(i, j);
}

GridOpenVDB::~GridOpenVDB() {}

float GridOpenVDB::fetch(const glm::ivec3& ipos) const {
    auto acc = grid->getConstAccessor(); // slow, use own accessor for more perf
    return acc.getValue(openvdb::Coord(ipos.x, ipos.y, ipos.z));
}

std::tuple<float, float> GridOpenVDB::minorant_majorant() const {
    float min, maj;
    grid->evalMinMax(min, maj);
    return { min, maj };
}

std::tuple<glm::ivec3, glm::ivec3> GridOpenVDB::index_aabb() const {
    const openvdb::CoordBBox box = grid->evalActiveVoxelBoundingBox();
    const glm::ivec3 lo = glm::ivec3(box.min().x(), box.min().y(), box.min().z());
    const glm::ivec3 hi = glm::ivec3(box.max().x(), box.max().y(), box.max().z());
    return { lo, hi };
}

std::tuple<glm::vec3, glm::vec3> GridOpenVDB::world_aabb() const {
    const auto [ibb_min, ibb_max] = index_aabb();
    const glm::vec3 lo = glm::vec3(transform * glm::vec4(ibb_min, 1.f));
    const glm::vec3 hi = glm::vec3(transform * glm::vec4(ibb_max, 1.f));
    return { lo, hi };
}

size_t GridOpenVDB::num_voxels() const {
    return size_t(grid->activeVoxelCount());
}

std::string GridOpenVDB::to_string(const std::string& indent) const {
    std::stringstream out;
    out << Grid::to_string(indent) << std::endl;
    grid->print(out);
    return out.str().erase(out.str().rfind("]") + 1);
}
