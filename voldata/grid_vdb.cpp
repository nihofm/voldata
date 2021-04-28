#include "grid_vdb.h"

namespace voldata {

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
    // compute minorant and majorant
    grid->evalMinMax(minorant, majorant);
    minorant = std::min(minorant, grid->background());
    // extract transform
    if (!grid->transform().isLinear()) throw std::runtime_error("Only linear transformations supported!");
    const openvdb::Mat4R mat4 = grid->transform().baseMap()->getAffineMap()->getMat4();
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            transform[i][j] = mat4(i, j);
}

OpenVDBGrid::OpenVDBGrid(const Grid& other) : Grid(other) {
    const auto [min, maj] = other.minorant_majorant();
    grid = openvdb::FloatGrid::create(min);
    grid->setGridClass(openvdb::GRID_FOG_VOLUME);
    const auto isize = other.index_extent();
    auto acc = grid->getAccessor();
    for (uint32_t z = 0; z < isize.z; ++z) {
        for (uint32_t y = 0; y < isize.y; ++y) {
            for (uint32_t x = 0; x < isize.x; ++x) {
                const float value = other.lookup(glm::ivec3(x, y, z));
                if (value > min)
                    acc.setValue(openvdb::Coord(x, y, z), value);
            }
        }
    }
    grid->pruneGrid();
    // compute index bounding box
    const openvdb::CoordBBox box = grid->evalActiveVoxelBoundingBox();
    ibb_min = num_voxels() == 0 ? glm::ivec3(0) : glm::ivec3(box.min().x(), box.min().y(), box.min().z());
    ibb_max = num_voxels() == 0 ? glm::ivec3(0) : glm::ivec3(box.max().x(), box.max().y(), box.max().z());
    // compute minorant and majorant
    grid->evalMinMax(minorant, majorant);
    minorant = std::min(minorant, grid->background());
    // set transform
    openvdb::Mat4R mat4;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            mat4(i, j) = transform[i][j];
    grid->setTransform(openvdb::math::Transform::createLinearTransform(mat4));
}

OpenVDBGrid::OpenVDBGrid(const std::shared_ptr<Grid>& other) : OpenVDBGrid(*other) {}

OpenVDBGrid::~OpenVDBGrid() {}

float OpenVDBGrid::lookup(const glm::uvec3& ipos) const {
    auto acc = grid->getConstUnsafeAccessor(); // use own accessor for more perf
    return acc.getValue(openvdb::Coord(ipos.x + ibb_min.x, ipos.y + ibb_min.y, ipos.z + ibb_min.z));
}

std::tuple<float, float> OpenVDBGrid::minorant_majorant() const {
    return { minorant, majorant };
}

glm::uvec3 OpenVDBGrid::index_extent() const {
    if (num_voxels() == 0) return glm::uvec3(0);
    return ibb_max - ibb_min;
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
    out << indent << "ibb_min: " << glm::to_string(ibb_min) << std::endl;
    out << indent << "ibb_max: " << glm::to_string(ibb_max) << std::endl;
    grid->print(out);
    return out.str().erase(out.str().rfind("]") + 1);
}

}
