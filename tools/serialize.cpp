#include <string>
#include <memory>
#include <filesystem>

#include <voldata.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Error: argument required. Usage: " + std::filesystem::path(argv[0]).filename().string() + " <path-to-volume-file>" << std::endl;
        exit(1);
    }
    const std::filesystem::path path = argv[1];

    std::cout << "------------------" << std::endl;
    std::cout << "Loading volume: " << path.string() << std::endl;
    std::shared_ptr<voldata::Volume> volume = std::make_shared<voldata::Volume>(path);
    std::cout << *volume << std::endl;

    std::cout << "------------------" << std::endl;
    std::filesystem::path path_dense = path.filename().replace_extension(".dense");
    std::cout << "Serializing dense grid: " << path_dense << "..." << std::endl;
    std::shared_ptr<voldata::DenseGrid> dense_grid = std::make_shared<voldata::DenseGrid>(volume->current_grid());
    voldata::write_grid(dense_grid, path_dense);

    std::cout << "------------------" << std::endl;
    std::filesystem::path path_brick = path.filename().replace_extension(".brick");
    std::cout << "Serializing brick grid: " << path_brick << "..." << std::endl;
    std::shared_ptr<voldata::BrickGrid> brick_grid = std::make_shared<voldata::BrickGrid>(volume->current_grid());
    voldata::write_grid(brick_grid, path_brick);

    std::cout << "------------------" << std::endl;
    std::filesystem::path path_vdb = path.filename().replace_extension(".vdb");
    std::cout << "Serializing VDB grid: " << path_vdb << "..." << std::endl;
    std::shared_ptr<voldata::OpenVDBGrid> vdb_grid = std::make_shared<voldata::OpenVDBGrid>(volume->current_grid());
    voldata::write_grid(vdb_grid, path_vdb);

    std::cout << "------------------" << std::endl;
    std::filesystem::path path_nvdb = path.filename().replace_extension(".nvdb");
    std::cout << "Serializing NVDB grid: " << path_nvdb << "..." << std::endl;
    std::shared_ptr<voldata::NanoVDBGrid> nvdb_grid = std::make_shared<voldata::NanoVDBGrid>(volume->current_grid());
    voldata::write_grid(nvdb_grid, path_nvdb);

    std::cout << "done." << std::endl;

    return 0;
}
