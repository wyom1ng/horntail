#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <iostream>
#include <nlohmann/json.hpp>

const std::array<std::string, 9> KEYS({"name", "thread_id", "process_id", "level", "message", "timestamp", "filename",
                                       "line", "function"});

bool is_valid_logentry(const nlohmann::json &log) {
  return std::ranges::all_of(KEYS.begin(), KEYS.end(), [&log](const std::string &key) { return log.contains(key); });
}

std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> get_time(const nlohmann::json &log) {
  auto ms = std::chrono::milliseconds(log["timestamp"].get<uint64_t>());
  auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(ms);
  return std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>(nanoseconds);
}

spdlog::source_loc get_location(const nlohmann::json &log) {
  spdlog::source_loc location = {};
  const auto *filename = log["filename"].get_ptr<const std::string*>();
  location.filename = filename->c_str();
  location.line = log["line"].get<int>();
  const auto *function = log["function"].get_ptr<const std::string*>();
  location.funcname = function->c_str();

  return location;
}

spdlog::level::level_enum get_level(const nlohmann::json &log) {
  const std::string level = log["level"];

  if (level == "trace") return spdlog::level::trace;
  if (level == "debug") return spdlog::level::debug;
  if (level == "info") return spdlog::level::info;
  if (level == "warn") return spdlog::level::warn;
  if (level == "err") return spdlog::level::err;
  if (level == "critical") return spdlog::level::critical;
  if (level == "off") return spdlog::level::off;

  return spdlog::level::n_levels;
}

int main() {
  spdlog::set_level(spdlog::level::trace);

  for (std::string line; std::getline(std::cin, line);) {
    if (!nlohmann::json::accept(line)) continue;
    auto log = nlohmann::json::parse(line, nullptr, false);
    if (!is_valid_logentry(log)) continue;

    auto time = get_time(log);
    auto location = get_location(log);
    auto name = log["name"].get<std::string>();
    auto level = get_level(log);
    auto message = log["message"].get<std::string>();

    if (!spdlog::get(name)) {
      spdlog::stdout_color_mt(name);
    }

    spdlog::get(name)->log(time, location, level, message);
  }
}