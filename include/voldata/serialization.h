#pragma once

#include <voldata/grid.h>
#include <voldata/grid_dense.h>
#include <voldata/grid_brick.h>

#include <memory>
#include <filesystem>
namespace fs = std::filesystem;

namespace voldata {
    // TODO inheritance?
    // TODO load/store volume?

    void write_grid(const std::shared_ptr<Grid>& grid, const fs::path& path);
    std::shared_ptr<DenseGrid> load_dense_grid(const fs::path& path);
    std::shared_ptr<BrickGrid> load_brick_grid(const fs::path& path);

} // namespace voldata
