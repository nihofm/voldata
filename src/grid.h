#pragma once

#include <glm/glm.hpp>

class Grid {
public:
    virtual ~Grid() {}

    virtual float fetch(const glm::ivec3& ipos) const = 0;
    float operator[](const glm::ivec3& ipos) const { return fetch(ipos); }

    virtual float minorant() const = 0;
    virtual float majorant() const = 0;

    virtual glm::vec3 index_aabb_min() const = 0;
    virtual glm::vec3 index_aabb_max() const = 0;
};
