//
// Created by wyoming on 29/10/2021.
//

#include "logging.h"

#include <drogon/drogon.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <ranges>

void Logging::on_application_start() {
  auto listeners = drogon::app().getListeners();
  std::unordered_set<std::string> addresses;

  SPDLOG_LOGGER_INFO(spdlog::get("app"), "started webserver");
  for (const auto &listener : listeners) {
    if (addresses.contains(listener.toIpPort())) continue;

    SPDLOG_LOGGER_INFO(spdlog::get("app"), "listening on {}:{}", listener.toIp(), listener.toPort());
    addresses.insert(listener.toIpPort());
  }
}

void Logging::on_request(const drogon::HttpRequestPtr &request) {
  SPDLOG_LOGGER_DEBUG(spdlog::get("app"), "=> {} {}", request->getMethodString(), request->getPath());
}

void Logging::on_response(const drogon::HttpRequestPtr &request, const drogon::HttpResponsePtr &response) {
  if (response->getStatusCode() == drogon::k302Found) {
    SPDLOG_LOGGER_DEBUG(spdlog::get("app"), "<= {} {} ({})", response->getStatusCode(), request->getPath(), request->getHeader("location"));
    return;
  }
  SPDLOG_LOGGER_DEBUG(spdlog::get("app"), "<= {} {}", response->getStatusCode(), request->getPath());
}
