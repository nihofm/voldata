#include "grid.h"

#include <filesystem>
namespace fs = std::filesystem;
#include <openvdb/openvdb.h>

class GridOpenVDB : public Grid {
public:
    GridOpenVDB(const fs::path& filename, const std::string& gridname = "density");
    ~GridOpenVDB();

    float fetch(const glm::ivec3& ipos) const;

    std::tuple<float, float> minorant_majorant() const;
    std::tuple<glm::ivec3, glm::ivec3> index_aabb() const;
    std::tuple<glm::vec3, glm::vec3> world_aabb() const;

    size_t num_voxels() const;

    virtual std::string to_string(const std::string& indent="") const override;

    // data
    openvdb::FloatGrid::Ptr grid;
};
