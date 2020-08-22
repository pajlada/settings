# Settings

My C++ Settings library

## Dependencies

- [pajlada/signals](https://github.com/pajlada/signals)
  Pulled in as a submodule
- [pajlada/serialize](https://github.com/pajlada/serialize)
  Pulled in as a submodule
- [RapidJSON](http://rapidjson.org/)
  Must be installed on the system
- [Boost](https://www.boost.org/)
  Must be installed on the system

## Run project with tests

1. mkdir build
2. cd build
3. cmake ..
4. make -j
5. ./settings

Available flags that can be passed to cmake: USE_BOOST_FILESYSTEM - this will decide whether or not to use boost-filesystem or c++'s stdlib filesystem

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
