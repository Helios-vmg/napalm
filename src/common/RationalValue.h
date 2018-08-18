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

#endif
