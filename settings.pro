TEMPLATE = app
CONFIG += c++14 console
CONFIG -= qt app_bundle

INCLUDEPATH += ./include

# DEFINES += ONLY_MINI_TEST

include(dependencies/rapidjson.pri)
include(dependencies/signals.pri)

SOURCES += src/main.cpp \
    src/test/foo.cpp \
    src/test/channel.cpp \
    src/settings/settingdata.cpp \
    src/settings/settingmanager.cpp \
    src/settings/jsonwrapper.cpp

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
    include/pajlada/settings/jsonwrapper.hpp
