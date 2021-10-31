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

DROGON_TEST(generate_link_rejects_invalid_payload) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_valid, guard.basic_auth_valid}) {
    {
      nlohmann::json empty_json = nlohmann::json::object();
      nlohmann::json random_keys = {
          {"a", 23},
          {"foo", "bar"},
          {"float", 3.14159},
      };
      nlohmann::json invalid_lifetime = {{"target", "url"}, {"lifetime_seconds", "not-an-int"}};
      nlohmann::json negative_lifetime = {{"target", "url"}, {"lifetime_seconds", -15}};
      nlohmann::json decimal_lifetime = {{"target", "url"}, {"lifetime_seconds", 1.61803398}};
      for (const auto &body :
           std::vector<std::string>({"", "corrupt-json", empty_json.dump(), random_keys.dump(), invalid_lifetime.dump(),
                                     negative_lifetime.dump(), decimal_lifetime.dump()})) {
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
    std::string target = "https://really-long-url.com/some-path'); DROP TABLE `links`; --";
    std::string id = guard.generate_id(authorization, target);

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/" + id);
    req->setMethod(drogon::Get);

    const auto &[result, response] = guard.http->sendRequest(req, 1);
    REQUIRE(result == drogon::ReqResult::Ok);
    REQUIRE(response->getStatusCode() == drogon::k302Found);
    CHECK(response->getHeader("location") == target);
  }
}

DROGON_TEST(get_link_rejects_unauthorized) {
  Guard guard(TEST_CTX);
  auto req = drogon::HttpRequest::newHttpRequest();
  req->setPath("/api/v1/link/some-id");
  req->setMethod(drogon::Get);

  const auto &[result, response] = guard.http->sendRequest(req, 1);
  REQUIRE(result == drogon::ReqResult::Ok);
  CHECK(response->getStatusCode() == drogon::k401Unauthorized);
}

DROGON_TEST(get_link_rejects_invalid_credentials) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_invalid, guard.basic_auth_invalid, guard.basic_auth_corrupt}) {
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/link/some-id");
    req->setMethod(drogon::Get);
    req->addHeader("authorization", authorization);

    const auto &[result, response] = guard.http->sendRequest(req, 1);
    REQUIRE(result == drogon::ReqResult::Ok);
    CHECK(response->getStatusCode() == drogon::k403Forbidden);
  }
}

DROGON_TEST(get_link_rejects_invalid_id) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_valid, guard.basic_auth_valid}) {
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/link/" + std::string(500, 'a'));
    req->setMethod(drogon::Get);
    req->addHeader("authorization", authorization);

    const auto &[result, response] = guard.http->sendRequest(req, 1);
    REQUIRE(result == drogon::ReqResult::Ok);
    CHECK(response->getStatusCode() == drogon::k400BadRequest);
  }
}

DROGON_TEST(get_link_rejects_nonexistant_id) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_valid, guard.basic_auth_valid}) {
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/link/some-id");
    req->setMethod(drogon::Get);
    req->addHeader("authorization", authorization);

    const auto &[result, response] = guard.http->sendRequest(req, 1);
    REQUIRE(result == drogon::ReqResult::Ok);
    CHECK(response->getStatusCode() == drogon::k404NotFound);
  }
}

DROGON_TEST(get_link_rejects_id_marked_for_deletion) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_valid, guard.basic_auth_valid}) {
    std::string target = "https://really-long-url.com/";
    std::string id = guard.generate_id(authorization, target, 1s);

    std::this_thread::sleep_for(1200ms);

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/link/" + id);
    req->setMethod(drogon::Get);
    req->addHeader("authorization", authorization);

    const auto &[result, response] = guard.http->sendRequest(req, 1);
    REQUIRE(result == drogon::ReqResult::Ok);
    CHECK(response->getStatusCode() == drogon::k404NotFound);
  }
}

