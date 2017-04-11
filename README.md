# Settings
My C++ Settings library

## Requirements
pajlada/signals: https://github.com/pajlada/signals

## Run project with tests
1. `git submodule update --init`
2. `qmake settings.pro`
3. `make`

## Intended usage
Store settings in each relevant class (static and non-static)

```
// servermanager.hpp
class ServerManager {
    ...

    static pajlada::settings::Setting serverPort;
}

// servermanager.cpp
pajlada::settings::Setting ServerManager::serverPort(new SettingData("serverPort", 1337 /* default port */)->setDescription("Port that the server will be hosted on"));
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

    pajlada::settings::Setting serverPort;
}
```
