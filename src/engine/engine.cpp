#include <cassert>
#include <cmath>
#include <iomanip>
#include <random>
#include <sstream>
#include <thread>

#include <scriptbuilder/scriptbuilder.h>

#include <engine/assetmanager.h>
#include <engine/context.h>
#include <engine/engine.h>
#include <engine/logger.h>

using namespace engine;
using namespace engine::character;

Engine::Engine(uint32_t width, uint32_t height, std::shared_ptr<Renderer> renderer)
    : mRunning(true)
    , mNeedRepaint(false)
    , mHasFocus(true)
    , mWidth(width)
    , mHeight(height)
    , mRenderer(std::move(renderer))
    , mMap(nullptr)
    , mPlayer(nullptr)
    , mEnableThreading(true)
{
    Logger::debug() << "Creating Engine";

    mScriptEngine = std::make_shared<script::ScriptEngine>();
    mContext = std::make_shared<Context>(*this, mScriptEngine, *mRenderer);

    mRenderer->setContext(mContext);
    mRenderer->init();

    mEngineObject = std::make_shared<EngineObject>(mContext);

    mHud = std::make_unique<engine::Hud>(mContext);
    mScreenEffects = std::make_unique<engine::ScreenEffects>(mContext);
    mCamera = std::make_unique<engine::Camera>(mContext, mWidth, mHeight);
    mClock = std::make_unique<engine::Clock>(mContext);
    mMap = std::make_unique<engine::Map>(mContext, "map");
    mPlayer = std::make_shared<engine::Player>(mContext);
    mPlayer->setPosition("map", std::floor((mMap->pixelWidth() - mPlayer->width()) / 2), std::floor((mMap->pixelHeight() - mPlayer->height()) / 2));

    mCharacters.emplace("player", mPlayer);

    std::shared_ptr<character::Character> dude = std::make_shared<character::Character>(mContext, "dude");
    dude->setPosition("map", 10 * 48, (9 * 48) - 24);
    dude->setDirection(Character::Direction::Down);
    mCharacters.emplace("dude", std::move(dude));

    std::shared_ptr<character::Character> horse = std::make_shared<Character>(mContext, "horse");
    horse->setPosition("map", 48, 96);
    horse->setDirection(Character::Direction::Right);
    mCharacters.emplace("horse", std::move(horse));

    mCamera->setTarget(mPlayer.get());
    mClock->setTime(6, 0);

    mLights.push_back(mPlayer);
    mMap->addLightSources(mLights);

    registerScript();

    if (mEnableThreading) {
        mThreads.push_back(std::make_unique<std::thread>(&Engine::animateAsync, this));
        mThreads.push_back(std::make_unique<std::thread>(&Engine::processAsync, this));
    }
}

Engine::~Engine()
{
    Logger::debug() << "~Engine";

    // Stop the running threads
    mRunning = false;
    for (auto& i: mThreads) {
        i->join();
    }

    Logger::debug() << "~Engine done";
}

int32_t Engine::bufferWidth() const
{
    return mMap->getTileDimension().width * 2;
}

int32_t Engine::bufferHeight() const
{
    return mMap->getTileDimension().height * 2;
}

bool Engine::process()
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

    return mNeedRepaint;
}

