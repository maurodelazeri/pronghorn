#pragma once

#include <cstdint>
#include <cmath>
#include <utility>
#include <limits>

namespace utils {

    // The original bitcoin client does proper, full-precision rounding for all values passed to it via the RPC interface.
    // So, for example, if the value 0.1 is converted to the value "0.099999999999" by your JSON-RPC library,
    // that value will be rounded to the nearest 0.00000001 bitcoin and will be treated as exactly 0.10 BTC.
    inline int64_t doubleToInt64(double value) {
        return (int64_t) (value * 1e8 + (value < 0.0 ? -.5 : .5));
    }

    inline double int64ToDouble(int64_t value) {
        return value / 1e8;
    }

    /** \brief Inverse fast square root
 * \param x The number from which the square root is extracted
 * \return Inverse square root
 */
    inline float inv_sqrt(const float x) {
        if (x == 0) return 0.0;
        const float halfx = 0.5f * x;
        float y = x;
        long i = *(long *) &y;
        i = 0x5f3759df - (i >> 1);
        y = *(float *) &i;
        y = y * (1.5f - (halfx * y * y));
        return y;
    }

    // Percentage change
    template<typename T>
    inline auto percentChange(T a, T b) {
        T result = 0.0;
        result = ((b - a) * 100) / a;
        if (result < 0) result *= -1;
        return result;
    }

    template<typename T>
    inline auto square(T &&x) {
        return x * x;
    }

    template<typename T>
    inline auto cube(T &&x) {
        return x * x * x;
    }

    template<typename T>
    inline auto quad(T &&x) {
        auto x2 = x * x;
        return x2 * x2;
    }

    inline double binomial(uint32_t n, uint32_t k) {
        if (k == 0 || n == k) return 1.0;
        if (k > n) return 0.0;
        if (k > (n - k)) k = n - k;
        if (k == 1) return (double) n;

        double b = 1.0;

        for (uint32_t i = 1; i <= k; ++i) {
            b *= (double) (n - (k - i));
            if (b < 0.0) return -1.0;
            b /= (double) i;
        }
        return b;
    }

    inline double logFactorial(uint32_t n) {
        return std::lgamma(n + 1);
    }

    inline double delta(uint32_t i, uint32_t j) {
        return (i == j) ? 1.0 : 0.0;
    }

    inline double factorial(uint32_t n) {
        double k = 1;
        for (uint32_t i = 2; i <= n; ++i) {
            k *= i;
        }
        return k;
    }

    inline double beta(double x, double y) {
        return std::exp(std::lgamma(x) + std::lgamma(y) - std::lgamma(x + y));
    }

    inline double logBeta(double x, double y) {
        return std::lgamma(x) + std::lgamma(y) - std::lgamma(x + y);
    }

    template<typename T, typename... Ts>
    auto product(T &&x, Ts &&... xs) {
        return std::forward<T>(x) * product(std::forward<Ts>(xs)...);
    }

    template<typename T>
    auto product(T &&x) {
        return x;
    }

    template<typename T, typename... Ts>
    auto sum(T &&x, Ts &&... xs) {
        return std::forward<T>(x) + sum(std::forward<Ts>(xs)...);
    }

    template<typename T>
    auto sum(T &&x) {
        return x;
    }

    template<typename T>
    auto min(T &&x) {
        return x;
    }

    template<typename T1, typename T2>
    auto min(T1 &&x, T2 &&y) {
        return (x < y) ? x : y;
    }

    inline double min(double x, double y) {
        return std::fmin(x, y);
    }

    template<typename T, typename... Ts>
    auto min(T &&x, Ts &&... xs) {
        return min(std::forward<T>(x), min(std::forward<Ts>(xs)...));
    }

    template<typename T>
    auto max(T &&x) {
        return x;
    }

    template<typename T1, typename T2>
    auto max(T1 &&x, T2 &&y) {
        return (x > y) ? x : y;
    }

    inline double max(double x, double y) {
        return std::fmax(x, y);
    }

    template<typename T, typename... Ts>
    auto max(T &&x, Ts &&... xs) {
        if (x.size() == 0) return 0.0;
        return max(std::forward<T>(x), max(std::forward<Ts>(xs)...));
    }

    inline double hyperGeometric21(double a, double b, double c, double z, double eps = 0.0) {
        double an = a,
                bn = b,
                cn = c,
                zn = z,
                fn = 1.0,
                sum = 1.0,
                delta = std::numeric_limits<double>::max(),
                deltaOld,
                sumOld;
        uint32_t i = 1;
        do {
            deltaOld = delta;
            delta = (an * bn * zn) / (cn * fn);

            // check for divergence
            if (std::fabs(delta) > std::fabs(deltaOld)) {
                return std::numeric_limits<double>::infinity();
            }

            sumOld = sum;
            sum += delta;
            an *= a + i;
            bn *= b + i;
            cn *= c + i;
            fn *= 1 + i;
            zn *= z;
            ++i;
        } while (std::fabs(sum - sumOld) > eps);

        return sum;
    }

    inline double logit(double p) {
        if (p == 0) return 0.0;
        return std::log(p / (p - 1.0));
    }

    inline double logistic(double alpha) {
        if (alpha == 0) return 0.0;
        return 1.0 / (1.0 + std::exp(-alpha));
    }

    inline double stddev(std::vector<double> const &func) {
        if (func.size() == 0) return 0.0;
        double mean = std::accumulate(func.begin(), func.end(), 0.0) / func.size();
        double sq_sum = std::inner_product(func.begin(), func.end(), func.begin(), 0.0,
                                           [](double const &x, double const &y) { return x + y; },
                                           [mean](double const &x, double const &y) {
                                               return (x - mean) * (y - mean);
                                           });
        return std::sqrt(sq_sum / (func.size() - 1));
    }
}