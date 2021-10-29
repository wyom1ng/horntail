#include <drogon/drogon.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "logging.h"

int main(int argc, char *argv[]) {
  spdlog::cfg::load_env_levels();
  spdlog::set_pattern(R"JSON({"name":"%n","thread_id":%t,"process_id":%P,"level":"%l","message":"%v","timestamp":%E%e,"filename":"%s","line":%#,"function":"%!"})JSON");
  spdlog::stdout_color_mt("console");

  std::string config_file = "config.json";
  if (argc >= 2) {
    config_file = argv[1];
  }

  drogon::app().loadConfigFile(config_file);
  SPDLOG_LOGGER_INFO(spdlog::get("console"), "successfully loaded config file: '{}'", config_file);

  drogon::app().registerBeginningAdvice(Logging::on_application_start);
  drogon::app().registerPreRoutingAdvice(Logging::on_request);
  drogon::app().registerPreSendingAdvice(Logging::on_response);
  drogon::app().run();
}