#include "database.plugin.h"

#include <drogon/HttpAppFramework.h>
#include <spdlog/spdlog.h>

#include "config.plugin.h"

void Database::initAndStart(const Json::Value &) {
  auto config = drogon::app().getPlugin<Config>()->get();
  auto link_id_length = std::to_string(config.link_id_length);

  auto db = drogon::app().getDbClient();

  try {
    db->execSqlSync(
        "CREATE TABLE IF NOT EXISTS `links` ("
        "`id` VARCHAR(256) NOT NULL,"
        "`link` TEXT NOT NULL,"
        "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "`delete_after` TIMESTAMP,"
        "PRIMARY KEY (`id`)"
        ");");
  } catch (const drogon::orm::DrogonDbException &e) {
    SPDLOG_LOGGER_ERROR(spdlog::get("database"), "failed to initialise table: '{}'", e.base().what());
    drogon::app().quit();
  }
}

void Database::shutdown() {}
