#define DROGON_TEST_MAIN
#include <drogon/drogon.h>
#include <drogon/drogon_test.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <future>

DROGON_TEST(app_is_running)
{
  MANDATE(drogon::app().isRunning());
}

int main(int argc, char *argv[])
{
  spdlog::stdout_color_mt("app");
  spdlog::stdout_color_mt("config");
  spdlog::stdout_color_mt("database");

  drogon::app().loadConfigFile("config.test.json");

  std::promise<void> started_promise;
  std::future<void> started_future = started_promise.get_future();

  std::thread app_thread([&]() {
    drogon::app().getLoop()->queueInLoop([&started_promise]() { started_promise.set_value(); });
    drogon::app().run();
  });
  started_future.get();

  auto status = drogon::test::run(argc, argv);

  drogon::app().getLoop()->queueInLoop([]() { drogon::app().quit(); });
  app_thread.join();

  return status;
}