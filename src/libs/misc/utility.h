#pragma once

#include <numeric>
#include <cstdint>
#include <fstream>

namespace utils {

    template<typename T>
    bool is_infinite(const T &value) {
        // Since we're a template, it's wise to use std::numeric_limits<T>
        //
        // Note: std::numeric_limits<T>::min() behaves like DBL_MIN, and is the smallest absolute value possible.
        //
        T max_value = std::numeric_limits<T>::max();
        T min_value = -max_value;

        return !(min_value <= value && value <= max_value);
    }

    template<typename T>
    bool is_nan(const T &value) {
        // True if NAN
        return value != value;
    }

    template<typename T>
    bool is_valid(const T &value) {
        return !is_infinite(value) && !is_nan(value);
    }

    // If the number if valid, returns itself otherwise 0
    template<typename T>
    inline T fix_number(const T &value) {
        if (!is_infinite(value) && !is_nan(value)) {
            return value;
        } else {
            return 0;
        }
    }

    template<typename C>
    auto accumulate(C &&c, typename C::value_type initial) {
        return std::accumulate(std::begin(std::forward<C>(c)), std::end(std::forward<C>(c)), initial);
    }

    template<typename T, typename F>
    void plot(const char *filename, T lower, T upper, uint32_t N, F &&f) {
        std::ofstream out(filename);

        T delta = (upper - lower) / (N - 1);
        T x = lower;
        for (uint32_t i = 0; i < N; ++i) {
            out << x << ',' << std::forward<F>(f)(x) << std::endl;
            x += delta;
        }
    }

    template<typename T, typename F>
	void plot2D( const char* filename, T lower1, T upper1,  T lower2, T upper2, uint32_t N1, uint32_t N2, F&& f ) {
		std::ofstream out(filename);

		T delta1 = ( upper1 - lower1 ) / ( N1 - 1 );
		T delta2 = ( upper2 - lower2 ) / ( N2 - 1 );
		
		out << ',';
		for(uint32_t j=0;j<N2;++j) {
			out << lower2 + delta2*j << ',';
		}
		out << std::endl;

		T x = lower1;
		for(uint32_t i=0;i<N1;++i) {
			out << x << ',';
			T y = lower2;
			for(uint32_t j=0;j<N2;++j) {
				out << std::forward<F>(f)(x,y) << ',';
				y += delta2;
			}
			out << std::endl;
            x += delta1;
        }

    }

    template<typename T, typename F>
    void plot2D(const char *filename, T lower, T upper, uint32_t N, F &&f) {
        plot2D(filename, lower, upper, lower, upper, N, N, std::forward<F>(f));
    }

    template<typename T>
    T clamp(T x, T lower, T upper) {
        if (x < lower) x = lower;
        if (x > upper) x = upper;
        return x;
    }
}