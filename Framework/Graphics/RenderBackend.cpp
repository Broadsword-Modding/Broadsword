#include "RenderBackend.hpp"
#include "DX11Backend.hpp"
// #include "DX12Backend.hpp"  // TODO: Phase 2

namespace Broadsword
{

std::unique_ptr<RenderBackend> CreateRenderBackend(RenderBackend::API api)
{
    // Phase 1: DX11 only
    // TODO Phase 2: Add DX12 support
    (void)api; // Unused for now
    return std::make_unique<DX11Backend>();
}

} // namespace Broadsword
