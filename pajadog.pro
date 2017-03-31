TEMPLATE = app
CONFIG += c++14
CONFIG -= app_bundle

INCLUDEPATH += ./include

include(dependencies/rapidjson.pri)

SOURCES += src/main.cpp \
    src/pajadog/setting.cpp \
    src/test/class1.cpp \
    src/test/class2.cpp

HEADERS += \
    include/pajadog/setting.hpp \
    include/test/class1.hpp \
    include/test/class2.hpp \
    include/libs/catch.hpp

DISTFILES += \
    test.json
