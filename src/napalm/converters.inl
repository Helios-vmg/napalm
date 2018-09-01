// IntU8 -> IntU16

template <>
struct converter<IntU8, IntU16>{
	static std::uint32_t f(std::uint32_t x){
		return ((x) << 8) | (x);
	}
};

// IntU8 -> IntU24

template <>
struct converter<IntU8, IntU24>{
	static std::uint32_t f(std::uint32_t x){
		return ((x) << 16) | ((x) << 8) | (x);
	}
};

// IntU8 -> IntU32

template <>
struct converter<IntU8, IntU32>{
	static std::uint32_t f(std::uint32_t x){
		return ((x) << 24) | ((x) << 16) | ((x) << 8) | (x);
	}
};

// IntU8 -> IntS8

template <>
struct converter<IntU8, IntS8>{
	static std::uint32_t f(std::uint32_t x){
		return (x) ^ 0x80;
	}
};

// IntU8 -> IntS16

template <>
struct converter<IntU8, IntS16>{
	static std::uint32_t f(std::uint32_t x){
		return (((x) << 8) | (x)) ^ 0x8000;
	}
};

// IntU8 -> IntS24

template <>
struct converter<IntU8, IntS24>{
	static std::uint32_t f(std::uint32_t x){
		return (((x) << 16) | ((x) << 8) | (x)) ^ 0x800000;
	}
};

// IntU8 -> IntS32

template <>
struct converter<IntU8, IntS32>{
	static std::uint32_t f(std::uint32_t x){
		return (((x) << 24) | ((x) << 16) | ((x) << 8) | (x)) ^ 0x80000000;
	}
};

// IntU8 -> Float32

template <>
struct converter<IntU8, Float32>{
	static float f(std::uint32_t x){
		return (x) * (1.f * 2 / 0x000000FF) - 1;
	}
};

// IntU8 -> Float64

template <>
struct converter<IntU8, Float64>{
	static double f(std::uint32_t x){
		return (x) * (1.0 * 2 / 0x000000FF) - 1;
	}
};

// IntU16 -> IntU8

template <>
struct converter<IntU16, IntU8>{
	static std::uint32_t f(std::uint32_t x){
		return (x) >> 8;
	}
};

// IntU16 -> IntU24

template <>
struct converter<IntU16, IntU24>{
	static std::uint32_t f(std::uint32_t x){
		return ((x) << 8) | ((x) >> 8);
	}
};

// IntU16 -> IntU32

template <>
struct converter<IntU16, IntU32>{
	static std::uint32_t f(std::uint32_t x){
		return ((x) << 16) | (x);
	}
};

// IntU16 -> IntS8

template <>
struct converter<IntU16, IntS8>{
	static std::uint32_t f(std::uint32_t x){
		return ((x) >> 8) ^ 0x80;
	}
};

// IntU16 -> IntS16

template <>
struct converter<IntU16, IntS16>{
	static std::uint32_t f(std::uint32_t x){
		return (x) ^ 0x8000;
	}
};

// IntU16 -> IntS24

template <>
struct converter<IntU16, IntS24>{
	static std::uint32_t f(std::uint32_t x){
		return (((x) << 8) | ((x) >> 8)) ^ 0x800000;
	}
};

// IntU16 -> IntS32

template <>
struct converter<IntU16, IntS32>{
	static std::uint32_t f(std::uint32_t x){
		return (((x) << 16) | (x)) ^ 0x80000000;
	}
};

// IntU16 -> Float32

template <>
struct converter<IntU16, Float32>{
	static float f(std::uint32_t x){
		return (x) * (1.f * 2 / 0x0000FFFF) - 1;
	}
};

// IntU16 -> Float64

template <>
struct converter<IntU16, Float64>{
	static double f(std::uint32_t x){
		return (x) * (1.0 * 2 / 0x0000FFFF) - 1;
	}
};

// IntU24 -> IntU8

template <>
struct converter<IntU24, IntU8>{
	static std::uint32_t f(std::uint32_t x){
		return (x) >> 16;
	}
};

// IntU24 -> IntU16

template <>
struct converter<IntU24, IntU16>{
	static std::uint32_t f(std::uint32_t x){
		return (x) >> 8;
	}
};

