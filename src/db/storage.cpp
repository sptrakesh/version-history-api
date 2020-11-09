//
// Created by Rakesh on 01/10/2019.
//

#include "storage.h"
#include "db/impl/connection.h"
#include "db/impl/impl.h"
#include "db/impl/pool.h"
#include "log/NanoLog.h"

#include <bsoncxx/builder/stream/document.hpp>

#include <chrono>
#include <unordered_map>

namespace spt::db::storage
{
  struct Pool
  {
    static Pool& instance()
    {
      static Pool instance;
      return instance;
    }

    auto acquire() -> auto
    {
      return pool->acquire();
    }

    Pool( const Pool& ) = delete;
    Pool( Pool&& ) = delete;
    Pool& operator=( const Pool& ) = delete;
    Pool& operator=( Pool&& ) = delete;

  private:
    Pool() : pool{ std::make_unique<spt::db::impl::Pool<spt::db::impl::Connection>>( spt::db::impl::create, impl::Configuration{} ) }
    {
    }

    ~Pool() = default;

    std::unique_ptr<spt::db::impl::Pool<spt::db::impl::Connection>> pool;
  };
}

void spt::db::init()
{
  auto& pool = storage::Pool::instance();
  auto client = pool.acquire();
  if ( !client )
  {
    LOG_CRIT << "Error acquiring connection from pool";
  }
}

auto spt::db::summary( std::string_view database,
    std::string_view collection, const bsoncxx::oid& id ) -> model::SummaryResponse
{
  const auto st = std::chrono::steady_clock::now();
  auto& pool = storage::Pool::instance();
  auto proxy = pool.acquire();
  if ( !proxy )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return { std::nullopt, 0, 500 };
  }

  auto& connection = proxy.value().operator*();
  const auto& [response, code] = impl::summary( connection, database, collection, id );

  const auto et = std::chrono::steady_clock::now();
  const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
  return { response, static_cast<int64_t>( delta.count() ), code };
}

auto spt::db::document( const bsoncxx::oid& id ) -> Document
{
  auto& pool = storage::Pool::instance();
  auto proxy = pool.acquire();
  if ( !proxy )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return {std::nullopt, 500};
  }

  auto& connection = proxy.value().operator*();
  return impl::document( connection, id );
}

auto spt::db::entity( const bsoncxx::oid& id ) -> Document
{
  auto& pool = storage::Pool::instance();
  auto proxy = pool.acquire();
  if ( !proxy )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return {};
  }

  auto& connection = proxy.value().operator*();
  return impl::entity( connection, id );
}

auto spt::db::revert( const bsoncxx::oid& history,
    std::string_view database, std::string_view collection,
    const bsoncxx::oid& entity ) -> Document
{
  auto& pool = storage::Pool::instance();
  auto proxy = pool.acquire();
  if ( !proxy )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return {};
  }

  auto& connection = proxy.value().operator*();
  return impl::revert( connection, history, database, collection, entity );
}

void spt::db::save( const model::Metric& metric )
{
  auto& pool = storage::Pool::instance();
  auto proxy = pool.acquire();
  if ( !proxy )
  {
    LOG_CRIT << "Error acquiring connection from pool";
    return;
  }

  auto& connection = proxy.value().operator*();
  impl::save( connection, metric );
}
