find_package(Qt5 COMPONENTS Core Network REQUIRED)
find_package(Qt5Test REQUIRED)
# Auto generate moc files
set(CMAKE_AUTOMOC ON)

file( GLOB_RECURSE api_SRCS *.cpp *.h )

set( api_SRCS ${api_SRCS} )

# Create the application
add_executable(apiTest ${api_SRCS} )

target_link_libraries(apiTest
  PRIVATE ${LIBMONGOCXX_STATIC_LIBRARIES} Qt5::Core Qt5::Network Qt5::Test Boost::boost ${OPENSSL_LIBRARIES} -lz
)

