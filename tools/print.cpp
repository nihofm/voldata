#include <string>
#include <memory>
#include <chrono>
#include <execution>
#include <glm/gtx/string_cast.hpp>

#include <voldata.h>

std::pair<float, float> benchmark_grid(const std::shared_ptr<voldata::Grid>& grid, const glm::uvec3& n_voxels) {
    auto start = std::chrono::system_clock::now();
    float norm_f = 1.f / (n_voxels.x * n_voxels.y * n_voxels.z);
    // prepare slices
    std::vector<uint32_t> slices(n_voxels.z);
    std::iota(slices.begin(), slices.end(), 0);
    // pass to find global minorant and majorant
    std::vector<float> partial_sums(n_voxels.z, 0.f);
    std::for_each(std::execution::par_unseq, slices.begin(), slices.end(),
    [&](uint32_t z)
    {
        for (uint32_t y = 0; y < n_voxels.y; ++y)
            for (uint32_t x = 0; x < n_voxels.x; ++x) {
                const float value = grid->lookup(glm::ivec3(x, y, z));
                partial_sums[z] += value * norm_f;
            }
    });
    // reduce
    float average = 0.f;
    for (uint32_t z = 0; z < n_voxels.z; ++z)
        average += partial_sums[z];
    auto end = std::chrono::system_clock::now();
    return { average, (end - start).count() / 1000000 }; // return average to avoid compiler optimizations and time in ms
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Error: argument required. Usage: " + std::filesystem::path(argv[0]).filename().string() + " <path-to-volume-file>" << std::endl;
        exit(1);
    }

    std::cout << "------------------------------------" << std::endl;
    std::cout << "Volume:" << std::endl;
    auto start_load = std::chrono::system_clock::now();
    std::shared_ptr<voldata::Volume> volume = std::make_shared<voldata::Volume>(argv[1]);
    auto end_load = std::chrono::system_clock::now();
    std::cout << volume->to_string("\t") << std::endl;

    std::cout << "------------------------------------" << std::endl;
    std::cout << "Dense grid:" << std::endl;
    auto start_dense = std::chrono::system_clock::now();
    std::shared_ptr<voldata::DenseGrid> dense_grid = std::make_shared<voldata::DenseGrid>(volume->current_grid());
    auto end_dense = std::chrono::system_clock::now();
    std::cout << dense_grid->to_string("\t") << std::endl;

    std::cout << "------------------------------------" << std::endl;
    std::cout << "Brick grid:" << std::endl;
    auto start_brick = std::chrono::system_clock::now();
    std::shared_ptr<voldata::BrickGrid> brick_grid = std::make_shared<voldata::BrickGrid>(volume->current_grid());
    auto end_brick = std::chrono::system_clock::now();
    std::cout << brick_grid->to_string("\t") << std::endl;

#ifdef VOLDATA_WITH_OPENVDB
    std::cout << "------------------------------------" << std::endl;
    std::cout << "OpenVDB grid:" << std::endl;
    auto start_vdb = std::chrono::system_clock::now();
    std::shared_ptr<voldata::OpenVDBGrid> vdb_grid = std::make_shared<voldata::OpenVDBGrid>(volume->current_grid());
    auto end_vdb = std::chrono::system_clock::now();
    std::cout << vdb_grid->to_string("\t") << std::endl;
#endif

    std::cout << "------------------------------------" << std::endl;
    std::cout << "NanoVDB grid:" << std::endl;
    auto start_nvdb = std::chrono::system_clock::now();
    std::shared_ptr<voldata::NanoVDBGrid> nvdb_grid = std::make_shared<voldata::NanoVDBGrid>(volume->current_grid());
    auto end_nvdb = std::chrono::system_clock::now();
    std::cout << nvdb_grid->to_string("\t") << std::endl;

    std::cout << "------------------------------------" << std::endl;
    std::cout << "Loading took " << (end_load - start_load).count() / 1000000 << "ms." << std::endl;
    std::cout << "Dense grid conversion took " << (end_dense - start_dense).count() / 1000000 << "ms." << std::endl;
    std::cout << "Brick grid conversion took " << (end_brick - start_brick).count() / 1000000 << "ms." << std::endl;
#ifdef VOLDATA_WITH_OPENVDB
    std::cout << "OpenVDB grid conversion took " << (end_vdb - start_vdb).count() / 1000000 << "ms." << std::endl;
#endif
    std::cout << "NanoVDB grid conversion took " << (end_nvdb - start_nvdb).count() / 1000000 << "ms." << std::endl;

    std::cout << "------------------------------------" << std::endl;
    std::cout << "Benchmarking grids (on CPU):" << std::endl;
    const glm::uvec3 n_voxels = volume->current_grid()->index_extent();
    const auto [avg_dense, time_dense] = benchmark_grid(dense_grid, n_voxels);
    std::cout << "Dense grid: avg: " << avg_dense << ", time: " << time_dense << "ms, size: " << round(dense_grid->size_bytes() / 1000000.0) << "MB." << std::endl;
    const auto [avg_brick, time_brick] = benchmark_grid(brick_grid, n_voxels);
    std::cout << "Brick grid: avg: " << avg_brick << ", time: " << time_brick << "ms, size: " << round(brick_grid->size_bytes() / 1000000.0) << "MB." << std::endl;
#ifdef VOLDATA_WITH_OPENVDB
    const auto [avg_vdb, time_vdb] = benchmark_grid(vdb_grid, n_voxels);
    std::cout << "OpenVDB grid: avg: " << avg_vdb << ", time: " << time_vdb << "ms, size: " << round(vdb_grid->size_bytes() / 1000000.0) << "MB." << std::endl;
#endif
    const auto [avg_nvdb, time_nvdb] = benchmark_grid(nvdb_grid, n_voxels);
    std::cout << "NanoVDB grid: avg: " << avg_nvdb << ", time: " << time_nvdb << "ms, size: " << round(nvdb_grid->size_bytes() / 1000000.0) << "MB." << std::endl;

    return 0;
}
