#pragma once

#include "grid.h"

#include <vector>
#include <memory>
#include <filesystem>
namespace fs = std::filesystem;
#include <imebra/imebra.h>

namespace voldata {

class DICOMGrid : public Grid {
public:
    DICOMGrid(const std::vector<fs::path>& files);
    virtual ~DICOMGrid();

    float lookup(const glm::uvec3& ipos) const;
    std::tuple<float, float> minorant_majorant() const;
    glm::uvec3 index_extent() const;
    size_t num_voxels() const;
    size_t size_bytes() const;

    // data
    glm::uvec3 n_voxels;
    float min_value, max_value;
    size_t size_bytes_total;
    std::vector<imebra::DataSet> dicom_datasets;
    std::vector<imebra::Image> dicom_images;
};

}
