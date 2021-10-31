#include "link.controller.h"

#include <nanoid/nanoid.h>

#include "config.plugin.h"

namespace controllers {

drogon::Task<> Link::visit(drogon::HttpRequestPtr req,
                            std::function<void(const drogon::HttpResponsePtr &)> callback,
                 const std::string &id) {
  if (id.size() > 255) {
    Json::Value response_json;
    response_json["message"] = "[id] must be no more than 255 characters";
    auto response = drogon::HttpResponse::newHttpJsonResponse(response_json);
    response->setStatusCode(drogon::k400BadRequest);
    callback(response);
    co_return;
  }

  auto sql = drogon::app().getDbClient();
  try {
    auto result = co_await sql->execSqlCoro("SELECT (`link`) FROM `links` WHERE `id` = ?;", id);

    if (result.empty()) {
      auto config = drogon::app().getPlugin<Config>()->get();
      if (config.not_found_redirect_url.empty()) {
        auto response = drogon::HttpResponse::newHttpResponse();
        response->setStatusCode(drogon::k404NotFound);
        callback(response);
        co_return;
      }

      auto response = drogon::HttpResponse::newHttpResponse();
      response->addHeader("Location", config.not_found_redirect_url);
      response->setStatusCode(drogon::k302Found);
      callback(response);
      co_return;
    }

    std::string url = result[0]["link"].c_str();

    auto response = drogon::HttpResponse::newHttpResponse();
    response->addHeader("Location", url);
    response->setStatusCode(drogon::k302Found);
    callback(response);
  } catch (const drogon::orm::DrogonDbException &e) {
    Json::Value response_json;
    response_json["message"] = e.base().what();
    auto response = drogon::HttpResponse::newHttpJsonResponse(response_json);
    response->setStatusCode(drogon::k500InternalServerError);
    callback(response);
    co_return;
  }
}

drogon::Task<> Link::generate(drogon::HttpRequestPtr req, std::function<void(const drogon::HttpResponsePtr&)> callback) {
  if (!req->getJsonObject()) {
    Json::Value response_json;
    response_json["message"] = "invalid json";
    auto response = drogon::HttpResponse::newHttpJsonResponse(response_json);
    response->setStatusCode(drogon::k400BadRequest);
    callback(response);
    co_return;
  }

  auto json = *req->getJsonObject();
  if (!json["target"].isString() || json["target"].asString().empty()) {
    Json::Value response_json;
    response_json["message"] = "[target] must be a non-empty string";
    auto response = drogon::HttpResponse::newHttpJsonResponse(response_json);
    response->setStatusCode(drogon::k400BadRequest);
    callback(response);
    co_return;
  }

  auto config = drogon::app().getPlugin<Config>()->get();
  nanoid::crypto_random<duthomhas::csprng> random;
  duthomhas::csprng csprng;
  auto id = nanoid::generate(random, config.link_alphabet, config.link_id_length);
  auto url = json["target"].asString();

  if (!(!json.isMember("lifetime_seconds") || json["lifetime_seconds"].isNull() || json["lifetime_seconds"].isNumeric())) {
    Json::Value response_json;
    response_json["message"] = "[lifetime_seconds] must be a uint64_t";
    auto response = drogon::HttpResponse::newHttpJsonResponse(response_json);
    response->setStatusCode(drogon::k400BadRequest);
    callback(response);
    co_return;
  }
  uint64_t lifetime_seconds = json.get("lifetime_seconds", config.default_link_lifetime_seconds).asUInt64();

  auto sql = drogon::app().getDbClient();
  try {
    if (lifetime_seconds) {
      co_await sql->execSqlCoro(
          "INSERT INTO `links` (`id`, `link`, `delete_after`)"
          "VALUES (?, ?, TIMESTAMPADD(SECOND, ?, UTC_TIMESTAMP));",
          id, url, lifetime_seconds, config.base_url);
    } else {
      co_await sql->execSqlCoro(
          "INSERT INTO `links` (`id`, `link`)"
          "VALUES (?, ?);",
          id, url);
    }

    auto result = co_await sql->execSqlCoro(
        "SELECT `id`, DATE_FORMAT(`delete_after`, '%Y-%m-%dT%TZ') as `available_until`, CONCAT(?, `id`) as short_url FROM `links` WHERE `id`= ?;",
        config.base_url, id);

    if (result.empty()) {
      auto response = drogon::HttpResponse::newHttpResponse();
      response->setStatusCode(drogon::k500InternalServerError);
      callback(response);
      co_return;
    }

    Json::Value response_json;
    for (const auto &field : result[0]) {
      if (field.isNull()) continue;
      response_json[field.name()] = field.as<std::string>();
    }
    if (!lifetime_seconds) {
      response_json.removeMember("available_until");
    }

    auto response = drogon::HttpResponse::newHttpJsonResponse(response_json);
    response->setStatusCode(drogon::k200OK);
    callback(response);
    co_return;
  } catch (const drogon::orm::DrogonDbException &e) {
    Json::Value response_json;
    response_json["message"] = e.base().what();
    auto response = drogon::HttpResponse::newHttpJsonResponse(response_json);
    response->setStatusCode(drogon::k500InternalServerError);
    callback(response);
    co_return;
  }
}

void Link::create(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
  auto response = drogon::HttpResponse::newHttpResponse();
  response->setStatusCode(drogon::k501NotImplemented);
  callback(response);
}

void Link::get(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
  auto response = drogon::HttpResponse::newHttpResponse();
  response->setStatusCode(drogon::k501NotImplemented);
  callback(response);
}

void Link::remove(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
  auto response = drogon::HttpResponse::newHttpResponse();
  response->setStatusCode(drogon::k501NotImplemented);
  callback(response);
}

};  // namespace controllers