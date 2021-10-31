#include "database.plugin.h"

#include <drogon/HttpAppFramework.h>
#include <spdlog/spdlog.h>

#include "config.plugin.h"

void Database::initAndStart(const Json::Value &database_config) {
  auto config = drogon::app().getPlugin<Config>()->get();

  auto db = drogon::app().getDbClient();

  try {
    db->execSqlSync(
        "CREATE TABLE IF NOT EXISTS `links` ("
        "`id` VARCHAR(256) NOT NULL,"
        "`link` TEXT NOT NULL,"
        "`created_at` TIMESTAMP DEFAULT UTC_TIMESTAMP,"
        "`delete_after` TIMESTAMP NULL DEFAULT NULL,"
        "PRIMARY KEY (`id`)"
        ");");
  } catch (const drogon::orm::DrogonDbException &e) {
    SPDLOG_LOGGER_ERROR(spdlog::get("database"), "failed to initialise table: '{}'", e.base().what());
    drogon::app().quit();
  }

  std::chrono::seconds deletion_interval(database_config.get("deletion_interval_seconds", 300).asUInt64());
  deletion_thread = std::jthread(Database::deletion_routine, deletion_interval);
  deletion_thread.detach();
}

void Database::shutdown() {
  deletion_thread.request_stop();
  if (deletion_thread.joinable())
    deletion_thread.join();
}

void Database::deletion_routine(const std::stop_token &stop_token, std::chrono::seconds interval) {
  while (!stop_token.stop_requested()) {
    SPDLOG_LOGGER_TRACE(spdlog::get("database"), "running deletion routine");
    drogon::app().getDbClient()->execSqlSync(
        "DELETE FROM `links` "
        "WHERE `delete_after` IS NOT NULL AND `delete_after` < UTC_TIMESTAMP;");
    SPDLOG_LOGGER_TRACE(spdlog::get("database"), "deletion routine completed");
    std::this_thread::sleep_for(interval);
  }
}
