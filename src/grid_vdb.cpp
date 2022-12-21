#include <voldata/grid_vdb.h>
#include <glm/gtx/string_cast.hpp>

#ifdef VOLDATA_WITH_OPENVDB

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
    // set some meta data
    grid->setName(gridname);
    grid->setGridClass(openvdb::GRID_FOG_VOLUME);
    // compute index bounding box
    const openvdb::CoordBBox box = grid->evalActiveVoxelBoundingBox();
    ibb_min = num_voxels() == 0 ? glm::ivec3(0) : glm::ivec3(box.min().x(), box.min().y(), box.min().z());
    const openvdb::Coord dim = grid->evalActiveVoxelDim();
    extent = glm::uvec3(dim.x(), dim.y(), dim.z());
    // compute minorant and majorant
    openvdb::math::MinMax extrema = openvdb::tools::minMax(grid->tree());
    minorant = std::min(extrema.min(), grid->background());
    majorant = extrema.max();
    // extract transform
    if (!grid->transform().isLinear()) throw std::runtime_error("Only linear transformations supported!");
    const openvdb::Mat4R mat4 = grid->transform().baseMap()->getAffineMap()->getMat4();
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            transform[i][j] = mat4(i, j);
    // translate by ibb_min in world space to align grid
    transform[3] += transform * glm::vec4(ibb_min.x, ibb_min.y, ibb_min.z, 0);
}

OpenVDBGrid::OpenVDBGrid(const Grid& other) : Grid(other) {
    const auto [min, maj] = other.minorant_majorant();
    // create fog volume grid
    grid = openvdb::FloatGrid::create(min);
    grid->setName("density");
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
    const openvdb::Coord dim = grid->evalActiveVoxelDim();
    extent = glm::uvec3(dim.x(), dim.y(), dim.z());
    const openvdb::CoordBBox box = grid->evalActiveVoxelBoundingBox();
    ibb_min = num_voxels() == 0 ? glm::ivec3(0) : glm::ivec3(box.min().x(), box.min().y(), box.min().z());
    // compute minorant and majorant
    openvdb::math::MinMax extrema = openvdb::tools::minMax(grid->tree());
    minorant = std::min(extrema.min(), grid->background());
    majorant = extrema.max();
    // set transform
    openvdb::Mat4R mat4;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            mat4(i, j) = transform[i][j];
    grid->setTransform(openvdb::math::Transform::createLinearTransform(mat4));
    // translate by ibb_min in world space to align grid
    transform[3] += transform * glm::vec4(ibb_min.x, ibb_min.y, ibb_min.z, 0);
}

OpenVDBGrid::OpenVDBGrid(const std::shared_ptr<Grid>& other) : OpenVDBGrid(*other) {}

OpenVDBGrid::~OpenVDBGrid() {}

float OpenVDBGrid::lookup(const glm::uvec3& ipos) const {
    auto acc = grid->getConstUnsafeAccessor(); // use own accessor for more perf
    return acc.getValue(openvdb::Coord(ipos.x + ibb_min.x, ipos.y + ibb_min.y, ipos.z + ibb_min.z));
}

std::pair<float, float> OpenVDBGrid::minorant_majorant() const {
    return { minorant, majorant };
}

glm::uvec3 OpenVDBGrid::index_extent() const {
    return extent;
}

size_t OpenVDBGrid::num_voxels() const {
    return size_t(grid->activeVoxelCount());
}

size_t OpenVDBGrid::size_bytes() const {
    return grid->memUsage();
}

void OpenVDBGrid::write(const fs::path& path) const {
    fs::path p = path; p.replace_extension(".vdb");
    openvdb::io::File file(p);
    openvdb::GridPtrVec grids;
    grids.push_back(grid);
    file.write(grids);
    file.close();
    std::cout << p << " written." << std::endl;
}

}

#endif
