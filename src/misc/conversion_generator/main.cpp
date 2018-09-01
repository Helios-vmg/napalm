#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <boost/rational.hpp>
#include <cassert>

void true_assert(bool q){
	if (!q)
		throw std::runtime_error("assertion failed");
}

enum NumberFormat{
	Invalid = 0,
	IntU8,
	IntU16,
	IntU24,
	IntU32,
	IntS8,
	IntS16,
	IntS24,
	IntS32,
	Float32,
	Float64,
	COUNT,
};

const char *to_string(NumberFormat f){
	static const char * const table[] = {
		"",
		"IntU8",
		"IntU16",
		"IntU24",
		"IntU32",
		"IntS8",
		"IntS16",
		"IntS24",
		"IntS32",
		"Float32",
		"Float64",
	};
	return table[f];
}

const char *to_type(NumberFormat f){
	static const char * const table[] = {
		"",
		"std::uint8_t",
		"std::uint16_t",
		"std::uint32_t",
		"std::uint32_t",
		"std::int8_t",
		"std::int16_t",
		"std::int32_t",
		"std::int32_t",
		"float",
		"double",
	};
	return table[f];
}

int size_of_type(NumberFormat f){
	static const int table[] = {
		0,
		1,
		2,
		3,
		4,
		1,
		2,
		3,
		4,
		4,
		8,
	};
	return table[f];
}

NumberFormat intermediate_type(NumberFormat f){
	static const NumberFormat table[] = {
		Invalid,
		IntU32,
		IntU32,
		IntU32,
		IntU32,
		IntU32,
		IntU32,
		IntU32,
		IntU32,
		Float32,
		Float64,
	};
	return table[f];
}

NumberFormat to_unsigned(NumberFormat f){
	static const NumberFormat table[] = {
		Invalid,
		IntU8,
		IntU16,
		IntU24,
		IntU32,
		IntU8,
		IntU16,
		IntU24,
		IntU32,
		Invalid,
		Invalid,
	};
	return table[f];
}

NumberFormat to_signed(NumberFormat f){
	static const NumberFormat table[] = {
		Invalid,
		IntS8,
		IntS16,
		IntS24,
		IntS32,
		IntS8,
		IntS16,
		IntS24,
		IntS32,
		Invalid,
		Invalid,
	};
	return table[f];
}

const char *limit_max(NumberFormat f){
	static const char * const table[] = {
		"",
		"0x000000FF",
		"0x0000FFFF",
		"0x00FFFFFF",
		"0xFFFFFFFF",
		"0x0000007F",
		"0x00007FFF",
		"0x007FFFFF",
		"0x7FFFFFFF",
		"",
		"",
	};
	return table[f];
}

const char *signedness_flipping_mask(NumberFormat f){
	static const char * const table[] = {
		"",
		"0x80",
		"0x8000",
		"0x800000",
		"0x80000000",
		"0x80",
		"0x8000",
		"0x800000",
		"0x80000000",
		"",
		"",
	};
	return table[f];
}

const char *unit(NumberFormat f){
	static const char * const table[] = {
		"",
		"1",
		"1",
		"1",
		"1",
		"1",
		"1",
		"1",
		"1",
		"1.f",
		"1.0",
	};
	return table[f];
}

int bits(NumberFormat f){
	return size_of_type(f) * 8;
}

std::string generate_load(NumberFormat src){
	std::string temp = to_type(IntU32);
	switch (src){
		case IntU8:
		case IntS8:
			return "(" + temp + ")src[0]";
		case IntU16:
		case IntS16:
			return "(" + temp + ")src[0] | ((" + temp + ")src[1] << 8)";
		case IntU24:
		case IntS24:
			return "(" + temp + ")src[0] | ((" + temp + ")src[1] << 8) | ((" + temp + ")src[2] << 16)";
		case IntU32:
		case IntS32:
			return "(" + temp + ")src[0] | ((" + temp + ")src[1] << 8) | ((" + temp + ")src[2] << 16) | ((" + temp + ")src[2] << 24)";
		case Float32:
		case Float64:
			return (std::string)"*(" + to_type(src) + " *)src";
	}
}

#define C(x, y) ((int)x | ((int)y << 8))

typedef std::pair<std::string, NumberFormat> expression_t;

const char *signed_upshift_mask(NumberFormat to, NumberFormat from){
	static const char * const table[] = {
		"0x80",
		"0x8080",
		"0x808080",
		"0x8000",
	};
	auto s1 = size_of_type(to);
	auto s2 = size_of_type(from);
	true_assert(s1 > s2);
	auto g = (boost::gcd(s1, s2) - 1) * 2;
	return table[s1 - s2 - 1 + g];
}

