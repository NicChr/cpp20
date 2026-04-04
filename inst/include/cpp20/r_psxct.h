#ifndef CPP20_R_PSXCT_H
#define CPP20_R_PSXCT_H

#include <cpp20/r_concepts.h>
#include <cpp20/r_protect.h>
#include <cpp20/r_str.h>
#include <cstdint>
#include <chrono> // For r_date/r_psxt

namespace cpp20 {
    
// R date-time that captures the number of seconds since epoch (1st Jan 1970)
template <typename T>
requires (any<T, r_int64, r_dbl>)
struct r_psxct_t : T {

    using inherited_type = T;

    r_psxct_t() : T{0} {}
    template <CppMathType U>
    explicit constexpr r_psxct_t(U seconds_since_epoch) : T{seconds_since_epoch} {}
    explicit constexpr r_psxct_t(T seconds_since_epoch) : T{seconds_since_epoch} {}

    // Construct r_date year/month/day
    explicit r_psxct_t(
    int32_t year, uint32_t month, uint32_t day, 
    uint32_t hour, uint32_t minute, uint32_t second
    ) : T(internal::get_seconds_since_epoch(year, month, day, hour, minute, second)) {}

    private: 
    
    auto chrono_tp() const {
    return std::chrono::time_point{
        std::chrono::sys_seconds{std::chrono::seconds{static_cast<int64_t>(T::value)}}
    };
    }

    // Decomposed date + time-of-day
    auto chrono_ymd() const {
    using namespace std::chrono;
    auto tp = chrono_tp();
    auto dp = floor<days>(tp);
    return year_month_day{dp};
    }

    auto chrono_hms() const {
    using namespace std::chrono;
    auto tp = chrono_tp();
    auto dp = floor<days>(tp);
    return hh_mm_ss{tp - dp};
    }

    public: 

    r_str datetime_str() const {
    auto ymd = chrono_ymd();
    auto hms = chrono_hms();
    char buf[30];
    std::snprintf(buf, sizeof(buf),
        "%04d-%02u-%02u %02u:%02u:%02u",
        static_cast<int32_t>(ymd.year()),
        static_cast<uint32_t>(ymd.month()),
        static_cast<uint32_t>(ymd.day()),
        static_cast<uint32_t>(hms.hours().count()),
        static_cast<uint32_t>(hms.minutes().count()),
        static_cast<uint32_t>(hms.seconds().count())
    );
    return r_str(static_cast<const char*>(buf));
    }
};

}

#endif
