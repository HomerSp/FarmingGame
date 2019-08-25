#pragma once

#include <cstdint>
#include <unordered_map>

#include <QFocusEvent>
#include <QKeyEvent>
#include <QOpenGLExtraFunctions>
#include <QOpenGLWindow>
#include <QTimer>

#include <engine/engine.h>
#include <engine/keys.h>

#include <ui/qtrenderer.h>

class QtWindow : public QOpenGLWindow, protected QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    QtWindow();
    ~QtWindow();
 
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;

private slots:
    void process();

public:
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    std::unordered_map<int32_t, engine::Keys::Type> mKeys;

    std::shared_ptr<engine::Engine> mEngine;
    std::shared_ptr<QtRenderer> mRenderer;
    QTimer mIdleTimer;
};
