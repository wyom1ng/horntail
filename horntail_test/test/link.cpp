#include <drogon/drogon_test.h>

#include "../guard.h"
#include "nlohmann/json.hpp"

DROGON_TEST(generate_link_rejects_unauthorized) {
  Guard guard(TEST_CTX);
  auto req = drogon::HttpRequest::newHttpRequest();
  req->setPath("/api/v1/link");
  req->setMethod(drogon::Post);

  const auto &[result, response] = guard.http->sendRequest(req, 1);
  REQUIRE(result == drogon::ReqResult::Ok);
  CHECK(response->getStatusCode() == drogon::k401Unauthorized);
}

DROGON_TEST(generate_link_rejects_invalid_credentials) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_invalid, guard.basic_auth_invalid, guard.basic_auth_corrupt}) {
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/link");
    req->setMethod(drogon::Post);
    req->addHeader("authorization", authorization);

    const auto &[result, response] = guard.http->sendRequest(req, 1);
    REQUIRE(result == drogon::ReqResult::Ok);
    CHECK(response->getStatusCode() == drogon::k403Forbidden);
  }
}

DROGON_TEST(generate_link_rejects_no_target) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_valid, guard.basic_auth_valid}) {
    {
      nlohmann::json empty_json = nlohmann::json::object();
      nlohmann::json random_keys = {
          {"a", 23},
          {"foo", "bar"},
          {"float", 3.14159},
      };
      for (const auto &body : std::vector<std::string>({"", "corrupt-json", empty_json.dump(), random_keys.dump()})) {
        auto req = drogon::HttpRequest::newHttpRequest();
        req->setPath("/api/v1/link");
        req->setMethod(drogon::Post);
        req->addHeader("authorization", authorization);
        req->setBody(body);

        const auto &[result, response] = guard.http->sendRequest(req, 1);
        REQUIRE(result == drogon::ReqResult::Ok);
        CHECK(response->getStatusCode() == drogon::k400BadRequest);
      }
    }
  }
}

DROGON_TEST(generate_link_rejects_invalid_lifetime) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_valid, guard.basic_auth_valid}) {
    {
      auto req = drogon::HttpRequest::newHttpRequest();
      req->setPath("/api/v1/link");
      req->setMethod(drogon::Post);
      req->addHeader("authorization", authorization);
      nlohmann::json body;
      body["target"] = "https://really-long-url.com/";
      body["lifetime_seconds"] = "not-an-int";
      req->setBody(body.dump());
      req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

      const auto &[result, response] = guard.http->sendRequest(req, 1);
      REQUIRE(result == drogon::ReqResult::Ok);
      REQUIRE(response->getStatusCode() == drogon::k400BadRequest);
    }
  }
}

DROGON_TEST(generate_link_accepts_target_default_lifetime) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_valid, guard.basic_auth_valid}) {
    {
      auto req = drogon::HttpRequest::newHttpRequest();
      req->setPath("/api/v1/link");
      req->setMethod(drogon::Post);
      req->addHeader("authorization", authorization);
      nlohmann::json body;
      body["target"] = "https://really-long-url.com/";
      req->setBody(body.dump());
      req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

      const auto &[result, response] = guard.http->sendRequest(req, 1);
      REQUIRE(result == drogon::ReqResult::Ok);
      REQUIRE(response->getStatusCode() == drogon::k200OK);
      REQUIRE(nlohmann::json::accept(response->body()));

      auto response_json = nlohmann::json::parse(response->body(), nullptr, false);
      for (const auto &key : {"available_until", "id", "short_url"}) {
        REQUIRE(response_json[key].is_string());
      }

      CHECK(response_json["short_url"].get<std::string>().starts_with(guard.base_url));
      CHECK(guard.validate_available_until(response_json["available_until"]));
    }
  }
}

DROGON_TEST(generate_link_accepts_target_custom_lifetime) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_valid, guard.basic_auth_valid}) {
    {
      auto req = drogon::HttpRequest::newHttpRequest();
      req->setPath("/api/v1/link");
      req->setMethod(drogon::Post);
      req->addHeader("authorization", authorization);
      nlohmann::json body;
      body["target"] = "https://really-long-url.com/";
      auto lifetime = 10s;
      body["lifetime_seconds"] = lifetime.count();
      req->setBody(body.dump());
      req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

      const auto &[result, response] = guard.http->sendRequest(req, 1);
      REQUIRE(result == drogon::ReqResult::Ok);
      REQUIRE(response->getStatusCode() == drogon::k200OK);
      REQUIRE(nlohmann::json::accept(response->body()));

      auto response_json = nlohmann::json::parse(response->body(), nullptr, false);
      for (const auto &key : {"available_until", "id", "short_url"}) {
        REQUIRE(response_json[key].is_string());
      }

      CHECK(response_json["short_url"].get<std::string>().starts_with(guard.base_url));
      CHECK(guard.validate_available_until(response_json["available_until"], lifetime));
    }
  }
}

