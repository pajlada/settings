#include <pajlada/settings/setting.hpp>

#include "pajlada/signals/signalholder.hpp"

namespace pajlada::Settings::detail {

template <>
void
connectToManager<Signals::ScopedConnection>(Signals::ScopedConnection &manager,
                                            Signals::Connection &&connection)
{
    manager = std::move(connection);
}

template <>
void
connectToManager<Signals::SignalHolder>(Signals::SignalHolder &manager,
                                        Signals::Connection &&connection)
{
    manager.addConnection(std::move(connection));
}

}  // namespace pajlada::Settings::detail
