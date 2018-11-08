#include <cmath>
#include <iomanip>
#include <sstream>
#include <thread>

#include <scripthandle/scripthandle.h>
#include <scriptstdstring/scriptstdstring.h>
#include <scriptbuilder/scriptbuilder.h>

#include <engine/engine.h>
#include <engine/logger.h>

using namespace engine;

Engine::Engine(uint32_t width, uint32_t height)
    : mHasFocus(true)
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
}

Engine::~Engine()
{
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
    bool needRepaint = true;

    mFrameTimer.start();

    if (mDownKeys.contains(engine::Keys::TestFastForward)) {
        mClock->fastForward(1.0f * (mFrameTimer.diff() / 50.0f));
    }

    mClock->process(mFrameTimer.diff());

    int8_t x = 0, y = 0;
    if (mHasFocus && !mDownKeys.empty()) {
        bool turned = false;
        for (auto it = mDownKeys.rbegin(); it != mDownKeys.rend(); it++) {
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

        if (mDownKeys.contains(engine::Keys::TestFriction)) {
            mHero->setFriction(0.1f);
        } else {
            mHero->setFriction(1.0f);
        }

        if (mDownKeys.contains(engine::Keys::Run)) {
            mHero->setSpeed(2.0f);
        } else if (mDownKeys.contains(engine::Keys::Walk)) {
            mHero->setSpeed(0.5f);
        } else {
            mHero->setSpeed(1.0f);
        }
    } else {
        mHero->setSpeed(1.0f);
        mHero->reset();
    }

    mHero->velocity(mFrameTimer.diff(), x, y);
    mHero->process(mFrameTimer.diff(), mMap.get());

    if (!mHero->isMoving()) {
        mHero->reset();
    }

    if (mHero->isMoving()) {
        mHero->animate(mFrameTimer.elapsed());
    }

    mMap->animate(mFrameTimer.elapsed());

    mCamera->process(mFrameTimer.diff(), mMap.get());

    if (mDownKeys.contains(engine::Keys::TestSlowMode)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    mFrameTimer.end();

    return needRepaint;
}

void Engine::paint(Renderer& renderer)
{
    renderer.fillRect(Types::Rect(0, 0, mWidth, mHeight), Types::Color(0, 0, 0));

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

    Types::Rect dst(mCamera->x(), mCamera->y(), mWidth, mHeight);
    mMap->draw(renderer, dst);

    Types::Dimension d = mMap->getTileDimension();

    int startY = std::ceil(mCamera->y() / d.height);
    bool drawnHero = false;
    for (int row = startY - 1; row <= startY + std::ceil(mHeight / d.height); row++) {
        if (!drawnHero && row * d.height >= mHero->y()) {
            mHero->draw(renderer, Types::Point(mCamera->x(), mCamera->y()));
            drawnHero = true;
        }

        mMap->drawRow(renderer, dst, row, TilesetAttribute::AboveRow);
    }

    if (!drawnHero) {
        mHero->draw(renderer, Types::Point(mCamera->x(), mCamera->y()));
    }

    mClock->draw(renderer);

    std::stringstream str;
    str << std::setw(2) << std::setfill('0') << mClock->hour() << ":" << std::setw(2) << std::setfill('0') << mClock->minute();
    renderer.drawText({-10, 10}, str.str(), {0, 0, 0}, 24, Types::TextAlign({Types::TextAlign::Right}));
}

void Engine::setKeyMap(const std::unordered_map<int, Keys::Type>& keys)
{
    for (auto key : keys) {
        mDownKeys[key.first] = key.second;
    }
}

void Engine::setKeyDown(int key)
{
    mDownKeys.append(key);
}

void Engine::setKeyUp(int key)
{
    mDownKeys.remove(key);
}

void Engine::setSize(int width, int height)
{
    mWidth = width;
    mHeight = height;
    mCamera->setViewport(Types::Dimension(mWidth, mHeight));
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
    if (mScriptEngine->SetMessageCallback(asFUNCTION(scriptMessageCallback), 0, asCALL_CDECL)) {
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
    if (builder.StartNewModule(mScriptEngine, "MainModule")) {
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
    if (!func) {
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
