#include "CBase.hpp"

int main(int argc, char const *argv[])
{
    CThreadPool pool( 10, 100 );

    pool.start();

    

    getchar();

    return 0;
}
