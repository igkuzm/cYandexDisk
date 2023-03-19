/**
 * File              : uuid.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 22.02.2022
 * Last Modified Date: 21.01.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

/*
 * C Library for generating uuid string
 */

#ifndef k_lib_uuid_h__
#define k_lib_uuid_h__

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

//allocate new uuid string 
static char * uuid_new();


/*
 * Implimation
 */

#ifndef UUID4_FUNCSPEC
	#define UUID4_FUNCSPEC
#endif
#ifndef UUID4_PREFIX
	#define UUID4_PREFIX(x) uuid4_##x
#endif

#ifndef UUID4_STR_BUFFER_SIZE
	#define UUID4_STR_BUFFER_SIZE (int)sizeof("xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx") // y is either 8, 9, a or b
#endif

typedef uint64_t UUID4_PREFIX(state_t);
#define UUID4_STATE_T UUID4_PREFIX(state_t)

typedef union
{
	uint8_t bytes[16];
	uint32_t dwords[4];
	uint64_t qwords[2];
} UUID4_PREFIX(t);
#define UUID4_T UUID4_PREFIX(t)

/**
 * Seeds the state of the PRNG used to generate version 4 UUIDs.
 *
 * @param a pointer to a variable holding the state.
 *
 * @return `true` on success, otherwise `false`.
 */
UUID4_FUNCSPEC
void UUID4_PREFIX(seed)(UUID4_STATE_T* seed);

/**
 * Generates a version 4 UUID, see https://tools.ietf.org/html/rfc4122.
 *
 * @param state the state of the PRNG used to generate version 4 UUIDs.
 * @param out the recipient for the UUID.
 */
UUID4_FUNCSPEC
void UUID4_PREFIX(gen)(UUID4_STATE_T* state, UUID4_T* out);

/**
 * Converts a UUID to a a `NUL` terminated string.
 *
 * @param out destination buffer
 * @param capacity destination buffer capacity, must be greater or equal to
 *   `UUID4_STR_BUFFER_SIZE`.
 *
 * @return `true` on success, otherwise `false`.
 */
UUID4_FUNCSPEC
bool UUID4_PREFIX(to_s)(const UUID4_T uuid, char* out, int capacity);
	

#if defined(__linux__)
	#if !defined(_GNU_SOURCE)
		#define _GNU_SOURCE
	#endif
#endif

// —————————————————————————————————————————————————————————————————————————————

// http://xoshiro.di.unimi.it/splitmix64.c
// Written in 2015 by Sebastiano Vigna (vigna@acm.org)
/*
   This is a fixed-increment version of Java 8's SplittableRandom generator
   See http://dx.doi.org/10.1145/2714064.2660195 and
   http://docs.oracle.com/javase/8/docs/api/java/util/SplittableRandom.html

   It is a very fast generator passing BigCrush.
*/
static inline uint64_t UUID4_PREFIX(splitmix64)(uint64_t* state)
{
	uint64_t z = (*state += 0x9E3779B97F4A7C15u);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9u;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBu;
	return z ^ (z >> 31);
}

// http://www.pcg-random.org/posts/developing-a-seed_seq-alternative.html
// Written in 2015 by Melissa O'Neil (oneill@pcg-random.org)
static inline uint32_t UUID4_PREFIX(hash)(uint32_t value)
{
	static uint32_t multiplier = 0x43b0d7e5u;

	value ^= multiplier;
	multiplier *= 0x931e8875u;
	value *= multiplier;
	value ^= value >> 16;

	return value;
}

static inline uint32_t UUID4_PREFIX(mix)(uint32_t x, uint32_t y)
{
	uint32_t result = 0xca01f9ddu * x - 0x4973f715u * y;
	result ^= result >> 16;
	return result;
}


// —————————————————————————————————————————————————————————————————————————————

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

UUID4_FUNCSPEC
void UUID4_PREFIX(seed)(uint64_t* state)
{
	static uint64_t state0 = 0;

	LARGE_INTEGER time;
	BOOL ok = QueryPerformanceCounter(&time);
	UUID4_ASSERT(ok);

	*state = state0++ + ((uintptr_t)&time ^ (uint64_t)time.QuadPart);

	uint32_t pid = (uint32_t)GetCurrentProcessId();
	uint32_t tid = (uint32_t)GetCurrentThreadId();

	*state = *state * 6364136223846793005u + ((uint64_t)(UUID4_PREFIX(mix)(UUID4_PREFIX(hash)(pid), UUID4_PREFIX(hash)(tid))) << 32);
	*state = *state * 6364136223846793005u + (uintptr_t)GetCurrentProcessId;
	*state = *state * 6364136223846793005u + (uintptr_t)UUID4_PREFIX(gen);
}

