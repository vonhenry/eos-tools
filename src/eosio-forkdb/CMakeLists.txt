

add_executable(eosio-forkdb forkdb.cpp)
target_link_libraries(eosio-forkdb ${LIBRARIES})

install( TARGETS eosio-forkdb
        RUNTIME DESTINATION /usr/local/eosio/bin )