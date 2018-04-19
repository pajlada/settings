TEMPLATE = app
CONFIG += c++14 console
CONFIG -= qt app_bundle

linux {
    QMAKE_CXXFLAGS += --std=c++14
}

INCLUDEPATH += ./include

# DEFINES += ONLY_MINI_TEST

include(dependencies/rapidjson.pri)
include(dependencies/signals.pri)

SOURCES += src/main.cpp \
    src/test/foo.cpp \
    src/test/channel.cpp \
    src/settings/settingdata.cpp \
    src/settings/settingmanager.cpp \
    src/catchimplementor.cpp

HEADERS += \
    include/pajlada/settings/setting.hpp \
    include/libs/catch.hpp \
    include/test/foo.hpp \
    include/test/channelmanager.hpp \
    include/test/channel.hpp \
    include/pajlada/settings/settingdata.hpp \
    include/pajlada/settings/settingmanager.hpp \
    include/pajlada/settings/types.hpp \
    include/pajlada/settings/types.hpp \
    include/pajlada/settings/merger.hpp

win32-msvc* {
    QMAKE_CXXFLAGS_WARN_ON = /W4 /wd4996
    QMAKE_CXXFLAGS_WARN_ON += /wd4996 # Ignore "unsafe" warnings
}

linux {
    QMAKE_CXXFLAGS_WARN_ON = -Wall
}


win32 {
    LIBS += -LC:/local/boost_1_62_0/lib64-msvc-14.0
    DEFINES += BOOST_USE_WINDOWS_H
    INCLUDEPATH += C:/local/boost_1_62_0
}
