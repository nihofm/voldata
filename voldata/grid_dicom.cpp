#include "grid_dicom.h"

#include <numeric>
//#include <algorithm>
//#include <execution>

namespace voldata {

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
    // TODO parallelize
    for (size_t i = 0; i < files.size(); ++i) {
        dicom_datasets.push_back(imebra::CodecFactory::load(files[i].c_str(), 2048));
        dicom_images.push_back(dicom_datasets[i].getImage(0));

        //outputDatasetTags(dicom_datasets[i]);

        n_voxels.x = std::max(n_voxels.x, dicom_images[i].getWidth());
        n_voxels.y = std::max(n_voxels.y, dicom_images[i].getHeight());
        n_voxels.z += 1;
        
        imebra::Image image = dicom_images[i];
        imebra::ReadingDataHandlerNumeric reader(image.getReadingDataHandler());
        size_t size_bytes;
        const char* data = reader.data(&size_bytes);
        const size_t bytes_per_value = reader.getUnitSize();
        const bool is_signed = reader.isSigned();
        const bool is_float = reader.isFloat();
        std::cout << "image: " << image.getWidth() << "x" << image.getHeight() << "x" << image.getChannelsNumber() << std::endl;
        std::cout << "color space: " << image.getColorSpace() << std::endl;
        std::cout << "img size bytes: " << size_bytes << std::endl;
        std::cout << "bytes_per_value: " << bytes_per_value << std::endl;
        std::cout << "signed: " << (is_signed ? 1 : 0) << std::endl;
        std::cout << "float: " << (is_float ? 1 : 0) << std::endl;
        std::cout << "high bit: " << image.getHighBit() << std::endl;
        for (size_t y = 0; y < image.getHeight(); ++y) {
            for (size_t x = 0; x < image.getWidth(); ++x) {
                const float value = reader.getFloat((y * image.getWidth() + x) * image.getChannelsNumber());
                min_value = std::min(min_value, value);
                max_value = std::max(max_value, value);
            }
        }
        size_bytes_total += image.getWidth() * image.getHeight() * image.getChannelsNumber() * reader.getUnitSize();
    }
    // TODO extract transform from DICOM tags
    transform = glm::mat4(1);

    /*
    std::vector<uint32_t> slices(n_voxels.z);
    std::iota(slices.begin(), slices.end(), 0);
    // encode dense grid data with 8bit per voxel
    voxel_data.resize(n_voxels.x * n_voxels.y * n_voxels.z);
    std::for_each(std::execution::par_unseq, slices.begin(), slices.end(),
    [&](uint32_t z)
    {
        for (uint32_t y = 0; y < n_voxels.y; ++y)
            for (uint32_t x = 0; x < n_voxels.x; ++x) {
                const float value = grid.lookup(glm::ivec3(x, y, z));
                const size_t idx = z * n_voxels.x * n_voxels.y + y * n_voxels.x + x;
                voxel_data[idx] = uint8_t(std::round(255 * (value - min_value) / (max_value - min_value)));
            }
    });
    */
}

DICOMGrid::~DICOMGrid() {}

float DICOMGrid::lookup(const glm::uvec3& ipos) const {
    if (ipos.z >= dicom_images.size()) return 0.f;
    const imebra::Image image = dicom_images[ipos.z];
    if (ipos.x >= image.getWidth() || ipos.y >= image.getHeight()) return 0.f;
    // TODO test
    imebra::ReadingDataHandlerNumeric reader(image.getReadingDataHandler());
    return reader.getFloat((ipos.y * image.getWidth() + ipos.x) * image.getChannelsNumber());
}

std::tuple<float, float> DICOMGrid::minorant_majorant() const { return { min_value, max_value }; }

glm::uvec3 DICOMGrid::index_extent() const { return n_voxels; }

size_t DICOMGrid::num_voxels() const { return size_t(n_voxels.x) * n_voxels.y * n_voxels.z; }

size_t DICOMGrid::size_bytes() const { return size_bytes_total; }

}
