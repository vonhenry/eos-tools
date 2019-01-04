#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <chrono>
#include <iostream>
#include <vector>


#include <eosio/chain/types.hpp>

#include <eosio/chain/controller.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/block.hpp>
#include <eosio/chain/types.hpp>
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

using std::vector;

using boost::asio::ip::tcp;
using boost::asio::ip::address_v4;
using boost::asio::ip::host_name;
using boost::intrusive::rbtree;
using boost::multi_index_container;

using fc::time_point;
using fc::time_point_sec;
using eosio::chain::transaction_id_type;
using eosio::chain::name;
using mvo = fc::mutable_variant_object;
namespace bip = boost::interprocess;


struct lwc_section_info {
   lwc_section_info(int a, int b):first(a),last(b){}
   int                   first;
   int                   last;
};

typedef multi_index_container<
   lwc_section_info,
   indexed_by<
      ordered_unique<
         tag< by_id >,
         member < lwc_section_info,
            int,
            &lwc_section_info::first > >
   >
>
   ibc_section_index;

ibc_section_index             local_sections;

int main()
{
   lwc_section_info a(1,1), b(2,2), c(3,3), d(4,4),e(4,5);

   auto [aa,bb ] = local_sections.insert(e);

   local_sections.erase(aa);

   auto [cc,dd ] = local_sections.insert(d);

   local_sections.insert(a);
   local_sections.insert(b);
   local_sections.insert(c);

   local_sections.insert(d);
   local_sections.insert(d);
   local_sections.insert(c);
   local_sections.insert(c);
   local_sections.insert(a);



//   local_sections.insert(local_sections.end(),a);
//   local_sections.insert(local_sections.end(),b);
//   local_sections.insert(local_sections.end(),c);
//   local_sections.insert(local_sections.end(),d);

   for( auto it = local_sections.rbegin(); it != local_sections.rend(); ++it ){
      ilog("======${n1},${n2}",("n1",it->first)("n2",it->last));
   }

   return 0;


}














