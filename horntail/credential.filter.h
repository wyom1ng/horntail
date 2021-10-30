//
// Created by wyoming on 30/10/2021.
//

#ifndef HORNTAIL_HORNTAIL_CREDENTIAL_FILTER_H_
#define HORNTAIL_HORNTAIL_CREDENTIAL_FILTER_H_

#include <drogon/HttpFilter.h>
class Credential : public drogon::HttpFilter<Credential> {
 public:
  virtual void doFilter(const drogon::HttpRequestPtr &req, drogon::FilterCallback &&fcb,
                        drogon::FilterChainCallback &&fccb) override;
};

#endif  // HORNTAIL_HORNTAIL_CREDENTIAL_FILTER_H_
