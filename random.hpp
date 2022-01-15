/** rico/random.hpp
 *
 * /!\ DO NOT USE FOR CRYPTOGRAPHY /!\
 *
 * This RNG use a linear congruential generator
 * modulus    m = 2^64
 * multiplier a = m / 3
 * increment  c = 1
 * this ensure a period of 2^64 for the state, regardless of the seed
 * low bits have a shorter period, so it only output the 32 highest bits
 *
 * std::time_t may not be meaningfully converted to uint64_t, but it is
 * garanteed that type punning can be done on it to initialize the seed
 * with its content
 *
 * Keeping the uniform distribution when calling Random::rangeUint
 * is not easy. 'Random::Uint() % range_size' does not keep it
 * (pigeonhole principle, some outputs have higher probability)
 *
 * sources :
 * LCG = wikipedia - linear congruential generator
 * rangeUint distribution = stackexchange - 1242163
 * class structure = youtube "The Cherno" - singleton
 */

#pragma once

#include <ctime> // std::time_t, std::time
#include <stdexcept> // std::domain_error

class Random {
public:

	static void Seed(uint64_t s) {
		get().state = s;
	}

	/* range [ 0 , 2^32-1 ] */
	static uint32_t Uint(void) {
		uint64_t retval = get().update();
		retval >>= 32;
		return static_cast<uint32_t>(retval);
	}

	/* range [ 0 , 2^31-1 ] */
	static int32_t Int(void) {
		return Uint() >> 1;
	}

	/* range [ min, max ] */
	static uint32_t rangeUint(uint32_t min, uint32_t max) {
		if (max < min) throw std::domain_error("empty range");
		uint64_t range_size = static_cast<uint64_t>(max) - min + 1;
		if (range_size == 1) return min;
		// find a range
		// - as large as possible so that the following process is fast
		// - divisible by range_size so that the % operation is safe
		uint64_t max_divisible_range = 1 + static_cast<uint64_t>(MAX);
		max_divisible_range /= range_size;
		max_divisible_range *= range_size;
		// wait for the random process to fall in this range
		uint32_t x;
		while (true) {
			x = Uint();
			if (x < max_divisible_range) break;
		}
		return min + x % range_size;
	}

	/* range [ 0, max ] */
	static uint32_t rangeUint(uint32_t max) {
		return rangeUint(0, max) ;
	}

	/* range [ min, max ] */
	static int32_t rangeInt(int32_t min, int32_t max) {
		if (max < min) throw std::domain_error("empty range");
		// ok even if max-min overflow on int32_t
		uint32_t umax = static_cast<uint32_t>(max - min);
		uint32_t uretval = rangeUint(umax);
		int32_t retval = static_cast<int32_t>(uretval);
		return min + retval;
	}

	/* range [ 0.0 , 1.0 ) */
	static double Double(void) {
		return static_cast<double>(Uint()) / MAX;
	}

	/* range [ min , max ) */
	static double rangeDouble(double min, double max) {
		if (max <= min) throw std::domain_error("empty range");
		return min + Double() * (max - min);
	}

private:

	uint64_t state;
	static constexpr uint32_t MAX = static_cast<uint32_t>(0xffffffff); // 2^32-1

	/**
	 * private constructor of the singleton
	 * seeded by hashing (FNV-1 64 bits) the return of std::time
	 */
	Random(void)
		: state(static_cast<uint64_t>(0xcbf29ce484222325)) // FNV-1 init
	{
		std::time_t now = std::time(nullptr);
		unsigned char *char_now = reinterpret_cast<unsigned char *>(&now);
		while (*char_now) {
			state *= static_cast<uint64_t>(0x100000001b3); // FNV-1 multiplier
			state ^= static_cast<uint64_t>(*char_now++);
		}
	}

	/* return the instance of the singleton */
	static Random& get(void) {
		static Random instance;
		return instance;
	}

	/* modify the state of the RNG and return it */
	uint64_t update(void) {
		state *= static_cast<uint64_t>(0x5555555555555555); // 2^64 / 3
		++state;
		return state;
	}

}; // class Random