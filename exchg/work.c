#include <work.h>
#include <stdlib.h>

static void minheapify(unsigned N, eltype *const nums, const unsigned i)
{
	const unsigned left = 2 * i + 1;
	const unsigned right = 2 * i + 2;
	
	unsigned smallest = 0;

	if(left < N && nums[left] < nums[i])
	{
		smallest = left;
	}
	else
	{
		smallest = i;
	}

	if(right < N && nums[right] < nums[smallest])
	{
		smallest = right;
	}

	if(smallest != i)
	{
		const eltype tmp = nums[i];
		nums[i] = nums[smallest];
		nums[smallest] = tmp;

		minheapify(N, nums, smallest);
	}
}

static void buildheap(const unsigned N, eltype *const nums)
{
	unsigned i = N / 2 + 1;

	do
	{
		i -= 1;
		minheapify(N, nums, i);	
	} while(i);
}

eltype heapsum(eltype *const nums, const unsigned N)
{
	unsigned n = N;

	while(n > 1)
	{
		buildheap(n, nums);
		eltype m1 = nums[0];
		
		nums[0] = nums[n - 1];
		buildheap(n - 1, nums);
		eltype m2 = nums[0];

		nums[0] = m1 + m2;
		buildheap(n - 1, nums);

		n -= 1;
	}

	return nums[0];	
}

eltype elrand(unsigned *const seed)
{
	return (eltype)1.0 / (eltype)((rand_r(seed) >> 24) + 1);
}
