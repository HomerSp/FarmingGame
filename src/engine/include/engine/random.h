#pragma once

#include <cstdint>

namespace engine {
class Random {
public:
    static int32_t range(int32_t min, int32_t max);
    static int32_t range(int32_t max);

};
}