// IntU24 -> IntU32

template <>
struct converter<IntU24, IntU32>{
	static std::uint32_t f(std::uint32_t x){
		return ((x) << 8) | ((x) >> 16);
	}
};

// IntU24 -> IntS8

template <>
struct converter<IntU24, IntS8>{
	static std::uint32_t f(std::uint32_t x){
		return ((x) >> 16) ^ 0x80;
	}
};

// IntU24 -> IntS16

template <>
struct converter<IntU24, IntS16>{
	static std::uint32_t f(std::uint32_t x){
		return ((x) >> 8) ^ 0x8000;
	}
};

// IntU24 -> IntS24

template <>
struct converter<IntU24, IntS24>{
	static std::uint32_t f(std::uint32_t x){
		return (x) ^ 0x800000;
	}
};

// IntU24 -> IntS32

template <>
struct converter<IntU24, IntS32>{
	static std::uint32_t f(std::uint32_t x){
		return (((x) << 8) | ((x) >> 16)) ^ 0x80000000;
	}
};

// IntU24 -> Float32

template <>
struct converter<IntU24, Float32>{
	static float f(std::uint32_t x){
		return (x) * (1.f * 2 / 0x00FFFFFF) - 1;
	}
};

// IntU24 -> Float64

template <>
struct converter<IntU24, Float64>{
	static double f(std::uint32_t x){
		return (x) * (1.0 * 2 / 0x00FFFFFF) - 1;
	}
};

// IntU32 -> IntU8

template <>
struct converter<IntU32, IntU8>{
	static std::uint32_t f(std::uint32_t x){
		return (x) >> 24;
	}
};

// IntU32 -> IntU16

template <>
struct converter<IntU32, IntU16>{
	static std::uint32_t f(std::uint32_t x){
		return (x) >> 16;
	}
};

// IntU32 -> IntU24

template <>
struct converter<IntU32, IntU24>{
	static std::uint32_t f(std::uint32_t x){
		return (x) >> 8;
	}
};

// IntU32 -> IntS8

template <>
struct converter<IntU32, IntS8>{
	static std::uint32_t f(std::uint32_t x){
		return ((x) >> 24) ^ 0x80;
	}
};

// IntU32 -> IntS16

template <>
struct converter<IntU32, IntS16>{
	static std::uint32_t f(std::uint32_t x){
		return ((x) >> 16) ^ 0x8000;
	}
};

// IntU32 -> IntS24

template <>
struct converter<IntU32, IntS24>{
	static std::uint32_t f(std::uint32_t x){
		return ((x) >> 8) ^ 0x800000;
	}
};

// IntU32 -> IntS32

template <>
struct converter<IntU32, IntS32>{
	static std::uint32_t f(std::uint32_t x){
		return (x) ^ 0x80000000;
	}
};

// IntU32 -> Float32

template <>
struct converter<IntU32, Float32>{
	static float f(std::uint32_t x){
		return (x) * (1.f * 2 / 0xFFFFFFFF) - 1;
	}
};

// IntU32 -> Float64

template <>
struct converter<IntU32, Float64>{
	static double f(std::uint32_t x){
		return (x) * (1.0 * 2 / 0xFFFFFFFF) - 1;
	}
};

// IntS8 -> IntU8

template <>
struct converter<IntS8, IntU8>{
	static std::uint32_t f(std::uint32_t x){
		return (x) ^ 0x80;
	}
};

// IntS8 -> IntU16

template <>
struct converter<IntS8, IntU16>{
	static std::uint32_t f(std::uint32_t x){
		return (((x) << 8) | (x)) ^ (0x80|0x8000);
	}
};

// IntS8 -> IntU24

template <>
struct converter<IntS8, IntU24>{
	static std::uint32_t f(std::uint32_t x){
		return (((x) << 16) | ((x) << 8) | (x)) ^ (0x8080|0x800000);
	}
};

// IntS8 -> IntU32

template <>
struct converter<IntS8, IntU32>{
	static std::uint32_t f(std::uint32_t x){
		return (((x) << 24) | ((x) << 16) | ((x) << 8) | (x)) ^ (0x808080|0x80000000);
	}
};

// IntS8 -> IntS16

