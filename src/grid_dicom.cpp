#include "grid_dicom.h"

#include <numeric>
#include <execution>
#include <glm/gtx/string_cast.hpp>

namespace voldata {

// TODO houndsfield range/filtering
const static int HF_CUTOFF = -750;

void outputDatasetTags(const imebra::DataSet& dataset, const std::wstring& prefix=L"") {
    // Output all the tags
    imebra::tagsIds_t tags = dataset.getTags();
    for(imebra::tagsIds_t::const_iterator scanTags(tags.begin()), endTags(tags.end()); scanTags != endTags; ++scanTags) {
        try {
            std::wstring tagName = imebra::DicomDictionary::getUnicodeTagDescription(*scanTags);
            std::wcout << prefix << L"Tag " << (*scanTags).getGroupId() << L"," << (*scanTags).getTagId() << L" (" << tagName << L")" << std::endl;
        }
        catch(const imebra::DictionaryUnknownTagError&) {
            std::wcout << prefix << L"Tag " << (*scanTags).getGroupId() << L"," << (*scanTags).getTagId() << L" (Unknown tag)" << std::endl;
        }
        imebra::Tag tag(dataset.getTag(*scanTags));
        for(size_t itemId(0); ; ++itemId) {
            try {
                imebra::DataSet sequence = tag.getSequenceItem(itemId);
                std::wcout << prefix << L"  SEQUENCE " << itemId << std::endl;
                outputDatasetTags(sequence, prefix + L"    ");
            }
            catch(const imebra::MissingDataElementError&) {
                break;
            }
        }
        for(size_t bufferId(0); bufferId != tag.getBuffersCount(); ++bufferId) {
            imebra::ReadingDataHandler handler = tag.getReadingDataHandler(bufferId);
            if(handler.getDataType() != imebra::tagVR_t::OW && handler.getDataType() != imebra::tagVR_t::OB)
                for(size_t scanHandler(0); scanHandler != handler.getSize(); ++scanHandler)
                    std::wcout << prefix << L"  buffer " << bufferId << L", position "<< scanHandler << ": " << handler.getUnicodeString(scanHandler) << std::endl;
            else
                std::wcout << prefix << L"  Not shown: size " << handler.getSize() << " elements" << std::endl;
        }
    }
}

DICOMGrid::DICOMGrid(const std::vector<fs::path>& files) :
    Grid(),
    n_voxels(0), 
    min_value(FLT_MAX),
    max_value(FLT_MIN),
    size_bytes_total(0)
{
    transform = glm::mat4(1);

    // load all dicom datasets and images
    for (size_t i = 0; i < files.size(); ++i) {
        dicom_datasets.push_back(imebra::CodecFactory::load(files[i].c_str()));
        dicom_images.push_back(dicom_datasets[i].getImage(0));
        // print tags?
        // if (i == 0) outputDatasetTags(dicom_datasets[i]);

        imebra::Image image = dicom_images[i];
        imebra::ReadingDataHandlerNumeric reader(image.getReadingDataHandler());

        n_voxels.x = std::max(n_voxels.x, image.getWidth());
        n_voxels.y = std::max(n_voxels.y, image.getHeight());
        n_voxels.z += 1;
        
        std::cout << "reading dicom image " << i << "/" << files.size() << ": " <<
            image.getWidth() << "x" << image.getHeight() << "x" << image.getChannelsNumber() << "\r" << std::flush;

        min_value = std::min(min_value, dicom_datasets[i].getFloat(imebra::TagId(imebra::tagId_t::SmallestImagePixelValue_0028_0106), 0, FLT_MAX));
        max_value = std::max(max_value, dicom_datasets[i].getFloat(imebra::TagId(imebra::tagId_t::LargestImagePixelValue_0028_0107), 0, FLT_MIN));
        size_bytes_total += image.getWidth() * image.getHeight() * image.getChannelsNumber() * reader.getUnitSize();

        if (i == 0) {
            // extract transform from DICOM tags in first dataset (I hope/assume they're all identical..)
            const float psx = dicom_datasets[i].getFloat(imebra::TagId(imebra::tagId_t::PixelSpacing_0028_0030), 0, 1.f);
            transform[0][0] = psx * dicom_datasets[i].getFloat(imebra::TagId(imebra::tagId_t::ImageOrientationPatient_0020_0037), 0, 1.f);
            transform[0][1] = psx * dicom_datasets[i].getFloat(imebra::TagId(imebra::tagId_t::ImageOrientationPatient_0020_0037), 1, 0.f);
            transform[0][2] = psx * dicom_datasets[i].getFloat(imebra::TagId(imebra::tagId_t::ImageOrientationPatient_0020_0037), 2, 0.f);

            const float psy = dicom_datasets[i].getFloat(imebra::TagId(imebra::tagId_t::PixelSpacing_0028_0030), 1, 1.f);
            transform[1][0] = psy * dicom_datasets[i].getFloat(imebra::TagId(imebra::tagId_t::ImageOrientationPatient_0020_0037), 3, 0.f);
            transform[1][1] = psy * dicom_datasets[i].getFloat(imebra::TagId(imebra::tagId_t::ImageOrientationPatient_0020_0037), 4, 1.f);
            transform[1][2] = psy * dicom_datasets[i].getFloat(imebra::TagId(imebra::tagId_t::ImageOrientationPatient_0020_0037), 5, 0.f);

            transform[2][2] = dicom_datasets[i].getFloat(imebra::TagId(imebra::tagId_t::SliceThickness_0018_0050), 0, 1.f) * std::max(psx, psy);

            transform[3][0] = dicom_datasets[i].getFloat(imebra::TagId(imebra::tagId_t::ImagePositionPatient_0020_0032), 0, 0.f);
            transform[3][1] = dicom_datasets[i].getFloat(imebra::TagId(imebra::tagId_t::ImagePositionPatient_0020_0032), 1, 0.f);
            transform[3][2] = dicom_datasets[i].getFloat(imebra::TagId(imebra::tagId_t::ImagePositionPatient_0020_0032), 2, 0.f);

            // extract houndsfield rescale parameters
            rescale_slope = dicom_datasets[i].getFloat(imebra::TagId(imebra::tagId_t::RescaleSlope_0028_1053), 0, 1.f);
            rescale_intercept = dicom_datasets[i].getFloat(imebra::TagId(imebra::tagId_t::RescaleIntercept_0028_1052), 0, 0.f);
        }
    }
    std::cout << std::endl;

    if (min_value == FLT_MAX && max_value == FLT_MIN) {
        // pass to find global minorant and majorant
        std::vector<uint32_t> slices(n_voxels.z);
        std::iota(slices.begin(), slices.end(), 0);
        std::vector<float> minima(n_voxels.z, FLT_MAX);
        std::vector<float> maxima(n_voxels.z, FLT_MIN);
        std::for_each(std::execution::par_unseq, slices.begin(), slices.end(),
        [&](uint32_t z)
        {
            for (uint32_t y = 0; y < n_voxels.y; ++y)
                for (uint32_t x = 0; x < n_voxels.x; ++x) {
                    const float value = lookup_raw(glm::uvec3(x, y, z));
                    minima[z] = std::min(minima[z], value);
                    maxima[z] = std::max(maxima[z], value);
                }
        });
        // reduce
        for (uint32_t z = 0; z < n_voxels.z; ++z) {
            min_value = std::min(min_value, minima[z]);
            max_value = std::max(max_value, maxima[z]);
        }
    }
}

DICOMGrid::~DICOMGrid() {
    // TODO imebra segfault on deconstruct (double free?)
    try {
        dicom_images.clear();
        dicom_datasets.clear();
    } catch (std::exception e) {
        std::cout << e.what() << std::endl;
    }
}

float DICOMGrid::lookup_raw(const glm::uvec3& ipos) const {
    if (ipos.z >= dicom_images.size()) return 0.f;
    const imebra::Image image = dicom_images[ipos.z];
    if (ipos.x >= image.getWidth() || ipos.y >= image.getHeight()) return 0.f;
    imebra::ReadingDataHandlerNumeric reader(image.getReadingDataHandler());
    return reader.getFloat((ipos.y * image.getWidth() + ipos.x) * image.getChannelsNumber());
}

float DICOMGrid::lookup(const glm::uvec3& ipos) const {
    return (lookup_raw(ipos) - min_value) / (max_value - min_value); // normalize to [0, 1]
}

float DICOMGrid::lookup_houndsfield(const glm::uvec3& ipos) const {
    return rescale_slope * lookup_raw(ipos) + rescale_intercept; // rescale to houndsfield units
}

std::pair<float, float> DICOMGrid::minorant_majorant() const { return { 0.f, 1.f }; }

glm::uvec3 DICOMGrid::index_extent() const { return n_voxels; }

size_t DICOMGrid::num_voxels() const { return size_t(n_voxels.x) * n_voxels.y * n_voxels.z; }

size_t DICOMGrid::size_bytes() const { return size_bytes_total; }

}
