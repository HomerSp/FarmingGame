#include <random>

#include <engine/random.h>

using namespace engine;

uint32_t Random::range(uint32_t min, uint32_t max)
{
    static std::random_device dev;
    static std::mt19937 rng(dev());

    std::uniform_int_distribution<uint32_t> dist(min, max);
    return dist(rng);
} 

uint32_t Random::range(uint32_t max)
{
    return range(0, max);
}