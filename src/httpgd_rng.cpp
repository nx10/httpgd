
#include "httpgd_rng.h"

#include <chrono>
#include <random>

namespace httpgd
{
namespace rng
{
std::string uuid()
{
  const int uuidLength = 36;  // Including hyphens
  std::string uuid;
  uuid.reserve(uuidLength);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 15);

  auto randomByte = [&]()
  {
    const char hexChars[] = "0123456789abcdef";
    return hexChars[dis(gen)];
  };

  for (int i = 0; i < 32; ++i)
  {
    if (i == 8 || i == 12 || i == 16 || i == 20)
    {
      uuid += '-';
    }
    uuid += randomByte();
  }

  return uuid;
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
