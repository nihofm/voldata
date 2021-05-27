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

    float lookup(const glm::uvec3& ipos) const; // lookup normalized value in [0, 1]
    float lookup_raw(const glm::uvec3& ipos) const; // lookup raw value from dicom in [min_value, max_value]
    float lookup_houndsfield(const glm::uvec3& ipos) const; // lookup rescaled houndsfield units
    std::tuple<float, float> minorant_majorant() const; // always returns (0, 1)
    glm::uvec3 index_extent() const;
    size_t num_voxels() const;
    size_t size_bytes() const;

    // data
    glm::uvec3 n_voxels;
    float min_value, max_value;
    float rescale_slope, rescale_intercept;
    size_t size_bytes_total;
    std::vector<imebra::DataSet> dicom_datasets;
    std::vector<imebra::Image> dicom_images;
};

}
