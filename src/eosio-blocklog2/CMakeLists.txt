
add_executable(eosio-blocklog2 blocklog.cpp)
target_link_libraries(eosio-blocklog2 ${LIBRARIES})

install( TARGETS eosio-blocklog2
        RUNTIME DESTINATION /usr/local/eosio/bin )