#define E "(" + e.first + ")"

expression_t convert_to(const expression_t &e, NumberFormat from, NumberFormat to);

expression_t convert_to(const expression_t &e, NumberFormat to){
	auto from = e.second;
	switch (C(to, from)){
		
		//Unsigned upshifts

		case C(IntU16,IntU8):
			return expression_t("(" E " << 8) | " E, IntU16);
		case C(IntU24,IntU8):
			return expression_t("(" E " << 16) | (" E " << 8) | " E, IntU24);
		case C(IntU32,IntU8):
			return expression_t("(" E " << 24) | (" E " << 16) | (" E " << 8) | " E, IntU32);
		case C(IntU24,IntU16):
			return expression_t("(" E " << 8) | (" E " >> 8)", IntU24);
		case C(IntU32,IntU16):
			return expression_t("(" E " << 16) | " E, IntU32);
		case C(IntU32,IntU24):
			return expression_t("(" E " << 8) | (" E " >> 16)", IntU32);
			
		//Right shifts
		
		case C(IntU8,IntU16):
		case C(IntS8,IntS16):
		case C(IntU16,IntU24):
		case C(IntS16,IntS24):
		case C(IntU24,IntU32):
		case C(IntS24,IntS32):
		case C(IntU8,IntU24):
		case C(IntS8,IntS24):
		case C(IntU16,IntU32):
		case C(IntS16,IntS32):
		case C(IntU8,IntU32):
		case C(IntS8,IntS32):
			{
				std::stringstream stream;
				stream << (E " >> ") << bits(from) - bits(to);
				return expression_t(stream.str(), to);
			}

		//Signedness flipping
		
		case C(IntU8,IntS8):
		case C(IntS8,IntU8):
		case C(IntU16,IntS16):
		case C(IntS16,IntU16):
		case C(IntU24,IntS24):
		case C(IntS24,IntU24):
		case C(IntU32,IntS32):
		case C(IntS32,IntU32):
			return expression_t(E " ^ " + signedness_flipping_mask(to), to);

		//Float to unsigned

		case C(IntU8,Float32):
		case C(IntU8,Float64):
		case C(IntU16,Float32):
		case C(IntU16,Float64):
		case C(IntU24,Float32):
		case C(IntU24,Float64):
		case C(IntU32,Float32):
		case C(IntU32,Float64):
			return expression_t("(std::uint32_t)((saturate(" E ") + 1) * ((" + to_type(from) + ")" + limit_max(to) + " / 2))", to);

		//Unsigned to float

		case C(Float32,IntU8):
		case C(Float32,IntU16):
		case C(Float32,IntU24):
		case C(Float32,IntU32):
		case C(Float64,IntU8):
		case C(Float64,IntU16):
		case C(Float64,IntU24):
		case C(Float64,IntU32):
			return expression_t(E " * (" + unit(to) + " * 2 / " + limit_max(from) + ") - 1", to);

		//Signed upshifts
		
		case C(IntS16,IntS8):
		case C(IntS24,IntS8):
		case C(IntS32,IntS8):
		case C(IntS24,IntS16):
		case C(IntS32,IntS16):
		case C(IntS32,IntS24):
			return expression_t("(" + convert_to(e, to_unsigned(from), to_unsigned(to)).first + ") ^ " + signed_upshift_mask(to, from), to);
		
		//Signed upshifts with sign flipping
		case C(IntU16,IntS8):
		case C(IntU24,IntS8):
		case C(IntU32,IntS8):
		case C(IntU24,IntS16):
		case C(IntU32,IntS16):
		case C(IntU32,IntS24):
			return expression_t("(" + convert_to(e, to_unsigned(from), to).first + ") ^ (" + signed_upshift_mask(to, from) + "|" + signedness_flipping_mask(to) + ")", to);

		//Float casts

		case C(Float32,Float64):
			return expression_t("(float)" E, Float32);
		
		case C(Float64,Float32):
			return expression_t("(double)" E, Float64);

		//Compounded conversions (size first)

		case C(IntU16,IntS24):
		case C(IntU16,IntS32):
		case C(IntU24,IntS32):
			return convert_to(convert_to(e, to_signed(to)), to);
		case C(IntS8,IntU16):
		case C(IntS8,IntU24):
		case C(IntS8,IntU32):
		case C(IntS8,Float32):
		case C(IntS8,Float64):
		case C(IntS16,IntU8):
		case C(IntS16,IntU24):
		case C(IntS16,IntU32):
		case C(IntS16,Float32):
		case C(IntS16,Float64):
		case C(IntS24,IntU8):
		case C(IntS24,IntU16):
		case C(IntS24,IntU32):
		case C(IntS24,Float32):
		case C(IntS24,Float64):
		case C(IntS32,IntU8):
		case C(IntS32,IntU16):
		case C(IntS32,IntU24):
		case C(IntS32,Float32):
		case C(IntS32,Float64):
			return convert_to(convert_to(e, to_unsigned(to)), to);
			
		//Compounded conversions (signedness first)

		case C(IntU8,IntS16):
		case C(IntU8,IntS24):
		case C(IntU8,IntS32):
		case C(Float32,IntS8):
		case C(Float64,IntS8):
		case C(Float32,IntS16):
		case C(Float64,IntS16):
		case C(Float32,IntS24):
		case C(Float64,IntS24):
		case C(Float32,IntS32):
		case C(Float64,IntS32):
			return convert_to(convert_to(e, to_unsigned(from)), to);

		default:
			throw std::exception();
	}
}

