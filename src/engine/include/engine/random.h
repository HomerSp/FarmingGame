#pragma once

#include <cstdint>

namespace engine {
class Random {
public:
    static uint32_t range(uint32_t min, uint32_t max);
    static uint32_t range(uint32_t max);

};
}
