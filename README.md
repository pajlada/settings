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
