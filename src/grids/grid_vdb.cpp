#include "grid_vdb.h"

OpenVDBGrid::OpenVDBGrid(const fs::path& filename, const std::string& gridname) {
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
    // compute index bounding box
    const openvdb::CoordBBox box = grid->evalActiveVoxelBoundingBox();
    ibb_min = num_voxels() == 0 ? glm::ivec3(0) : glm::ivec3(box.min().x(), box.min().y(), box.min().z());
    ibb_max = num_voxels() == 0 ? glm::ivec3(0) : glm::ivec3(box.max().x(), box.max().y(), box.max().z());
    // extract transform
    if (!grid->transform().isLinear()) throw std::runtime_error("Only linear transformations supported!");
    const openvdb::Mat4R mat4 = grid->transform().baseMap()->getAffineMap()->getMat4();
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            transform[i][j] = mat4(i, j);
    // TODO translate to origin in index-space
}

OpenVDBGrid::~OpenVDBGrid() {}

float OpenVDBGrid::lookup(const glm::ivec3& ipos) const {
    auto acc = grid->getConstUnsafeAccessor(); // use own accessor for more perf
    return acc.getValue(openvdb::Coord(ipos.x + ibb_min.x, ipos.y + ibb_min.y, ipos.z + ibb_min.z));
}

std::tuple<float, float> OpenVDBGrid::minorant_majorant() const {
    float min, maj;
    grid->evalMinMax(min, maj);
    return { min, maj };
}

glm::ivec3 OpenVDBGrid::index_extent() const {
    if (num_voxels() == 0) return glm::ivec3(0);
    return ibb_max - ibb_min + 1; // OpenVDB uses exclusive bounds
}

size_t OpenVDBGrid::num_voxels() const {
    return size_t(grid->activeVoxelCount());
}

size_t OpenVDBGrid::size_bytes() const {
    return grid->memUsage();
}

std::string OpenVDBGrid::to_string(const std::string& indent) const {
    std::stringstream out;
    out << Grid::to_string(indent) << std::endl;
    grid->print(out);
    return out.str().erase(out.str().rfind("]") + 1);
}
