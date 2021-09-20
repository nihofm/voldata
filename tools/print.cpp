#include <string>
#include <memory>
#include <chrono>
#include <glm/gtx/string_cast.hpp>

#include "voldata.h"

int main(int argc, char** argv) {
    std::string path = argc > 1 ? argv[1] : "/home/niko/render-data/volumetric/smoke.vdb";

    std::cout << "------------------" << std::endl;
    std::cout << "Volume:" << std::endl;
    auto start_load = std::chrono::system_clock::now();
    std::shared_ptr<voldata::Volume> volume = std::make_shared<voldata::Volume>(path);
    auto end_load = std::chrono::system_clock::now();
    std::cout << *volume << std::endl;

    std::cout << "------------------" << std::endl;
    std::cout << "Dense grid:" << std::endl;
    auto start_dense = std::chrono::system_clock::now();
    std::shared_ptr<voldata::DenseGrid> dense_grid = std::make_shared<voldata::DenseGrid>(volume->current_grid());
    auto end_dense = std::chrono::system_clock::now();
    std::cout << *dense_grid << std::endl;

    std::cout << "------------------" << std::endl;
    std::cout << "VDB grid:" << std::endl;
    auto start_vdb = std::chrono::system_clock::now();
    std::shared_ptr<voldata::OpenVDBGrid> vdb_grid = std::make_shared<voldata::OpenVDBGrid>(volume->current_grid());
    auto end_vdb = std::chrono::system_clock::now();
    std::cout << *vdb_grid << std::endl;

    std::cout << "------------------" << std::endl;
    std::cout << "Brick grid:" << std::endl;
    auto start_brick = std::chrono::system_clock::now();
    std::shared_ptr<voldata::BrickGrid> brick_grid = std::make_shared<voldata::BrickGrid>(volume->current_grid());
    auto end_brick = std::chrono::system_clock::now();
    std::cout << *brick_grid << std::endl;

    std::cout << "------------------" << std::endl;
    const glm::ivec3 ipos = glm::ivec3(63, 64, 65) + glm::ivec3(32);
    std::cout << "lookup grid " << glm::to_string(ipos) << ": " << volume->current_grid()->lookup(ipos) << std::endl;
    std::cout << "lookup dense " << glm::to_string(ipos) << ": " << dense_grid->lookup(ipos) << std::endl;
    std::cout << "lookup vdb " << glm::to_string(ipos) << ": " << vdb_grid->lookup(ipos) << std::endl;
    std::cout << "lookup brick " << glm::to_string(ipos) << ": " << brick_grid->lookup(ipos) << std::endl;

    std::cout << "------------------" << std::endl;
    std::cout << "Loading took " << (end_load - start_load).count() / 1000000 << "ms." << std::endl;
    std::cout << "Dense grid conversion took " << (end_dense - start_dense).count() / 1000000 << "ms." << std::endl;
    std::cout << "Brick grid conversion took " << (end_brick - start_brick).count() / 1000000 << "ms." << std::endl;
    std::cout << "VDB grid conversion took " << (end_vdb - start_vdb).count() / 1000000 << "ms." << std::endl;
    return 0;
}
