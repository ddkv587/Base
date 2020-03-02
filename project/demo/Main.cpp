#include <iostream>
#include "CBase.hpp"

static Base::INT s_mark = 0;

void task()
{
    s_mark++;
    ::std::cout << "mark: " << s_mark << ::std::endl;
}

int main(int argc, char const *argv[])
{
    Base::CThreadPool pool( 10, 100 );

    pool.start();

    for ( Base::INT i=0; i < 100; ++i ) {
        pool.addTask( task );
    }

    getchar();

    return 0;
}
