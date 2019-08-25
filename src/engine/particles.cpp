#include <engine/graphics/bufferwriter.h>
#include <engine/graphics/matrix.h>
#include <engine/graphics/renderer.h>
#include <engine/graphics/transform.h>
#include <engine/particles.h>
#include <engine/random.h>
#include <engine/weather.h>

using namespace engine;

Particle::Particle(const graphics::Color& color, const Types::Dimension<>& size)
    : color(color)
    , rect(Types::Rect<float_t>(0, 0, size.width, size.height))
    , speed(1.0f, 1.0f)
    , startLife(0.0f)
    , life(0.0f)
    , angle(0.0f)
{
}

float_t Particle::alpha() const
{
    float_t l = std::abs((life * 2.0f) - startLife);
    return color.a() * std::abs(startLife - l);
}

Particles::Particles(graphics::Renderer& renderer, uint32_t count, const graphics::Color& c, const Types::Dimension<>& size, bool enabled)
    : mOriginParticle(c, size)
    , mEnabled(enabled)
    , mCount(count)
    , mFrameSpeed(100.0f)
    , mMoveSpeed(20.0f)
    , mMinLife(0)
    , mMaxLife(100)
    , mAngle(-1.0f)
    , mSpawnRect(-1, -1, -1, -1)
{
    mParticles.resize(mCount, mOriginParticle);

    mBuffer = renderer.createBuffer((graphics::Matrix::Size + graphics::Color::Size) * count);
}

void Particles::draw(graphics::Renderer& renderer, Camera& camera)
{
    std::lock_guard<std::mutex> locker(mMutex);

    if (mSpawnRect.width >= 0.0f) {
        auto transform = renderer.createTransform();

        engine::graphics::BufferWriter writer(*mBuffer);
        for (const auto& p: mParticles) {
            writer += engine::graphics::Color(p.color.r(), p.color.g(), p.color.b(), p.alpha());

            transform->reset();
            transform->translate(p.rect.x, p.rect.y);
            transform->rotate(180.0f - p.angle);
            transform->scale(p.rect.width, p.rect.height);

            writer += *transform;
        }

        writer.release();

        renderer.drawParticles({-camera.x(), -camera.y()}, *this);
    }
}

void Particles::processAsync(uint64_t frameDiff, Camera& camera, Weather& weather)
{
    std::lock_guard<std::mutex> locker(mMutex);
    if (mSpawnRect.width >= 0.0f) {
        double m = (frameDiff / mFrameSpeed);

        double_t windSpeed = weather.windSpeed();
        double_t windDirection = (weather.windDirection() * windSpeed * 45.0f);

        auto cameraRc = mSpawnRect;
        cameraRc.x += camera.x();
        cameraRc.y += camera.y();
        for (Particle& p: mParticles) {
            if (p.life > 0.0f) {
                if (cameraRc.intersects(p.rect)) {
                    float_t r = ((180.0f - p.angle) * Types::PI()) / 180.0f;
                    float_t windX = sin(r) * m;
                    float_t windY = -cos(r) * m;
                    p.rect.x += windX * p.speed.x * mMoveSpeed;
                    p.rect.y += windY * p.speed.y * mMoveSpeed;
                    p.life -= m * 0.1f;
                    if (p.life < 0.0f) {
                        p.life = 0.0f;
                    }
                } else {
                    p.life = 0.0f;
                }
            }

            if (mEnabled && p.life == 0.0f) {
                initNew(p, camera, windDirection);
            }
        }
    }
}

graphics::Buffer& Particles::buffer() const
{
    return *mBuffer;
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
    mBuffer->resize((graphics::Matrix::Size + graphics::Color::Size) * count);
    mParticles.resize(mCount, mOriginParticle);
}

void Particles::setFrameSpeed(float_t frameSpeed)
{
    mFrameSpeed = frameSpeed;
}

void Particles::setMoveSpeed(float_t moveSpeed)
{
    mMoveSpeed = moveSpeed;
}

void Particles::setLifeRange(uint8_t minLife, uint8_t maxLife)
{
    mMinLife = minLife;
    mMaxLife = maxLife;
}

void Particles::setAngle(float_t angle)
{
    mAngle = angle;
}

void Particles::setSpawnRect(const Types::Rect<float_t>& rc)
{
    mSpawnRect = rc;
}

const Particle &Particles::operator[](int index) const
{
    return mParticles.at(index);
}

void Particles::initNew(Particle& particle, Camera& camera, float_t angle)
{
    float_t x = Random::range(mSpawnRect.left(), mSpawnRect.right());
    float_t y = Random::range(mSpawnRect.top(), mSpawnRect.bottom());
    float_t r = (Random::range(100) - 50) / 10.0f;

    float_t life = Random::range(mMinLife, mMaxLife) / 100.0f;
    particle.startLife = particle.life = life;
    particle.rect.x = camera.x() + x;
    particle.rect.y = camera.y() + y;
    particle.angle = angle + r;
}