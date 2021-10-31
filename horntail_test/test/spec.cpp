#include <drogon/HttpClient.h>
#include <drogon/drogon_test.h>
#include <drogon/utils/coroutine.h>
#include <nlohmann/json.hpp>
#include "../guard.h"

DROGON_TEST(serves_indexhtml_by_default) {
  Guard guard(TEST_CTX);
  auto req = drogon::HttpRequest::newHttpRequest();
  req->setPath("/api/v1/spec");

  const auto &[result, response] = guard.http->sendRequest(req, 1);
  REQUIRE(result == drogon::ReqResult::Ok);
  CHECK(response->getStatusCode() == drogon::k200OK);
  CHECK(response->contentType() == drogon::CT_TEXT_HTML);
}

DROGON_TEST(serves_spec_json) {
  Guard guard(TEST_CTX);
  auto req = drogon::HttpRequest::newHttpRequest();
  req->setPath("/api/v1/spec.json");

  const auto &[result, response] = guard.http->sendRequest(req, 1);
  REQUIRE(result == drogon::ReqResult::Ok);
  CHECK(response->getStatusCode() == drogon::k200OK);
  CHECK(response->contentType() == drogon::CT_APPLICATION_JSON);

  CHECK(nlohmann::json::accept(response->body()));
}