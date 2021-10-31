/**
 *
 *  config.cc
 *
 */

#include "config.plugin.h"

#include <drogon/drogon.h>
#include <spdlog/spdlog.h>

void Config::initAndStart(const Json::Value &) {
  auto custom_config = drogon::app().getCustomConfig();

  config.default_link_lifetime_seconds = custom_config.get("default_link_lifetime_seconds", 432000).asUInt64();  // 5 days

  auto config_link_length = custom_config.get("link_id_length", 10).asUInt();
  if (config_link_length > std::numeric_limits<uint8_t>::max()) {
    SPDLOG_LOGGER_ERROR(spdlog::get("config"), "invalid link_id_length: '{}'", config_link_length);
    drogon::app().quit();
  }
  config.link_id_length = static_cast<uint8_t>(config_link_length);
  config.base_url = custom_config.get("base_url", "localhost").asString();
  config.not_found_redirect_url = custom_config.get("not_found_redirect_url", "").asString();
  config.link_alphabet = custom_config.get("link_alphabet", "_-0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ").asString();

  if (custom_config["credentials"].isArray()) {
    for (const auto &credential : custom_config["credentials"]) {
      if (!credential.isString()) continue;
      config.credentials.push_back(credential.asString());
    }
  }
}

void Config::shutdown() {}

const struct Config::config &Config::get() const { return config; }
