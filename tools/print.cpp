#include <string>
#include <memory>

#include "voldata/grid_vdb.h"
#include "voldata/grid_dense.h"
#include "voldata/grid_brick.h"

int main(int argc, char** argv) {
    std::string path = argc > 1 ? argv[1] : "/home/niko/render-data/volumetric/smoke.vdb";

    std::cout << "------------------" << std::endl;
    std::cout << "VDB grid:" << std::endl;
    std::shared_ptr<voldata::Grid> grid = std::make_shared<voldata::OpenVDBGrid>(path);
    std::cout << *grid << std::endl;

    std::cout << "------------------" << std::endl;
    std::cout << "Dense grid:" << std::endl;
    std::shared_ptr<voldata::DenseGrid> dense_grid = std::make_shared<voldata::DenseGrid>(grid);
    std::cout << *dense_grid << std::endl;

    std::cout << "------------------" << std::endl;
    std::cout << "VDB from dense grid:" << std::endl;
    std::shared_ptr<voldata::OpenVDBGrid> vdb_from_dense_grid = std::make_shared<voldata::OpenVDBGrid>(*dense_grid);
    std::cout << *vdb_from_dense_grid << std::endl;

    std::cout << "------------------" << std::endl;
    std::cout << "Brick grid:" << std::endl;
    std::shared_ptr<voldata::BrickGrid> brick_grid = std::make_shared<voldata::BrickGrid>(grid);
    std::cout << *brick_grid << std::endl;

    std::cout << "------------------" << std::endl;
    const glm::ivec3 ipos = glm::ivec3(63, 64, 65) + glm::ivec3(32);
    std::cout << "lookup vdb " << glm::to_string(ipos) << ": " << grid->lookup(ipos) << std::endl;
    std::cout << "lookup dense " << glm::to_string(ipos) << ": " << dense_grid->lookup(ipos) << std::endl;
    std::cout << "lookup vdb from dense " << glm::to_string(ipos) << ": " << vdb_from_dense_grid->lookup(ipos) << std::endl;
    std::cout << "lookup brick " << glm::to_string(ipos) << ": " << brick_grid->lookup(ipos) << std::endl;
    return 0;
}
