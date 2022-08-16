#include <cmath>

#include <scriptbuilder/scriptbuilder.h>

#include <engine/context.h>
#include <engine/engine.h>
#include <engine/graphics/renderer.h>
#include <engine/graphics/vector.h>
#include <engine/logger.h>

using namespace engine;
using namespace engine::character;

Engine::Engine(uint32_t width, uint32_t height, std::shared_ptr<graphics::Renderer> renderer)
    : mRunning(true)
    , mHasFocus(true)
    , mWidth(width)
    , mHeight(height)
    , mRenderer(std::move(renderer))
    , mMap(nullptr)
    , mPlayer(nullptr)
    , mEnableThreading(true)
{
    Logger::debug("Engine") << "Constructor";

    mScriptEngine = std::make_shared<script::ScriptEngine>();
    mContext = std::make_shared<Context>(*this, mScriptEngine, *mRenderer);

    mRenderer->setContext(mContext);

    mEngineObject = std::make_shared<EngineObject>(mContext);

    mHud = std::make_unique<engine::Hud>(mContext, *mRenderer);
    mCamera = std::make_unique<engine::Camera>(mContext, mWidth, mHeight);
    mClock = std::make_unique<engine::Clock>(mContext);
    mMap = std::make_unique<engine::Map>(mContext, *mRenderer, "map");
    mPlayer = std::make_shared<engine::Player>(mContext, *mRenderer);
    mWeather = std::make_unique<engine::Weather>(mContext, *mRenderer, *mClock);

    mClock->connect(mClock->DayChanged, std::bind(&Weather::dayChanged, std::ref(*mWeather), std::cref(*mClock)));
    mClock->connect(mClock->DaylightChanged, std::bind(&Weather::daylightChanged, std::ref(*mWeather), std::cref(*mClock)));

    mPlayer->setPosition("map", 9 * 48, 12 * 48);

    mCharacters.emplace("player", mPlayer);

    std::shared_ptr<character::Character> dude = std::make_shared<character::Character>(mContext, *mRenderer, "dude");
    dude->setPosition("map", 15 * 48, 8 * 48);
    dude->setDirection(Character::Direction::Down);
    mCharacters.emplace("dude", std::move(dude));

    std::shared_ptr<character::Character> horse = std::make_shared<Character>(mContext, *mRenderer, "horse");
    horse->setPosition("map", 48, 96);
    horse->setDirection(Character::Direction::Right);
    mCharacters.emplace("horse", std::move(horse));

    mCamera->setTarget(mPlayer.get());
    mClock->setTime(6, 0);

    mLights.push_back(mPlayer);
    mMap->addLightSources(mLights);

    mScreenEffects = std::make_unique<engine::ScreenEffects>(mContext, *mRenderer, mLights.size());

    registerScript();

    if (mEnableThreading) {
        mThreads.push_back(std::make_unique<std::thread>(&Engine::animateAsync, this));
        mThreads.push_back(std::make_unique<std::thread>(&Engine::processAsync, this));
    }
}

Engine::~Engine()
{
    Logger::debug("Engine") << "Destructor";

    // Stop the running threads
    mRunning = false;
    for (auto& i: mThreads) {
        i->join();
    }

    Logger::debug("Engine") << "Destructor done";
}

int32_t Engine::bufferWidth() const
{
    return mMap->getTileDimension().width * 2;
}

int32_t Engine::bufferHeight() const
{
    return mMap->getTileDimension().height * 2;
}

void Engine::process()
{
    if (!mEnableThreading) {
        animateAsync();
        processAsync();
    }

    mCamera->processListeners();
    mClock->processListeners();
    mPlayer->processListeners();
    for(auto &i: mCharacters) {
        i.second->processListeners();
    }
}

void Engine::animateAsync()
{
    while (mRunning) {
        uint64_t diff = mAnimationFrameTimer.start();

        mMap->animate(diff);

        Types::Dimension<> d = mMap->getTileDimension();
        for(auto &i: mCharacters) {
            if (mCamera->contains(i.second->rect(), d)) {
                i.second->animate(diff, !i.second->isMoving());
            }
        }

        if (!mEnableThreading) {
            break;
        }

        std::this_thread::yield();
    }
}

