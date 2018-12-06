#include <iostream>

#include <eosio/chain/incremental_merkle.hpp>

using namespace eosio::chain;
using namespace std;



int main() {
   const int NN = 20;

//   std::srand(std::time(nullptr));
   std::srand(0);
   vector<digest_type> dv;

   for (int n = 0; n < NN; n++) {
      string s;
      for (int i = 0; i < 10; i++) { s.append(to_string(std::rand())); }
      dv.push_back(digest_type{s});
      s = "";
   }

   // -- 1 --
   incremental_merkle im;
   digest_type root;
   for (int n = 0; n < NN; n++) {
      root = im.append(dv[n]);
      cout << "dv[" << n << "] = " << string(dv[n]) << endl;
      cout << "root = " << string(root) << endl << endl;
   }

   cout << string(root)  << endl;

   // -- 2 --

   cout << string(merkle(dv))  << endl;

   return 0;
}




//      cout << "nodes : " << im._node_count << endl;
//      for(auto& n: im._active_nodes){
//         cout << string(n) << endl;
//      }
































