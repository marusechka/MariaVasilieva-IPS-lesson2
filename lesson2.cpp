#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_max.h>
#include <cilk/reducer_min.h>
#include <cilk/reducer_vector.h>
#include <chrono>
#include <iostream>

using namespace std::chrono;

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
	printf("\nМаксимальный элемент = %d с индексом = %d\n\n",
		maximum->get_reference(), maximum->get_index_reference());
}

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
	printf("Минимальный элемент = %d с индексом = %d\n\n",
		minimum->get_reference(), minimum->get_index_reference());
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

/// Функция CompareForAndCilk_For выводит на консоль
///время выполнения цикла for
///время выполнения цикла cilk_for
void CompareForAndCilk_For(size_t sz)
{
	std::vector<int> myVector; // мы создали пустой вектор типа int
	myVector.reserve(sz);      // тут мы зарезервировали память под sz элементов типа int
	high_resolution_clock::time_point t1, t2, t3, t4;
	
	t1 = high_resolution_clock::now(); //засекли время
	
	for (int i = 0; i < sz; i++)
		myVector.push_back(rand() % 20000 + 1);

	t2 = high_resolution_clock::now();
	duration<double> duration = (t2 - t1); //время выполнения цикла for
	std::cout << "Длительность выполнения цикла for: " << duration.count() << " секунд" << std::endl;
	
	cilk::reducer<cilk::op_vector<int>>red_vec; //объявление reducer вектора 
	t3 = high_resolution_clock::now();
	
	cilk_for(long i = 0; i < sz; ++i)
		red_vec->push_back(rand() % 20000 + 1); //заполнение reducer вектора

	t4 = high_resolution_clock::now();
	duration = (t4 - t3); // время выполнения cilk_for
	std::cout << "Длительность выполнения цикла cilk_for: " << duration.count() << " секунд" << std::endl; 
	myVector.clear();

}


int main()
{
	srand((unsigned)time(0));
	setlocale(LC_ALL, "RUS");

	// устанавливаем количество работающих потоков = 4
	__cilkrts_set_param("nworkers", "4");

	long i;
	const long mass_size = 10000;
	int *mass_begin, *mass_end;
	int *mass = new int[mass_size];


	for (i = 0; i < mass_size; ++i)
	{
		mass[i] = (rand() % 25000) + 1;
	}

	mass_begin = mass;
	mass_end = mass_begin + mass_size;
	std::cout << "Количество элементов массива: " << mass_size << std::endl;
	ReducerMaxTest(mass, mass_size);
	ReducerMinTest(mass, mass_size);

	high_resolution_clock::time_point t1, t2;
	t1 = high_resolution_clock::now();
	ParallelSort(mass_begin, mass_end);
	t2 = high_resolution_clock::now();
	duration<double> duration = (t2 - t1);

	std::cout << "Время выполнения сортировки: " << duration.count() << " секунд\n" << std::endl;
	std::cout << "---------После сортировки--------- \n" <<  std::endl;
	ReducerMaxTest(mass, mass_size);
	ReducerMinTest(mass, mass_size);

	std::cout << "-------CompareForAndCilk_For------- \n" << std::endl;

	int mas_for[8] = { 10, 50, 100, 500, 1000, 10000, 100000, 1000000 };
	for (i = 0; i < 8; i++)
	{
		std::cout << "\nКоличество элементов массива: " << mas_for[i] << std::endl;
		CompareForAndCilk_For(mas_for[i]);

	}
	
	delete[]mass;
	system("pause");
	return 0;
}
