# version-history-api

* [Implementation](#implementation)
* [Endpoints](#endpoints)
    * [List Versions](#list-versions)
    * [Retrieve History Document](#retrieve-history-document)
    * [Retrieve Entity](#retrieve-entity)
    * [Revert Entity](#revert-entity)
* [Configuration](#configuration)
* [Acknowledgements](#acknowledgements)

Simple API for retrieving and viewing *version history* for entities stored
in **MongoDB**.  This is a supporting service for [mongo-service](https://github.com/sptrakesh/mongo-service) 
and exposes via a simple **REST API** version history for documents.

## Implementation
This is intended as a internal (intranet) application.  There is no authentication
or authorisation implemented or enforced.  Those may be applied as appropriate
by upstream proxies.

This is a HTTP/2 only service.  If using `curl` the `--http2-prior-knowledge`
option is required.  If hosting behind services such as **Envoy**, clients may
use HTTP/1.1 or HTTP/2 to access the endpoints.

Clients may request responses in either **BSON** (`accept: application/bson` header)
or **JSON** (`accept: application/json` header).  In both cases compression
of the response is supported (`accept-encoding: gzip` header).

## Endpoints
The following endpoints are exposed by this service.

### List Versions
An endpoint to return some basic metadata about versions stored for the specified
document.

```shell script
GET /version/history/list/<database>/<collection>/<document bson oid>
```

An array of history document metadata will be returned.

```json
[
  {
    "id": "bsonid1",
    "action": "create",
    "time": "timestamp"
  },
  {
    "id": "bsonid2",
    "action": "update",
    "time": "timestamp"
  },
  {
    "id": "bsonid3",
    "action": "update",
    "time": "timestamp"
  }
]
```

### Retrieve History Document
An endpoint to retrieve a specific history document by its *object id*.

```shell script
GET /version/history/document/<document bson oid>
```

The full version history document will be returned.  Note that the document of
interest is stored under an `entity` sub-document.

### Retrieve Entity
An endpoint to retrieve an entity by its specific history document *object id*.

```shell script
GET /version/history/entity/<document bson oid>
```

This endpoint returns the entity without its parent wrapper.

### Revert Entity
An endpoint to revert an entity to the specified version.  This endpoint may
not be of much use depending on the revert policy.  In particular this endpoint
restores the versioned entity (which versioning the current *live* document).
Clients may wish to modify certain attributes (modification date etc.) before
reverting.  It should be easy to accomplish that by retrieving the desired
version, modifying as appropriate and saving through the usual means.

```shell script
GET /version/history/revert/<version document bson oid>/<database>/<collection>/<entity bson oid>
```

## Configuration
The service can be configured via command line parameters.  The following options
are supported:
* `mongoServiceHost` - The service name for the [mongo-service](https://github.com/sptrakesh/mongo-service)
to use to store request metrics. Specify using the `--mongo-host` command line.
Default is `mongo-service`.
* `mongoServicePort` - The port for the `mongo-service`.  Default is `2000`.
Specify using the `--mongo-port` command line option.
* `metricsDatabase` - The database in which api metrics will be saved.  Specify
via the `--metrics-database` command line option.  This option is used only if
`mongoServiceHost` is defined.
* `metricsCollection` - The collection in which api metrics will be saved.  Specify
via the `--metrics-collection` command line option.  This option is used only if
`mongoServiceHost` is defined.
* `port` - The port the service listens on.  Default `6000`.
* `threads` - The number of Asio I/O threads to use for the service.  Defaults
to the *hardware concurrency* value.
* `logLevel` - The logging level for the service.  One of `critical|warn|info|debug`.
Default `info`.  Specify via the `--log-level` command line option.

## Acknowledgements
* [nghttp2](https://github.com/nghttp2/nghttp2)