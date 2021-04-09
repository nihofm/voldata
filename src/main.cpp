#include <string>
#include <memory>

#include "volume.h"
#include "grids/grid_vdb.h"

int main(int argc, char** argv) {
    std::string path = argc > 1 ? argv[1] : "/home/niko/render-data/volumetric/smoke.vdb";
    std::shared_ptr<Volume> vol = std::make_shared<Volume>(std::make_shared<GridOpenVDB>(path));
    vol->set_attribute("albedo", glm::vec3(0.5));
    vol->set_attribute("density_scale", 0.5f);
    std::cout << *vol << std::endl;
    std::cout << vol->get_attribute<float>("density_scale") << std::endl;
    std::cout << vol->get_attribute<glm::vec3>("albedo").x << std::endl;
    return 0;
}
