#include <iostream>
#include <chrono>
#include <functional>
#include "CBase.hpp"

using namespace std::chrono_literals;

::std::mutex s_mutex;

void task(  void *pData )
{
    ::std::lock_guard< ::std::mutex > guard( s_mutex );

    ::Base::INT index = 0;
    if ( pData )
        index = *( static_cast< ::Base::INT* >( pData ) );

    ::std::cout << "thread: " << ::std::this_thread::get_id() << ", index: " << index << ::std::endl;

    //std::this_thread::sleep_for(1s);
}

int main(int argc, char const *argv[])
{

    // json
    // create an empty structure (null)
    ::Base::JSON j;

    // add a number that is stored as double (note the implicit conversion of j to an object)
    j["pi"] = 3.141;

    // add a Boolean that is stored as bool
    j["happy"] = true;

    // add a string that is stored as std::string
    j["name"] = "Niels";

    // add another null object by passing nullptr
    j["nothing"] = nullptr;

    // add an object inside the object
    j["answer"]["everything"] = 42;

    // add an array that is stored as std::vector (using an initializer list)
    j["list"] = { 1, 0, 2 };

    // add another object (using an initializer list of pairs)
    j["object"] = { {"currency", "USD"}, {"value", 42.99} };
    
    ::std::cout << "=================" << ::std::endl;
    ::std::cout << j.dump() << ::std::endl;

    ::std::cout << "+++++++++++++++++" << ::std::endl;
    std::cout << j.dump(4) << std::endl;

    ::std::cout << "******************" << ::std::endl;
    for ( ::Base::JSON::iterator it = j.begin(); it != j.end(); ++it) {
        ::std::cout << *it << ::std::endl;
    }

    ::std:;cout << "XXXXXXXXXXXXXXXXXXX" << ::std::endl;
    const auto tmp = j[0].get<::Base::FLOAT>();
    ::std::cout << tmp << ::std::endl;

    const auto tmp2 = j["name"].get<::Base::STRING>();
    ::std::cout << tmp2 << ::std::endl;

#if 0
    // sample thread pool
    ::Base::CThreadPool pool( 10, 100 );

    pool.start();

    for ( ::Base::INT i=0; i < 100; ++i ) {
        pool.addTask( task, new ::Base::INT( i ) );
    }

    pool.stop();

    getchar();
#endif

    return 0;
}
