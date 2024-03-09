# Settings

My C++ Settings library

## Dependencies

- [pajlada/signals](https://github.com/pajlada/signals)
  Pulled in as a submodule
- [pajlada/serialize](https://github.com/pajlada/serialize)
  Pulled in as a submodule
- [RapidJSON](http://rapidjson.org/)
  Must be installed on the system

## Run tests

```sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ../tests
cmake --build
ctest
```

## Intended usage

Store settings in each relevant class (static and non-static)

```
// servermanager.hpp
class ServerManager {
    ...

    static pajlada::Settings::Setting serverPort;
}

// servermanager.cpp
pajlada::Settings::Setting ServerManager::serverPort(new SettingData("serverPort", 1337 /* default port */)->setDescription("Port that the server will be hosted on"));
```

OR

```
// server.hpp
class Server {
    ...

    Server(int index)
        : serverPort(new SettingData("serverPort", 1337 + index)->setParent(ServerManager::serverHolder))
    {

    }

    pajlada::Settings::Setting serverPort;
}
```

## Platform support

### Linux

| Version      | Compiler                      |
| ------------ | ----------------------------- |
| Ubuntu 20.04 | ❌ GCC 9 (default `g++`)      |
| Ubuntu 20.04 | ✅ GCC 10 (`g++-10`)          |
| Ubuntu 20.04 | ❌ Clang 10 (default `clang`) |
| Ubuntu 20.04 | ✅ Clang 11 (`clang-11`)      |
| Ubuntu 20.04 | ✅ Clang 12 (`clang-12`)      |
| Ubuntu 22.04 | ✅ GCC 11 (default `g++`)     |
| Ubuntu 22.04 | ✅ GCC 12 (`g++-12`)          |
| Ubuntu 22.04 | ✅ Clang 14 (default `clang`) |
| Ubuntu 22.04 | ✅ Clang 15 (`clang-15`)      |

### Windows

To be filled in

### macOS

To be filled in