template <>
struct converter<IntS8, IntS16>{
	static std::uint32_t f(std::uint32_t x){
		return (((x) << 8) | (x)) ^ 0x80;
	}
};

// IntS8 -> IntS24

template <>
struct converter<IntS8, IntS24>{
	static std::uint32_t f(std::uint32_t x){
		return (((x) << 16) | ((x) << 8) | (x)) ^ 0x8080;
	}
};

// IntS8 -> IntS32

template <>
struct converter<IntS8, IntS32>{
	static std::uint32_t f(std::uint32_t x){
		return (((x) << 24) | ((x) << 16) | ((x) << 8) | (x)) ^ 0x808080;
	}
};

// IntS8 -> Float32

template <>
struct converter<IntS8, Float32>{
	static float f(std::uint32_t x){
		return ((x) ^ 0x80) * (1.f * 2 / 0x000000FF) - 1;
	}
};

// IntS8 -> Float64

template <>
struct converter<IntS8, Float64>{
	static double f(std::uint32_t x){
		return ((x) ^ 0x80) * (1.0 * 2 / 0x000000FF) - 1;
	}
};

// IntS16 -> IntU8

template <>
struct converter<IntS16, IntU8>{
	static std::uint32_t f(std::uint32_t x){
		return ((x) ^ 0x8000) >> 8;
	}
};

// IntS16 -> IntU16

template <>
struct converter<IntS16, IntU16>{
	static std::uint32_t f(std::uint32_t x){
		return (x) ^ 0x8000;
	}
};

// IntS16 -> IntU24

template <>
struct converter<IntS16, IntU24>{
	static std::uint32_t f(std::uint32_t x){
		return (((x) << 8) | ((x) >> 8)) ^ (0x80|0x800000);
	}
};

// IntS16 -> IntU32

template <>
struct converter<IntS16, IntU32>{
	static std::uint32_t f(std::uint32_t x){
		return (((x) << 16) | (x)) ^ (0x8000|0x80000000);
	}
};

// IntS16 -> IntS8

template <>
struct converter<IntS16, IntS8>{
	static std::uint32_t f(std::uint32_t x){
		return (x) >> 8;
	}
};

// IntS16 -> IntS24

template <>
struct converter<IntS16, IntS24>{
	static std::uint32_t f(std::uint32_t x){
		return (((x) << 8) | ((x) >> 8)) ^ 0x80;
	}
};

// IntS16 -> IntS32

template <>
struct converter<IntS16, IntS32>{
	static std::uint32_t f(std::uint32_t x){
		return (((x) << 16) | (x)) ^ 0x8000;
	}
};

// IntS16 -> Float32

template <>
struct converter<IntS16, Float32>{
	static float f(std::uint32_t x){
		return ((x) ^ 0x8000) * (1.f * 2 / 0x0000FFFF) - 1;
	}
};

// IntS16 -> Float64

template <>
struct converter<IntS16, Float64>{
	static double f(std::uint32_t x){
		return ((x) ^ 0x8000) * (1.0 * 2 / 0x0000FFFF) - 1;
	}
};

// IntS24 -> IntU8

template <>
struct converter<IntS24, IntU8>{
	static std::uint32_t f(std::uint32_t x){
		return ((x) ^ 0x800000) >> 16;
	}
};

// IntS24 -> IntU16

template <>
struct converter<IntS24, IntU16>{
	static std::uint32_t f(std::uint32_t x){
		return ((x) >> 8) ^ 0x8000;
	}
};

// IntS24 -> IntU24

template <>
struct converter<IntS24, IntU24>{
	static std::uint32_t f(std::uint32_t x){
		return (x) ^ 0x800000;
	}
};

// IntS24 -> IntU32

template <>
struct converter<IntS24, IntU32>{
	static std::uint32_t f(std::uint32_t x){
		return (((x) << 8) | ((x) >> 16)) ^ (0x80|0x80000000);
	}
};

// IntS24 -> IntS8

template <>
struct converter<IntS24, IntS8>{
	static std::uint32_t f(std::uint32_t x){
		return (x) >> 16;
	}
};

// IntS24 -> IntS16

template <>
struct converter<IntS24, IntS16>{
	static std::uint32_t f(std::uint32_t x){
		return (x) >> 8;
	}
};

