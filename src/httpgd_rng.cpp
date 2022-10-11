
#include "httpgd_rng.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <random>

namespace httpgd
{
namespace rng
{
std::string uuid()
{
  boost::uuids::random_generator uuid_gen;
  return boost::uuids::to_string(uuid_gen());
}

std::string token(int length)
{
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  static auto rseed = static_cast<long unsigned int>(
      std::chrono::high_resolution_clock::now().time_since_epoch().count());
  static std::mt19937 generator(rseed);
  static std::uniform_int_distribution<int> distribution{
      0, static_cast<int>((sizeof(alphanum) / sizeof(alphanum[0])) - 2)};

  std::string rand_str(length, '\0');
  for (int i = 0; i < length; ++i)
  {
    rand_str[i] = alphanum[distribution(generator)];
  }

  return rand_str;
}
}  // namespace rng
}  // namespace httpgd