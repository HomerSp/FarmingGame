#include <string>

#include <QApplication>
#include <QDateTime>
#include <QQmlApplicationEngine>
#include <QSurfaceFormat>

#include <ui/qtwindow.h>

int main(int argc, char** argv)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setApplicationName("Farming");
    QCoreApplication::setOrganizationName("aqba");

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    //format.setProfile(QSurfaceFormat::CoreProfile);
    //format.setVersion(3, 3);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);

    QtWindow window;
    window.setFormat(format);
    window.resize(QSize(1280, 720));
    window.show();

    /*qmlRegisterType<MapItemView>("se.aqba.qt.farming", 1, 0, "MapItemView");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }*/

    return QApplication::exec();
}