// IntS24 -> IntS32

template <>
struct converter<IntS24, IntS32>{
	static std::uint32_t f(std::uint32_t x){
		return (((x) << 8) | ((x) >> 16)) ^ 0x80;
	}
};

// IntS24 -> Float32

template <>
struct converter<IntS24, Float32>{
	static float f(std::uint32_t x){
		return ((x) ^ 0x800000) * (1.f * 2 / 0x00FFFFFF) - 1;
	}
};

// IntS24 -> Float64

template <>
struct converter<IntS24, Float64>{
	static double f(std::uint32_t x){
		return ((x) ^ 0x800000) * (1.0 * 2 / 0x00FFFFFF) - 1;
	}
};

// IntS32 -> IntU8

template <>
struct converter<IntS32, IntU8>{
	static std::uint32_t f(std::uint32_t x){
		return ((x) ^ 0x80000000) >> 24;
	}
};

// IntS32 -> IntU16

template <>
struct converter<IntS32, IntU16>{
	static std::uint32_t f(std::uint32_t x){
		return ((x) >> 16) ^ 0x8000;
	}
};

// IntS32 -> IntU24

template <>
struct converter<IntS32, IntU24>{
	static std::uint32_t f(std::uint32_t x){
		return ((x) >> 8) ^ 0x800000;
	}
};

// IntS32 -> IntU32

template <>
struct converter<IntS32, IntU32>{
	static std::uint32_t f(std::uint32_t x){
		return (x) ^ 0x80000000;
	}
};

// IntS32 -> IntS8

template <>
struct converter<IntS32, IntS8>{
	static std::uint32_t f(std::uint32_t x){
		return (x) >> 24;
	}
};

// IntS32 -> IntS16

template <>
struct converter<IntS32, IntS16>{
	static std::uint32_t f(std::uint32_t x){
		return (x) >> 16;
	}
};

// IntS32 -> IntS24

template <>
struct converter<IntS32, IntS24>{
	static std::uint32_t f(std::uint32_t x){
		return (x) >> 8;
	}
};

// IntS32 -> Float32

template <>
struct converter<IntS32, Float32>{
	static float f(std::uint32_t x){
		return ((x) ^ 0x80000000) * (1.f * 2 / 0xFFFFFFFF) - 1;
	}
};

// IntS32 -> Float64

template <>
struct converter<IntS32, Float64>{
	static double f(std::uint32_t x){
		return ((x) ^ 0x80000000) * (1.0 * 2 / 0xFFFFFFFF) - 1;
	}
};

// Float32 -> IntU8

template <>
struct converter<Float32, IntU8>{
	static std::uint32_t f(float x){
		return (std::uint32_t)((saturate((x)) + 1) * ((float)0x000000FF / 2));
	}
};

// Float32 -> IntU16

template <>
struct converter<Float32, IntU16>{
	static std::uint32_t f(float x){
		return (std::uint32_t)((saturate((x)) + 1) * ((float)0x0000FFFF / 2));
	}
};

// Float32 -> IntU24

template <>
struct converter<Float32, IntU24>{
	static std::uint32_t f(float x){
		return (std::uint32_t)((saturate((x)) + 1) * ((float)0x00FFFFFF / 2));
	}
};

// Float32 -> IntU32

template <>
struct converter<Float32, IntU32>{
	static std::uint32_t f(float x){
		return (std::uint32_t)((saturate((x)) + 1) * ((float)0xFFFFFFFF / 2));
	}
};

// Float32 -> IntS8

template <>
struct converter<Float32, IntS8>{
	static std::uint32_t f(float x){
		return ((std::uint32_t)((saturate((x)) + 1) * ((float)0x000000FF / 2))) ^ 0x80;
	}
};

// Float32 -> IntS16

template <>
struct converter<Float32, IntS16>{
	static std::uint32_t f(float x){
		return ((std::uint32_t)((saturate((x)) + 1) * ((float)0x0000FFFF / 2))) ^ 0x8000;
	}
};

// Float32 -> IntS24

template <>
struct converter<Float32, IntS24>{
	static std::uint32_t f(float x){
		return ((std::uint32_t)((saturate((x)) + 1) * ((float)0x00FFFFFF / 2))) ^ 0x800000;
	}
};

