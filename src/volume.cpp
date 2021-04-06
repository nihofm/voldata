#include "volume.h"

Volume::Volume(const std::shared_ptr<Grid>& grid) : grid(grid) {}

Volume::~Volume() {}

glm::mat4 Volume::world_to_index_transform() const { return transform; }

glm::mat4 Volume::index_to_world_transform() const { return glm::inverse(transform); }

void Volume::set_world_to_index_transform(const glm::mat4& transform) { this->transform = transform; }

std::tuple<glm::vec3, glm::vec3> Volume::index_bounding_box() const { return { grid->index_aabb_min(), grid->index_aabb_max() }; }

std::tuple<glm::vec3, glm::vec3> Volume::world_bounding_box() const {
    const auto [ibb_min, ibb_max] = index_bounding_box();
    const glm::mat4 to_world = index_to_world_transform();
    return { glm::vec3(to_world * glm::vec4(ibb_min, 1.f)), glm::vec3(to_world * glm::vec4(ibb_max, 1.f))};
}

std::shared_ptr<Grid> Volume::get_grid() const { return grid; }

void Volume::set_grid(const std::shared_ptr<Grid>& grid) { this->grid = grid; }
