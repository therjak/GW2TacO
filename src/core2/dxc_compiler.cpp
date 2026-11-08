#include "src/core2/dxc_compiler.h"

#ifdef _WIN32
#include <atlbase.h>    // CComPtr
#include <dxcapi.h>     // IDxcCompiler3, IDxcUtils, IDxcResult
#include <windows.h>
#pragma comment(lib, "dxcompiler.lib")
#else
#include <dxc/WinAdapter.h>
#include <dxc/dxcapi.h>
#endif

#include <string>
#include <vector>

#include "src/base/logger.h"

namespace renderer {

bool CompileHlslToSpirv(const std::string& hlsl_source,
                        const std::string& entry_point,
                        const std::string& target_profile,
                        std::vector<uint32_t>* spirv_out,
                        std::string* error) {
  spirv_out->clear();
  if (error) error->clear();

  CComPtr<IDxcUtils> utils;
  CComPtr<IDxcCompiler3> compiler;

  if (FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils))) ||
      FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)))) {
    if (error) *error = "DXC: failed to create compiler instances";
    Log_Err("[vk] DXC: failed to create IDxcUtils / IDxcCompiler3");
    return false;
  }

  // Convert source to wide strings for DXC.
  std::wstring w_entry(entry_point.begin(), entry_point.end());
  std::wstring w_profile(target_profile.begin(), target_profile.end());

  // Build argument list.
  std::vector<LPCWSTR> args = {
      L"-spirv",                      // output SPIR-V
      L"-fspv-target-env=vulkan1.3",  // target Vulkan 1.3
      L"-E", w_entry.c_str(),         // entry point
      L"-T", w_profile.c_str(),       // target profile (vs_6_0, ps_6_0, …)
      L"-fvk-use-dx-layout",          // match HLSL cbuffer layout
  };

  DxcBuffer source_buf{
      .Ptr = hlsl_source.data(),
      .Size = hlsl_source.size(),
      .Encoding = DXC_CP_UTF8,
  };

  CComPtr<IDxcResult> result;
  HRESULT hr = compiler->Compile(&source_buf, args.data(),
                                 static_cast<uint32_t>(args.size()), nullptr,
                                 IID_PPV_ARGS(&result));
  if (FAILED(hr) || !result) {
    if (error) *error = "DXC: Compile() call failed";
    Log_Err("[vk] DXC: compile failed with HRESULT 0x{:08X}",
            static_cast<uint32_t>(hr));
    return false;
  }

  // Check compile status.
  HRESULT status;
  result->GetStatus(&status);
  if (FAILED(status)) {
    CComPtr<IDxcBlobUtf8> errors_blob;
    result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors_blob), nullptr);
    std::string msg;
    if (errors_blob && errors_blob->GetStringLength() > 0) {
      msg = errors_blob->GetStringPointer();
    }
    if (error) *error = msg;
    Log_Warn("[vk] DXC: {}", msg);
    return false;
  }

  // Extract SPIR-V binary blob.
  CComPtr<IDxcBlob> spirv_blob;
  result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&spirv_blob), nullptr);
  if (!spirv_blob) {
    if (error) *error = "DXC: no SPIR-V output blob";
    Log_Err("[vk] DXC: no SPIR-V output blob in result");
    return false;
  }

  const uint32_t* words =
      reinterpret_cast<const uint32_t*>(spirv_blob->GetBufferPointer());
  size_t word_count = spirv_blob->GetBufferSize() / sizeof(uint32_t);
  spirv_out->assign(words, words + word_count);
  return true;
}

}  // namespace renderer
