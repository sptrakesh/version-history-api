//
// Created by Rakesh on 11/03/2020.
//

#include "contextholder.h"
#include "server.h"
#include "db/storage.h"
#include "http/handlers.h"
#include "log/NanoLog.h"
#include "model/config.h"

#include <cstdlib>

#include <nghttp2/asio_http2_server.h>

int spt::start()
{
  const auto& conf = model::Configuration::instance();
  auto& ch = ContextHolder::instance();

  db::init();

  boost::system::error_code ec;
  nghttp2::asio_http2::server::http2 server;
  server.num_threads( conf.threads );

  server.handle( "/", &spt::http::handleRoot );
  server.handle( "/version/history/list/", &spt::http::handleList );
  server.handle( "/version/history/document/", &spt::http::handleDocument );
  server.handle( "/version/history/entity/", &spt::http::handleEntity );
  server.handle( "/version/history/revert/", &spt::http::handleRevert );

  boost::asio::signal_set signals( ch.ioc, SIGINT, SIGTERM );
  signals.async_wait( [&ch](auto const&, int ) { ch.ioc.stop(); } );

  boost::asio::signal_set sigpipe( ch.ioc, SIGPIPE );
  sigpipe.async_wait( [](auto const& ec, int )
    {
      LOG_CRIT << "Sigpipe received. " << ec.message();
    }
  );

  if ( server.listen_and_serve( ec, "0.0.0.0", conf.port, true ) ) {
    LOG_CRIT << "error: " << ec.message();
    return 1;
  }

  ch.ioc.run();

  LOG_INFO << "Stopping server";
  server.stop();
  server.join();

  return 0;
}

