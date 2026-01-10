#pragma once

#include <array>

#include "channel.hpp"

namespace pajlada::test {

constexpr int NUM_CHANNELS = 5;

class ChannelManager
{
public:
    ChannelManager(const std::shared_ptr<Settings::SettingManager> &sm)
        : channels{{
              {0, sm},
              {1, sm},
              {2, sm},
              {3, sm},
              {4, sm},
          }}
    {
    }

    std::array<IndexedChannel, NUM_CHANNELS> channels;
};

}  // namespace pajlada::test
