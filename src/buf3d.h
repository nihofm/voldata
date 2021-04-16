#pragma once

#include <vector>
#include <glm/glm.hpp>

template <typename T> class Buf3D {
public:
    Buf3D(const glm::uvec3& stride = glm::uvec3(0)) :
    stride(stride),
    data(size_t(stride.x) * stride.y * stride.z)
    {}

    inline T& operator[](const glm::uvec3& at) { return data[linear_index(at)]; }
    inline const T& operator[](const glm::uvec3& at) const { return data[linear_index(at)]; }
    inline glm::uvec3 size() const { return stride; }

    inline void prune(size_t slices) {
        this->stride.z = slices;
        data.resize(stride.x * stride.y * stride.z);
    }

    inline void resize(const glm::uvec3& stride) {
        this->stride = stride;
        data.resize(size_t(stride.x) * stride.y * stride.z);
    }

    inline size_t linear_index(const glm::uvec3& v) const {
        return v.z * stride.x * stride.y + v.y * stride.x + v.x;
    }

    inline glm::uvec3 linear_coord(size_t idx) const {
        return glm::uvec3(idx % stride.x, (idx / stride.x) % stride.y, idx / (stride.x * stride.y));
    }

protected:
    // data
    glm::uvec3 stride;
    std::vector<T> data;
};