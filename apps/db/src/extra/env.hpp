#ifndef ENV
#define ENV

#include <string>
#include <unordered_map>

std::unordered_map<std::string, std::string>
read_env(const std::string &path = "");

#endif
