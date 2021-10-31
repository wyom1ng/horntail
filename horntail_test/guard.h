//
// Created by wyoming on 31/10/2021.
//

#ifndef HORNTAIL_HORNTAIL_TEST_GUARD_H_
#define HORNTAIL_HORNTAIL_TEST_GUARD_H_

#include <drogon/HttpClient.h>
#include <drogon/drogon_test.h>

#include <chrono>
#include <memory>
using namespace std::chrono_literals;
class Guard {
 private:
  std::shared_ptr<drogon::test::Case> TEST_CTX;
  std::chrono::time_point<std::chrono::high_resolution_clock> test_start = std::chrono::high_resolution_clock::now();

 public:
  Guard(std::shared_ptr<drogon::test::Case> TEST_CTX);
  ~Guard();

  const std::string base_url = "https://wyoming.みんな/";

  const std::string bearer_auth_valid =
      "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiJ3aHkgZGlkIHlvdSBkZWNvZGUgdGhpcz8ifQ";
  const std::string bearer_auth_invalid = "Bearer not-the-right-token";

  const std::string basic_auth_valid = "Basic YWRtaW46cGFzc3dvcmQ=";
  const std::string basic_auth_invalid = "Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==";
  const std::string basic_auth_corrupt = "Basic not base64 $%^&*()_ みんな";

  static inline const std::chrono::seconds default_lifetime = 432000s;

  drogon::HttpClientPtr http = drogon::HttpClient::newHttpClient("http://localhost:9001");

  std::chrono::time_point<std::chrono::high_resolution_clock> parse_timestamp(const std::string &timestamp);
  bool validate_available_until(const std::string &timestamp, const std::chrono::seconds &lifetime = default_lifetime);
};

#endif  // HORNTAIL_HORNTAIL_TEST_GUARD_H_
