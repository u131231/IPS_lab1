#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_max.h>
#include <cilk/reducer_min.h>
#include <cilk/reducer_vector.h>
#include <chrono>

using namespace std::chrono;

/// Функция ReducerMinTest() определяет минимальный элемент массива,
/// переданного ей в качестве аргумента, и его позицию
/// mass_pointer - указатель исходный массив целых чисел
/// size - количество элементов в массиве
void ReducerMinTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_min_index<long, int>> minimum;
	cilk_for(long i = 0; i < size; ++i)
	{
		minimum->calc_min(i, mass_pointer[i]);
	}
	printf("Minimal element = %d has index = %d\n\n",
		minimum->get_reference(), minimum->get_index_reference());
}

/// Функция ReducerMaxTest() определяет максимальный элемент массива,
/// переданного ей в качестве аргумента, и его позицию
/// mass_pointer - указатель исходный массив целых чисел
/// size - количество элементов в массиве
void ReducerMaxTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_max_index<long, int>> maximum;
	cilk_for(long i = 0; i < size; ++i)
	{
		maximum->calc_max(i, mass_pointer[i]);
	}
	printf("Maximal element = %d has index = %d\n\n",
		maximum->get_reference(), maximum->get_index_reference());
}

/// Функция ParallelSort() сортирует массив в порядке возрастания
/// begin - указатель на первый элемент исходного массива
/// end - указатель на последний элемент исходного массива
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

/// Функция CompareForAndCilk_For() создает два вектора, первый обычный,
/// второй reducer вектор и выводит время их заполнения
/// sz - количество элементов в массиве
void CompareForAndCilk_For(size_t sz)
{
	std::vector<int>vec;
	cilk::reducer<cilk::op_vector<int>>red_vec;

	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	for (long i = 0; i < sz; ++i)
	{
		vec.push_back(rand() % 20000 + 1);
	}
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> duration = (t2 - t1);
	printf("Duration For is: %f seconds\n", duration.count());

	t1 = high_resolution_clock::now();
	cilk_for(long i = 0; i < sz; ++i)
	{
		red_vec->push_back(rand() % 20000 + 1);
	}
	t2 = high_resolution_clock::now();
	duration = (t2 - t1);
	printf("Duration Cilk_For is: %f seconds\n", duration.count());
}

int main()
{
	srand((unsigned)time(0));

	// устанавливаем количество работающих потоков = 4
	__cilkrts_set_param("nworkers", "4");


	long i;
	const long mass_size = 10000;
	int *mass_begin, *mass_end;
	int *mass = new int[mass_size]; 

	for(i = 0; i < mass_size; ++i)
	{
		mass[i] = (rand() % 25000) + 1;
	}
	
	mass_begin = mass;
	mass_end = mass_begin + mass_size;
	printf("mass_size = %d\n\n", mass_size);
	ReducerMaxTest(mass, mass_size);
	ReducerMinTest(mass, mass_size);

	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	ParallelSort(mass_begin, mass_end);
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> duration = (t2 - t1);
	printf("Duration is: %f seconds\n\n", duration.count());

	ReducerMaxTest(mass, mass_size);
	ReducerMinTest(mass, mass_size);
	
	printf("-----------------------------------------\n\n");

	// запуск в цикле функции CompareForAndCilk_For с разными значениями размеров векторов
	const long sz[8] = { 1000000, 100000, 10000, 1000, 500, 100, 50, 10 };
	for (int i = 0; i < 8; ++i)
	{
		printf("sz = %d\n", sz[i]);
		CompareForAndCilk_For(sz[i]);
	}

	delete[]mass;
	getchar();
	return 0;
}