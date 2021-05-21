#include "volume.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "grid_vdb.h"
#include "grid_brick.h"
#include "grid_dense.h"
#include "grid_dicom.h"
#include "serialization.h"

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

// TODO put weird data types in own file (factory)
void Volume::load_grid(const fs::path& path) {
    std::string extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    // handle .dat files (Siemens renderer)
    if (extension == ".dat") {
        std::ifstream dat_file(path);
        if (!dat_file.is_open())
            throw std::runtime_error("Unable to read file: " + path.string());
        // read meta data
        fs::path raw_path;
        glm::ivec3 dim;
        std::string format;
        glm::vec3 slice_thickness;
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
        if (format == "UCHAR")
            grid = std::make_shared<DenseGrid>(dim.x, dim.y, dim.z, data.data());
        // TODO USHORT
        else if (format == "FLOAT")
            grid = std::make_shared<DenseGrid>(dim.x, dim.y, dim.z, (const float*)data.data());
        else
            throw std::runtime_error("Unsupported data format for .dat file: " + format);
        // scale and map from z up to y up
        grid->transform = glm::scale(glm::rotate(glm::mat4(1), float(1.5 * M_PI), glm::vec3(1, 0, 0)), slice_thickness);
        grids.push_back(grid);
    }
    // handle OpenVDB files
    else if (extension == ".vdb") {
        // TODO gridname parameter
        grids.push_back(std::make_shared<OpenVDBGrid>(path, "density"));
    }
    // handle dicom files
    else if (extension == ".dcm") {
        // search directory for other dicom files
        std::vector<fs::path> dicom_files;
        for(auto& p : fs::directory_iterator(path.parent_path())) {
            std::string ext = p.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".dcm") dicom_files.push_back(p);
        }
        // lexographic sort
        std::sort(dicom_files.begin(), dicom_files.end(), [](const fs::path& lhs, const fs::path& rhs) {
            return lhs.string().size() < rhs.string().size() || lhs.string() < rhs.string();
        });
        grids.push_back(std::make_shared<DICOMGrid>(dicom_files));
    }
    // handle binary dense grid
    else if (extension == ".dense") {
        grids.push_back(load_dense_grid(path));
    }
    // handle binary brick grid
    else if (extension == ".brick") {
        grids.push_back(load_brick_grid(path));
    }
    else
        throw std::runtime_error("Unable to load file extension: " + extension);
}

std::shared_ptr<Grid> Volume::current_grid() const {
    return grids[grid_frame];
}

glm::mat4 Volume::get_transform() const {
    return model * current_grid()->transform;
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

std::string Volume::to_string(const std::string& indent) const {
    std::stringstream out;
    const auto [bb_min, bb_max] = AABB();
    out << indent << "AABB: " << glm::to_string(bb_min) << " / " << glm::to_string(bb_max) << std::endl;
    out << indent << "modelmatrix: " << glm::to_string(model) << std::endl;
    out << indent << "current grid frame: " << grid_frame << " / " << grids.size() << std::endl;
    out << indent << "current grid: " << current_grid()->to_string(indent + "  ");
    //out << indent << "albedo: " << glm::to_string(get_albedo()) << std::endl;;
    //out << indent << "phase: " << get_phase() << std::endl;
    //out << indent << "density scale: " << get_density_scale();
    return out.str();

}

std::ostream& operator<<(std::ostream& out, const Volume& volume) { return out << volume.to_string(); }

}
