#ifndef HORNTAIL_CONFIG_PLUGIN_H
#define HORNTAIL_CONFIG_PLUGIN_H

#include <drogon/plugins/Plugin.h>

class Config : public drogon::Plugin<Config> {
 private:
  struct basic_auth {
    std::string username;
    std::string password;
  };

  struct config {
    uint64_t default_link_lifetime_seconds;
    uint8_t link_id_length;
    std::string link_alphabet;
    std::string base_url;
    std::string not_found_redirect_url;
    std::vector<std::string> credentials;
  } config;

 public:
  void initAndStart(const Json::Value &config) override;
  void shutdown() override;

  [[nodiscard]] const struct config &get() const;
};

#endif