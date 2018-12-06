cmake_minimum_required(VERSION 3.12)
project(eos-tools)

set(CMAKE_CXX_STANDARD 14)

include_directories(
        /usr/local/include
        /usr/local/eosio/include
        /usr/local/opt/openssl/include
        /usr/local/eosio/include/wasm-jit
        /usr/local/eosio/include/softfloat
        )

link_directories(
        /usr/local/lib
        /usr/local/eosio/lib
        /usr/local/opt/openssl/lib
        )

list(APPEND LIBRARIES
        eosio_chain
        appbase
        chainbase

        boost_system
        boost_filesystem
        boost_date_time
        boost_chrono
        boost_program_options
        boost_iostreams
        boost_exception

        fc gmp ssl crypto secp256k1 softfloat builtins Platform
        )

add_subdirectory(src)
#add_subdirectory(cpptest)
