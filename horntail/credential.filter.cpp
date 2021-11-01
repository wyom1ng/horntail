//
// Created by wyoming on 30/10/2021.
//

#include "credential.filter.h"

#include <drogon/drogon.h>

#include "config.plugin.h"

void Credential::doFilter(const drogon::HttpRequestPtr& req, drogon::FilterCallback&& invalid_cb,
                          drogon::FilterChainCallback&& valid_cb) {
  auto config = drogon::app().getPlugin<Config>()->get();
  auto authorization = req->getHeader("authorization");

  if (std::find(config.credentials.begin(), config.credentials.end(), authorization) != config.credentials.end()) {
    valid_cb();
    return;
  }

  if (authorization.empty()) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k401Unauthorized);
    invalid_cb(response);
    return;
  }

  auto response = drogon::HttpResponse::newHttpResponse();
  response->setStatusCode(drogon::k403Forbidden);
  invalid_cb(response);
}
