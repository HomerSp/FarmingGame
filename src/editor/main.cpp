#include <QApplication>
#include <QQmlApplicationEngine>

#include <ui/mapitemview.h>

int main(int argc, char** argv)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setApplicationName("FarmingEditor");
    QCoreApplication::setOrganizationName("aqba");
    QApplication app(argc, argv);

    qmlRegisterType<MapItemView>("se.aqba.qt.farming", 1, 0, "MapItemView");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return QApplication::exec();
}
