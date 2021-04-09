#pragma once

#include <map>
#include <any>
#include <tuple>
#include <memory>
#include <glm/glm.hpp>

#include "dict.h"
#include "grid.h"

class Volume {
public:
    Volume();
    Volume(const std::shared_ptr<Grid>& grid);
    virtual ~Volume();

    std::tuple<glm::ivec3, glm::ivec3> index_bounding_box() const;
    std::tuple<glm::vec3, glm::vec3> world_bounding_box() const;

    std::any get_attribute(const std::string& name) { return attributes[name]; }
    template <typename T> T get_attribute(const std::string& name) { return attributes.get<T>(name); }
    void set_attribute(const std::string& name, const std::any& attribute) { attributes.set(name, attribute); }

    std::string to_string(const std::string& indent="") const;

    // convenience operators
    float operator[](const glm::ivec3& ipos) const { return grid->fetch(ipos); }
    std::any operator[](const std::string& name) { return attributes[name]; }

    // data
    std::shared_ptr<Grid> grid;     // underlying voxel grid
    // TODO map/vector of multiple grids?
    Dictionary attributes;          // dictionary containing volume attributes
};

std::ostream& operator<<(std::ostream& out, const Volume& volume);
