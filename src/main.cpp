//
// Created by Rakesh on 2020-03-11.
//
// gdb -ex run --args ./a.out arg1 arg2 ...

#include <iostream>

#include "server.h"
#include "log/NanoLog.h"
#include "model/config.h"
#include "util/clara.h"

int main( int argc, char const * const * argv )
{
  using clara::Opt;

  bool console = false;
  int logBuffer = 3;
  std::string dir{"logs/"};
  bool help = false;
  auto& conf = spt::model::Configuration::instance();

  auto options = clara::Help(help) |
      Opt(conf.mongoServiceHost, "mongo-service")["-a"]["--mongo-service-host"]("Mongo service host to connect to for metrics.") |
      Opt(conf.mongoServicePort, "2000")["-b"]["--mongo-service-port"]("Mongo service port to connect to for metrics (default 2000)") |
      Opt(conf.versionDatabase, "versionHistory")["-c"]["--version-database"]("Mongo database where metrics are stored (default versionHistory)") |
      Opt(conf.versionCollection, "entities")["-d"]["--version-collection"]("Mongo collection where metrics are stored (default entities)") |
      Opt(conf.metricsDatabase, "metrics")["-e"]["--metrics-database"]("Mongo database where metrics are stored (default metrics)") |
      Opt(conf.metricsCollection, "api")["-f"]["--metrics-collection"]("Mongo collection where metrics are stored (default api)") |
      Opt(conf.port, "6100")["-p"]["--port"]("Port on which to listen (default 6100)") |
      Opt(conf.threads, "8")["-n"]["--threads"]("Number of server threads to spawn (default system)") |
      Opt(logBuffer, "3")["-g"]["--buffer"]("Buffer size for async logging (default 3)") |
      Opt(conf.logLevel, "info")["-l"]["--log-level"]("Log level to use [debug|info|warn|critical] (default info).") |
      Opt(console, "false")["-i"]["--console"]("Log to console (default false)") |
      Opt(dir, "logs/")["-o"]["--dir"]("Log directory (default logs/)");

  auto result = options.parse(clara::Args(argc, argv));
  if ( !result )
  {
    std::cerr << "Error in command line: " << result.errorMessage() << std::endl;
    exit( 1 );
  }

  if ( help )
  {
    options.writeToStream( std::cout );
    exit( 0 );
  }

  std::cout << "Starting server with options\n" <<
    "console: " << std::boolalpha << console << '\n' <<
    "logBuffer: " << logBuffer << '\n' <<
    "options: " << conf.str() << '\n' <<
    "dir: " << dir << '\n';

  if ( conf.logLevel == "debug" ) nanolog::set_log_level( nanolog::LogLevel::DEBUG );
  else if ( conf.logLevel == "info" ) nanolog::set_log_level( nanolog::LogLevel::INFO );
  else if ( conf.logLevel == "warn" ) nanolog::set_log_level( nanolog::LogLevel::WARN );
  else if ( conf.logLevel == "critical" ) nanolog::set_log_level( nanolog::LogLevel::CRIT );
  nanolog::initialize( nanolog::NonGuaranteedLogger( logBuffer ), dir, "batch-api", console );

  return spt::start();
}
