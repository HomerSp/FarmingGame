#include <random>

#include <engine/random.h>

using namespace engine;

int32_t Random::range(int32_t min, int32_t max)
{
    static std::random_device dev;
    static std::mt19937 rng(dev());

    std::uniform_int_distribution<int32_t> dist(min, max);
    return dist(rng);
} 

int32_t Random::range(int32_t max)
{
    return range(0, max);
}