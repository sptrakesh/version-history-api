FROM sptrakesh/mongocxx as mongo
FROM sptrakesh/nghttp2 as base

COPY --from=mongo /opt/local /opt/local
COPY CMakeLists.txt /opt/spt/version-history-api/CMakeLists.txt
COPY src /opt/spt/version-history-api/src

WORKDIR /opt/spt/version-history-api/build
RUN apk --no-cache add snappy-dev \
  && rm -rf * \
  && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/opt/local .. \
  && make -j8 \
  && make install

FROM cppruntime

RUN addgroup spt \
  && adduser -DS -g "SPT User" -G spt -s /bin/sh -h /home/spt spt \
  && mkdir -p /opt/spt/logs /opt/spt/data \
  && chown spt:spt /opt/spt/logs /opt/spt/data

COPY --from=base /opt/spt/bin /opt/spt/bin
COPY docker/scripts/entrypoint.sh /opt/spt/bin

WORKDIR /opt/spt
ENV LD_LIBRARY_PATH=/usr/lib:/opt/local/lib
USER spt
ENTRYPOINT [ "/opt/spt/bin/entrypoint.sh" ]
