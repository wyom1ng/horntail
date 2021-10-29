#include "link.controller.h"

namespace controllers {

void Link::create(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k501NotImplemented);
    callback(response);
}

void Link::visit(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                 const std::string &id) {
  auto response = drogon::HttpResponse::newHttpResponse();
  response->setStatusCode(drogon::k200OK);
  response->setBody(id);
  callback(response);
}

};  // namespace controllers