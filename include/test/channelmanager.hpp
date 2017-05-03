#pragma once

#include "test/channel.hpp"

#include <array>

namespace pajlada {
namespace test {

constexpr int NUM_CHANNELS = 5;

class ChannelManager
{
public:
    ChannelManager()
        : root("channelsArray")
        , channels{{
              {0, this->root},   //
              {1, this->root},   //
              {2, this->root},   //
              {3, this->root},   //
              {4, this->root}/*,   //
              {5, this->root},   //
              {6, this->root},   //
              {7, this->root},   //
              {8, this->root},   //
              {9, this->root},   //
              {10, this->root},  //
              {11, this->root},  //
              {12, this->root},  //
              {13, this->root},  //
              {14, this->root},  //
              {15, this->root},  //
              {16, this->root},  //
              {17, this->root},  //
              {18, this->root},  //
              {19, this->root},  //
              {20, this->root},  //
              {21, this->root},  //
              {22, this->root},  //
              {23, this->root},  //
              {24, this->root},  //
              {25, this->root},  //
              {26, this->root},  //
              {27, this->root},  //
              {28, this->root},  //
              {29, this->root},  //
              {30, this->root},  //
              {31, this->root},  //
              {32, this->root},  //
              {33, this->root},  //
              {34, this->root},  //
              {35, this->root},  //
              {36, this->root},  //
              {37, this->root},  //
              {38, this->root},  //
              {39, this->root},  //
              {40, this->root},  //
              {41, this->root},  //
              {42, this->root},  //
              {43, this->root},  //
              {44, this->root},  //
              {45, this->root},  //
              {46, this->root},  //
              {47, this->root},  //
              {48, this->root},  //
              {49, this->root}   //
                             */
          }}
    {
    }

    Settings::Setting<Settings::Array> root;
    std::array<IndexedChannel, NUM_CHANNELS> channels;
};

}  // namespace test
}  // namespace pajlada
