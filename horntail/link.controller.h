#ifndef HORNTAIL_LINK_CONTROLLER_H
#define HORNTAIL_LINK_CONTROLLER_H

#include <drogon/HttpController.h>

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
};

}  // namespace controllers

#endif