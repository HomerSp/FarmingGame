#pragma once

#include <mutex>
#include <set>

#include <engine/camera.h>
#include <engine/graphics/buffer.h>
#include <engine/graphics/color.h>
#include <engine/graphics/renderer.h>
#include <engine/types.h>

namespace engine {

class Weather;

namespace graphics {
class Renderer;
}

struct Particle {
    Particle(const graphics::Color& color, const Types::Dimension<>& size);

    float_t alpha() const;

    graphics::Color color;
    Types::Rect<float_t> rect;
    Types::Point<float_t> speed;
    float_t startLife, life;
    float_t angle;
    bool needInit;
};

class Particles {
public:
    Particles(graphics::Renderer& renderer, uint32_t count, const graphics::Color& c, const Types::Dimension<>& size, bool enabled = false);

    void lock();
    void unlock();

    void draw(graphics::Renderer& renderer, const Types::Point<> dst);

    void processAsync(uint64_t frameDiff, Camera& camera, Weather& weather);

    graphics::Buffer& buffer() const;
    bool enabled() const;
    uint32_t size() const;

    void setEnabled(bool enabled, bool immediate = false);
    void setCount(uint32_t count);
    void setFrameSpeed(float_t frameSpeed);
    void setMoveSpeed(float_t moveSpeed);
    void setLifeRange(uint8_t minLife, uint8_t maxLife);
    void setAngle(float_t angle);
    void setSpawnRect(const Types::Rect<float_t> &rc);
    void setRound(bool round);

    const Particle &operator[](int index) const;

protected:
    void initNew(Particle& particle, Camera& camera, float_t angle);

private:
    const Particle mOriginParticle;
    bool mSetEnabled, mEnabled;
    std::mutex mMutex;
    uint32_t mCount;
    float_t mFrameSpeed;
    float_t mMoveSpeed;
    std::vector<Particle> mParticles;
    uint8_t mMinLife, mMaxLife;
    Types::Point<float_t> mMinSpeed, mMaxSpeed;
    float_t mAngle;
    Types::Rect<float_t> mSpawnRect;
    bool mRound;

    std::unique_ptr<graphics::Buffer> mBuffer;
};
}
