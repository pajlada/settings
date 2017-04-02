TEMPLATE = app
CONFIG += c++14
CONFIG -= app_bundle

INCLUDEPATH += ./include

include(dependencies/rapidjson.pri)

SOURCES += src/main.cpp \
    src/pajadog/setting.cpp \
    src/test/foo.cpp

HEADERS += \
    include/pajadog/setting.hpp \
    include/libs/catch.hpp \
    include/test/foo.hpp

DISTFILES += \
    test.json
