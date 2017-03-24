TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle

INCLUDEPATH += ./include

SOURCES += src/main.cpp \
    src/pajadog/settingsmanager.cpp \
    src/pajadog/setting.cpp \
    src/test/class1.cpp \
    src/test/class2.cpp

HEADERS += \
    include/pajadog/settingsmanager.hpp \
    include/pajadog/setting.hpp \
    include/test/class1.hpp \
    include/test/class2.hpp


win32 {
    LIBS += -LC:/local/boost/lib64-msvc-14.0
    DEFINES += BOOST_USE_WINDOWS_H
    INCLUDEPATH += C:/local/boost
}
