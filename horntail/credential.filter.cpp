//
// Created by wyoming on 30/10/2021.
//

#include "credential.filter.h"

#include <drogon/drogon.h>

#include "base64.h"
#include "config.plugin.h"

void Credential::doFilter(const drogon::HttpRequestPtr& req, drogon::FilterCallback&& invalid_cb,
                          drogon::FilterChainCallback&& valid_cb) {
  auto config = drogon::app().getPlugin<Config>()->get();
  auto authorization = req->getHeader("authorization");
  if (authorization.starts_with("Basic ")) {
    auto encoded = authorization.substr(6);
    auto decoded = base64::decode(encoded);

    auto delimiter = decoded.find(':');
    if (delimiter == std::string::npos) {
      auto response = drogon::HttpResponse::newHttpResponse();
      response->setStatusCode(drogon::k403Forbidden);
      invalid_cb(response);
      return;
    }

    auto username = decoded.substr(0, delimiter);
    auto password = decoded.substr(delimiter + 1);

    for (const auto &credential : config.basic_auth_credentials) {
      if (credential.username == username && credential.password == password) {
        valid_cb();
        return;
      }
    }

    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k403Forbidden);
    invalid_cb(response);
    return;
  }

  if (authorization.starts_with("Bearer ")) {
    auto token = authorization.substr(7);
    for (const auto &credential : config.bearer_token_credentials) {
      if (token == credential) {
        valid_cb();
        return;
      }
    }

    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k403Forbidden);
    invalid_cb(response);
    return;
  }

  auto response = drogon::HttpResponse::newHttpResponse();
  response->setStatusCode(drogon::k401Unauthorized);
  invalid_cb(response);
}