DROGON_TEST(generate_link_accepts_target_indefinite_lifetime) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_valid, guard.basic_auth_valid}) {
    {
      auto req = drogon::HttpRequest::newHttpRequest();
      req->setPath("/api/v1/link");
      req->setMethod(drogon::Post);
      req->addHeader("authorization", authorization);
      nlohmann::json body;
      body["target"] = "https://really-long-url.com/";
      body["lifetime_seconds"] = 0;
      req->setBody(body.dump());
      req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

      const auto &[result, response] = guard.http->sendRequest(req, 1);
      REQUIRE(result == drogon::ReqResult::Ok);
      REQUIRE(response->getStatusCode() == drogon::k200OK);
      REQUIRE(nlohmann::json::accept(response->body()));

      auto response_json = nlohmann::json::parse(response->body(), nullptr, false);
      for (const auto &key : {"id", "short_url"}) {
        REQUIRE(response_json[key].is_string());
      }

      CHECK(response_json["short_url"].get<std::string>().starts_with(guard.base_url));
      CHECK(!response_json.contains("available_until"));
    }
  }
}

DROGON_TEST(visit_link_rejects_invalid_id) {
  Guard guard(TEST_CTX);

  auto req = drogon::HttpRequest::newHttpRequest();
  auto id = std::string(500, 'a');
  req->setPath("/" + id);
  req->setMethod(drogon::Get);

  const auto &[result, response] = guard.http->sendRequest(req, 1);
  REQUIRE(result == drogon::ReqResult::Ok);
  REQUIRE(response->getStatusCode() == drogon::k400BadRequest);
}

DROGON_TEST(visit_link_rejects_nonexistant_id) {
  Guard guard(TEST_CTX);

  auto req = drogon::HttpRequest::newHttpRequest();
  req->setPath("/some-id");
  req->setMethod(drogon::Get);

  const auto &[result, response] = guard.http->sendRequest(req, 1);
  REQUIRE(result == drogon::ReqResult::Ok);
  REQUIRE(response->getStatusCode() == drogon::k404NotFound);
}

DROGON_TEST(visit_link_accepts_valid_id) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_valid, guard.basic_auth_valid}) {
    {
      auto req = drogon::HttpRequest::newHttpRequest();
      req->setPath("/api/v1/link");
      req->setMethod(drogon::Post);
      req->addHeader("authorization", authorization);
      nlohmann::json body;
      std::string target = "https://really-long-url.com/";
      body["target"] = target;
      req->setBody(body.dump());
      req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

      nlohmann::json response_json;
      {
        const auto &[result, response] = guard.http->sendRequest(req, 1);
        REQUIRE(result == drogon::ReqResult::Ok);
        REQUIRE(response->getStatusCode() == drogon::k200OK);
        REQUIRE(nlohmann::json::accept(response->body()));
        response_json = nlohmann::json::parse(response->body(), nullptr, false);
      }

      req = drogon::HttpRequest::newHttpRequest();
      req->setPath("/" + response_json["id"].get<std::string>());
      req->setMethod(drogon::Get);

      const auto &[result, response] = guard.http->sendRequest(req, 1);
      REQUIRE(result == drogon::ReqResult::Ok);
      REQUIRE(response->getStatusCode() == drogon::k302Found);
      CHECK(response->getHeader("location") == target);
    }
  }
}

DROGON_TEST(delete_link_rejects_unauthorized) { Guard guard(TEST_CTX); }

DROGON_TEST(delete_link_rejects_invalid_credentials) { Guard guard(TEST_CTX); }

DROGON_TEST(delete_link_rejects_invalid_id) { Guard guard(TEST_CTX); }

DROGON_TEST(delete_link_accepts_nonexistant_id) { Guard guard(TEST_CTX); }

DROGON_TEST(delete_link_accepts_valid_id) { Guard guard(TEST_CTX); }

DROGON_TEST(get_link_rejects_unauthorized) { Guard guard(TEST_CTX); }

DROGON_TEST(get_link_rejects_invalid_credentials) { Guard guard(TEST_CTX); }

DROGON_TEST(get_link_rejects_invalid_id) { Guard guard(TEST_CTX); }

DROGON_TEST(get_link_rejects_nonexistant_id) { Guard guard(TEST_CTX); }

DROGON_TEST(get_link_rejects_id_marked_for_deletion) { Guard guard(TEST_CTX); }

DROGON_TEST(get_link_accepts_valid_id) { Guard guard(TEST_CTX); }

DROGON_TEST(create_link_rejects_unauthorized) { Guard guard(TEST_CTX); }

DROGON_TEST(create_link_rejects_invalid_credentials) { Guard guard(TEST_CTX); }

DROGON_TEST(create_link_rejects_invalid_id) { Guard guard(TEST_CTX); }

DROGON_TEST(create_link_accepts_nonexistant_id) { Guard guard(TEST_CTX); }

DROGON_TEST(create_link_accepts_id_marked_for_deletion) { Guard guard(TEST_CTX); }

DROGON_TEST(create_link_accepts_valid_id) { Guard guard(TEST_CTX); }
