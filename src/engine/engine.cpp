#include <cmath>
#include <iomanip>
#include <sstream>
#include <thread>

#include <scriptbuilder/scriptbuilder.h>
#include <scripthandle/scripthandle.h>
#include <scriptstdstring/scriptstdstring.h>

#include <engine/engine.h>
#include <engine/logger.h>

using namespace engine;

Engine::Engine(uint32_t width, uint32_t height)
    : mRunning(true)
    , mNeedRepaint(false)
    , mHasFocus(true)
    , mWidth(width)
    , mHeight(height)
    , mMap(nullptr)
    , mHero(nullptr)
    , mScriptEngine(nullptr)
    , mScriptContext(nullptr)
{
    Logger::debug() << "Creating Engine";

    mCamera = std::make_shared<engine::Camera>(mWidth, mHeight);
    mClock = std::make_shared<engine::Clock>();
    mMap = std::make_shared<engine::Map>("map");
    mHero = std::make_shared<engine::Character>("hero");
    mHero->setX(std::floor((mMap->pixelWidth() - mHero->width()) / 2));
    mHero->setY(std::floor((mMap->pixelHeight() - mHero->height()) / 2));

    mCamera->setTarget(mHero.get());
    mClock->setTime(8, 0);

    registerScript();

    mProcessThread = std::make_unique<std::thread>(&Engine::processAsync, this);
}

Engine::~Engine()
{
    mRunning = false;
    mProcessThread->join();

    Logger::debug() << "~Engine";
    if(mScriptContext != nullptr) {
        mScriptContext->Release();
    }

    mCamera.reset();
    mClock.reset();
    mHero.reset();

    if(mScriptEngine != nullptr) {
        mScriptEngine->ShutDownAndRelease();
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
    mCamera->processListeners();
    mClock->processListeners();
    mHero->processListeners();

    return mNeedRepaint;
}

void Engine::processAsync()
{
    FrameTimer frameTimer;
    while (mRunning) {
        engine::KeyList downKeys;
        {
            std::lock_guard<std::mutex> lock(mDownKeysMutex);
            downKeys = mDownKeys;
        }

        if (downKeys.contains(Keys::TestPause)) {
            std::this_thread::yield();
            frameTimer.reset();
            continue;
        }

        if (downKeys.contains(engine::Keys::TestFastForward)) {
            mClock->fastForward(50.0f * frameTimer[0]);
        }

        if (downKeys.contains(engine::Keys::TestSlowMode)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
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
                        mHero->turnTo(engine::Character::Direction::Up);
                        turned = true;
                    }
                    break;
                case engine::Keys::Down:
                    if (y == 0) {
                        y = 1;
                    }
                    if (!turned) {
                        mHero->turnTo(engine::Character::Direction::Down);
                        turned = true;
                    }
                    break;
                case engine::Keys::Left:
                    if (x == 0) {
                        x = -1;
                    }
                    if (!turned) {
                        mHero->turnTo(engine::Character::Direction::Left);
                        turned = true;
                    }
                    break;
                case engine::Keys::Right:
                    if (x == 0) {
                        x = 1;
                    }
                    if (!turned) {
                        mHero->turnTo(engine::Character::Direction::Right);
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
                mHero->setFriction(0.1f);
            } else {
                mHero->setFriction(1.0f);
            }

            if (downKeys.contains(engine::Keys::Run)) {
                mHero->setSpeed(2.0f);
            } else if (downKeys.contains(engine::Keys::Walk)) {
                mHero->setSpeed(0.5f);
            } else {
                mHero->setSpeed(1.0f);
            }
        } else {
            mHero->setSpeed(1.0f);
        }

        mHero->velocity(frameTimer[1], x, y);

        // Process movement, etc
        if (mCamera->processAsync(frameTimer[2], mMap.get())) {
            mNeedRepaint = true;
        }
        if (mClock->processAsync(frameTimer[3])) {
            mNeedRepaint = true;
        }
        if (mHero->processAsync(frameTimer[4], mMap.get())) {
            mNeedRepaint = true;
        }

        // Process animations
        if (mHero->animate(frameTimer[5], !mHero->isMoving())) {
            mNeedRepaint = true;
        }

        if (mMap->animate(frameTimer[6])) {
            mNeedRepaint = true;
        }

        std::this_thread::yield();
    }
}

