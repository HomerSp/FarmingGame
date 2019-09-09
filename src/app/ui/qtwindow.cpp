#include <ui/qtwindow.h>

QtWindow::QtWindow()
{
    mKeys[Qt::Key_Up] = engine::Keys::Up;
    mKeys[Qt::Key_Down] = engine::Keys::Down;
    mKeys[Qt::Key_Left] = engine::Keys::Left;
    mKeys[Qt::Key_Right] = engine::Keys::Right;
    mKeys[Qt::Key_Shift] = engine::Keys::Run;
    mKeys[Qt::Key_E] = engine::Keys::Use;
    mKeys[Qt::Key_Q] = engine::Keys::ExpandHudItems;
    mKeys[Qt::Key_P] = engine::Keys::TestPause;
    mKeys[Qt::Key_F] = engine::Keys::TestFriction;
    mKeys[Qt::Key_S] = engine::Keys::TestSlowMode;
    mKeys[Qt::Key_T] = engine::Keys::TestFastForward;
    mKeys[Qt::Key_P] = engine::Keys::TestPause;
}

QtWindow::~QtWindow()
{
    makeCurrent();
}

void QtWindow::initializeGL()
{
    initializeOpenGLFunctions();

    mRenderer = std::make_shared<QtRenderer>();
    mRenderer->setSize(width(), height(), devicePixelRatio());
    mEngine = std::make_shared<engine::Engine>(width(), height(), mRenderer);
    mEngine->setKeyMap(mKeys);

    connect(&mIdleTimer, &QTimer::timeout, this, &QtWindow::process);
    mIdleTimer.start(0);
}

void QtWindow::resizeGL(int width, int height)
{
    mEngine->setSize(width, height);
    mRenderer->setSize(width, height, devicePixelRatio());
}

void QtWindow::paintGL()
{
    glViewport(0, 0, width(), height());

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    mRenderer->paint(mEngine);
}

void QtWindow::process()
{
    mEngine->process();
    update();
}

void QtWindow::focusInEvent(QFocusEvent* event)
{
    Q_UNUSED(event);
    if (mEngine) {
        mEngine->setFocus(true);
    }
}

void QtWindow::focusOutEvent(QFocusEvent* event)
{
    Q_UNUSED(event);
    if (mEngine) {
        mEngine->setFocus(false);
    }
}

void QtWindow::keyPressEvent(QKeyEvent* event)
{
    if (mEngine && !event->isAutoRepeat()) {
        mEngine->setKeyDown(event->key());
    }
}

void QtWindow::keyReleaseEvent(QKeyEvent* event)
{
    if (mEngine && !event->isAutoRepeat()) {
        mEngine->setKeyUp(event->key());
    }
}
