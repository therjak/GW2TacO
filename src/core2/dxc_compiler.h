#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace renderer {

bool CompileHlslToSpirv(const std::string& hlsl_source,
                        const std::string& entry_point,
                        const std::string& target_profile,
                        std::vector<uint32_t>* spirv_out,
                        std::string* error);

}  // namespace renderer
