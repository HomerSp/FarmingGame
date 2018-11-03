TEMPLATE = lib
QT += 
CONFIG += c++11 staticlib link_pkgconfig debug

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

PKGCONFIG += jsoncpp libpng

SOURCES += types.cpp \
	engine.cpp \
	debug.cpp \
	frametimer.cpp \
	physics.cpp \
	image.cpp \
	renderer.cpp \
	assetmanager.cpp \
	character.cpp \
    keys.cpp \
    map.cpp \
    charset.cpp \
    tileset.cpp

RESOURCES += 

INCLUDEPATH += include

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += include/engine/types.h \
	include/engine/engine.h \
	include/engine/debug.h \
	include/engine/frametimer.h \
	include/engine/physics.h \
	include/engine/image.h \
	include/engine/renderer.h \
	include/engine/assetmanager.h \
	include/engine/character.h \
	include/engine/keys.h \
	include/engine/map.h \
	include/engine/charset.h \
	include/engine/tileset.h
