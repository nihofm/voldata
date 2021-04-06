#include "grid.h"

#include <filesystem>
namespace fs = std::filesystem;
#include <openvdb/openvdb.h>

class GridOpenVDB : public Grid {
    GridOpenVDB(const fs::path& filename, const std::string& gridname = "density");
    ~GridOpenVDB();

    float fetch(const glm::ivec3& ipos) const;

    float minorant() const;
    float majorant() const;

    glm::vec3 index_aabb_min() const;
    glm::vec3 index_aabb_max() const;

    // data
    openvdb::FloatGrid::Ptr grid;
};
