#include <iostream>

#include <eosio/chain/incremental_merkle.hpp>
#include <eosio/chain/block_header.hpp>

using namespace eosio::chain;
using namespace std;

std::vector<char> to_vector(string src){
   std::vector<char> r;
   for(int i=0;i<src.length();i+=2){
      char first = src[i] - '0';
      char second = src[i+1] - '0';
      char result = (first << 4) + second;
      r.push_back(result);
   }
   return r;
}


bool vector_eq(std::vector<char> v1,std::vector<char> v2){
   if(v1.size()!=v2.size())
      return false;

   for(int i=0;i<v1.size();i++){
      if(v1[i]!=v2[i])
         return false;
   }

   return true;

}






int main() {

//
//   std::vector<int> src{1,2,3,4,5};
//   bytes s = fc::raw::pack(src);
//
   std::vector<signed_block_header> headers_src{signed_block_header{}};

   auto saaa = sizeof(signature_type);

   auto bbbbb = sizeof(public_key_type);

   auto ss = sizeof(signed_block_header);
   std::vector<char> hs = fc::raw::pack(headers_src);

   auto s = hs.size();

   cout << fc::to_hex(hs.data(),hs.size()) << "\n";

   string hhh = "03e6c81c470000000000ea3055000000000001bcf2f448225d099685f14da76803028926af04d2607eafcf609c265c0000000000000000000000000000000000000000000000000000000000000000747d103e24c96deb1beebc13eb31f7c2188126946c8677dfd1691af9f9c03ab1000000000000002012c4b681deb1646407a8e1a116da9af6c2dc2eb0c09235201625f2b55d7ad97a0f3e604dbacc285fb880d71279b29f8becce3ec35d1bb639df723921cb965247e7c81c470000000000ea3055000000000002b6dda92cdecfa3eb92205acd516d439ba256c8444bce5755c12b7737000000000000000000000000000000000000000000000000000000000000000070913048c1bd6bbbb4ee02321a01ad01314af13ec0d0da2197347c84c493d409000000000000001f317db9d74c2e81e486bc7f021c57a40633f7dd73700f7f063fc012531e8f6c524ce26da0847b6cbaabf145a95fff00e8149e407ee34649e7e843922ec7eb609ce8c81c470000000000ea3055000000000003472b6987ef3c4a9cbb128271281ee8c0bafd623848d71b1b41348934000000000000000000000000000000000000000000000000000000000000000058f4783013d61dc60baf21ef4fc65968969301076d0878f217e3c11b7c314c7700000000000000203f762c22992329a3aaabc8ff80cb49e2bca3c6cbff178b30b75ea87f318b843319b9bc762ff4b2754e9f52dc74bc4a40b361811902660aa486b2ffa9437df299";
   std::vector<char> headerstr = to_vector(hhh);


//   bool b = vector_eq(hs,headerstr);


//   std::vector<signed_block_header> headers = fc::raw::unpack<std::vector<signed_block_header>>(hs);
   std::vector<signed_block_header>  headers = fc::raw::unpack<std::vector<signed_block_header>>(headerstr);

   int bbb =0 ;

//
//
//   string h = "03e6c81c470000000000ea3055000000000001bcf2f448225d099685f14da76803028926af04d2607eafcf609c265c0000000000000000000000000000000000000000000000000000000000000000747d103e24c96deb1beebc13eb31f7c2188126946c8677dfd1691af9f9c03ab1000000000000002012c4b681deb1646407a8e1a116da9af6c2dc2eb0c09235201625f2b55d7ad97a0f3e604dbacc285fb880d71279b29f8becce3ec35d1bb639df723921cb965247e7c81c470000000000ea3055000000000002b6dda92cdecfa3eb92205acd516d439ba256c8444bce5755c12b7737000000000000000000000000000000000000000000000000000000000000000070913048c1bd6bbbb4ee02321a01ad01314af13ec0d0da2197347c84c493d409000000000000001f317db9d74c2e81e486bc7f021c57a40633f7dd73700f7f063fc012531e8f6c524ce26da0847b6cbaabf145a95fff00e8149e407ee34649e7e843922ec7eb609ce8c81c470000000000ea3055000000000003472b6987ef3c4a9cbb128271281ee8c0bafd623848d71b1b41348934000000000000000000000000000000000000000000000000000000000000000058f4783013d61dc60baf21ef4fc65968969301076d0878f217e3c11b7c314c7700000000000000203f762c22992329a3aaabc8ff80cb49e2bca3c6cbff178b30b75ea87f318b843319b9bc762ff4b2754e9f52dc74bc4a40b361811902660aa486b2ffa9437df299";
//   std::vector<char> headerstr(h.begin(),h.end());
//
//   std::vector<signed_block_header> headers = fc::raw::unpack<std::vector<signed_block_header>>(headerstr);
////   fc::raw::unpack(headers);


   vector<digest_type> dv1;
   dv1.push_back(digest_type{"5209329302892569182278441589045987214553276121327238411040043610"});
   dv1.push_back(digest_type{"2002830094190670678012698709261028169396178961481040015178815798"});

   vector<digest_type> dv2;
   dv2.push_back(digest_type{"1370738826195036721363136108356902215481066218711631203411993254"});
//   dv2.push_back(digest_type{"1370738826195036721363136108356902215481066218711631203411993254"});

   cout << string(merkle(dv1))  << endl;
   cout << string(merkle(dv2))  << endl;
   cout << string(merkle(vector<digest_type>{merkle(dv1),merkle(dv2)}))  << endl;
   return 0;
}





