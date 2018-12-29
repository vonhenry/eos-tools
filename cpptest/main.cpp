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




template<typename T>
struct ratio {
   static_assert(std::is_integral<T>::value, "ratios must have integral types");
   T numerator;
   T denominator;
};

template<typename T>
ratio<T> make_ratio(T n, T d) {
   return ratio<T>{n, d};
}

template<typename T>
T operator* (T value, const ratio<T>& r) {
//   eosio_assert(r.numerator == T(0) || std::numeric_limits<T>::max() / r.numerator >= value, "Usage exceeds maximum value representable after extending for precision");
   return (value * r.numerator) / r.denominator;
}

const static uint64_t Precision = 100;

struct accumulator
{
   accumulator( uint32_t value_ws, uint32_t times_ws ):
   last_ordinal(0),value_ws(value_ws),times_ws(times_ws),value_ex(0),times_ex(0){}

   uint32_t   last_ordinal;    ///< The ordinal of the last period which has contributed to the average
   uint64_t   value_ex;       
   uint64_t   times_ex;     

   void addvalue( uint64_t value, uint32_t ordinal ) {
      add( value_ex, value, ordinal, value_ws, false );
      add( times_ex, 1, ordinal, times_ws );
   }

private:
   uint32_t   value_ws;
   uint32_t   times_ws;

   void add( uint64_t& src, uint64_t value, uint32_t ordinal, uint32_t window_size, bool update_ordinal = true ) {
      if( last_ordinal != ordinal ) {
//         eosio_assert( ordinal > last_ordinal, "new ordinal cannot be less than the previous ordinal" );
         if( (uint64_t)last_ordinal + window_size > (uint64_t)ordinal ) {
            const auto delta = ordinal - last_ordinal; // clearly 0 < delta < window_size
            const auto decay = make_ratio(
               (uint64_t)window_size - delta,
               (uint64_t)window_size
            );
            src = src * decay;
         } else {
            src = 0;
         }
         if( update_ordinal ){
            last_ordinal = ordinal;
         }
      }
      src += value * Precision;
   }
};


int main()
{
   int day = 3600*24;
   int minute = 60;
   accumulator acc( day, minute );

   for( auto i = 0; i < 24*60*4; ++i){
      acc.addvalue( 50, 60 * (i + 1) );
      if ( i % 60 == 0 ){

         auto res = 300000 * (i/60) - acc.value_ex;
         std::cout << acc.value_ex  << "  " <<  res << "      " << res * 100.0 / (300000 * (i/60)) << "      "<< acc.times_ex << std::endl;
      }
   }

   accumulator bcc( day, minute );

   for( auto i = 0; i < 24*4; ++i){
      bcc.addvalue( 50, 3600 * (i + 1) );
      auto res = 5000 * (i + 1) - bcc.value_ex;
      std::cout << bcc.value_ex  << "  " <<  res << "      " << res * 100.0 / (5000 * (i+1)) << "      "<< bcc.times_ex << std::endl;

   }



   return 0;


}


// 24*60*50 = 72000











