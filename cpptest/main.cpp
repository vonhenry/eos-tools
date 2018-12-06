#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <chrono>
#include <iostream>

using namespace boost::asio;

int main()
{
   io_service ioservice;

   steady_timer timer1{ioservice, std::chrono::seconds{3}};
   timer1.wait();
   std::cout << "--1--" << std::endl;

   steady_timer timer2{ioservice, std::chrono::seconds{3}};
   timer2.wait();
   std::cout << "--2--" << std::endl;

}































