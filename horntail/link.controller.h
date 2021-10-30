#ifndef HORNTAIL_LINK_CONTROLLER_H
#define HORNTAIL_LINK_CONTROLLER_H

#include <drogon/HttpController.h>
#include <nanoid/crypto_random.h>

#include <duthomhas/csprng.hpp>
#include <random>

namespace controllers {

class Link : public drogon::HttpController<Link> {
 public:
  METHOD_LIST_BEGIN

  ADD_METHOD_TO(Link::create, "/", drogon::Post);
  ADD_METHOD_TO(Link::visit, "/{id}", drogon::Get);

  METHOD_LIST_END

  void create(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
  void visit(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback,
             const std::string &id);

 private:
  nanoid::crypto_random<duthomhas::csprng> random;
  duthomhas::csprng csprng;
  std::mt19937 mersenne_twister;
};

}  // namespace controllers

#endif