#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_max.h>
#include <cilk/reducer_min.h>
#include <cilk/reducer_vector.h>
#include <chrono>
using namespace std::chrono;


template <typename Duration = std::chrono::microseconds, typename F, typename ... Args>
typename Duration::rep timeit(F&& fun,  Args&&... args)
{
	auto begin = std::chrono::high_resolution_clock::now();
	std::forward<F>(fun)(std::forward<Args>(args)...);
	auto end = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<Duration>(end - begin).count();
}

/// Ôóíêöèÿ ReducerMaxTest() îïðåäåëÿåò ìàêñèìàëüíûé ýëåìåíò ìàññèâà,
/// ïåðåäàííîãî åé â êà÷åñòâå àðãóìåíòà, è åãî ïîçèöèþ
/// mass_pointer - óêàçàòåëü èñõîäíûé ìàññèâ öåëûõ ÷èñåë
/// size - êîëè÷åñòâî ýëåìåíòîâ â ìàññèâå
void ReducerMinTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_min_index<long, int>> minimum;
	cilk_for(long i = 0; i < size; ++i)
	{
		minimum->calc_min(i, mass_pointer[i]);
	}
	printf("Minimal element = %d has index = %d\n",
		minimum->get_reference(), minimum->get_index_reference());
}

void ReducerMaxTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_max_index<long, int>> maximum;
	cilk_for(long i = 0; i < size; ++i)
	{
		maximum->calc_max(i, mass_pointer[i]);
	}
	printf("Maximal element = %d has index = %d\n",
		maximum->get_reference(), maximum->get_index_reference());
}

/// Ôóíêöèÿ ParallelSort() ñîðòèðóåò ìàññèâ â ïîðÿäêå âîçðàñòàíèÿ
/// begin - óêàçàòåëü íà ïåðâûé ýëåìåíò èñõîäíîãî ìàññèâà
/// end - óêàçàòåëü íà ïîñëåäíèé ýëåìåíò èñõîäíîãî ìàññèâà
void ParallelSort(int *begin, int *end)
{
	if (begin != end)
	{
		--end;
		int *middle = std::partition(begin, end, std::bind2nd(std::less<int>(), *end));
		std::swap(*end, *middle);
		cilk_spawn ParallelSort(begin, middle);
		ParallelSort(++middle, ++end);
		cilk_sync;
	}
}

void CompareForAndCilk_For(size_t sz)
{
	printf("\nComparing for array size = %d\n", (int)sz);
	std::vector<int> vtest;
	auto begin = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < sz; i++)
		vtest.push_back(rand() % 20000 + 1);
	auto end = std::chrono::high_resolution_clock::now();
	printf("FOR duration: %d\n", (int)std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
	cilk::reducer<cilk::op_vector<int>> ctest;
	begin = std::chrono::high_resolution_clock::now();
	cilk_for (size_t i = 0; i < sz; i++)
		ctest->push_back(rand() % 20000 + 1);
	end = std::chrono::high_resolution_clock::now();
	printf("CILK_FOR duration: %d\n", (int)std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
}


int main()
{
	srand((unsigned)time(0));

	// óñòàíàâëèâàåì êîëè÷åñòâî ðàáîòàþùèõ ïîòîêîâ = 4
	__cilkrts_set_param("nworkers", "4");

	long i;
	const long mass_size = 10000;
	int *mass_begin, *mass_end;
	int *mass = new int[mass_size];

	printf("Array size = %d\n", mass_size);

	for(i = 0; i < mass_size; ++i)
		mass[i] = (rand() % 25000) + 1;

	mass_begin = mass;
	mass_end = mass_begin + mass_size;
	ReducerMinTest(mass, mass_size);
	ReducerMaxTest(mass, mass_size);

	printf("Sorting time: %d microseconds\n",
			(int)timeit(ParallelSort, mass_begin, mass_end));

	ReducerMinTest(mass, mass_size);
	ReducerMaxTest(mass, mass_size);

	delete[]mass;
	return 0;
}
