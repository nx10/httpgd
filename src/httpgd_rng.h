#ifndef __UNIGD_HTTPGD_RNG_H__
#define __UNIGD_HTTPGD_RNG_H__

#include <string>

namespace httpgd
{
// Can not use R's RNG for this for security reasons.
// (Seed could be predicted)
namespace rng
{
std::string uuid();
std::string token(int length);
}  // namespace rng
}  // namespace httpgd

#endif /* __UNIGD_HTTPGD_RNG_H__ */
