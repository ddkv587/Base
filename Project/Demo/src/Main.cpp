#include <iostream>
#include <chrono>
#include <functional>
#include "CBase.hpp"

using namespace std::chrono_literals;

::std::mutex s_mutex;

void task(  void *pData )
{
    //::std::lock_guard< ::std::mutex > guard( s_mutex );

    ::Base::INT index = 0;
    if ( pData )
        index = *( static_cast< ::Base::INT* >( pData ) );

    ::std::cout << "thread: " << ::std::this_thread::get_id() << ", index: " << index << ::std::endl;

    //std::this_thread::sleep_for(1s);
}

int main(int argc, char const *argv[])
{
    ::Base::CThreadPool pool( 10, 100 );

    pool.start();

    for ( ::Base::INT i=0; i < 100; ++i ) {
        int index = i;
        pool.addTask( task, &index );
    }

    pool.stop();

    getchar();

    return 0;
}
