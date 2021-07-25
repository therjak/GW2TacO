#pragma once

#include <cstdint>
#include <memory>

std::unique_ptr<uint8_t[]> DecompressImage(const uint8_t* ImageData,
                                           int32_t ImageDataSize,
                                           int32_t& XSize, int32_t& YSize);
