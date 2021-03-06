//
// Created by amourao on 26-06-2019.
//

#include "utils.h"

uint64_t CurrentTimeMs() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch())
      .count();
}

std::string RandomString(size_t length) {
  auto randchar = []() -> char {
    const char charset[] = "0123456789"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[rand() % max_index];
  };
  std::string str(length, 0);
  std::generate_n(str.begin(), length, randchar);
  return str;
}

void SetupLogging(YAML::Node &general_parameters) {
  if (general_parameters["log_level"].IsDefined())
    spdlog::set_level(spdlog::level::from_str(
        general_parameters["log_level"].as<std::string>()));
  if (general_parameters["log_file"].IsDefined())
    spdlog::set_default_logger(spdlog::basic_logger_mt(
        "basic_logger", general_parameters["log_file"].as<std::string>()));
}

void SetupLogging(std::string &level, std::string &file) {
  spdlog::set_level(spdlog::level::from_str(level));
  if (!file.empty())
    spdlog::set_default_logger(spdlog::basic_logger_mt("basic_logger", file));
}
