#pragma once

#include <string>

namespace httpgd
{
    // Can not use R's RNG for this for security reasons.
    // (Seed could be predicted)
    namespace rng
    {
        std::string uuid();
        std::string token(int length);
    } // namespace rng
} // namespace httpgd
