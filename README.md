# Settings

My C++ Settings library

## Dependencies

- [pajlada/signals](https://github.com/pajlada/signals)
  Pulled in as a submodule
- [pajlada/serialize](https://github.com/pajlada/serialize)
  Pulled in as a submodule
- [RapidJSON](http://rapidjson.org/)
  Must be installed on the system

## Run project with tests

```sh
mkdir build
cd build
cmake -DPAJLADA_SETTINGS_BUILD_TESTS=On ..
make -j
./settings-test
```

Available flags that can be passed to cmake:

- PAJLADA_SETTINGS_BUILD_TESTS `On` or `Off` (default `Off`)
  This option will instruct cmake to build the tests executable `settings-test`.

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