void Engine::processAsync()
{
    while (mRunning) {
        uint64_t diff = mProcessFrameTimer.start();

        engine::KeyList keys;
        {
            std::lock_guard<std::mutex> lock(mDownKeysMutex);
            keys = mKeys;
            mKeys.update();
        }

        if (keys.down(Keys::TestPause)) {
            if (!mEnableThreading) {
                break;
            }

            std::this_thread::yield();
            continue;
        }

        if (keys.down(engine::Keys::TestFastForward)) {
            if (keys.down(engine::Keys::Run)) {
                mClock->fastForward(diff * 0.5f);
            } else {
                mClock->fastForward(diff * 0.05f);
            }
        }

        if (keys.down(engine::Keys::TestSlowMode)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        int8_t x = 0, y = 0;
        if (mPlayer->canControl()) {
            if (keys.longPress(Keys::ExpandHudItems)) {
                mHud->expandItems(true);

                if (keys.up(Keys::Left)) {
                    mPlayer->decrementItem();
                } else if(keys.up(Keys::Right)) {
                    mPlayer->incrementItem();
                }

            } else {
                mHud->expandItems(false);
            }

            if (keys.up(Keys::ExpandHudItems)) {
                mPlayer->incrementItem();
            }

            if (!mHud->isExpanded()) {
                bool turned = false;
                for (auto it = keys.rbegin(); it != keys.rend(); it++) {
                    if (!it->down) {
                        continue;
                    }

                    switch (it->key) {
                    case engine::Keys::Up:
                        y = (y == 0) ? -1 : y;
                        if (!turned) {
                            mPlayer->turnToDirection(engine::character::Character::Direction::Up);
                            turned = true;
                        }
                        break;
                    case engine::Keys::Down:
                        y = (y == 0) ? 1 : y;
                        if (!turned) {
                            mPlayer->turnToDirection(engine::character::Character::Direction::Down);
                            turned = true;
                        }
                        break;
                    case engine::Keys::Left:
                        x = (x == 0) ? -1 : x;
                        if (!turned) {
                            mPlayer->turnToDirection(engine::character::Character::Direction::Left);
                            turned = true;
                        }
                        break;
                    case engine::Keys::Right:
                        x = (x == 0) ? 1 : x;
                        if (!turned) {
                            mPlayer->turnToDirection(engine::character::Character::Direction::Right);
                            turned = true;
                        }
                        break;
                    default:
                        break;
                    }

                    if (x != 0 && y != 0) {
                        break;
                    }
                }

                if (!turned) {
                    mPlayer->turnToDirection(engine::character::Character::Direction::None);
                }

                if (keys.down(engine::Keys::TestFriction)) {
                    mPlayer->setFriction(0.1f);
                } else {
                    mPlayer->setFriction(1.0f);
                }

                if (keys.down(engine::Keys::Run)) {
                    mPlayer->setSpeed(2.0f);
                } else if (keys.down(engine::Keys::Walk)) {
                    mPlayer->setSpeed(0.5f);
                } else {
                    mPlayer->setSpeed(1.0f);
                }

                if (keys.up(Keys::Use)) {
                    mPlayer->useItem();
                }

                mClock->processAsync(diff, mMap.get());
            } else {
                mPlayer->setSpeed(1.0f);
            }
        }

        mPlayer->velocity(diff, x, y);

        // Process movement, etc
        mCamera->processAsync(diff, mMap.get());

        for(auto &i: mCharacters) {
            i.second->processAsync(diff, *mMap, &mCharacters, mCamera.get());
        }

        mScreenEffects->processAsync(diff);
        mWeather->processAsync(diff, *mCamera);

        if (!mEnableThreading) {
            break;
        }

        std::this_thread::yield();
    }
}

void Engine::paint()
{
    mDrawingTimer.start();

    auto& renderer = *mRenderer;

    mMap->updateBuffers(renderer);
    for (auto& i: mCharacters) {
        i.second->updateBuffers(renderer, *mMap);
    }

    // Centre small maps.
    float_t translateX = 0.0f, translateY = 0.0f;
    if (mMap->pixelWidth() < mWidth) {
        translateX = std::floor((mWidth / 2) - (mMap->pixelWidth() / 2));
    }

    if (mMap->pixelHeight() < mHeight) {
        translateY = std::floor((mHeight / 2) - (mMap->pixelHeight() / 2));
    }

    Types::Point<> dst(translateX - mCamera->x(), translateY - mCamera->y());
    renderer.beginNative();

    // Draw water tiles
    mMap->drawBuffer(renderer, dst, TilesetAbove::Water);
    mWeather->drawWater(renderer, dst);

    mMap->drawBuffer(renderer, dst, TilesetAbove::None);

    for (auto& i: mCharacters) {
        i.second->drawBuffer(renderer, dst);
    }

    mMap->drawBuffer(renderer, dst, TilesetAbove::Row);
    mMap->drawBuffer(renderer, dst, TilesetAbove::All);

    mWeather->drawWeather(renderer, dst);

    mScreenEffects->draw(renderer, dst, *mClock, mLights, *mWeather);

    mHud->draw(renderer, *mClock, *mPlayer, mDrawingTimer);
    renderer.endNative();
}

void Engine::setKeyMap(const std::unordered_map<int32_t, Keys::Type>& keys)
{
    std::lock_guard<std::mutex> lock(mDownKeysMutex);
    for (auto key : keys) {
        mKeys.mapKey(key.first, key.second);
    }
}

void Engine::setKeyDown(int32_t key)
{
    std::lock_guard<std::mutex> lock(mDownKeysMutex);
    mKeys.setDown(key);
}

void Engine::setKeyUp(int32_t key)
{
    std::lock_guard<std::mutex> lock(mDownKeysMutex);
    mKeys.setUp(key);
}

void Engine::setSize(uint32_t width, uint32_t height, double devicePixelRatio)
{
    mWidth = width;
    mHeight = height;
    mCamera->setViewport({mWidth, mHeight});
    mWeather->setSize({mWidth, mHeight});
    mRenderer->setSize(width, height, devicePixelRatio);
}

bool Engine::registerScript()
{
    if (!mScriptEngine->create()) {
        return false;
    }

    // Generic objects
    script::ScriptObject::registerCallback<script::ScriptCallback>(mScriptEngine->engine());

    Character::registerClass(mScriptEngine->engine());
    Clock::registerClass(mScriptEngine->engine());
    Player::registerClass(mScriptEngine->engine());
    Camera::registerClass(mScriptEngine->engine());
    EngineObject::registerClass(mScriptEngine->engine());

    mEngineObject->registerObject();

    mScriptEngine->engine()->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(Logger::scriptPrint), asCALL_CDECL);

    // The CScriptBuilder helper is an add-on that loads the file,
    // performs a pre-processing pass if necessary, and then tells
    // the engine to build a script module.
    CScriptBuilder builder;
    if (builder.StartNewModule(mScriptEngine->engine(), "MainModule") != 0) {
        Logger::critical("Engine") << "registerScript, Unrecoverable error while starting a new module.";
        return false;
    }

    std::string data = mContext->assetManager().script("main");
    if (builder.AddSectionFromMemory("main", data.c_str(), data.length()) < 0) {
        Logger::critical("Engine") << "registerScript, Please correct the errors in the script and try again.";
        return false;
    }

    if (builder.BuildModule() < 0) {
        Logger::critical("Engine") << "registerScript, Please correct the errors in the script and try again.";
        return false;
    }

    // Find the function that is to be called. 
    asIScriptModule *mod = mScriptEngine->engine()->GetModule("MainModule");
    asIScriptFunction *func = mod->GetFunctionByDecl("void main()");
    if (func == nullptr) {
        Logger::critical("Engine") << "registerScript, The script must have the function 'void main()'. Please add it and try again.";
        return false;
    }

    // Create our context, prepare it, and then execute
    if (!mScriptEngine->createContext()) {
        return false;
    }

    mScriptEngine->context()->Prepare(func);

    int32_t r = mScriptEngine->context()->Execute();
    if (r == asEXECUTION_EXCEPTION) {
        Logger::critical("engine") << "registerScript, An exception" << mScriptEngine->context()->GetExceptionString() << "occurred. Please correct the code and try again.";
        return false;
    }

    return (r == asEXECUTION_FINISHED);
}

