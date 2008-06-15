#ifndef INC_MT_PSEUDO_RANDOM_NUMBER_GENERATOR_H_
#define INC_MT_PSEUDO_RANDOM_NUMBER_GENERATOR_H_

/* 
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)  
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

/** C++ wrapper and optimizations by Werner BEROUX
 * E-Mail: werner@beroux.com
 *
 * @version 1.12
 * @date 2007-06-11
 */

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <time.h>

#if !defined(ASSERT)
	#include <assert.h>
	#ifdef _DEBUG
		#define ASSERT(f)			assert(f)
	#else
		#define ASSERT(f)			((void)0)
	#endif
#endif

namespace Beroux{ namespace Math
{
	class RandomMT
	{
	// Construction
	public:
		RandomMT()
		{
			Initialize( 5489UL );
		}
		RandomMT(unsigned long seed)
		{
			Initialize( seed );
		}

		// Initializes mt[N] with a seed..
		void Initialize(unsigned long seed)
		{
			mt[0]= seed & 0xffffffffUL;
			for (mti=1; mti<N; mti++) {
				mt[mti] = 
					(1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
				/* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
				/* In the previous versions, MSBs of the seed affect   */
				/* only MSBs of the array mt[].                        */
				/* 2002/01/09 modified by Makoto Matsumoto             */
				mt[mti] &= 0xffffffffUL;
				/* for >32 bit machines */
			}
		}

		/** Initialize by an array with array-length
		* init_key is the array for initializing keys
		* key_length is its length
		* slight change for C++, 2004/2/26
		*/
		void InitialiteByArray(unsigned long init_key[], int key_length)
		{
			int i, j, k;
			Initialize(19650218UL);
			i=1; j=0;
			k = (N>key_length ? N : key_length);
			for (; k; k--) {
				mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
					+ init_key[j] + j; /* non linear */
				mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
				i++; j++;
				if (i>=N) { mt[0] = mt[N-1]; i=1; }
				if (j>=key_length) j=0;
			}
			for (k=N-1; k; k--) {
				mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
					- i; /* non linear */
				mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
				i++;
				if (i>=N) { mt[0] = mt[N-1]; i=1; }
			}

			mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */ 
		}

		// Initialize using current time.
		void Randomize(void)
		{
			Initialize((unsigned int) time(NULL) );
		}

	// Operations
		// Generates a random number on [0,0xffffffff]-interval.
		inline unsigned long RandomInt(void)
		{
			unsigned long y;
			static unsigned long mag01[2]={0x0UL, MATRIX_A};
			/* mag01[x] = x * MATRIX_A  for x=0,1 */

			if (mti >= N) { /* generate N words at one time */
				int kk;

				for (kk=0;kk<N-M;kk++) {
					y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
					mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
				}
				for (;kk<N-1;kk++) {
					y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
					mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
				}
				y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
				mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

				mti = 0;
			}

			y = mt[mti++];

			/* Tempering */
			y ^= (y >> 11);
			y ^= (y << 7) & 0x9d2c5680UL;
			y ^= (y << 15) & 0xefc60000UL;
			y ^= (y >> 18);

			return y;
		}

		// Generates a random number on [0,0x7fffffff]-interval.
		inline long RandomLong(void)
		{
			return (long)(RandomInt()>>1);
		}

		// Generates a random number on [0,1]-real-interval.
		inline double RandomReal1(void)
		{
			return RandomInt()*(1.0/4294967295.0); 
			/* divided by 2^32-1 */ 
		}

		// Generates a random number on [0,1)-real-interval.
		inline double RandomReal2(void)
		{
			return RandomInt()*(1.0/4294967296.0); 
			/* divided by 2^32 */
		}

		// Generates a random number on (0,1)-real-interval.
		inline double RandomReal3(void)
		{
			return (((double)RandomInt()) + 0.5)*(1.0/4294967296.0); 
			/* divided by 2^32 */
		}

		// Generates a random number on [0,1) with 53-bit resolution.
		inline double RandomRes53(void) 
		{ 
			unsigned long a=RandomInt()>>5, b=RandomInt()>>6; 
			return(a*67108864.0+b)*(1.0/9007199254740992.0); 
		} 
		/* These real versions are due to Isaku Wada, 2002/01/09 added */

		// Generate a random boolean like a 2 faced coin.
		inline bool RandomBool()
		{
			return RandomInt() % 2 == 0; 
		}

		// Generates a random number on [low,hi]-interval.
		inline int RandomRange(int low, int hi)
		{
			ASSERT(hi > low);
			return RandomInt() % (hi - low + 1) + low;
		}

		// Generates a random number on [low,hi]-interval.
		inline double RandomRange(double low, double hi)
		{
			ASSERT(hi > low);
			return RandomInt()*(hi-low)/4294967295.0 + low;
		}

		// Generates a random number on [low,hi]-interval.
		inline float RandomRange(float low, float hi)
		{
			ASSERT(hi > low);
			return RandomInt()*(hi-low)/4294967295.0f + low;
		}

		// Generates a random number on [base-delta,base+delta]-interval.
		inline float RandomApprox(float base, float delta)
		{
			ASSERT(delta > 0.0f);
			return (RandomInt()*(2.0f/4294967295.0f)-1.0f)*delta + base;
		}

		// Generates random dice numbers for RPG like 2d6.
		inline int RollDice(int nDices, int nFaces)
		{
			int sum = 0;
			for (int loop=0; loop<nDices; loop++)
			{
				sum += RandomRange(1,nFaces);
			}
			return sum;
		}

	// Implementation
	private:
		// Period parameters
		static const int N = 624;
		static const int M = 397;
		static const unsigned long MATRIX_A = 0x9908b0dfUL;	/* constant vector a */
		static const unsigned long UPPER_MASK = 0x80000000UL;	/* most significant w-r bits */
		static const unsigned long LOWER_MASK = 0x7fffffffUL;	/* least significant r bits */

		unsigned long mt[N]; /* the array for the state vector  */
		int mti;
	};
}}

#endif

