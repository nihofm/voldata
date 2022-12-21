#include <voldata/volume.h>
#include <voldata/grid_dicom.h>
#include <voldata/serialization.h>

#include <fstream>
#include <iostream>
#include <algorithm>
#include <execution>
#include <filesystem>
namespace fs = std::filesystem;
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

// #include <openvdb/tools/GridTransformer.h>

namespace voldata {

Volume::Volume() : grid_frame_counter(0), model(glm::mat4(1)), albedo(1), phase(0.0), density_scale(1.0), emission_scale(1.0) {}

Volume::Volume(const GridPtr& grid, const std::string& gridname) : Volume() {
    GridFrame frame;
    frame[gridname] = grid;
    add_grid_frame(frame);
}

Volume::Volume(const std::string& filename, const std::string& gridname) : Volume() {
    GridFrame frame;
    frame[gridname] = load_grid(filename, gridname);
    add_grid_frame(frame);
}

Volume::Volume(size_t w, size_t h, size_t d, const uint8_t* data, const std::string& gridname) : Volume() {
    GridFrame frame;
    frame[gridname] = std::make_shared<DenseGrid>(w, h, d, data);
    add_grid_frame(frame);
}

Volume::Volume(size_t w, size_t h, size_t d, const float* data, const std::string& gridname) : Volume() {
    GridFrame frame;
    frame[gridname] = std::make_shared<DenseGrid>(w, h, d, data);
    add_grid_frame(frame);
}

Volume::~Volume() {}

void Volume::clear() {
    grids.clear();
}

void Volume::add_grid_frame(const GridFrame& frame) {
    grids.push_back(frame);
}

void Volume::update_grid_frame(const size_t i, const GridPtr& grid, const std::string& gridname) {
    grids[i][gridname] = grid;
}

bool Volume::has_grid(const size_t i, const std::string& gridname) const {
    return grids.at(i).find(gridname) != grids.at(i).end();
}

size_t Volume::n_grid_frames() const {
    return grids.size();
}

Volume::GridFrame Volume::current_grid_frame() const {
    return grids.at(grid_frame_counter);
}

Volume::GridPtr Volume::current_grid(const std::string& gridname) const {
    return current_grid_frame().at(gridname);
}

Volume::DenseGridPtr Volume::current_grid_dense(const std::string& gridname) const {
    return to_dense_grid(current_grid(gridname));
}

Volume::BrickGridPtr Volume::current_grid_brick(const std::string& gridname) const {
    return to_brick_grid(current_grid(gridname));
}

Volume::OpenVDBGridPtr Volume::current_grid_vdb(const std::string& gridname) const {
    return to_vdb_grid(current_grid(gridname));
}

Volume::NanoVDBGridPtr Volume::current_grid_nvdb(const std::string& gridname) const {
    return to_nvdb_grid(current_grid(gridname));
}

glm::mat4 Volume::get_transform(const std::string& gridname) const {
    if (grids.size() <= grid_frame_counter) return model;
    return model * current_grid(gridname)->transform;
}

glm::vec4 Volume::to_world(const glm::vec4& index, const std::string& gridname) const {
    return get_transform(gridname) * index;
}

glm::vec4 Volume::to_index(const glm::vec4& world, const std::string& gridname) const {
    return glm::inverse(get_transform(gridname)) * world;
}

std::pair<glm::vec3, glm::vec3> Volume::AABB(const std::string& gridname) const {
    if (grids.size() <= grid_frame_counter) return { glm::vec4(0), glm::vec4(0) };
    const glm::vec3 wbb_min = glm::vec3(to_world(glm::vec4(0, 0, 0, 1), gridname));
    const glm::vec3 wbb_max = glm::vec3(to_world(glm::vec4(glm::vec3(current_grid()->index_extent()), 1), gridname));
    return { wbb_min, wbb_max };
}

std::pair<float, float> Volume::minorant_majorant(const std::string& gridname) const {
    if (grids.size() <= grid_frame_counter) return { 0.f, 0.f };
    return current_grid(gridname)->minorant_majorant();
}

std::string Volume::to_string(const std::string& indent) const {
    std::stringstream out;
    const auto [bb_min, bb_max] = AABB();
    out << indent << "AABB: " << glm::to_string(bb_min) << " / " << glm::to_string(bb_max) << std::endl;
    out << indent << "modelmatrix: " << glm::to_string(model) << std::endl;
    out << indent << "current grid frame: " << grid_frame_counter << " / " << grids.size() << std::endl;
    out << indent << "current grid: " << current_grid()->to_string(indent + "  ") << std::endl;
    out << indent << "albedo: " << glm::to_string(albedo) << std::endl;
    out << indent << "phase: " << phase << std::endl;
    out << indent << "density scale: " << density_scale;
    return out.str();
}

void Volume::scale_and_move_to_unit_cube() {
    // compute max AABB over whole volume (animation)
    glm::vec3 bb_min = glm::vec3(FLT_MAX), bb_max = glm::vec3(FLT_MIN);
    for (const auto frame : grids) {
        const auto grid = frame.at("density");
        bb_min = glm::min(bb_min, glm::vec3(grid->transform * glm::vec4(0, 0, 0, 1)));
        bb_max = glm::max(bb_max, glm::vec3(grid->transform * glm::vec4(glm::vec3(grid->index_extent()), 1)));
    }
    // scale to unit cube and move to origin
    const glm::vec3 extent = bb_max - bb_min;
    const float size = fmaxf(extent.x, fmaxf(extent.y, extent.z));
    model = glm::translate(glm::scale(glm::mat4(1), glm::vec3(1.f / size)), -bb_min - 0.5f * extent);
    density_scale = size;
}

Volume::GridPtr Volume::load_grid(const std::string& filename, const std::string& gridname) {
    fs::path path = filename;
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
        // TODO handle USHORT
        else if (format == "FLOAT")
            grid = std::make_shared<DenseGrid>(dim.x, dim.y, dim.z, (const float*)data.data());
        else
            throw std::runtime_error("Unsupported data format for .dat file: " + format);
        // scale and map from z up to y up
        grid->transform = glm::scale(glm::rotate(glm::mat4(1), float(1.5 * M_PI), glm::vec3(1, 0, 0)), slice_thickness);
        return grid;
    }
    // handle OpenVDB files
    else if (extension == ".vdb") {
        return std::make_shared<OpenVDBGrid>(path, gridname);
    }
    // handle NanoVDB files
    else if (extension == ".nvdb") {
        return std::make_shared<NanoVDBGrid>(path, gridname);
    }
    // handle dicom files
    else if (extension == ".dcm") {
        // search directory for other dicom files
        std::vector<fs::path> dicom_files;
        for(auto& p : fs::directory_iterator(path.parent_path())) {
            std::string ext = p.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".dcm")
                dicom_files.push_back(p);
        }
        // lexographic sort
        std::sort(dicom_files.begin(), dicom_files.end(), [](const fs::path& lhs, const fs::path& rhs) {
            if (lhs.string().size() == rhs.string().size())
                return lhs.string() < rhs.string();
            else
                return lhs.string().size() < rhs.string().size();
        });
        return std::make_shared<DICOMGrid>(dicom_files);
    }
    // handle binary dense grid
    else if (extension == ".dense") {
        return load_dense_grid(path);
    }
    // handle binary brick grid
    else if (extension == ".brick") {
        return load_brick_grid(path);
    }
    else
        throw std::runtime_error("Unable to load file extension: " + extension);
}

