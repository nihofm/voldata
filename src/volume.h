#pragma once

#include <map>
#include <any>
#include <memory>
#include <glm/glm.hpp>

#include "grid.h"

class Volume {
public:
    Volume(const std::shared_ptr<Grid>& grid);
    virtual ~Volume();

    glm::mat4 world_to_index_transform() const;
    glm::mat4 index_to_world_transform() const;
    void set_world_to_index_transform(const glm::mat4& transform);

    std::tuple<glm::vec3, glm::vec3> index_bounding_box() const;
    std::tuple<glm::vec3, glm::vec3> world_bounding_box() const;

    //std::any get_attribute(const std::string& name);
    //void set_attribute(const std::string& name, const std::any& attribute);

    std::shared_ptr<Grid> get_grid() const;
    void set_grid(const std::shared_ptr<Grid>& grid);

    // convenience operators
    float operator[](const glm::ivec3& ipos) const { return grid->fetch(ipos); }
    //std::any operator[](const std::string& name) { return attributes[name]; }

protected:
    // data
    glm::mat4 transform;
    std::shared_ptr<Grid> grid;
    //std::map<std::string, std::any> attributes; // TODO
};