#elif defined(__linux__)

#if !defined(UUID4_CLOCK_ID)
	#define UUID4_CLOCK_ID CLOCK_MONOTONIC_RAW
#endif

#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>

UUID4_FUNCSPEC
void UUID4_PREFIX(seed)(uint64_t* state)
{
	static uint64_t state0 = 0;

	struct timespec time;
	bool ok = clock_gettime(UUID4_CLOCK_ID, &time) == 0;
	UUID4_ASSERT(ok);

	*state = state0++ + ((uintptr_t)&time ^ (uint64_t)(time.tv_sec * 1000000000 + time.tv_nsec));

	uint32_t pid = (uint32_t)getpid();
	uint32_t tid = (uint32_t)syscall(SYS_gettid);
	*state = *state * 6364136223846793005u + ((uint64_t)(UUID4_PREFIX(mix)(UUID4_PREFIX(hash)(pid), UUID4_PREFIX(hash)(tid))) << 32);
	*state = *state * 6364136223846793005u + (uintptr_t)getpid;
	*state = *state * 6364136223846793005u + (uintptr_t)UUID4_PREFIX(gen);
}

#elif defined(__APPLE__)

#include <mach/mach_time.h>
#include <unistd.h>
#include <pthread.h>

UUID4_FUNCSPEC
void UUID4_PREFIX(seed)(uint64_t* state)
{
	static uint64_t state0 = 0;

	uint64_t time = mach_absolute_time();

	*state = state0++ + time;

	uint32_t pid = (uint32_t)getpid();
	uint64_t tid;
	pthread_threadid_np(NULL, &tid);
	*state = *state * 6364136223846793005u + ((uint64_t)(UUID4_PREFIX(mix)(UUID4_PREFIX(hash)(pid), UUID4_PREFIX(hash)((uint32_t)tid))) << 32);
	*state = *state * 6364136223846793005u + (uintptr_t)getpid;
	*state = *state * 6364136223846793005u + (uintptr_t)UUID4_PREFIX(gen);
}

#else

#error unsupported platform

#endif

#include <stdio.h>
#include <inttypes.h>

static void UUID4_PREFIX(randomize)(UUID4_STATE_T* state, UUID4_T* out)
{
	out->qwords[0] = UUID4_PREFIX(splitmix64)(state);
	out->qwords[1] = UUID4_PREFIX(splitmix64)(state);
}

UUID4_FUNCSPEC
void UUID4_PREFIX(gen)(UUID4_STATE_T* state, UUID4_T* out)
{
	UUID4_PREFIX(randomize)(state, out);

	out->bytes[6] = (out->bytes[6] & 0xf) | 0x40;
	out->bytes[8] = (out->bytes[8] & 0x3f) | 0x80;
}

UUID4_FUNCSPEC
bool UUID4_PREFIX(to_s)(const UUID4_T uuid, char* out, int capacity)
{
	static const char hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	static const int groups[] = { 8, 4, 4, 4, 12 };
	int b = 0;

	if (capacity < UUID4_STR_BUFFER_SIZE)
		return false;

	for (int i = 0; i < (int)(sizeof(groups) / sizeof(groups[0])); ++i)
	{
		for (int j = 0; j < groups[i]; j += 2)
		{
			uint8_t byte = uuid.bytes[b++];
			
			*out++ = hex[byte >> 4];
			*out++ = hex[byte & 0xf];
		}
		*out++ = '-';
	}

	*--out = 0;

	return true;
}

char * uuid_new(){
	//create uuid
	char * uuid = malloc(37);
	if (!uuid)
		return NULL;
	UUID4_STATE_T state; UUID4_T identifier;
	uuid4_seed(&state);
	uuid4_gen(&state, &identifier);
	if (!uuid4_to_s(identifier, uuid, 37)){
		return NULL;
	}

	return uuid;
}


#ifdef __cplusplus
}
#endif

#endif //k_lib_uuid_h__
