#include <iostream>
#include <chrono>
#include "CBase.hpp"

using namespace std::chrono_literals;

static ::Base::INT s_mark = 0;
::std::mutex s_mutex;

void task()
{
    //::std::lock_guard< ::std::mutex > guard( s_mutex );

    ::std::cout << "thread: " << ::std::this_thread::get_id() << ", mark: " << s_mark++ << ::std::endl;

    std::this_thread::sleep_for(2s);
}

int main(int argc, char const *argv[])
{
    ::Base::CThreadPool pool( 10, 100 );

    pool.start();

    for ( ::Base::INT i=0; i < 100; ++i ) {
        pool.addTask( task );
    }

    pool.stop();

    getchar();

    return 0;
}
