#include <fstream>
#include <string>
#include <unordered_map>

// Env key value pair reader
std::unordered_map<std::string, std::string>
read_env(const std::string &path = "") {
  // read file
  std::ifstream file(path);

  // create storage
  std::unordered_map<std::string, std::string> env;

  // temp placeholder for the lines
  std::string line;

  while (std::getline(file, line)) {
    // seeking = sign
    auto pos = line.find('=');

    // if no = skip
    if (pos == std::string::npos)
      continue;

    // divide into key and value
    auto key = line.substr(0, pos);
    auto value = line.substr(pos + 1);

    env[key] = value; // store the key-value pair
  }

  return env;
}
