#include <QApplication>
#include <QCoreApplication>
#include <QDateTime>
#include <QSize>
#include <QSurfaceFormat>

#include <ui/qtwindow.h>

int main(int argc, char** argv)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setApplicationName("Farming");
    QCoreApplication::setOrganizationName("aqba");

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setDepthBufferSize(24);

    //format.setProfile(QSurfaceFormat::CoreProfile);
    //format.setVersion(3, 3);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);

    QtWindow window;
    window.setFormat(format);
    window.resize(QSize(1280, 720));
    window.show();

    return QApplication::exec();
}
