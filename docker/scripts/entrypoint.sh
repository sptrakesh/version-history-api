#!/bin/sh

LOGDIR=/opt/spt/logs

Check()
{
  echo "Checking if $MONGO_SERVICE_HOST is available"
  for i in $(seq 1 30)
  do
    nc -z $MONGO_SERVICE_HOST $MONGO_SERVICE_PORT
    status=$?
    if [ $status -ne 0 ]
    then
      echo "[$i] Mongo Service $MONGO_SERVICE_HOST:$MONGO_SERVICE_PORT not available ($status).  Sleeping 1s..."
      sleep 1
    else
      return 0
    fi
  done
}

Required()
{
  if [ -z "$MONGO_SERVICE_HOST" ]
  then
    echo "MONGO_SERVICE_HOST not defined."
    exit 1
  fi
}

Defaults()
{
  if [ -z "$PORT" ]
  then
    PORT=6100
    echo "PORT not set.  Will default to $PORT"
  fi

  if [ -z "$THREADS" ]
  then
    THREADS=8
    echo "THREADS not set.  Will default to $THREADS"
  fi

  if [ -z "$LOG_LEVEL" ]
  then
    LOG_LEVEL="info"
    echo "LOG_LEVEL not set.  Will default to $LOG_LEVEL"
  fi

  if [ -z "$MONGO_SERVICE_PORT" ]
  then
    MONGO_SERVICE_PORT="2000"
    echo "MONGO_SERVICE_PORT not set.  Will default to $MONGO_SERVICE_PORT"
  fi

  if [ -z "$VERSION_DATABASE" ]
  then
    VERSION_DATABASE="versionHistory"
    echo "VERSION_DATABASE not set.  Will default to $VERSION_DATABASE"
  fi

  if [ -z "$VERSION_COLLECTION" ]
  then
    VERSION_COLLECTION="entities"
    echo "VERSION_COLLECTION not set.  Will default to $VERSION_COLLECTION"
  fi

  if [ -z "$METRICS_DATABASE" ]
  then
    METRICS_DATABASE="metrics"
    echo "METRICS_DATABASE not set.  Will default to $METRICS_DATABASE"
  fi

  if [ -z "$METRICS_COLLECTION" ]
  then
    METRICS_COLLECTION="api"
    echo "METRICS_COLLECTION not set.  Will default to $METRICS_COLLECTION"
  fi
}

Service()
{
  if [ ! -d $LOGDIR ]
  then
    mkdir -p $LOGDIR
  fi

  echo "Starting up Version History API service on port $PORT"
  /opt/spt/bin/version-history-api --console true --log-level $LOG_LEVEL --dir ${LOGDIR}/ \
    --port $PORT --threads $THREADS \
    --mongo-service-host $MONGO_SERVICE_HOST \
    --mongo-service-port $MONGO_SERVICE_PORT \
    --version-database $VERSION_DATABASE \
    --version-collection $VERSION_COLLECTION \
    --metrics-database $METRICS_DATABASE \
    --metrics-collection $METRICS_COLLECTION
}

Required && Defaults && Check && Service
