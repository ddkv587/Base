#include "CBase.hpp"

static INT s_mark = 0;

void task()
{
    s_mark++;
    ::std::cout << "mark: " << s_mark << ::std::endl;
}

int main(int argc, char const *argv[])
{
    CThreadPool pool( 10, 100 );

    pool.start();

    for ( INT i=0; i < 100; ++i ) {
        pool.addTask( task );
    }

    getchar();

    return 0;
}
