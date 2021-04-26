#include "volume.h"
#include <iostream>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>

#if defined(WITH_DCMTK)
#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmimgle/dcmimage.h>
#endif

#include "grid_vdb.h"
#include "grid_brick.h"
#include "grid_dense.h"

namespace voldata {

Volume::Volume() : model(glm::mat4(1)), grid_frame(0) {}

Volume::Volume(const fs::path& path) : Volume() {
    load_grid(path);
}

Volume::Volume(size_t w, size_t h, size_t d, const uint8_t* data) : Volume() {
    grids.push_back(std::make_shared<DenseGrid>(w, h, d, data));
}

Volume::Volume(size_t w, size_t h, size_t d, const float* data) : Volume() {
    grids.push_back(std::make_shared<DenseGrid>(w, h, d, data));
}

Volume::~Volume() {}

void Volume::clear() {
    params.clear();
    grids.clear();
}

void Volume::load_grid(const fs::path& path) {
    const fs::path extension = path.extension();
    if (extension == ".dat") { // handle .dat
        // TODO put weird data types in own file (factory)
        std::ifstream dat_file(path);
        if (!dat_file.is_open())
            throw std::runtime_error("Unable to read file: " + path.string());
        // read meta data
        fs::path raw_path;
        glm::ivec3 dim;
        std::string format;
        glm::vec3 slice_thickness; // TODO
        int bits;
        while (!dat_file.eof()) {
            std::string key;
            dat_file >> key;
            if (key == "ObjectFileName:") {
                dat_file >> raw_path;
                printf("Scan raw file: %s\n", raw_path.c_str());
            } else if (key == "Resolution:") {
                dat_file >> dim.x;
                dat_file >> dim.y;
                dat_file >> dim.z;
                printf("Scan resolution: %i, %i, %i\n", dim.x, dim.y, dim.z);
            } else if (key == "SliceThickness:") {
                dat_file >> slice_thickness.x;
                dat_file >> slice_thickness.y;
                dat_file >> slice_thickness.z;
                printf("Scan slice thickness: %f, %f, %f\n", slice_thickness.x, slice_thickness.y, slice_thickness.z);
            } else if (key == "Format:") {
                dat_file >> format;
                printf("Scan format: %s\n", format.c_str());
            } else if (key == "BitsUsed:") {
                dat_file >> bits;
                printf("Scan bits used: %i\n", bits);
            } else
                std::cout << "Skipping key: " << key << "..." << std::endl;
        }
        // read raw data
        raw_path = path.parent_path() / raw_path;
        std::ifstream raw_file(raw_path, std::ios::binary);
        if (!raw_file.is_open())
            throw std::runtime_error("Unable to read file: " + raw_path.string());
        // parse data type and setup volume texture
        std::vector<uint8_t> data(std::istreambuf_iterator<char>(raw_file), {});
        std::cout << "data size bytes: " << data.size() << " / " << dim.x*dim.y*dim.z << std::endl;
        std::shared_ptr<Grid> grid;
        if (true || format.find("UCHAR"))
            grid = std::make_shared<DenseGrid>(dim.x, dim.y, dim.z, data.data());
        else if (format.find("FLOAT"))
            grid = std::make_shared<DenseGrid>(dim.x, dim.y, dim.z, (const float*)data.data());
        else
            throw std::runtime_error("Unsupported data format for .dat file: " + format);
        // scale and map from z up to y up
        grid->transform = glm::scale(glm::rotate(glm::mat4(1), float(1.5 * M_PI), glm::vec3(1, 0, 0)), slice_thickness);
        grids.push_back(grid);
    }
    else if (extension == ".vdb") {
        // TODO gridname
        grids.push_back(std::make_shared<OpenVDBGrid>(path, "density"));
    }
    else
        throw std::runtime_error("Unable to load file extension: " + extension.string());
}

std::shared_ptr<Grid> Volume::current_grid() const {
    return grids[grid_frame];
}

glm::mat4 Volume::get_transform() const {
    return current_grid()->transform * model;
}

glm::vec4 Volume::to_world(const glm::vec4& index) const {
    return get_transform() * index;
}

glm::vec4 Volume::to_index(const glm::vec4& world) const {
    return glm::inverse(get_transform()) * world;
}

std::tuple<glm::vec3, glm::vec3> Volume::AABB() const {
    const glm::vec3 wbb_min = glm::vec3(to_world(glm::vec4(0, 0, 0, 1)));
    const glm::vec3 wbb_max = glm::vec3(to_world(glm::vec4(glm::vec3(current_grid()->index_extent()), 1)));
    return { wbb_min, wbb_max };
}

}
