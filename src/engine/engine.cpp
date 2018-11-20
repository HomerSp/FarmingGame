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

Engine::Engine(uint32_t width, uint32_t height, std::shared_ptr<Renderer> renderer)
    : mRunning(true)
    , mNeedRepaint(false)
    , mHasFocus(true)
    , mWidth(width)
    , mHeight(height)
    , mRenderer(renderer)
    , mMap(nullptr)
    , mPlayer(nullptr)
    , mEnableThreading(true)
{
    Logger::debug() << "Creating Engine";

    AssetManager::get()->setRenderer(mRenderer);

    mScript = std::make_shared<Engine::ScriptCreator>();

    mScreenEffects = std::make_shared<engine::ScreenEffects>();
    mCamera = std::make_shared<engine::Camera>(mWidth, mHeight);
    mClock = std::make_shared<engine::Clock>();
    mMap = std::make_shared<engine::Map>("map");
    mPlayer = std::make_shared<engine::Player>();
    mPlayer->setX(std::floor((mMap->pixelWidth() - mPlayer->width()) / 2));
    mPlayer->setY(std::floor((mMap->pixelHeight() - mPlayer->height()) / 2));

    std::random_device r;
    std::default_random_engine gen(r());
    std::uniform_int_distribution<> dis(0, static_cast<int>(Character::Direction::Up));
    for(uint32_t i = 0; i < 24 * 10; i++) {
        mCharacters.push_back(std::make_shared<engine::Character>("dude"));
        mCharacters.back()->setX((8 * 48) + ((i % 24) * 48));
        mCharacters.back()->setY(48 + (std::floor(i / 24) * 48));
        mCharacters.back()->setDirection(static_cast<Character::Direction::Type>(dis(gen)));
    }

    mCharacters.push_back(std::make_shared<engine::Character>("horse"));
    mCharacters.back()->setX(48);
    mCharacters.back()->setY(48);
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

int Engine::bufferWidth() const
{
    return mMap->getTileDimension().width * 2;
}

int Engine::bufferHeight() const
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
    FrameTimer frameTimer;
    while (mRunning) {
        uint64_t diff = frameTimer.diff();

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
                i->animate(diff, false);
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
    FrameTimer frameTimer;
    while (mRunning) {
        uint64_t diff = frameTimer.diff();

        engine::KeyList downKeys;
        {
            std::lock_guard<std::mutex> lock(mDownKeysMutex);
            downKeys = mDownKeys;
        }

        if (downKeys.contains(Keys::TestPause)) {
            if (!mEnableThreading) {
                break;
            }

            std::this_thread::yield();
            continue;
        }

        if (downKeys.contains(engine::Keys::TestFastForward)) {
            if (downKeys.contains(engine::Keys::Run)) {
                mClock->fastForward(diff * 0.5f);
            } else {
                mClock->fastForward(diff * 0.05f);
            }
        }

        if (downKeys.contains(engine::Keys::TestSlowMode)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        float x = 0, y = 0;
        if (mHasFocus && !downKeys.empty()) {
            bool turned = false;
            for (auto it = downKeys.rbegin(); it != downKeys.rend(); it++) {
                switch (*it) {
                case engine::Keys::Up:
                    if (y == 0) {
                        y = -1;
                    }
                    if (!turned) {
                        mPlayer->turnTo(engine::Character::Direction::Up);
                        turned = true;
                    }
                    break;
                case engine::Keys::Down:
                    if (y == 0) {
                        y = 1;
                    }
                    if (!turned) {
                        mPlayer->turnTo(engine::Character::Direction::Down);
                        turned = true;
                    }
                    break;
                case engine::Keys::Left:
                    if (x == 0) {
                        x = -1;
                    }
                    if (!turned) {
                        mPlayer->turnTo(engine::Character::Direction::Left);
                        turned = true;
                    }
                    break;
                case engine::Keys::Right:
                    if (x == 0) {
                        x = 1;
                    }
                    if (!turned) {
                        mPlayer->turnTo(engine::Character::Direction::Right);
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

            if (downKeys.contains(engine::Keys::TestFriction)) {
                mPlayer->setFriction(0.1f);
            } else {
                mPlayer->setFriction(1.0f);
            }

            if (downKeys.contains(engine::Keys::Run)) {
                mPlayer->setSpeed(2.0f);
            } else if (downKeys.contains(engine::Keys::Walk)) {
                mPlayer->setSpeed(0.5f);
            } else {
                mPlayer->setSpeed(1.0f);
            }
        } else {
            mPlayer->setSpeed(1.0f);
        }

        mPlayer->velocity(diff, x, y);

        // Process movement, etc
        if (mCamera->processAsync(diff, mMap.get())) {
            mNeedRepaint = true;
        }
        if (mClock->processAsync(diff, mMap.get())) {
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
    float translateX = 0.0f, translateY = 0.0f;
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
    int startY = std::ceil(mCamera->y() / d.height);

    std::multimap<int, Character*> drawCharacters;
    for (auto& i: mCharacters) {
        if (mCamera->contains(*i, d)) {
            drawCharacters.emplace(std::make_pair(i->y(), i.get()));
        }
    }

    drawCharacters.emplace(std::make_pair(mPlayer->y(), mPlayer.get()));

    for (int row = startY - 1; row <= startY + std::ceil(mHeight / d.height) + 1; row++) {
        auto it = drawCharacters.begin();
        while (it != drawCharacters.end()) {
            if (row * d.height >= it->first) {
                it->second->draw(renderer, Types::Point<>(mCamera->x(), mCamera->y()));
                it = drawCharacters.erase(it);
            } else {
                it++;
            }
        }

        mMap->drawRow(renderer, dst, row, TilesetAttribute::AboveRow);
    }

    for (int row = startY - 1; row <= startY + std::ceil(mHeight / d.height); row++) {
        mMap->drawRow(renderer, dst, row, TilesetAttribute::AboveAll);
    }

    mScreenEffects->draw(renderer, *mClock.get(), *mCamera.get(), mLights);

    std::stringstream str;
    str << std::setw(2) << std::setfill('0') << mClock->hour() << ":" << std::setw(2) << std::setfill('0') << mClock->minute();
    renderer.drawText({-10, 10}, str.str(), {0, 0, 0}, 24, Types::TextAlign({Types::TextAlign::Right}));

    mNeedRepaint = false;
}

void Engine::setKeyMap(const std::unordered_map<int, Keys::Type>& keys)
{
    std::lock_guard<std::mutex> lock(mDownKeysMutex);
    for (auto key : keys) {
        mDownKeys[key.first] = key.second;
    }
}

void Engine::setKeyDown(int key)
{
    std::lock_guard<std::mutex> lock(mDownKeysMutex);
    mDownKeys.append(key);
}

void Engine::setKeyUp(int key)
{
    std::lock_guard<std::mutex> lock(mDownKeysMutex);
    mDownKeys.remove(key);
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

    int r = mScript->context()->Execute();
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
    registerInstance<Engine>(mScript->engine(), "engine", this);
}

void Engine::registerContext()
{
    mCamera->setContext(mScript->context());
    mClock->setContext(mScript->context());
    mPlayer->setContext(mScript->context());
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
