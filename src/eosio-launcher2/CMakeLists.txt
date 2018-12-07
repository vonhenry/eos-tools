add_executable( eosio-launcher2 main.cpp )

configure_file(config.hpp.in config.hpp ESCAPE_QUOTES)
target_include_directories(eosio-launcher2 PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(eosio-launcher2 ${LIBRARIES} )

install( TARGETS eosio-launcher2
        RUNTIME DESTINATION /usr/local/eosio/bin )