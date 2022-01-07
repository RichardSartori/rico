/** rico/random.hpp
 *
 * DO NOT USE FOR CRYPTOGRAPHY
 *
 * This RNG is a linear congruential generator with uniform distribution
 * modulus, multiplier and increment are carefully chosen to give a period
 * of 2**31 for any seed (except for the following case)
 *
 * Calling Random::rand_r() repeatedly on a range of size N will shorten
 * the period to N if N is a power of 2
 *
 * Random::MAX is INT32_MAX, so the results of rand can safely be
 * casted to int32_t (aka int)
 *
 * std::time_t may not be meaningfully converted to uint32_t, but it is
 * garanteed that type punning can be done on it to initialize the seed
 * with its content
 *
 * <something> &= 0x7fffffff;
 * is a more efficient way to do
 * <something> %= 0x80000000;
 *
 * Any call to Random::rand, Random::rand_r, Random::randd or Random::randd_r
 * is based on the result of a call to Random::update
 *
 * Keeping the uniform distribution of Random::rand
 * when calling Random::rand_r is not easy
 * Random::rand() % range_size does not keep it
 *
 * sources :
 * LCG = wikipedia - Linear congruential generator
 * rand_r distribution = stackexchange - 1242163
 * hex value = youtube one lone coder - procedural universe
 * class structure = youtube the cherno - singleton
 */

#pragma once

#include <ctime> // std::time_t, std::time
#include <stdexcept> // std::domain_error

class Random {
public:

	// seed the RNG with <s>
	// if not called, the RNG is seeded using std::time
	static void seed(uint32_t s)
	{
		s &= 0x7fffffff;
		get().state = s;
	}

	// return a random number in the range [0, Random::MAX]
	static uint32_t rand(void)
	{
		return get().update();
	}

	// return a random number in the range [lower_bound, upper_bound]
	static uint32_t rand_r(uint32_t lb, uint32_t ub)
	{
		if (ub < lb)
		{
			throw std::domain_error("empty range");
		}
		uint32_t range_size = ub - lb + 1;
		if (range_size > _max_range_size || range_size == 0)
		// /!\ if lb = INT_MIN and ub = INT_MAX then range_size = 0
		{
			throw std::domain_error("range too wide");
		}
		else
		{
			uint32_t max_divisible_range = _max_range_size;
			max_divisible_range /= range_size;
			max_divisible_range *= range_size;
			while (true)
			{
				uint32_t x = rand();
				if (x < max_divisible_range)
				{
					return lb + x % range_size;
				}
			}
		}
	}

	// overload of Random::rand_r for int32_t
	static int32_t rand_r(int32_t lb, int32_t ub)
	{
		if (ub < lb)
		{
			throw std::domain_error("empty range");
		}
		uint32_t u_lb = static_cast<uint32_t>(0);
		uint32_t u_ub = static_cast<uint32_t>(ub - lb);
		uint32_t u_retval = rand_r(u_lb, u_ub);
		int32_t retval = static_cast<int32_t>(u_retval);
		return retval + lb;
	}

	// return a random double in the range [0, 1)
	static double randd(void)
	{
		return static_cast<double>(rand()) / _max_range_size;
	}

	// return a random double in the range [lower_bound, upper_bound)
	static double randd_r(double lb, double ub)
	{
		if (ub <= lb)
		{
			throw std::domain_error("empty range");
		}
		else
		{
			return lb + Random::randd() * (ub - lb);
		}
	}

	// maximum return value of Random::rand
	static constexpr uint32_t MAX = 0x7fffffff; // 2**31-1

private:
	// store the state of the RNG
	uint32_t state;
	// maximum size of the range in Random::randd_r
	static constexpr uint32_t _max_range_size = 0x80000000; // 2**31

	// Constructor of the singleton, seeded with std::time
	Random(void)
		: state(0)
	{
		std::time_t now = std::time(nullptr);
		char *char_now = reinterpret_cast<char *>(&now);
		for (size_t i = 0; i < sizeof(now); ++i)
		{
			state += 0xe120fc15 * char_now[i];
		}
		state &= 0x7fffffff;
	}

	// return the instance of the singleton
	static Random& get(void)
	{
		static Random instance;
		return instance;
	}

	// modify the state of the RNG and return the result
	uint32_t update(void)
	{
		state *= 0xe120fc15; // multiplier
		state += 0x00000001; // increment
	//	state %= 0x80000000; // modulus
		state &= 0x7fffffff;
		return state;
	}

}; // class Random