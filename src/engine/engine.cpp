#include <cassert>
#include <cmath>
#include <iomanip>
#include <random>
#include <sstream>
#include <thread>

#include <scriptbuilder/scriptbuilder.h>
#include <scripthandle/scripthandle.h>
#include <scriptstdstring/scriptstdstring.h>

#include <engine/assetmanager.h>
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

    AssetManager::get()->setRenderer(mRenderer);

    mScript = std::make_shared<Engine::ScriptCreator>();

    mHud = std::make_shared<engine::Hud>();
    mScreenEffects = std::make_shared<engine::ScreenEffects>();
    mCamera = std::make_shared<engine::Camera>(mWidth, mHeight);
    mClock = std::make_shared<engine::Clock>();
    mMap = std::make_shared<engine::Map>("map");
    mPlayer = std::make_shared<engine::Player>();
    mPlayer->setX(std::floor((mMap->pixelWidth() - mPlayer->width()) / 2));
    mPlayer->setY(std::floor((mMap->pixelHeight() - mPlayer->height()) / 2));

    mCharacters.push_back(std::make_shared<Character>("dude"));
    mCharacters.back()->setX((8 * 48));
    mCharacters.back()->setY(48);
    mCharacters.back()->setDirection(Character::Direction::Down);

    mCharacters.push_back(std::make_shared<Character>("horse"));
    mCharacters.back()->setX(48);
    mCharacters.back()->setY(96);
    mCharacters.back()->setDirection(Character::Direction::Right);

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
        i->processListeners();
    }
    return mNeedRepaint;
}

void Engine::animateAsync()
{
    while (mRunning) {
        uint64_t diff = mAnimationFrameTimer.start();

        // Process animations
        if (mPlayer->animate(diff, !mPlayer->isMoving())) {
            mNeedRepaint = true;
        }

        if (mMap->animate(diff)) {
            mNeedRepaint = true;
        }

        Types::Dimension<> d = mMap->getTileDimension();
        for(auto &i: mCharacters) {
            if (mCamera->contains(*i, d)) {
                if (i->animate(diff, false)) {
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

            float_t x = 0, y = 0;
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

            mPlayer->velocity(diff, x, y);
        }

        // Process movement, etc
        if (mCamera->processAsync(diff, mMap.get())) {
            mNeedRepaint = true;
        }
        if (mPlayer->processAsync(diff, mMap.get(), &mCharacters, mCamera.get())) {
            mNeedRepaint = true;
        }

        for(auto &i: mCharacters) {
            if (i->processAsync(diff, mMap.get())) {
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

    std::multimap<int32_t, Character*> drawCharacters;
    for (auto& i: mCharacters) {
        if (mCamera->contains(*i, d)) {
            drawCharacters.emplace(std::make_pair(i->y() + i->height(), i.get()));
        }
    }

    drawCharacters.emplace(std::make_pair(mPlayer->y() + mPlayer->height(), mPlayer.get()));

    for (int32_t row = startY - 1; row <= startY + std::ceil(mHeight / d.height) + 1; row++) {
        auto it = drawCharacters.begin();
        while (it != drawCharacters.end()) {
            if (row * d.height >= it->first - d.height) {
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

    mScreenEffects->draw(renderer, *mClock.get(), *mCamera.get(), mLights);
    mHud->draw(renderer, *mClock.get(), *mPlayer.get(), mProcessFrameTimer);

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
    FunctionPtrHelper::init();

    if (!mScript->create()) {
        return false;
    }

    // This needs to be done after all other types have been registered.
    registerClass();

    mScript->engine()->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(Logger::scriptPrint), asCALL_CDECL);

    // The CScriptBuilder helper is an add-on that loads the file,
    // performs a pre-processing pass if necessary, and then tells
    // the engine to build a script module.
    CScriptBuilder builder;
    if (builder.StartNewModule(mScript->engine(), "MainModule") != 0) {
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
    asIScriptModule *mod = mScript->engine()->GetModule("MainModule");
    asIScriptFunction *func = mod->GetFunctionByDecl("void main()");
    if (func == nullptr) {
        Logger::error() << ("The script must have the function 'void main()'. Please add it and try again.");
        return false;
    }

    // Create our context, prepare it, and then execute
    if (!mScript->createContext()) {
        return false;
    }

    mScript->context()->Prepare(func);
    registerContext();

    int32_t r = mScript->context()->Execute();
    if (r == asEXECUTION_EXCEPTION) {
        Logger::error() << "An exception" << mScript->context()->GetExceptionString() << "occurred. Please correct the code and try again.";
        return false;
    }

    return (r == asEXECUTION_FINISHED);
}

std::string Engine::className()
{
    return "Engine";
}

void Engine::registerClass()
{
    registerReference<Engine>(mScript->engine());
    REGISTER_FUNC(mScript->engine(), Engine, Camera&, camera);
    REGISTER_FUNC(mScript->engine(), Engine, Clock&, clock);
    REGISTER_FUNC(mScript->engine(), Engine, Player&, player);
    REGISTER_FUNC_ARGS(mScript->engine(), Engine, Character&, character, const std::string);
    registerInstance<Engine>(mScript->engine(), "engine", this);
}

void Engine::registerContext()
{
    mCamera->setContext(mScript->context());
    mClock->setContext(mScript->context());
    mPlayer->setContext(mScript->context());
    for (auto& i: mCharacters) {
        i->setContext(mScript->context());
    }
}

engine::Camera* Engine::camera()
{
    return mCamera.get();
}

engine::Clock* Engine::clock()
{
    return mClock.get();
}

engine::Player* Engine::player()
{
    return mPlayer.get();
}

engine::character::Character* Engine::character(const std::string& id)
{
    for (auto& i: mCharacters) {
        if (i->id() == id) {
            return i.get();
        }
    }

    return nullptr;
}

Engine::ScriptCreator::ScriptCreator()
    : mScriptEngine(nullptr)
    , mScriptContext(nullptr)
{
}

Engine::ScriptCreator::~ScriptCreator()
{
    if (mScriptContext != nullptr) {
        mScriptContext->Release();
    }

    if (mScriptEngine != nullptr) {
        mScriptEngine->ShutDownAndRelease();
    }
}

void scriptMessageCallback(const asSMessageInfo *msg, void *param)
{
    ((void) param);

    std::stringstream stream;
    stream << "["
        << msg->section
        << ":"
        << msg->row
        << ":"
        << msg->col
        << "]";

    Logger::error() << stream.str() << msg->message;
}

bool Engine::ScriptCreator::create()
{
    assert(mScriptEngine == nullptr);
    mScriptEngine = asCreateScriptEngine();
    if (mScriptEngine->SetMessageCallback(asFUNCTION(scriptMessageCallback), nullptr, asCALL_CDECL) != 0) {
        Logger::error() << "Could not register message callback";
        return false;
    }

    RegisterStdString(mScriptEngine);
    RegisterScriptHandle(mScriptEngine);

    // Generic objects
    ScriptObject::registerCallback<ScriptCallback>(mScriptEngine);

    Character::registerClass(mScriptEngine);
    Clock::registerClass(mScriptEngine);
    Player::registerClass(mScriptEngine);
    Camera::registerClass(mScriptEngine);

    return true;
}

bool Engine::ScriptCreator::createContext()
{
    assert(mScriptContext == nullptr);
    mScriptContext = mScriptEngine->CreateContext();

    return true;
}

asIScriptEngine* Engine::ScriptCreator::engine()
{
    return mScriptEngine;
}

asIScriptContext* Engine::ScriptCreator::context()
{
    return mScriptContext;
}
