#pragma once

#include <mutex>
#include <set>

#include <engine/camera.h>
#include <engine/types.h>

namespace engine {

class Weather;

struct Particle {
    Particle(const graphics::Color& color, const Types::Dimension<>& size);

    float_t alpha() const;

    graphics::Color color;
    Types::Rect<float_t> rect;
    Types::Point<float_t> speed;
    float_t startLife, life;
};

class Particles {
public:
    constexpr static uint32_t MAX = 1000;

    Particles(uint32_t count, const graphics::Color& c, const Types::Dimension<>& size, bool enabled = false);

    void lock();
    void unlock();

    void processAsync(uint64_t frameDiff, Camera& camera, Weather& weather);

    uint32_t size() const;

    void setEnabled(bool enabled);
    void setCount(uint32_t count);

    const Particle &operator[](int index) const;

protected:
    void initNew(Particle& particle, Camera& camera);

private:
    bool mEnabled;
    std::mutex mMutex;
    uint32_t mCount;
    std::vector<Particle> mParticles;
    Types::Point<float_t> mMinSpeed;
    Types::Point<float_t> mMaxSpeed;
};
}
