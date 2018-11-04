#include <string>

#include <QApplication>
#include <QDateTime>
#include <QQmlApplicationEngine>

#include <ui/mapitemview.h>

int main(int argc, char** argv)
{
    qsrand(QDateTime::currentMSecsSinceEpoch() / 1000);

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setApplicationName("Farming");
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