Engine::EngineObject::EngineObject(std::shared_ptr<Context> &ctx)
    : ScriptObject(ctx)
{
}

std::string Engine::EngineObject::className()
{
    return "Engine";
}

void Engine::EngineObject::registerClass(asIScriptEngine* engine)
{
    registerReference<EngineObject>(engine);
    REGISTER_FUNC(engine, EngineObject, Camera&, camera);
    REGISTER_FUNC(engine, EngineObject, Clock&, clock);
    REGISTER_FUNC(engine, EngineObject, Player&, player);
    REGISTER_FUNC_ARGS(engine, EngineObject, Character&, character, const std::string);
}

void Engine::EngineObject::registerObject()
{
    registerInstance<EngineObject>(context().scriptEngine().engine(), "engine", this);
}

engine::Camera* Engine::EngineObject::camera()
{
    return context().engine().mCamera.get();
}

engine::Clock* Engine::EngineObject::clock()
{
    return context().engine().mClock.get();
}

engine::Player* Engine::EngineObject::player()
{
    return context().engine().mPlayer.get();
}

engine::character::Character* Engine::EngineObject::character(const std::string& id)
{
    for (auto& i: context().engine().mCharacters) {
        if (i.second->id() == id) {
            return i.second.get();
        }
    }

    return nullptr;
}
