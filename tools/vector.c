#include <malloc.h>
#include <stdint.h>
#include <string.h>

#ifndef MYY_TYPES
#define MYY_TYPES
#ifndef __cplusplus
enum bool_value { false, true };
typedef uint_fast8_t bool;
#endif
#endif

struct myy_vector {
	uintptr_t begin;
	uintptr_t last;
	uintptr_t end;
};

static inline bool myy_vector_can_add(
	struct myy_vector const * __restrict const vector,
	size_t const n_octets)
{
	return ((vector->last + n_octets) < vector->end);
}

static inline size_t myy_vector_size(
	struct myy_vector const * __restrict const vector)
{
	return (size_t) (vector->end - vector->begin);
}

static inline size_t myy_vector_last_offset(
	struct myy_vector const * __restrict const vector)
{
	return vector->last - vector->begin;
}

static inline bool myy_vector_expand_to_store_at_least(
	struct myy_vector * const vector,
	size_t const n_octets)
{
	size_t const vector_size     = myy_vector_size(vector);
	size_t const new_vector_size = vector_size + n_octets;
	size_t const vector_last_offset =
		myy_vector_last_offset(vector);
	uintptr_t new_begin = (uintptr_t) realloc(
		(uint8_t * __restrict) vector->begin,
		new_vector_size);

	bool success = (new_begin != 0);

	if (success) {
		vector->begin = new_begin;
		vector->last  = new_begin + vector_last_offset;
		vector->end   = new_begin + new_vector_size;
	}

	return success;
}

static bool myy_vector_add(
	struct myy_vector * const vector,
	size_t const n_octets,
	uint8_t const * __restrict const source)
{
	bool can_add = myy_vector_can_add(vector, n_octets);
	bool added = false;

	if (can_add ||
	    myy_vector_expand_to_store_at_least(vector, n_octets))
	{
		memcpy(
			(uint8_t * __restrict) vector->last,
			source,
			n_octets);
		vector->last += n_octets;
		added = true;
	}

	return added;
}

static struct myy_vector myy_vector_init(
	size_t const n_octets)
{
	struct myy_vector vector;

	uintptr_t const begin = (uintptr_t) (malloc(n_octets));
	vector.begin = begin;
	vector.last  = begin;
	vector.end   = begin + n_octets;

	return vector;
}

static void myy_vector_free_content(
	struct myy_vector const vector)
{
	free((uint8_t * __restrict) vector.begin);
}

static inline bool myy_vector_is_valid(
	struct myy_vector const * __restrict const vector)
{
	return (((uint8_t const * __restrict) vector->begin) != NULL);
}

int main() {
	struct myy_vector vector = myy_vector_init(512);
	if (!myy_vector_is_valid(&vector))
		fprintf(stderr, "WHAT !!?");
	myy_vector_free_content(vector);
	return 0;
}
