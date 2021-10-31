//
// Created by wyoming on 31/10/2021.
//
#include "guard.h"

#include <drogon/drogon.h>

#include <chrono>

#include "lib/date.h"
#include "nlohmann/json.hpp"

Guard::Guard(std::shared_ptr<drogon::test::Case> TEST_CTX) : TEST_CTX(TEST_CTX) {
  auto result = drogon::app().getDbClient()->execSqlSync("SELECT * FROM `links`;");
  MANDATE(result.empty());
}

Guard::~Guard() { drogon::app().getDbClient()->execSqlSync("TRUNCATE TABLE `links`;"); }

std::chrono::time_point<std::chrono::high_resolution_clock> Guard::parse_timestamp(const std::string &timestamp) {
  std::stringstream stream;
  stream << timestamp;
  std::chrono::time_point<std::chrono::high_resolution_clock> tp;
  stream >> date::parse("%FT%TZ", tp);
  return tp;
}

bool Guard::validate_available_until(const std::string &timestamp, const std::chrono::seconds &lifetime) {
  CHECK_NOTHROW(parse_timestamp(timestamp));
  auto parsed = parse_timestamp(timestamp);
  parsed -= lifetime;

  return duration_cast<std::chrono::seconds>(test_start.time_since_epoch()) <= duration_cast<std::chrono::seconds>(parsed.time_since_epoch())
      && duration_cast<std::chrono::seconds>(parsed.time_since_epoch()) <= duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
}

std::string Guard::generate_id(const std::string &authorization, const std::string &target, std::optional<std::chrono::seconds> lifetime) {
  auto req = drogon::HttpRequest::newHttpRequest();
  req->setPath("/api/v1/link");
  req->setMethod(drogon::Post);
  req->addHeader("authorization", authorization);
  nlohmann::json body;
  body["target"] = target;

  if (lifetime.has_value()) {
    body["lifetime_seconds"] = lifetime.value().count();
  }

  req->setBody(body.dump());
  req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

  const auto &[result, response] = http->sendRequest(req, 1);
  MANDATE(result == drogon::ReqResult::Ok);
  MANDATE(response->getStatusCode() == drogon::k200OK);
  MANDATE(nlohmann::json::accept(response->body()));

  auto response_json = nlohmann::json::parse(response->body(), nullptr, false);
  return response_json["id"];
}
