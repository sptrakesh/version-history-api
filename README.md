# Version History Api

* [Implementation](#implementation)
* [Endpoints](#endpoints)
    * [Version History](#version-history-api)
        * [List Versions](#list-versions)
        * [Retrieve History Document](#retrieve-history-document)
        * [Retrieve Entity](#retrieve-entity)
        * [Revert Entity](#revert-entity)
    * [CRUD](#crud)
        * [Create Document](#create-document)
        * [Retrieve Document](#retrieve-document)
        * [Update Document](#update-document)
        * [Delete Document](#delete-document)
* [Configuration](#configuration)
* [Acknowledgements](#acknowledgements)

[Simple API](openapi.yaml) for retrieving and viewing *version history* for
entities stored in **MongoDB**.  This is a supporting service for
[mongo-service](https://github.com/sptrakesh/mongo-service) 
and exposes version history for documents via a simple **REST API**.

The Open API specification documentation can be viewed on a running instance by
making a request to `/docs/index.html` (eg. [localhost](http://localhost:6100/docs/index.html)).
**Note:** redirect for `/docs/` to `/docs/index.html` is not implemented, hence
the explicit path must be used.

## Implementation
This is intended as a internal (intranet) application.  There is no authentication
or authorisation implemented or enforced.  Those may be applied as appropriate
by upstream proxies.

This is a HTTP/2 only service.  If using `curl` the `--http2-prior-knowledge`
option is required.  If hosting behind services such as
**[Envoy](https://www.envoyproxy.io/)** (see docker
[stack](docker/stack.yml) and [configuration](docker/envoy.yml)),
clients may use HTTP/1.1 or HTTP/2 to access the endpoints.

Clients may request responses in either **BSON** (`accept: application/bson` header)
or **JSON** (`accept: application/json` header).  In both cases compression
of the response is supported (`accept-encoding: gzip` header).

**Note:** The JSON representation is the standard MongoDB representation of the
source BSON document.  We use the `k_relaxed` mode for generating the JSON.

## Endpoints
Two groups of endpoints are implemented by this service:
* **VersionHistory** - services for interacting with document version history.
* **CRUD** - utility services to perform CRUD operations on documents via the
[mongo-service](https://github.com/sptrakesh/mongo-service).

### Version History API
The following simple endpoints are provided to interact with version history for
documents.

#### List Versions
An endpoint to return basic metadata about versions stored for the specified
document.

```shell script
GET /version/history/list/<database>/<collection>/<document bson oid>
```

A document that contains a `results` array of summary documents will be returned.
We wrap the response into a document to keep it consistent with other responses
(from other endpoints or error messages).

```json
{
  "results":
  [
    {
      "_id": { "$oid": "5f3bc9e29ba4f45f810edf22" },
      "action": "create",
      "created": { "$date": "2020-08-18T12:30:26.659Z" }
    },
    {
      "_id": { "$oid": "5f3bc9e29ba4f45f810edf29" },
      "action": "update",
      "created": { "$date": "2020-08-18T12:30:26.855Z" }
    },
    {
      "_id": { "$oid": "5f3bc9e29ba4f45f810edf2c" },
      "action": "delete",
      "created": { "$date": "2020-08-18T12:30:26.870Z" }
    }
  ]
}
```

#### Retrieve History Document
An endpoint to retrieve a specific history document by its *object id*.

```shell script
GET /version/history/document/<document bson oid>
```

The full version history document will be returned.  Note that the document of
interest is stored under an `entity` sub-document.

```json
{
  "_id": { "$oid": "5f3bc9e29ba4f45f810edf22" },
  "action": "create",
  "collection": "test",
  "created": { "$date": "2020-08-18T12:30:26.659Z" },
  "database": "itest",
  "entity": {
    "_id": { "$oid": "5f3bc9e2502422053e08f9f1" },
    "key": "value"
  }
}
```

#### Retrieve Entity
An endpoint to retrieve an entity by its specific history document *object id*.

```shell script
GET /version/history/entity/<document bson oid>
```

This endpoint returns the entity without its parent wrapper.

```json
{
  "_id": { "$oid": "5f3bc9e2502422053e08f9f1" },
  "key": "value"
}
```

#### Revert Entity
An endpoint to revert an entity to the specified version. The implementation
replaces the current document with the versioned entity.
This is a *true revert* in that the exact same version of the document is restored.
The *version history* document created for the revert (exact same nested entity
as the version being reverted to) will also hold a custom `metadata` sub-document
with a `revertedFrom` property that holds the version history id that was reverted.
This information can be used to track revert actions that were applied on the document.

This endpoint may not be follow the revert policy followed by the client.
Clients may wish to modify certain attributes (modification date etc.) before
reverting.  It should be easy to accomplish that by retrieving the desired
version, modifying as appropriate and saving through the usual means.

```shell script
PUT /version/history/revert/<version document bson oid>/<database>/<collection>/<entity bson oid>
```

**Note:** This is a `PUT` endpoint since data is modified in the database.  No
payload is expected from the client for this API endpoint.

### CRUD
Utility endpoints to perform *CRUD* operations using the `mongo-service`.  This
API is most useful for access from scripting languages where performing raw
socket communication is not preferred.

#### Create Document
Endpoint to create a new document in the specified `database:collection`.  As
required by the `mongo-service` caller must specify a valid `BSON ObjectId` value
in the request payload.  Request (and response) can be encoded as either BSON 
or JSON.  Response can also be compressed using `gzip`.

```shell script
POST /crud/create/<database>/<collection>

{
  "_id": { "$oid": "5f3bc9e29ba4f45f810edf22" },
  "created": { "$date": "2020-08-18T12:30:26.659Z" },
  "string": "string value",
  "number": 1234,
  "double": 1234.56,
  "bool": false
}
```

#### Retrieve Document
Two endpoints are available to retrieve documents.  A simple `GET` request to
retrieve documents by a specified `property` and `value`, or a more robust `POST`
request to execute a specified MongoDB query.

##### Get Request
Make a `GET` request with the `database`, `collection`, `property`, and `value`
as `path` parameters.

```shell script
GET /crud/retrieve/<database>/<collection>/<property>/<value>

{
  "results": [
    {
      "_id": { "$oid": "5f3bc9e29ba4f45f810edf22" },
      "created": { "$date": "2020-08-18T12:30:26.659Z" },
      "string": "string value",
      "number": 1234,
      "double": 1234.56,
      "bool": false
    }
  ]
}
```

##### POST Request
Make a `POST` request with the MongoDB query and options as the request payload.

```shell
POST /crud/query/<database>/<collection>

{
  "query": {
    "string": "string value",
    "number": 1234 
  },
  "options": {
    "limit": 5,
    "sort": {"_id": -1},
    "projection": {"string": 1, "number": 1}
  }
}
```

#### Update Document

#### Delete Document
Make a `DELETE` request with the `database`, `collection`, and the BSON ObjectId
value as `path` parameters to delete a document.

```shell script
DELETE /crud/delete/<database>/<collection>/<bson object id>
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
* `port` - The port the service listens on.  Default `6100`.  Specify via the
`--port` command line option.
* `threads` - The number of Asio I/O threads to use for the service.  Defaults
to the *hardware concurrency* value.  Specify via the `--threads` command line option.
* `logLevel` - The logging level for the service.  One of `critical|warn|info|debug`.
Default `info`.  Specify via the `--log-level` command line option.

## Acknowledgements
* **[nghttp2](https://github.com/nghttp2/nghttp2)** - HTTP/2 server framework.
* **[MongoCXX](http://mongocxx.org/)** - MongoDB C++ driver for BSON library.
* **[NanoLog](https://github.com/Iyengar111/NanoLog)** - Logging framework used
for the server.  I modified the implementation for daily rolling log files.
* **[Clara](https://github.com/catchorg/Clara)** - Command line options parser.
