#include <engine/particles.h>
#include <engine/random.h>
#include <engine/weather.h>

using namespace engine;

constexpr uint32_t Particles::MAX;

Particle::Particle(const graphics::Color& color, const Types::Dimension<>& size)
    : color(color)
    , rect(Types::Rect<float_t>(0, 0, size.width, size.height))
    , speed(1.0f, 1.0f)
    , startLife(0.0f)
    , life(0.0f)
{
}

float_t Particle::alpha() const
{
    float_t l = std::abs((life * 2.0f) - startLife);
    return color.a() * std::abs(startLife - l);
}

Particles::Particles(uint32_t count, const graphics::Color& c, const Types::Dimension<>& size, bool enabled)
    : mEnabled(enabled)
    , mCount(count)
{
    mParticles.resize(mCount, Particle(c, size));
}

void Particles::processAsync(uint64_t frameDiff, Camera& camera, Weather& weather)
{
    double m = (frameDiff / 100.0f);
    std::lock_guard<std::mutex> locker(mMutex);

    float_t windDirection = weather.windDirectionRad();
    float_t windSpeed = weather.windSpeed();
    float_t windX = cos(windDirection) * m * windSpeed;
    float_t windY = sin(windDirection) * m * windSpeed;

    const auto& cameraRc = camera.rect();
    for (Particle& p: mParticles) {
        if (p.life > 0.0f) {
            if (p.rect.left() < cameraRc.left() || p.rect.right() > cameraRc.right()
                || p.rect.top() < cameraRc.top() || p.rect.bottom() > cameraRc.bottom()) {
                p.life = 0.0f;
            }

            p.rect.x += windX * p.speed.x;
            p.rect.y += windY * p.speed.y;
            p.life -= m * 0.1f;
            if (p.life < 0.0f) {
                p.life = 0.0f;
            }
        }

        if (mEnabled && p.life == 0.0f) {
            initNew(p, camera);
        }
    }
}

void Particles::lock()
{
    mMutex.lock();
}

void Particles::unlock()
{
    mMutex.unlock();
}

uint32_t Particles::size() const
{
    return mParticles.size();
}

void Particles::setEnabled(bool enabled)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mEnabled = enabled;
}

void Particles::setCount(uint32_t count)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mCount = count;
}

const Particle &Particles::operator[](int index) const
{
    return mParticles.at(index);
}

void Particles::initNew(Particle& particle, Camera& camera)
{
    float_t life = Random::range(0, 100) / 100.0f;
    particle.startLife = particle.life = life;
    particle.rect.x = camera.x() + Random::range(camera.width());
    particle.rect.y = camera.y() + Random::range(camera.height());
}