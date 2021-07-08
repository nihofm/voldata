#include "serialization.h"
#include "grid_vdb.h"

#include <fstream>

#include <cereal/types/utility.hpp>
#include <cereal/types/atomic.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/archives/portable_binary.hpp>

// external serialization functions
namespace cereal {
    // glm types
    template <class Archive> void serialize(Archive& archive, glm::vec2& v) { archive(v.x, v.y); }
    template <class Archive> void serialize(Archive& archive, glm::ivec2& v) { archive(v.x, v.y); }
    template <class Archive> void serialize(Archive& archive, glm::uvec2& v) { archive(v.x, v.y); }
    template <class Archive> void serialize(Archive& archive, glm::vec3& v) { archive(v.x, v.y, v.z); }
    template <class Archive> void serialize(Archive& archive, glm::ivec3& v) { archive(v.x, v.y, v.z); }
    template <class Archive> void serialize(Archive& archive, glm::uvec3& v) { archive(v.x, v.y, v.z); }
    template <class Archive> void serialize(Archive& archive, glm::vec4& v) { archive(v.x, v.y, v.z, v.w); }
    template <class Archive> void serialize(Archive& archive, glm::ivec4& v) { archive(v.x, v.y, v.z, v.w); }
    template <class Archive> void serialize(Archive& archive, glm::uvec4& v) { archive(v.x, v.y, v.z, v.w); }
    template <class Archive> void serialize(Archive& archive, glm::mat3& m) { archive(m[0], m[1], m[2]); }
    template <class Archive> void serialize(Archive& archive, glm::mat4& m) { archive(m[0], m[1], m[2], m[3]); }
}

namespace voldata {
    // buf3d
    template <class Archive, typename T> void serialize(Archive& archive, Buf3D<T>& buf) {
        archive(buf.stride, buf.data);
    }

    // dense grid
    template <class Archive> void serialize(Archive& archive, DenseGrid& grid) {
        archive(grid.transform, grid.n_voxels, grid.min_value, grid.max_value, grid.voxel_data);
    }

    // brick grid
    template <class Archive> void serialize(Archive& archive, BrickGrid& grid) {
        archive(grid.transform, grid.n_bricks, grid.min_maj, grid.brick_counter, grid.indirection, grid.range, grid.atlas, grid.range_mipmaps);
    }

    // general write func
    template <typename T> void write(const T& data, const fs::path& path) {
        std::ofstream file(path, std::ios::binary);
        cereal::PortableBinaryOutputArchive archive(file);
        archive(data);
    }

    void write_grid(const std::shared_ptr<Grid>& grid, const fs::path& path) {
        if(DenseGrid* dense = dynamic_cast<DenseGrid*>(grid.get()))
            write<DenseGrid>(*dense, path);
        else if(BrickGrid* brick = dynamic_cast<BrickGrid*>(grid.get()))
            write<BrickGrid>(*brick, path);
        else if(OpenVDBGrid* vdb = dynamic_cast<OpenVDBGrid*>(grid.get()))
            vdb->write(path); // simply write out vdb file
        else
            throw std::runtime_error("Unsupported grid type!");
    }

    std::shared_ptr<DenseGrid> load_dense_grid(const fs::path& path) {
        std::ifstream file(path, std::ios::binary);
        cereal::PortableBinaryInputArchive archive(file);
        std::shared_ptr<DenseGrid> grid = std::make_shared<DenseGrid>();
        archive(*grid.get());
        return grid;
    }

    std::shared_ptr<BrickGrid> load_brick_grid(const fs::path& path) {
        std::ifstream file(path, std::ios::binary);
        cereal::PortableBinaryInputArchive archive(file);
        std::shared_ptr<BrickGrid> grid = std::make_shared<BrickGrid>();
        archive(*grid.get());
        return grid;
    }
}
