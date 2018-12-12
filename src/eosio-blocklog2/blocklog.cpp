/**
 *  @file
 *  @copyright defined in eosio/LICENSE.txt
 */
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/chain/block_log.hpp>
#include <eosio/chain/config.hpp>
#include <eosio/chain/reversible_block_object.hpp>

#include <fc/io/json.hpp>
#include <fc/filesystem.hpp>
#include <fc/variant.hpp>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

using namespace eosio::chain;
namespace bfs = boost::filesystem;
namespace bpo = boost::program_options;
using bpo::options_description;
using bpo::variables_map;

struct blocklog {
   blocklog()
   {}

   void read_log();
   void set_program_options(options_description& cli);
   void initialize(const variables_map& options);

   bfs::path                        blocks_dir;
   bfs::path                        output_file;
   uint32_t                         first_block;
   uint32_t                         last_block;
   bool                             no_pretty_print;
   bool                             as_json_array;

   bool                             info;
   bool                             print_packed_header;
   bool                             print_packed_trx;

   uint32_t                         pack_headers_from;
   uint32_t                         pack_headers_interval;
   uint32_t                         pack_headers_times;
};

template <typename T>
void print_packed_data(std::ostream* out, const string& name, const T &v){
   bytes s = fc::raw::pack(v);
   *out << name << '=' << "'\"" << fc::to_hex(s.data(),s.size()) <<"\"'" << "\n";
};

void print_hex(std::ostream* out, const string& name, const char* str, const int size){
   *out << name << '=' << "'\"" << fc::to_hex(str,size)  <<"\"'" << "\n";
};

template <typename T>
void print_var(std::ostream* out, const string& name, const T &v){
   *out << name << '=' << "'"<<  fc::json::to_string(v)  <<"'" << "\n";
};

struct transaction_receipt_type : public transaction_receipt_header {
   packed_transaction trx;
};

FC_REFLECT_DERIVED(transaction_receipt_type, (eosio::chain::transaction_receipt_header), (trx) )

void blocklog::read_log() {
   block_log block_logger(blocks_dir);
   const auto end = block_logger.read_head();
   EOS_ASSERT( end, block_log_exception, "No blocks found in block log" );
   EOS_ASSERT( end->block_num() > 1, block_log_exception, "Only one block found in block log" );

   std::cout << "block.log and block.index contains block(s): [ 1 - " << end->block_num() << " ]" << std::endl;

   optional<chainbase::database> reversible_blocks;
   try {
      reversible_blocks.emplace(blocks_dir / config::reversible_blocks_dir_name, chainbase::database::read_only, config::default_reversible_cache_size);
      reversible_blocks->add_index<reversible_block_index>();
      const auto& idx = reversible_blocks->get_index<reversible_block_index,by_num>();
      auto first = idx.lower_bound(end->block_num());
      auto last = idx.rbegin();
      if (first != idx.end() && last != idx.rend())
         std::cout << "existing reversible block num: [ " << first->get_block()->block_num() << " - " << last->get_block()->block_num() << " ]" << std::endl;
      else {
         elog( "no blocks available in reversible block database: only block_log blocks are available" );
         reversible_blocks.reset();
      }
   } catch( const std::runtime_error& e ) {
      if( std::string(e.what()) == "database dirty flag set" ) {
         elog( "database dirty flag set (likely due to unclean shutdown): only block_log blocks are available" );
      } else if( std::string(e.what()) == "database metadata dirty flag set" ) {
         elog( "database metadata dirty flag set (likely due to unclean shutdown): only block_log blocks are available" );
      } else {
         throw;
      }
   }
   if(info) return;

   std::ofstream output_blocks;
   std::ostream* out;
   if (!output_file.empty()) {
      output_blocks.open(output_file.generic_string().c_str());
      if (output_blocks.fail()) {
         std::ostringstream ss;
         ss << "Unable to open file '" << output_file.string() << "'";
         throw std::runtime_error(ss.str());
      }
      out = &output_blocks;
   }
   else
      out = &std::cout;

   if (as_json_array)
      *out << "[";
   uint32_t block_num = (first_block < 1) ? 1 : first_block;
   signed_block_ptr next;
   fc::variant pretty_output;
   const fc::microseconds deadline = fc::seconds(10);
   auto print_block = [&](signed_block_ptr& next) {
      abi_serializer::to_variant(*next,
                                 pretty_output,
                                 []( account_name n ) { return optional<abi_serializer>(); },
                                 deadline);
      const auto block_id = next->id();
      const uint32_t ref_block_prefix = block_id._hash[1];
      const auto enhanced_object = fc::mutable_variant_object
            ("block_num",next->block_num())
            ("id", block_id)
            ("ref_block_prefix", ref_block_prefix)
            (pretty_output.get_object());
      fc::variant v(std::move(enhanced_object));
      if (no_pretty_print)
         fc::json::to_stream(*out, v, fc::json::stringify_large_ints_and_doubles);
      else
         *out << fc::json::to_pretty_string(v) << "\n";

      if( print_packed_header ){
         signed_block_header header = *next;
         bytes s = fc::raw::pack(header);
         print_hex( out, string("packed_header_") + std::to_string( header.block_num() ), s.data(), s.size());
      }

      if( print_packed_trx ){
         for( auto const & trx : next->transactions ){
            transaction_receipt_type tx;
            tx.net_usage_words = trx.net_usage_words;
            tx.cpu_usage_us = trx.cpu_usage_us;
            tx.status = trx.status;
            tx.trx = trx.trx.get<packed_transaction>();

            bytes s = fc::raw::pack( tx );
            print_hex( out, string("packed_trx_") + trx.trx.get<packed_transaction>().id().str(), s.data(), s.size());
         }
      }
   };

   if( pack_headers_from == 0 ){
      bool contains_obj = false;
      while((block_num <= last_block) && (next = block_logger.read_block_by_num( block_num ))) {
         if (as_json_array && contains_obj)
            *out << ",";
         print_block(next);
         ++block_num;
         contains_obj = true;
      }
      if ( reversible_blocks ) {
         const reversible_block_object* obj = nullptr;
         while( (block_num <= last_block) && (obj = reversible_blocks->find<reversible_block_object,by_num>(block_num)) ) {
            if (as_json_array && contains_obj)
               *out << ",";
            auto next = obj->get_block();
            print_block(next);
            ++block_num;
            contains_obj = true;
         }
      }
   } else {
      block_num = pack_headers_from;
      std::vector<signed_block_header> headers;
      while(( block_num < pack_headers_from + pack_headers_interval ) && ( next = block_logger.read_block_by_num( block_num ))) {
         headers.push_back( *next );
         ++block_num;
      }
      bytes s = fc::raw::pack( headers );
      print_hex( out, string("packed_header") + std::to_string(pack_headers_from) + "-" + std::to_string( pack_headers_interval ), s.data(), s.size());
   }

   if (as_json_array)
      *out << "]";
}

