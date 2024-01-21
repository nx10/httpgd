#ifndef __UNIGD_OPTIONAL_LEX_H__
#define __UNIGD_OPTIONAL_LEX_H__

#include <compat/optional.hpp>
#include <cstring>
#include <string>

namespace httpgd
{

/**
 * @brief Convert a string to a std::experimental::optional<T>.
 *
 * @tparam T The type to convert to.
 * @param t_param The string to convert.
 * @return std::experimental::optional<T> The converted value.
 */
template <class T>
inline std::experimental::optional<T> param_to(const char* t_param)
{
  static_assert(sizeof(T) == 0, "Unsupported type");
  return std::experimental::nullopt;
}

// Specializations

template <>
inline std::experimental::optional<int> param_to(const char* t_param)
{
  if (t_param == nullptr)
  {
    return std::experimental::nullopt;
  }
  try
  {
    return std::stoi(t_param);
  }
  catch (const std::invalid_argument& ia)
  {
    return std::experimental::nullopt;
  }
  catch (const std::out_of_range& oor)
  {
    return std::experimental::nullopt;
  }
}

template <>
inline std::experimental::optional<unsigned int> param_to(const char* t_param)
{
  if (t_param == nullptr)
  {
    return std::experimental::nullopt;
  }
  try
  {
    return std::stoul(t_param);
  }
  catch (const std::invalid_argument& ia)
  {
    return std::experimental::nullopt;
  }
  catch (const std::out_of_range& oor)
  {
    return std::experimental::nullopt;
  }
}

template <>
inline std::experimental::optional<bool> param_to(const char* t_param)
{
  if (t_param == nullptr)
  {
    return std::experimental::nullopt;
  }
  if (strcmp(t_param, "true") == 0)
  {
    return true;
  }
  if (strcmp(t_param, "false") == 0)
  {
    return false;
  }
  return std::experimental::nullopt;
}

template <>
inline std::experimental::optional<double> param_to(const char* t_param)
{
  if (t_param == nullptr)
  {
    return std::experimental::nullopt;
  }
  try
  {
    return std::stod(t_param);
  }
  catch (const std::invalid_argument& ia)
  {
    return std::experimental::nullopt;
  }
  catch (const std::out_of_range& oor)
  {
    return std::experimental::nullopt;
  }
}

template <>
inline std::experimental::optional<float> param_to(const char* t_param)
{
  if (t_param == nullptr)
  {
    return std::experimental::nullopt;
  }
  try
  {
    return std::stof(t_param);
  }
  catch (const std::invalid_argument& ia)
  {
    return std::experimental::nullopt;
  }
  catch (const std::out_of_range& oor)
  {
    return std::experimental::nullopt;
  }
}

template <>
inline std::experimental::optional<long> param_to(const char* t_param)
{
  if (t_param == nullptr)
  {
    return std::experimental::nullopt;
  }
  try
  {
    return std::stol(t_param);
  }
  catch (const std::invalid_argument& ia)
  {
    return std::experimental::nullopt;
  }
  catch (const std::out_of_range& oor)
  {
    return std::experimental::nullopt;
  }
}

template <>
inline std::experimental::optional<unsigned long> param_to(const char* t_param)
{
  if (t_param == nullptr)
  {
    return std::experimental::nullopt;
  }
  try
  {
    return std::stoul(t_param);
  }
  catch (const std::invalid_argument& ia)
  {
    return std::experimental::nullopt;
  }
  catch (const std::out_of_range& oor)
  {
    return std::experimental::nullopt;
  }
}

template <>
inline std::experimental::optional<long long> param_to(const char* t_param)
{
  if (t_param == nullptr)
  {
    return std::experimental::nullopt;
  }
  try
  {
    return std::stoll(t_param);
  }
  catch (const std::invalid_argument& ia)
  {
    return std::experimental::nullopt;
  }
  catch (const std::out_of_range& oor)
  {
    return std::experimental::nullopt;
  }
}

template <>
inline std::experimental::optional<unsigned long long> param_to(const char* t_param)
{
  if (t_param == nullptr)
  {
    return std::experimental::nullopt;
  }
  try
  {
    return std::stoull(t_param);
  }
  catch (const std::invalid_argument& ia)
  {
    return std::experimental::nullopt;
  }
  catch (const std::out_of_range& oor)
  {
    return std::experimental::nullopt;
  }
}

template <>
inline std::experimental::optional<std::string> param_to(const char* t_param)
{
  if (t_param == nullptr)
  {
    return std::experimental::nullopt;
  }
  return std::string(t_param);
}

template <>
inline std::experimental::optional<const char*> param_to(const char* t_param)
{
  if (t_param == nullptr)
  {
    return std::experimental::nullopt;
  }
  return t_param;
}
}  // namespace httpgd

#endif /* __UNIGD_OPTIONAL_LEX_H__ */
