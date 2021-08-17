#include <string>
#include <memory>
#include <glm/gtx/string_cast.hpp>

#include "voldata.h"

int main(int argc, char** argv) {
    std::string path = argc > 1 ? argv[1] : "/home/niko/render-data/volumetric/smoke.vdb";

    std::cout << "------------------" << std::endl;
    std::cout << "Volume:" << std::endl;
    std::shared_ptr<voldata::Volume> volume = std::make_shared<voldata::Volume>(path);
    std::cout << *volume << std::endl;

    std::cout << "------------------" << std::endl;
    std::cout << "Dense grid:" << std::endl;
    std::shared_ptr<voldata::DenseGrid> dense_grid = std::make_shared<voldata::DenseGrid>(volume->current_grid());
    std::cout << *dense_grid << std::endl;

    std::cout << "------------------" << std::endl;
    std::cout << "VDB grid:" << std::endl;
    std::shared_ptr<voldata::OpenVDBGrid> vdb_grid = std::make_shared<voldata::OpenVDBGrid>(volume->current_grid());
    std::cout << *vdb_grid << std::endl;

    std::cout << "------------------" << std::endl;
    std::cout << "Brick grid:" << std::endl;
    std::shared_ptr<voldata::BrickGrid> brick_grid = std::make_shared<voldata::BrickGrid>(volume->current_grid());
    std::cout << *brick_grid << std::endl;

    std::cout << "------------------" << std::endl;
    const glm::ivec3 ipos = glm::ivec3(63, 64, 65) + glm::ivec3(32);
    std::cout << "lookup grid " << glm::to_string(ipos) << ": " << volume->current_grid()->lookup(ipos) << std::endl;
    std::cout << "lookup dense " << glm::to_string(ipos) << ": " << dense_grid->lookup(ipos) << std::endl;
    std::cout << "lookup vdb " << glm::to_string(ipos) << ": " << vdb_grid->lookup(ipos) << std::endl;
    std::cout << "lookup brick " << glm::to_string(ipos) << ": " << brick_grid->lookup(ipos) << std::endl;
    return 0;
}