expression_t convert_to(const expression_t &e, NumberFormat from, NumberFormat to){
	return convert_to(expression_t(e.first, from), to);
}

#if 0
std::string generate_converter(NumberFormat dst, NumberFormat src){
	auto S = intermediate_type(src);
	auto D = intermediate_type(dst);
	std::stringstream stream;
	stream <<
			"static read_buffer_t convert_" << to_string(dst) << "_" << to_string(src) << "(read_buffer_t &&buffer, size_t bytes_per_sample, size_t channels){\n"
			"    read_buffer_t ret = allocate_buffer(buffer->sample_count * bytes_per_sample);\n"
			"    auto src = buffer->data;\n"
			"    auto dst = ret->data;\n"
			"    auto elements = buffer->sample_count * channels;\n"
			"    for (size_t i = 0; i < elements; i++){\n"
			"        " << to_type(unsigned_type(src)) << " src_val = " << generate_load(src) << ";\n";
	if (is_signed(src)){
		if (src != IntS32)
			stream <<
			"        src_val = src_val | ((src_val > " << limit_min(src) << ") * " << sign_mask(src) << ");\n";
		stream <<
			"        src_val += " << limit_min(src) << ";\n";
	}
	if (!is_float(src) && !is_float(dst))
		stream <<
			"        auto low_byte = src_val & 0xFF;\n";
	stream <<
			"        " << to_type(D) << " dst_val = " << generate_convert(D, S, dst, src, size_of_type(dst) - size_of_type(src)) << ";\n"
			"        helper<D, S>::f(dst, src);\n"
			"    }\n"
			"    return ret;\n"
			"}\n"
	return stream.str();
}
#endif

int main(){
	try{
		for (NumberFormat source = IntU8; source < COUNT; source = (NumberFormat)(source + 1)){
			for (NumberFormat dest = IntU8; dest < COUNT; dest = (NumberFormat)(dest + 1)){
				if (source == dest){
					
					continue;
				}

				std::cout << "// " << to_string(source) << " -> " << to_string(dest) << "\n\n";

				auto temp = convert_to(expression_t("x", source), dest);
				true_assert(temp.second == dest);

				std::cout <<
					"template <>\n"
					"struct converter<" << to_string(source) << ", " << to_string(dest) << ">{\n"
					"\tstatic " << to_type(intermediate_type(dest)) << " f(" << to_type(intermediate_type(source)) << " x){\n"
					"\t\treturn " << temp.first << ";\n"
					"\t}\n"
					"};\n"
					"\n"
					;
			}
		}
		for (NumberFormat i = IntU8; i < COUNT; i = (NumberFormat)(i + 1)){
			std::cout << "// " << to_string(i) << " -> " << to_string(i) << "\n\n";
			std::cout <<
				"template <>\n"
				"struct converter<" << to_string(i) << ", " << to_string(i) << ">{\n"
				"\tstatic " << to_type(intermediate_type(i)) << " f(" << to_type(intermediate_type(i)) << " x){\n"
				"\t\treturn x;\n"
				"\t}\n"
				"};\n"
				"\n"
				;
		}
	}catch (std::exception &e){
		std::cerr << e.what() << std::endl;
		return -1;
	}
	return 0;
}
