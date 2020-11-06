//
// Created by Rakesh on 2019-05-14.
//

#pragma once

#include <optional>
#include <string>
#include <thread>

namespace spt::model
{
  class Configuration
  {
  public:
    static Configuration& instance()
    {
      static Configuration instance;
      return instance;
    }

    Configuration(const Configuration&) = delete;
    Configuration& operator=(const Configuration&) = delete;

    std::string mongoServiceHost{ "mongo-service" };
    std::string mongoServicePort{ "2000" };
    std::string versionDatabase{ "versionHistory" };
    std::string versionCollection{ "entities" };
    std::string metricsDatabase{ "metrics" };
    std::string metricsCollection{ "api" };
    std::string logLevel{ "info" };
#if __APPLE__
    std::string port{ "6106" };
#else
    std::string port{ "6100" };
#endif
    uint32_t threads = std::thread::hardware_concurrency();

    [[nodiscard]] std::string str() const;

  private:
    Configuration() = default;
    ~Configuration() = default;
  };
}
