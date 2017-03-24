# pajadog
test settings lib

## Intended usage
Store settings in each relevant class (static and non-static)

```
// servermanager.hpp
class ServerManager {
    ...

    static pajadog::Setting serverPort;
}

// servermanager.cpp
pajadog::Setting ServerManager::serverPort(new SettingData("serverPort", 1337 /* default port */)->setDescription("Port that the server will be hosted on"));
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

    pajadog::Setting serverPort;
}
```
