#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <chrono>
#include <iostream>


#include <eosio/chain/types.hpp>

#include <eosio/chain/controller.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/block.hpp>
#include <eosio/chain/contract_types.hpp>


#include <fc/network/message_buffer.hpp>
#include <fc/network/ip.hpp>
#include <fc/io/json.hpp>
#include <fc/io/raw.hpp>
#include <fc/log/appender.hpp>
#include <fc/container/flat.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/crypto/rand.hpp>
#include <fc/exception/exception.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/intrusive/set.hpp>

using mvo = fc::mutable_variant_object;

using namespace boost::asio;
using namespace eosio::chain;

struct lwc_init_message {
   lwc_init_message():header(),active_schedule(),blockroot_merkle(){}
   signed_block_header     header;
   producer_schedule_type  active_schedule;
   incremental_merkle      blockroot_merkle;
};


template<typename T>
T dejsonify(const string& s) {
   return fc::json::from_string(s).as<T>();
}

int main()
{

   auto var = mvo()  ("header",           block_header())
                     ("active_schedule",  producer_schedule_type())
                     ("blockroot_merkle", incremental_merkle());

   auto str =  fc::json::to_string(var);

   std::cout << str << std::endl;

   return 0;

}


