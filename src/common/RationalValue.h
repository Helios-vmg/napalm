#pragma once

#include <stdint.h>

typedef struct{
	int64_t numerator;
	int64_t denominator;
} RationalValue;

#ifdef __cplusplus
#include <boost/rational.hpp>
typedef boost::rational<std::int64_t> rational_t;

inline rational_t to_rational(const RationalValue &r){
	return rational_t(r.numerator, r.denominator);
}

inline RationalValue to_RationalValue(const rational_t &r){
	return {r.numerator(), r.denominator()};
}

template <typename T>
double to_double(const boost::rational<T> &r){
	return (double)r.numerator() / (double)r.denominator();
}

template <typename T, T limit>
boost::rational<T> to_rational(double x){
	typedef boost::rational<T> R;
	if (!x)
		return R(0, 1);
	bool negative = x < 0;
	if (negative)
		x = -x;
	R ret;
	{
		R hi(1, 1);
		R lo(0, 1);
		while (x > to_double(hi)){
			if (hi >= std::numeric_limits<T>::max() / 2)
				throw std::overflow_error("double out of bounds");
			lo = hi;
			hi *= 2;
		}

		if (to_double(hi) == x)
			return hi;
		if (to_double(lo) == x)
			return lo;

		auto delta = hi - lo;
		while (true){
			delta /= 2;
			auto c = lo + delta;
			if (c.numerator() >= limit || c.denominator() >= limit){
				ret = lo;
				break;
			}
			if (x < to_double(c))
				hi = c;
			else if (x > to_double(c))
				lo = c;
			else{
				ret = c;
				break;
			}
		}
	}
	return negative ? -ret : ret;
}

#endif
