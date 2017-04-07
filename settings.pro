TEMPLATE = app
CONFIG += c++14 console
CONFIG -= qt app_bundle

INCLUDEPATH += ./include

include(dependencies/rapidjson.pri)

SOURCES += src/main.cpp \
    src/settings/setting.cpp \
    src/test/foo.cpp \
    src/test/channel.cpp

HEADERS += \
    include/pajlada/settings/setting.hpp \
    include/libs/catch.hpp \
    include/test/foo.hpp \
    include/test/channelmanager.hpp \
    include/test/channel.hpp
