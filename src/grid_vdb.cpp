#include "grid_vdb.h"

GridOpenVDB::GridOpenVDB(const fs::path& filename, const std::string& gridname) {
    // open file
    openvdb::initialize();
    openvdb::io::File vdb_file(filename.string());
    vdb_file.open();
    // load grid
    openvdb::GridBase::Ptr baseGrid = 0;
    for (openvdb::io::File::NameIterator nameIter = vdb_file.beginName(); nameIter != vdb_file.endName(); ++nameIter) {
        std::cout << nameIter.gridName() << std::endl;
        if (nameIter.gridName() == gridname) {
            baseGrid = vdb_file.readGrid(nameIter.gridName());
            break;
        }
    }
    vdb_file.close();
    // cast to FloatGrid
    if (!baseGrid) throw std::runtime_error("No OpenVDB grid with name \"" + gridname + "\" found in " + filename.string());
    grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
    /*
    const openvdb::CoordBBox box = grid->evalActiveVoxelBoundingBox();
    const openvdb::Coord dim = grid->evalActiveVoxelDim() + openvdb::Coord(1); // inclusive bounds
    float min_value, max_value;
    grid->evalMinMax(min_value, max_value);
    // read into linearized array of uint8_t
    std::vector<uint8_t> data(dim.x() * dim.y() * dim.z());
    for (auto iter = grid->cbeginValueOn(); iter.test(); ++iter) {
        if (iter.isVoxelValue()) {
            const float value = *iter;
            const auto coord = iter.getCoord() - box.getStart();
            data[coord.z() * dim.x() * dim.y() + coord.y() * dim.x() + coord.x()] = uint8_t(std::round(value * 255.f));
            // compute majorant
            majorant = std::max(majorant, value);
        }
    }
    */
}

GridOpenVDB::~GridOpenVDB() {}

float GridOpenVDB::fetch(const glm::ivec3& ipos) const {
    // TODO
    return 0.f;
}

float GridOpenVDB::minorant() const {
    // TODO
    return 0.f;
}

float GridOpenVDB::majorant() const {
    // TODO
    return 0.f;
}

glm::vec3 GridOpenVDB::index_aabb_min() const {
    // TODO
    return glm::ivec3(0);
}

glm::vec3 GridOpenVDB::index_aabb_max() const {
    // TODO
    return glm::ivec3(0);
}
