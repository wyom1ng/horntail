//
// Created by wyoming on 31/10/2021.
//
#include "guard.h"

#include <drogon/drogon.h>

#include <chrono>

#include "lib/date.h"

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