// Float32 -> IntS32

template <>
struct converter<Float32, IntS32>{
	static std::uint32_t f(float x){
		return ((std::uint32_t)((saturate((x)) + 1) * ((float)0xFFFFFFFF / 2))) ^ 0x80000000;
	}
};

// Float32 -> Float64

template <>
struct converter<Float32, Float64>{
	static double f(float x){
		return (double)(x);
	}
};

// Float64 -> IntU8

template <>
struct converter<Float64, IntU8>{
	static std::uint32_t f(double x){
		return (std::uint32_t)((saturate((x)) + 1) * ((double)0x000000FF / 2));
	}
};

// Float64 -> IntU16

template <>
struct converter<Float64, IntU16>{
	static std::uint32_t f(double x){
		return (std::uint32_t)((saturate((x)) + 1) * ((double)0x0000FFFF / 2));
	}
};

// Float64 -> IntU24

template <>
struct converter<Float64, IntU24>{
	static std::uint32_t f(double x){
		return (std::uint32_t)((saturate((x)) + 1) * ((double)0x00FFFFFF / 2));
	}
};

// Float64 -> IntU32

template <>
struct converter<Float64, IntU32>{
	static std::uint32_t f(double x){
		return (std::uint32_t)((saturate((x)) + 1) * ((double)0xFFFFFFFF / 2));
	}
};

// Float64 -> IntS8

template <>
struct converter<Float64, IntS8>{
	static std::uint32_t f(double x){
		return ((std::uint32_t)((saturate((x)) + 1) * ((double)0x000000FF / 2))) ^ 0x80;
	}
};

// Float64 -> IntS16

template <>
struct converter<Float64, IntS16>{
	static std::uint32_t f(double x){
		return ((std::uint32_t)((saturate((x)) + 1) * ((double)0x0000FFFF / 2))) ^ 0x8000;
	}
};

// Float64 -> IntS24

template <>
struct converter<Float64, IntS24>{
	static std::uint32_t f(double x){
		return ((std::uint32_t)((saturate((x)) + 1) * ((double)0x00FFFFFF / 2))) ^ 0x800000;
	}
};

// Float64 -> IntS32

template <>
struct converter<Float64, IntS32>{
	static std::uint32_t f(double x){
		return ((std::uint32_t)((saturate((x)) + 1) * ((double)0xFFFFFFFF / 2))) ^ 0x80000000;
	}
};

// Float64 -> Float32

template <>
struct converter<Float64, Float32>{
	static float f(double x){
		return (float)(x);
	}
};

// IntU8 -> IntU8

template <>
struct converter<IntU8, IntU8>{
	static std::uint32_t f(std::uint32_t x){
		return x;
	}
};

// IntU16 -> IntU16

template <>
struct converter<IntU16, IntU16>{
	static std::uint32_t f(std::uint32_t x){
		return x;
	}
};

// IntU24 -> IntU24

template <>
struct converter<IntU24, IntU24>{
	static std::uint32_t f(std::uint32_t x){
		return x;
	}
};

// IntU32 -> IntU32

template <>
struct converter<IntU32, IntU32>{
	static std::uint32_t f(std::uint32_t x){
		return x;
	}
};

// IntS8 -> IntS8

template <>
struct converter<IntS8, IntS8>{
	static std::uint32_t f(std::uint32_t x){
		return x;
	}
};

// IntS16 -> IntS16

template <>
struct converter<IntS16, IntS16>{
	static std::uint32_t f(std::uint32_t x){
		return x;
	}
};

// IntS24 -> IntS24

template <>
struct converter<IntS24, IntS24>{
	static std::uint32_t f(std::uint32_t x){
		return x;
	}
};

// IntS32 -> IntS32

template <>
struct converter<IntS32, IntS32>{
	static std::uint32_t f(std::uint32_t x){
		return x;
	}
};

// Float32 -> Float32

template <>
struct converter<Float32, Float32>{
	static float f(float x){
		return x;
	}
};

// Float64 -> Float64

template <>
struct converter<Float64, Float64>{
	static double f(double x){
		return x;
	}
};

