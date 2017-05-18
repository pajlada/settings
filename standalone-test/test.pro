TEMPLATE = app
CONFIG += c++14 console
CONFIG -= qt app_bundle

linux {
    QMAKE_CXXFLAGS += --std=c++14
}

INCLUDEPATH += ../include

# DEFINES += ONLY_MINI_TEST

include(../dependencies/rapidjson.pri)
include(../dependencies/signals.pri)

SOURCES += ../src/test1.cpp \
    ../src/settings/settingdata.cpp \
    ../src/settings/settingmanager.cpp

HEADERS += \
    include/pajlada/settings/setting.hpp \
    include/pajlada/settings/settingdata.hpp \
    include/pajlada/settings/settingmanager.hpp \
    include/pajlada/settings/types.hpp \
    include/pajlada/settings/types.hpp

win32-msvc* {
    QMAKE_CXXFLAGS_WARN_ON = /W4 /wd4996
    QMAKE_CXXFLAGS_WARN_ON += /wd4996 # Ignore "unsafe" warnings
}

linux {
    QMAKE_CXXFLAGS_WARN_ON = -Wall
}

# Add sanitizer flags
asan {
    # Address Sanitizer
    QMAKE_CXXFLAGS += -g -fsanitize=address -fPIE -fno-omit-frame-pointer -O1
    QMAKE_CFLAGS   += -g -fsanitize=address -fPIE -fno-omit-frame-pointer -O1
    QMAKE_LFLAGS += -fsanitize=address -pie

    message("Address Sanitizer enabled")
}

msan {
    # Memory Sanitizer
    QMAKE_CXXFLAGS += -g -fsanitize=memory -fPIE -fsanitize-memory-track-origins -fno-omit-frame-pointer -O2
    QMAKE_CFLAGS   += -g -fsanitize=memory -fPIE -fsanitize-memory-track-origins -fno-omit-frame-pointer -O2
    QMAKE_LFLAGS += -fsanitize=memory -pie

    message("Memory Sanitizer enabled")
}

tsan {
    # Thread Sanitizer
    QMAKE_CXXFLAGS += -g -fsanitize=thread -fPIE -fno-omit-frame-pointer -O1
    QMAKE_CFLAGS   += -g -fsanitize=thread -fPIE -fno-omit-frame-pointer -O1
    QMAKE_LFLAGS += -fsanitize=thread -pie

    message("Thread Sanitizer enabled")
}
