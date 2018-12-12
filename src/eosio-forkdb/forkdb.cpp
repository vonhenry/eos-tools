/**
 *  @file
 *  @copyright defined in eosio/LICENSE.txt
 */
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/chain/block_log.hpp>
#include <eosio/chain/config.hpp>
#include <eosio/chain/reversible_block_object.hpp>
#include <eosio/chain/fork_database.hpp>

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
optional<chainbase::database> dumy; //非常奇怪的错误，在有些MAC上编译必须要有此声明，不然报链接的错误；

struct forkdb {
   forkdb()
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
   uint32_t                         pack_header_from;
   uint32_t                         pack_header_interval;
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


void forkdb::read_log() {
   fork_database fork_db(blocks_dir);
   const auto end = fork_db.head();
   EOS_ASSERT( end, block_log_exception, "No blocks found in forkdb." );

   block_state_ptr first = end;
   while(fork_db.get_block(first->header.previous) != block_state_ptr()){
      first = fork_db.get_block(first->header.previous);
   }

   std::cout << "forkdb.bat contains " << end->block_num - first->block_num + 1 << " block(s): [ "
             << first->block_num << " - " << end->block_num << " ]" << std::endl;
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
   uint32_t block_num = (first_block < first->block_num) ? first->block_num : first_block;
   block_state_ptr next;
   fc::variant pretty_output;
   const fc::microseconds deadline = fc::seconds(10);

   auto print_block = [&](block_state_ptr& next) {
      abi_serializer::to_variant(*next,
                                 pretty_output,
                                 []( account_name n ) { return optional<abi_serializer>(); },
                                 deadline);
      const auto block_id = next->id;
      const uint32_t ref_block_prefix = block_id._hash[1];
      const auto enhanced_object = fc::mutable_variant_object
            ("block_num",next->block_num)
            ("id", block_id)
            ("ref_block_prefix", ref_block_prefix)
            (pretty_output.get_object());
      fc::variant v(std::move(enhanced_object));
      if (no_pretty_print)
         fc::json::to_stream(*out, v, fc::json::stringify_large_ints_and_doubles);
      else
         *out << fc::json::to_pretty_string(v) << "\n";

      if(true){
         auto n = *next;
         print_packed_data( out, "header",n.header);
         print_hex(out,"pending_schedule_hash",n.pending_schedule_hash.data(),n.pending_schedule_hash.data_size());
         print_var(out,"pending_schedule",n.pending_schedule);
         print_var(out,"active_schedule",n.active_schedule);
         print_var(out,"blockroot_merkle",n.blockroot_merkle);
         print_var(out,"confirm_count",n.confirm_count);

         print_var(out,"sig_digest",n.sig_digest());

         print_hex(out,"header_digest",n.header.digest().data(),n.header.digest().data_size());
         print_var(out,"producer_signature",n.header.producer_signature);
         print_packed_data( out, "packed producer_signature",n.header.producer_signature);
      }
   };


   if( pack_header_from == 0 ){
      bool contains_obj = false;
      while((block_num <= last_block) && (next = fork_db.get_block_in_current_chain_by_num( block_num ))) {
         if (as_json_array && contains_obj)
            *out << ",";
         print_block(next);
         ++block_num;
         contains_obj = true;
      }
   } else {
      block_num = pack_header_from;
      std::vector<signed_block_header> headers;
      while((block_num <= pack_header_from + pack_header_interval) && (next = fork_db.get_block_in_current_chain_by_num(  block_num ))) {
         headers.push_back(next->header);
         ++block_num;
      }
      bytes s = fc::raw::pack(headers);
      print_hex(out,"packed_headers", s.data(), s.size());
   }

   if (as_json_array)
      *out << "]";
}

void forkdb::set_program_options(options_description& cli)
{
   cli.add_options()
         ("blocks-dir,d", bpo::value<bfs::path>()->default_value("forkdb.dat"),
          "the forkdb file path (absolute path or relative to the current directory)")
         ("output-file,o", bpo::value<bfs::path>(),
          "the file to write output to (absolute or relative path).  If not specified then output is to stdout.")
         ("first,f", bpo::value<uint32_t>(&first_block)->default_value(1),
          "the first block number to parse")
         ("last,l", bpo::value<uint32_t>(&last_block)->default_value(std::numeric_limits<uint32_t>::max()),
          "the last block number (inclusive) to parse")
         ("no-pretty-print", bpo::bool_switch(&no_pretty_print)->default_value(false),
          "Do not pretty print the output.  Useful if piping to jq to improve performance.")
         ("as-json-array", bpo::bool_switch(&as_json_array)->default_value(false),
          "Print out json blocks wrapped in json array (otherwise the output is free-standing json objects).")
         ("info,i", bpo::bool_switch(&info)->default_value(false),
          "Only print the first and last block number in forkdb of current chain.")
         ("pack-header-from", bpo::value<uint32_t>(&pack_header_from)->default_value(0),
          "Print packed headers.")
         ("pack-header-interval", bpo::value<uint32_t>(&pack_header_interval)->default_value(10),
          "Print packed headers.")
         ("help,h", "Print this help message and exit.")
         ;

}

fc::path tmp_dir;

void forkdb::initialize(const variables_map& options) {
   try {
      auto bld = options.at( "blocks-dir" ).as<bfs::path>();
      if( bld.is_relative())
         blocks_dir = bfs::current_path() / bld;
      else
         blocks_dir = bld;

      tmp_dir = blocks_dir / "tmp";
      if (!fc::is_directory(tmp_dir))
         fc::create_directories(tmp_dir);
      auto tmp_file = tmp_dir / config::forkdb_filename;

      if (exists(tmp_file))
         remove(tmp_file);
      copy_file(blocks_dir/config::forkdb_filename, tmp_dir/config::forkdb_filename);

      blocks_dir = tmp_dir;

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
   std::ios::sync_with_stdio(false); // for potential performance boost for large forkdb files
   options_description cli ("bos-forkdb command line options");
   try {
      forkdb fdb;
      fdb.set_program_options(cli);
      variables_map vmap;
      bpo::store(bpo::parse_command_line(argc, argv, cli), vmap);
      bpo::notify(vmap);
      if (vmap.count("help") > 0) {
         cli.print(std::cerr);
         return 0;
      }
      fdb.initialize(vmap);
      fdb.read_log();
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

   fc::remove_all(tmp_dir);

   return 0;
}