void Engine::paint(Renderer& renderer)
{
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
    bool drawnHero = false;
    for (int row = startY - 1; row <= startY + std::ceil(mHeight / d.height); row++) {
        if (!drawnHero && row * d.height >= mHero->y()) {
            mHero->draw(renderer, Types::Point<>(mCamera->x(), mCamera->y()));
            drawnHero = true;
        }

        mMap->drawRow(renderer, dst, row, TilesetAttribute::AboveRow);
    }

    if (!drawnHero) {
        mHero->draw(renderer, Types::Point<>(mCamera->x(), mCamera->y()));
    }

    mClock->draw(renderer);

    std::stringstream str;
    str << std::setw(2) << std::setfill('0') << mClock->hour() << ":" << std::setw(2) << std::setfill('0') << mClock->minute();
    renderer.drawText({-10, 10}, str.str(), {0, 0, 0}, 24, Types::TextAlign({Types::TextAlign::Right}));
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

bool Engine::registerScript()
{
    FunctionPtrHelper::init();

    // Create the script engine
    mScriptEngine = asCreateScriptEngine();
    if (mScriptEngine->SetMessageCallback(asFUNCTION(scriptMessageCallback), nullptr, asCALL_CDECL) != 0) {
        Logger::error() << "Could not register message callback";
        return false;
    }

    RegisterStdString(mScriptEngine);
    RegisterScriptHandle(mScriptEngine);

    registerGeneric();
    mClock->registerClass(mScriptEngine);
    mHero->registerClass(mScriptEngine);
    mCamera->registerClass(mScriptEngine);

    // This needs to be done after all other types have been registered.
    registerClass();

    mScriptEngine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(Logger::scriptPrint), asCALL_CDECL);

    // The CScriptBuilder helper is an add-on that loads the file,
    // performs a pre-processing pass if necessary, and then tells
    // the engine to build a script module.
    CScriptBuilder builder;
    if (builder.StartNewModule(mScriptEngine, "MainModule") != 0) {
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
    asIScriptModule *mod = mScriptEngine->GetModule("MainModule");
    asIScriptFunction *func = mod->GetFunctionByDecl("void main()");
    if (func == nullptr) {
        Logger::error() << ("The script must have the function 'void main()'. Please add it and try again.");
        return false;
    }

    // Create our context, prepare it, and then execute
    mScriptContext = mScriptEngine->CreateContext();
    mScriptContext->Prepare(func);
    registerContext();

    int r = mScriptContext->Execute();
    if (r == asEXECUTION_EXCEPTION) {
        Logger::error() << "An exception" << mScriptContext->GetExceptionString() << "occurred. Please correct the code and try again.";
        return false;
    }

    return (r == asEXECUTION_FINISHED);
}

std::string Engine::className()
{
    return "Engine";
}

void Engine::registerGeneric()
{
    ScriptObject::registerCallback<ScriptCallback>(mScriptEngine);
}

void Engine::registerClass()
{
    registerReference<Engine>(mScriptEngine);
    REGISTER_FUNC(mScriptEngine, Engine, Camera&, camera);
    REGISTER_FUNC(mScriptEngine, Engine, Clock&, clock);
    REGISTER_FUNC(mScriptEngine, Engine, Character&, hero);
    registerInstance<Engine>(mScriptEngine, "engine", this);
}

void Engine::registerContext()
{
    mCamera->setContext(mScriptContext);
    mClock->setContext(mScriptContext);
    mHero->setContext(mScriptContext);
}

engine::Camera* Engine::camera()
{
    return mCamera.get();
}

engine::Clock* Engine::clock()
{
    return mClock.get();
}

engine::Character* Engine::hero()
{
    return mHero.get();
}
