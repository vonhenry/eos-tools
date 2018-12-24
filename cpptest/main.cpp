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

namespace eosio {  namespace chain { namespace resource_limits {


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
      EOS_ASSERT(r.numerator == T(0) || std::numeric_limits<T>::max() / r.numerator >= value, rate_limiting_state_inconsistent, "Usage exceeds maximum value representable after extending for precision");
      return (value * r.numerator) / r.denominator;
   }

   template<typename UnsignedIntType>
   constexpr UnsignedIntType integer_divide_ceil(UnsignedIntType num, UnsignedIntType den ) {
      return (num / den) + ((num % den) > 0 ? 1 : 0);
   }


   template<typename LesserIntType, typename GreaterIntType>
   constexpr bool is_valid_downgrade_cast =
         std::is_integral<LesserIntType>::value &&  // remove overloads where type is not integral
         std::is_integral<GreaterIntType>::value && // remove overloads where type is not integral
         (std::numeric_limits<LesserIntType>::max() <= std::numeric_limits<GreaterIntType>::max()); // remove overloads which are upgrades not downgrades

/**
 * Specialization for Signedness matching integer types
 */
   template<typename LesserIntType, typename GreaterIntType>
   constexpr auto downgrade_cast(GreaterIntType val) ->
   std::enable_if_t<is_valid_downgrade_cast<LesserIntType,GreaterIntType> && std::is_signed<LesserIntType>::value == std::is_signed<GreaterIntType>::value, LesserIntType>
   {
      const GreaterIntType max = std::numeric_limits<LesserIntType>::max();
      const GreaterIntType min = std::numeric_limits<LesserIntType>::min();
      EOS_ASSERT( val >= min && val <= max, rate_limiting_state_inconsistent, "Casting a higher bit integer value ${v} to a lower bit integer value which cannot contain the value, valid range is [${min}, ${max}]", ("v", val)("min", min)("max",max) );
      return LesserIntType(val);
   };

/**
 * Specialization for Signedness mismatching integer types
 */
   template<typename LesserIntType, typename GreaterIntType>
   constexpr auto downgrade_cast(GreaterIntType val) ->
   std::enable_if_t<is_valid_downgrade_cast<LesserIntType,GreaterIntType> && std::is_signed<LesserIntType>::value != std::is_signed<GreaterIntType>::value, LesserIntType>
   {
      const GreaterIntType max = std::numeric_limits<LesserIntType>::max();
      const GreaterIntType min = 0;
      EOS_ASSERT( val >= min && val <= max, rate_limiting_state_inconsistent, "Casting a higher bit integer value ${v} to a lower bit integer value which cannot contain the value, valid range is [${min}, ${max}]", ("v", val)("min", min)("max",max) );
      return LesserIntType(val);
   };

/**
 *  This class accumulates and exponential moving average based on inputs
 *  This accumulator assumes there are no drops in input data
 *
 *  The value stored is Precision times the sum of the inputs.
 */

   const static uint64_t Precision = 1000*1000;
   static constexpr uint64_t max_raw_value = std::numeric_limits<uint64_t>::max() / Precision;

   struct accumulator
   {
      accumulator():last_ordinal(0),value_ex(0){}
      uint32_t   last_ordinal;  ///< The ordinal of the last period which has contributed to the average
      uint64_t   value_ex;      ///< The current average pre-multiplied by Precision

      uint64_t average() const {
         return integer_divide_ceil(value_ex, Precision);
      }

      void add( uint64_t units, uint32_t ordinal, uint32_t window_size ) {
//      eosio_assert( units <= max_raw_value, "Usage exceeds maximum value representable after extending for precision");

         auto value_ex_contrib = downgrade_cast<uint64_t>(integer_divide_ceil((uint128_t)units * Precision, (uint128_t)window_size));
//      EOS_ASSERT(std::numeric_limits<decltype(value_ex)>::max() - value_ex >= value_ex_contrib, rate_limiting_state_inconsistent, "Overflow in accumulated value when adding usage!");

         if( last_ordinal != ordinal ) {
//         EOS_ASSERT( ordinal > last_ordinal, resource_limit_exception, "new ordinal cannot be less than the previous ordinal" );
            if( (uint64_t)last_ordinal + window_size > (uint64_t)ordinal ) {
               const auto delta = ordinal - last_ordinal; // clearly 0 < delta < window_size
               const auto decay = make_ratio(
                     (uint64_t)window_size - delta,
                     (uint64_t)window_size
               );

               value_ex = value_ex * decay;
            } else {
               value_ex = 0;
            }

            last_ordinal = ordinal;
         }

         value_ex += value_ex_contrib;
      }
   };


}}}


int main()
{
   eosio::chain::resource_limits::exponential_moving_average_accumulator acc;


   acc.add( 50, 100, 200);
   acc.add( 50, 200, 200);


   acc.add( 50, 500, 200);
   acc.add( 50, 550, 200);


   return 0;


}












