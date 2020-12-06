file(GLOB_RECURSE version-history_SRCS *.cpp *.cc *.hpp *.h)
set(version-history_SRCS ${version-history_SRCS})
add_executable(version-history-api ${version-history_SRCS} )

function(make_includable input_file output_file)
  file(READ ${input_file} content)
  set(delim "for_c++_include")
  set(content "R\"${delim}(\n${content})${delim}\"")
  file(WRITE ${output_file} "${content}")
endfunction(make_includable)
make_includable(../openapi.yaml /tmp/version-history-api.yaml)

target_compile_definitions(version-history-api
  PRIVATE ${LIBMONGOCXX_STATIC_DEFINITIONS}
)

if (APPLE)
  target_link_libraries(version-history-api
    PRIVATE ${LIBMONGOCXX_STATIC_LIBRARIES} Boost::boost Boost::iostreams -lnghttp2_asio ${OPENSSL_LIBRARIES}
  )
else()
  target_link_libraries(version-history-api
    PRIVATE ${LIBMONGOCXX_STATIC_LIBRARIES} Boost::boost Boost::iostreams -lnghttp2 -lnghttp2_asio ${OPENSSL_LIBRARIES}
  )
endif()

install(TARGETS version-history-api DESTINATION bin)
