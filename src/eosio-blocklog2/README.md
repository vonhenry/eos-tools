#include <iostream>

#include <eosio/chain/incremental_merkle.hpp>
#include <eosio/chain/controller.hpp>
#include <eosio/chain/block_log.hpp>
#include <eosio/chain/fork_database.hpp>


#include <fc/io/json.hpp>
#include <fc/filesystem.hpp>
#include <fc/variant.hpp>



using namespace eosio::chain;
using namespace std;



// digest_type   sig_digest()const {
//   auto header_bmroot = digest_type::hash( std::make_pair( header.digest(), blockroot_merkle.get_root() ) );
//   return digest_type::hash( std::make_pair(header_bmroot, pending_schedule_hash) );
//}


// pending_schedule_hash    = digest_type::hash( *header.new_producers );











void task1(){

   block_log blog("/Code/github.com/vchengsong/eos-tools/blog_parser/blog/data/blocks");

   signed_block_ptr     sbp = blog.read_block_by_num(2);
   signed_block_header  sbh = *sbp;
   block_header         bh = sbh;


   private_key_type     prikey{std::string("5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3")};

   digest_type          block_header_digest = bh.digest();


   incremental_merkle                blockroot_merkle;
   blockroot_merkle.append(bh.previous);


   auto header_bmroot = digest_type::hash( std::make_pair( bh.digest(), blockroot_merkle.get_root() ) );


   producer_schedule_type initial_schedule{ 0, {{config::system_account_name, public_key_type{std::string{"EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"}}}} };

   auto pending_schedule_hash = fc::sha256::hash(initial_schedule);

   digest_type d =  digest_type::hash( std::make_pair(header_bmroot, pending_schedule_hash) );



   signature_type       block_header_sign   = prikey.sign(d);

}



int main() {

   task1();



   return 0;
}


