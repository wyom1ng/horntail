#ifndef HORNTAIL_LINK_CONTROLLER_H
#define HORNTAIL_LINK_CONTROLLER_H

#include <drogon/HttpController.h>
#include <drogon/utils/coroutine.h>
#include <nanoid/crypto_random.h>

#include <duthomhas/csprng.hpp>
#include <random>

namespace controllers {

class Link : public drogon::HttpController<Link> {
 public:
  METHOD_LIST_BEGIN

  ADD_METHOD_TO(Link::visit, "/{id}", drogon::Get);

  ADD_METHOD_TO(Link::generate, "/api/v1/link", drogon::Post, "Credential");
  ADD_METHOD_TO(Link::get, "/api/v1/link", drogon::Get, "Credential");
  ADD_METHOD_TO(Link::create, "/api/v1/link", drogon::Put, "Credential");
  ADD_METHOD_TO(Link::remove, "/api/v1/link", drogon::Delete, "Credential");

  METHOD_LIST_END

  void visit(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback,
             const std::string &id);

  static drogon::Task<> generate(drogon::HttpRequestPtr req,
                                 std::function<void(const drogon::HttpResponsePtr &)> callback);
  void get(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
  void create(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
  void remove(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};

}  // namespace controllers

#endif