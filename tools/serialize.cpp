#include <string>
#include <memory>
#include <filesystem>
namespace fs = std::filesystem;

#include "voldata/voldata.h"

int main(int argc, char** argv) {
    fs::path path = argc > 1 ? argv[1] : "/home/niko/render-data/volumetric/smoke.vdb";

    std::cout << "------------------" << std::endl;
    std::cout << "Loading volume: " << path.string() << std::endl;
    std::shared_ptr<voldata::Volume> volume = std::make_shared<voldata::Volume>(path);
    std::cout << *volume << std::endl;

    std::cout << "------------------" << std::endl;
    fs::path path_dense = path.filename().replace_extension(".dense");
    std::cout << "Serializing dense grid: " << path_dense << "..." << std::endl;
    std::shared_ptr<voldata::DenseGrid> dense_grid = std::make_shared<voldata::DenseGrid>(volume->current_grid());
    voldata::write_grid(dense_grid, path_dense);

    std::cout << "------------------" << std::endl;
    fs::path path_brick = path.filename().replace_extension(".brick");
    std::cout << "Serializing brick grid: " << path_brick << "..." << std::endl;
    std::shared_ptr<voldata::BrickGrid> brick_grid = std::make_shared<voldata::BrickGrid>(volume->current_grid());
    voldata::write_grid(brick_grid, path_brick);

    std::cout << "done." << std::endl;

    return 0;
}
