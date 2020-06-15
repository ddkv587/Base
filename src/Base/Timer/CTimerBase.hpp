#ifndef __CTIMERDEFINE_HPP__
#define __CTIMERDEFINE_HPP__

#include <chrono>

namespace Base
{
    class CTimerBase
    {
    public:
        // The duration type.
        typedef std::chrono::steady_clock::duration duration;

        // The duration's underlying arithmetic representation.
        typedef duration::rep rep;

        // The ratio representing the duration's tick period.
        typedef duration::period period;

        // An absolute time point represented using the clock.
        typedef std::chrono::time_point<time_t_clock> time_point;

        // The clock is not monotonically increasing.
        static constexpr bool is_steady = false;

        // Get the current time.
        static time_point now() noexcept
        {
            return time_point() + std::chrono::seconds(std::time(0));
        }

        // Determine how long until the clock should be next polled to determine
        // whether the duration has elapsed.
        static duration to_wait_duration(
            const duration& d)
        {
            if ( d > std::chrono::seconds( 1 ) )
                return d - std::chrono::seconds( 1 );
            else if ( d > std::chrono::seconds( 0 ) )
                return std::chrono::milliseconds( 10 );
            else
                return std::chrono::seconds( 0 );
        }

        // Determine how long until the clock should be next polled to determine
        // whether the absoluate time has been reached.
        static duration to_wait_duration(
            const time_point& t)
        {
            return to_wait_duration( t - now() );
        }
    };
}
#endif