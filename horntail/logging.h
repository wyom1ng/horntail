//
// Created by wyoming on 29/10/2021.
//

#ifndef HORNTAIL_HORNTAIL_LOGGING_H_
#define HORNTAIL_HORNTAIL_LOGGING_H_

#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>

class Logging {
 public:
  static void on_application_start();
  static void on_request(const drogon::HttpRequestPtr &);
  static void on_response(const drogon::HttpRequestPtr &,
                          const drogon::HttpResponsePtr &);
};

#endif  // HORNTAIL_HORNTAIL_LOGGING_H_