DROGON_TEST(get_link_accepts_valid_id) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_valid, guard.basic_auth_valid}) {
    std::string target = "https://really-long-url.com/";
    std::string id = guard.generate_id(authorization, target);

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/link/" + id);
    req->setMethod(drogon::Get);
    req->addHeader("authorization", authorization);

    const auto &[result, response] = guard.http->sendRequest(req, 1);
    REQUIRE(result == drogon::ReqResult::Ok);
    REQUIRE(response->getStatusCode() == drogon::k200OK);
    REQUIRE(nlohmann::json::accept(response->body()));
    auto response_json = nlohmann::json::parse(response->body(), nullptr, false);

    for (const auto &key : {"available_until", "short_url", "target", "created_at"}) {
      REQUIRE(response_json[key].is_string());
    }

    CHECK(response_json["target"] == target);
    CHECK(response_json["short_url"] == guard.base_url + id);
    CHECK(guard.validate_available_until(response_json["available_until"]));
  }
}
DROGON_TEST(get_link_accepts_indefinite_lifetime) {
  Guard guard(TEST_CTX);

  for (const auto &authorization : {guard.bearer_auth_valid, guard.basic_auth_valid}) {
    std::string target = "https://really-long-url.com/";
    std::string id = guard.generate_id(authorization, target, 0s);

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/link/" + id);
    req->setMethod(drogon::Get);
    req->addHeader("authorization", authorization);

    const auto &[result, response] = guard.http->sendRequest(req, 1);
    REQUIRE(result == drogon::ReqResult::Ok);
    REQUIRE(response->getStatusCode() == drogon::k200OK);
    REQUIRE(nlohmann::json::accept(response->body()));
    auto response_json = nlohmann::json::parse(response->body(), nullptr, false);

    for (const auto &key : {"short_url", "target", "created_at"}) {
      REQUIRE(response_json[key].is_string());
    }

    CHECK(response_json["target"] == target);
    CHECK(response_json["short_url"] == guard.base_url + id);
    CHECK(!response_json.contains("available_until"));
  }
}

DROGON_TEST(create_link_rejects_unauthorized) {
  Guard guard(TEST_CTX);
  auto req = drogon::HttpRequest::newHttpRequest();
  req->setPath("/api/v1/link/some-id");
  req->setMethod(drogon::Put);

  const auto &[result, response] = guard.http->sendRequest(req, 1);
  REQUIRE(result == drogon::ReqResult::Ok);
  CHECK(response->getStatusCode() == drogon::k401Unauthorized);
}

DROGON_TEST(create_link_rejects_invalid_credentials) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_invalid, guard.basic_auth_invalid, guard.basic_auth_corrupt}) {
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/link/some-id");
    req->setMethod(drogon::Put);
    req->addHeader("authorization", authorization);

    const auto &[result, response] = guard.http->sendRequest(req, 1);
    REQUIRE(result == drogon::ReqResult::Ok);
    CHECK(response->getStatusCode() == drogon::k403Forbidden);
  }
}

DROGON_TEST(create_link_rejects_invalid_id) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_valid, guard.basic_auth_valid}) {
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/link/" + std::string(500, 'a'));
    req->setMethod(drogon::Put);
    req->addHeader("authorization", authorization);

    const auto &[result, response] = guard.http->sendRequest(req, 1);
    REQUIRE(result == drogon::ReqResult::Ok);
    CHECK(response->getStatusCode() == drogon::k400BadRequest);
  }
}

DROGON_TEST(create_link_rejects_invalid_payload) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_valid, guard.basic_auth_valid}) {
    nlohmann::json empty_json = nlohmann::json::object();
    nlohmann::json random_keys = {
        {"a", 23},
        {"foo", "bar"},
        {"float", 3.14159},
    };
    nlohmann::json invalid_lifetime = {{"target", "url"}, {"lifetime_seconds", "not-an-int"}};
    nlohmann::json negative_lifetime = {{"target", "url"}, {"lifetime_seconds", -15}};
    nlohmann::json decimal_lifetime = {{"target", "url"}, {"lifetime_seconds", 1.61803398}};
    for (const auto &body :
         std::vector<std::string>({"", "corrupt-json", empty_json.dump(), random_keys.dump(), invalid_lifetime.dump(),
                                   negative_lifetime.dump(), decimal_lifetime.dump()})) {
      auto req = drogon::HttpRequest::newHttpRequest();
      req->setPath("/api/v1/link/some-id");
      req->setMethod(drogon::Put);
      req->addHeader("authorization", authorization);
      req->setBody(body);

      const auto &[result, response] = guard.http->sendRequest(req, 1);
      REQUIRE(result == drogon::ReqResult::Ok);
      CHECK(response->getStatusCode() == drogon::k400BadRequest);
    }
  }
}

DROGON_TEST(create_link_accepts_nonexistant_id) {
  Guard guard(TEST_CTX);
  std::string id = "some-id";

  auto req = drogon::HttpRequest::newHttpRequest();
  req->setPath("/api/v1/link/" + id);
  req->setMethod(drogon::Put);
  req->addHeader("authorization", guard.bearer_auth_valid);
  nlohmann::json body;
  body["target"] = "url";

  req->setBody(body.dump());
  req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

  const auto &[result, response] = guard.http->sendRequest(req, 1);
  REQUIRE(result == drogon::ReqResult::Ok);
  CHECK(response->getStatusCode() == drogon::k201Created);
}

