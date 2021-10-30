#ifndef HORNTAIL_DATABASE_PLUGIN_H
#define HORNTAIL_DATABASE_PLUGIN_H

#include <drogon/plugins/Plugin.h>

class Database : public drogon::Plugin<Database> {
 public:
  void initAndStart(const Json::Value &config) override;

  void shutdown() override;

 private:
  static void deletion_routine(uint64_t interval_seconds);

  std::thread deletion_thread;
};

#endif