Volume::DenseGridPtr Volume::to_dense_grid(const GridPtr& grid) {
    auto dense = std::dynamic_pointer_cast<DenseGrid>(grid); // check type
    if (!dense) dense = std::make_shared<DenseGrid>(grid); // type not matching, convert grid
    return dense;
}

Volume::BrickGridPtr Volume::to_brick_grid(const GridPtr& grid) {
    auto brick = std::dynamic_pointer_cast<BrickGrid>(grid); // check type
    if (!brick) brick = std::make_shared<BrickGrid>(grid); // type not matching, convert grid
    return brick;
}

Volume::OpenVDBGridPtr Volume::to_vdb_grid(const GridPtr& grid) {
    auto vdb = std::dynamic_pointer_cast<OpenVDBGrid>(grid); // check type
    if (!vdb) vdb = std::make_shared<OpenVDBGrid>(grid); // type not matching, convert grid
    return vdb;
}

Volume::NanoVDBGridPtr Volume::to_nvdb_grid(const GridPtr& grid) {
    auto nvdb = std::dynamic_pointer_cast<NanoVDBGrid>(grid); // check type
    if (!nvdb) nvdb = std::make_shared<NanoVDBGrid>(grid); // type not matching, convert grid
    return nvdb;
}

Volume::VolumePtr Volume::load_folder(const std::string& path, std::vector<std::string> gridnames) {
    // TODO: debug crash on loading empty grids?
    VolumePtr result = std::make_shared<Volume>();
    std::cout << "Loading grid files from " << path << "..." << std::endl;
    // list files in given directory
    std::vector<fs::path> files;
    for(auto& p : fs::directory_iterator(fs::path(path)))
        files.push_back(p);
    // TODO: filter files based on type?
    // lexographic sort
    std::sort(files.begin(), files.end(), [](const fs::path& lhs, const fs::path& rhs) {
        if (lhs.string().size() == rhs.string().size())
            return lhs.string() < rhs.string();
        else
            return lhs.string().size() < rhs.string().size();
    });
    // load and add grid frames in parallel
    result->grids.resize(files.size());
    std::vector<size_t> slices(files.size());
    std::iota(slices.begin(), slices.end(), 0);
    std::for_each(std::execution::par_unseq, slices.begin(), slices.end(), [&](size_t i) {
        const fs::path file = files[i];
        GridFrame& frame = result->grids[i];
        for (const auto& gridname : gridnames) {
            try {
                result->update_grid_frame(i, load_grid(file, gridname), gridname);
            } catch (std::runtime_error& e) {}
        }
    });
    return result;
}

}
