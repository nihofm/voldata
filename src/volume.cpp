#include "volume.h"

Volume::Volume() {}

Volume::Volume(const std::shared_ptr<Grid>& grid) : grid(grid) {}

Volume::~Volume() {}

std::tuple<glm::ivec3, glm::ivec3> Volume::index_bounding_box() const {
    return grid->index_aabb();
}

std::tuple<glm::vec3, glm::vec3> Volume::world_bounding_box() const {
    return grid->world_aabb();
}

std::string Volume::to_string(const std::string& indent) const {
    std::stringstream out;
    const std::string indent2 = indent + "  ";
    out << "Grid: " << std::endl << grid->to_string(indent2) << std::endl;
    out << "Attributes: " << std::endl << attributes.to_string(indent2);
    return out.str();
}

std::ostream& operator<<(std::ostream& out, const Volume& volume) {
    return out << volume.to_string();
}