void blocklog::set_program_options(options_description& cli)
{
   cli.add_options()
         ("blocks-dir,d", bpo::value<bfs::path>()->default_value("blocks"),
          "the location of the blocks directory (absolute path or relative to the current directory)")
         ("output-file,o", bpo::value<bfs::path>(),
          "the file to write the block log output to (absolute or relative path).  If not specified then output is to stdout.")
         ("first,f", bpo::value<uint32_t>(&first_block)->default_value(1),
          "the first block number to log")
         ("last,l", bpo::value<uint32_t>(&last_block)->default_value(std::numeric_limits<uint32_t>::max()),
          "the last block number (inclusive) to log")
         ("no-pretty-print", bpo::bool_switch(&no_pretty_print)->default_value(false),
          "Do not pretty print the output.  Useful if piping to jq to improve performance.")
         ("as-json-array", bpo::bool_switch(&as_json_array)->default_value(false),
          "Print out json blocks wrapped in json array (otherwise the output is free-standing json objects).")
         ("info,i", bpo::bool_switch(&info)->default_value(false),
          "Only print the first and last block number in forkdb of current chain.")
         ("print-packed-header", bpo::bool_switch(&print_packed_header)->default_value(false),
          "Print packed header.")
         ("print-packed-trx", bpo::bool_switch(&print_packed_trx)->default_value(false),
          "Print packed transaction.")
         ("pack-headers-from", bpo::value<uint32_t>(&pack_headers_from)->default_value(0),
          "Packed headers from.")
         ("pack-headers-interval", bpo::value<uint32_t>(&pack_headers_interval)->default_value(10),
          "Packed headers amount.")
         ("pack-headers-times", bpo::value<uint32_t>(&pack_headers_times)->default_value(1),
          "Print Packed headers times.")
         ("help,h", "Print this help message and exit.")
         ;

}

void blocklog::initialize(const variables_map& options) {
   try {
      auto bld = options.at( "blocks-dir" ).as<bfs::path>();
      if( bld.is_relative())
         blocks_dir = bfs::current_path() / bld;
      else
         blocks_dir = bld;

      if (options.count( "output-file" )) {
         bld = options.at( "output-file" ).as<bfs::path>();
         if( bld.is_relative())
            output_file = bfs::current_path() / bld;
         else
            output_file = bld;
      }
   } FC_LOG_AND_RETHROW()

}


int main(int argc, char** argv)
{
   std::ios::sync_with_stdio(false); // for potential performance boost for large block log files
   options_description cli ("eosio-blocklog command line options");
   try {
      blocklog blog;
      blog.set_program_options(cli);
      variables_map vmap;
      bpo::store(bpo::parse_command_line(argc, argv, cli), vmap);
      bpo::notify(vmap);
      if (vmap.count("help") > 0) {
         cli.print(std::cerr);
         return 0;
      }
      blog.initialize(vmap);
      blog.read_log();
   } catch( const fc::exception& e ) {
      elog( "${e}", ("e", e.to_detail_string()));
      return -1;
   } catch( const boost::exception& e ) {
      elog("${e}", ("e",boost::diagnostic_information(e)));
      return -1;
   } catch( const std::exception& e ) {
      elog("${e}", ("e",e.what()));
      return -1;
   } catch( ... ) {
      elog("unknown exception");
      return -1;
   }

   return 0;
}




//
//if(0)
//{
//transaction_receipt trxrpt = next->transactions[0];
//packed_transaction ptrx = trxrpt.trx.get<packed_transaction>();
//
//
//*out <<"packed_transaction digest:" << "\n";
//auto dg2 = trxrpt.digest();
//*out << string( dg2 ) << "\n";
//
//
//
//*out <<"packed_transaction:" << "\n";
//bytes s = fc::raw::pack(ptrx);
//*out << fc::to_hex(s.data(),s.size()) << "\n";
//
//*out <<"transaction_receipt_header:" << "\n";
//transaction_receipt_header h = trxrpt;
//
//bytes s2 = fc::raw::pack(h);
//*out << fc::to_hex(s2.data(),s2.size()) << "\n";
//
//*out <<"packed_digest:" << "\n";
//auto dg = ptrx.packed_digest();
//*out << string( dg ) << "\n";
//}