void Engine::animateAsync()
{
    while (mRunning) {
        uint64_t diff = mAnimationFrameTimer.start();

        if (mMap->animate(diff)) {
            mNeedRepaint = true;
        }

        Types::Dimension<> d = mMap->getTileDimension();
        for(auto &i: mCharacters) {
            if (mCamera->contains(i.second->rect(), d)) {
                if (i.second->animate(diff, !i.second->isMoving())) {
                    mNeedRepaint = true;
                }
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

        float_t x = 0, y = 0;
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
                        if (y == 0) {
                            y = -1;
                        }
                        if (!turned) {
                            mPlayer->turnTo(engine::character::Character::Direction::Up);
                            turned = true;
                        }
                        break;
                    case engine::Keys::Down:
                        if (y == 0) {
                            y = 1;
                        }
                        if (!turned) {
                            mPlayer->turnTo(engine::character::Character::Direction::Down);
                            turned = true;
                        }
                        break;
                    case engine::Keys::Left:
                        if (x == 0) {
                            x = -1;
                        }
                        if (!turned) {
                            mPlayer->turnTo(engine::character::Character::Direction::Left);
                            turned = true;
                        }
                        break;
                    case engine::Keys::Right:
                        if (x == 0) {
                            x = 1;
                        }
                        if (!turned) {
                            mPlayer->turnTo(engine::character::Character::Direction::Right);
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

                if (mClock->processAsync(diff, mMap.get())) {
                    mNeedRepaint = true;
                }
            } else {
                mPlayer->setSpeed(1.0f);
            }
        }

        mPlayer->velocity(diff, x, y);

        // Process movement, etc
        if (mCamera->processAsync(diff, mMap.get())) {
            mNeedRepaint = true;
        }

        for(auto &i: mCharacters) {
            if (i.second->processAsync(diff, *mMap, &mCharacters, mCamera.get())) {
                mNeedRepaint = true;
            }
        }

        if (mScreenEffects->processAsync(diff)) {
            mNeedRepaint = true;
        }

        if (!mEnableThreading) {
            break;
        }

        std::this_thread::yield();
    }
}

void Engine::paint()
{
    mDrawingTimer.start();

    Renderer& renderer = *mRenderer;
    renderer.fillRect(Types::Rect<>(0, 0, mWidth, mHeight), Types::Color(0, 0, 0));

    // Centre small maps.
    float_t translateX = 0.0f, translateY = 0.0f;
    if (mMap->pixelWidth() < mWidth) {
        translateX = std::floor((mWidth / 2) - (mMap->pixelWidth() / 2));
    }

    if (mMap->pixelHeight() < mHeight) {
        translateY = std::floor((mHeight / 2) - (mMap->pixelHeight() / 2));
    }

    if (translateX != 0.0f || translateY != 0.0f) {
        renderer.translate(translateX, translateY);
    }

    Types::Rect<> dst(mCamera->x(), mCamera->y(), mWidth, mHeight);
    mMap->draw(renderer, dst);

    Types::Dimension<> d = mMap->getTileDimension();
    int32_t startY = std::ceil(mCamera->y() / d.height);

    // Select what characters we need to draw
    std::multimap<int32_t, Character*> drawCharacters;
    for (auto& i: mCharacters) {
        auto rc = i.second->rect();
        if (i.second->map() == mMap->id() && mCamera->contains(rc, d)) {
            drawCharacters.emplace(rc.y + rc.height - d.height, i.second.get());
        }
    }

    for (int32_t row = startY - 1; (row <= startY + std::ceil(mHeight / d.height) + 1) || !drawCharacters.empty(); row++) {
        auto it = drawCharacters.begin();
        while (it != drawCharacters.end()) {
            if (row * d.height >= it->first) {
                it->second->draw(renderer, Types::Point<>(mCamera->x(), mCamera->y()));
                it = drawCharacters.erase(it);
            } else {
                it++;
            }
        }

        mMap->drawRow(renderer, dst, row, TilesetAbove::Row);
    }

    for (int32_t row = startY - 1; row <= startY + std::ceil(mHeight / d.height); row++) {
        mMap->drawRow(renderer, dst, row, TilesetAbove::All);
    }

    mScreenEffects->draw(renderer, *mClock, *mCamera, mLights);

    if (translateX != 0.0f || translateY != 0.0f) {
        renderer.translate(-translateX, -translateY);
    }

    mHud->draw(renderer, *mClock, *mPlayer, mDrawingTimer);

    mNeedRepaint = false;
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

void Engine::setSize(uint32_t width, uint32_t height)
{
    mWidth = width;
    mHeight = height;
    mCamera->setViewport({mWidth, mHeight});
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
        Logger::error() << ("Unrecoverable error while starting a new module.");
        return false;
    }

    if (builder.AddSectionFromFile("assets/script/main.as") < 0) {
        Logger::error() << ("Please correct the errors in the script and try again.");
        return false;
    }

    if (builder.BuildModule() < 0) {
        Logger::error() << ("Please correct the errors in the script and try again.");
        return false;
    }

    // Find the function that is to be called. 
    asIScriptModule *mod = mScriptEngine->engine()->GetModule("MainModule");
    asIScriptFunction *func = mod->GetFunctionByDecl("void main()");
    if (func == nullptr) {
        Logger::error() << ("The script must have the function 'void main()'. Please add it and try again.");
        return false;
    }

    // Create our context, prepare it, and then execute
    if (!mScriptEngine->createContext()) {
        return false;
    }

    mScriptEngine->context()->Prepare(func);

    int32_t r = mScriptEngine->context()->Execute();
    if (r == asEXECUTION_EXCEPTION) {
        Logger::error() << "An exception" << mScriptEngine->context()->GetExceptionString() << "occurred. Please correct the code and try again.";
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
