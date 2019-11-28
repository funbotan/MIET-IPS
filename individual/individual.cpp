#include <iostream>
#include <cmath>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_max.h>
#include <cilk/reducer_min.h>
#include <cilk/reducer_vector.h>
#include <cilk/reducer_opadd.h>
#include <chrono>
#include <thread>
using namespace std;


template <typename Duration = std::chrono::microseconds, typename F, typename ... Args>
typename Duration::rep timeit(F&& fun, Args&&... args)
{
	auto begin = std::chrono::high_resolution_clock::now();
	std::forward<F>(fun)(std::forward<Args>(args)...);
	auto end = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<Duration>(end - begin).count();
}

double fun(double x)
{
	return 4/((1+x*x)*(1+x*x));
}

double trapezeS(double a, double b, double h)
{
	double result = 0;
	int n = (b - a) / h;
	double total = h * (fun(a) + fun(b)) / 2.0;
	for (int i = 0; i < n; i++)
		result += h * fun(a + h * i);
	return result + total;
}

double trapezeP(double a, double b, double h)
{
	cilk::reducer_opadd<double> result(0.0);
	int n = (b - a) / h;
	double total = h * (fun(a) + fun(b)) / 2.0;
	cilk_for (int i = 0; i < n; i++)
		*result += h * fun(a + h * i);
	return result.get_value() + total;
}

int main()
{
	double a = -1.0, b = 1.0, h = 0.000000001;

	// cout << "Serial trapeze = " << trapezeS(a, b, h) << endl;
	// cout << "Parallel trapeze = " << trapezeP(a, b, h) << endl;

	int pt = (int)timeit(trapezeP, a, b, h);
	int st = (int)timeit(trapezeS, a, b, h);
	cout << "Serial duration: " << st << " microseconds" << endl;
	cout << "Parallel duration: " << pt << " microseconds" << endl;
	cout << "Boost ratio: " << (double)st/(double)pt << endl;

	return 0;
}
