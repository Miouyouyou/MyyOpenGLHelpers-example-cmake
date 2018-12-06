#include <malloc.h>
#include <stdint.h>
#include <string.h>

#include <vector.h>

#define ALIGN_ON_POW2(x, p) ((x)+(p-1) & ~(p-1))

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

struct myy_vector myy_vector_init(
	size_t const n_octets)
{
	struct myy_vector vector;

	size_t allocated_size = ALIGN_ON_POW2(allocated_size, 4096);
	uintptr_t const begin = (uintptr_t) (malloc(allocated_size));
	vector.begin = begin;
	vector.last  = begin;
	vector.end   = begin + allocated_size;

	return vector;
}