DROGON_TEST(create_link_accepts_id_marked_for_deletion) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_valid, guard.basic_auth_valid}) {
    std::string id = guard.generate_id(authorization, "some-url", 1s);

    std::this_thread::sleep_for(1200ms);

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/link/" + id);
    req->setMethod(drogon::Put);
    req->addHeader("authorization", authorization);
    nlohmann::json body;
    body["target"] = "url";

    req->setBody(body.dump());
    req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

    const auto &[result, response] = guard.http->sendRequest(req, 1);
    REQUIRE(result == drogon::ReqResult::Ok);
    CHECK(response->getStatusCode() == drogon::k201Created);
  }
}

DROGON_TEST(create_link_accepts_valid_id) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_valid, guard.basic_auth_valid}) {
    std::string id = guard.generate_id(authorization, "some-url");

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/link/" + id);
    req->setMethod(drogon::Put);
    req->addHeader("authorization", authorization);
    nlohmann::json body;
    body["target"] = "url";

    req->setBody(body.dump());
    req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

    const auto &[result, response] = guard.http->sendRequest(req, 1);
    REQUIRE(result == drogon::ReqResult::Ok);
    CHECK(response->getStatusCode() == drogon::k200OK);
  }
}

DROGON_TEST(remove_link_rejects_unauthorized) {
  Guard guard(TEST_CTX);
  auto req = drogon::HttpRequest::newHttpRequest();
  req->setPath("/api/v1/link/some-id");
  req->setMethod(drogon::Delete);

  const auto &[result, response] = guard.http->sendRequest(req, 1);
  REQUIRE(result == drogon::ReqResult::Ok);
  CHECK(response->getStatusCode() == drogon::k401Unauthorized);
}

DROGON_TEST(remove_link_rejects_invalid_credentials) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_invalid, guard.basic_auth_invalid, guard.basic_auth_corrupt}) {
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/link/some-id");
    req->setMethod(drogon::Delete);
    req->addHeader("authorization", authorization);

    const auto &[result, response] = guard.http->sendRequest(req, 1);
    REQUIRE(result == drogon::ReqResult::Ok);
    CHECK(response->getStatusCode() == drogon::k403Forbidden);
  }
}

DROGON_TEST(remove_link_rejects_invalid_id) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_valid, guard.basic_auth_valid}) {
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/link/" + std::string(500, 'a'));
    req->setMethod(drogon::Put);
    req->addHeader("authorization", authorization);

    const auto &[result, response] = guard.http->sendRequest(req, 1);
    REQUIRE(result == drogon::ReqResult::Ok);
    CHECK(response->getStatusCode() == drogon::k400BadRequest);
  }
}

DROGON_TEST(remove_link_accepts_nonexistant_id) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_valid, guard.basic_auth_valid}) {
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/v1/link/some-id");
    req->setMethod(drogon::Put);
    req->addHeader("authorization", authorization);

    const auto &[result, response] = guard.http->sendRequest(req, 1);
    REQUIRE(result == drogon::ReqResult::Ok);
    CHECK(response->getStatusCode() == drogon::k204NoContent);
  }
}

DROGON_TEST(remove_link_accepts_valid_id) {
  Guard guard(TEST_CTX);
  for (const auto &authorization : {guard.bearer_auth_valid, guard.basic_auth_valid}) {
    std::string id = guard.generate_id(authorization, "some-url");
    {
      auto req = drogon::HttpRequest::newHttpRequest();
      req->setPath("/api/v1/link/" + id);
      req->setMethod(drogon::Put);
      req->addHeader("authorization", authorization);
      nlohmann::json body;
      body["target"] = "url";

      req->setBody(body.dump());
      req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

      const auto &[result, response] = guard.http->sendRequest(req, 1);
      REQUIRE(result == drogon::ReqResult::Ok);
      CHECK(response->getStatusCode() == drogon::k200OK);
    }
    {
      auto req = drogon::HttpRequest::newHttpRequest();
      req->setPath("/" + id);
      req->setMethod(drogon::Get);

      const auto &[result, response] = guard.http->sendRequest(req, 1);
      REQUIRE(result == drogon::ReqResult::Ok);
      CHECK(response->getStatusCode() == drogon::k404NotFound);
    }
  }
}
