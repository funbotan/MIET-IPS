#include <stdio.h>
#include <ctime>
#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h>
#include <chrono>

using namespace std::chrono;
template <typename Duration = std::chrono::microseconds, typename F, typename ... Args>
typename Duration::rep timeit(F&& fun, Args&&... args)
{
	auto begin = std::chrono::high_resolution_clock::now();
	std::forward<F>(fun)(std::forward<Args>(args)...);
	auto end = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<Duration>(end - begin).count();
}

// ���������� ����� � �������� ���������� �������
const int MATRIX_SIZE = 1500;

/// ������� InitMatrix() ��������� ���������� � �������� 
/// ��������� ���������� ������� ���������� ����������
/// matrix - �������� ������� ����
void InitMatrix( double** matrix )
{
	for ( int i = 0; i < MATRIX_SIZE; ++i )
		matrix[i] = new double[MATRIX_SIZE + 1];
	for ( int i = 0; i < MATRIX_SIZE; ++i )
		for ( int j = 0; j <= MATRIX_SIZE; ++j )
			matrix[i][j] = rand() % 2500 + 1;
}

/// ������� SerialGaussMethod() ������ ���� ������� ������ 
/// matrix - �������� ������� �������������� ���������, �������� � ����,
/// ��������� ������� ������� - �������� ������ ������ ���������
/// rows - ���������� ����� � �������� �������
/// result - ������ ������� ����
int SerialGaussMethod( double **matrix, const int rows, double* result )
{
	int k;
	double koef;
	auto begin = std::chrono::high_resolution_clock::now();

	// ������ ��� ������ ������
	for ( k = 0; k < rows; ++k )
		for (int i = k + 1; i < rows; ++i) {
			koef = -matrix[i][k] / matrix[k][k];
			for (int j = k; j <= rows; ++j)
				matrix[i][j] += koef * matrix[k][j];
		}
	auto end = std::chrono::high_resolution_clock::now();

	// �������� ��� ������ ������
	result[rows - 1] = matrix[rows - 1][rows] / matrix[rows - 1][rows - 1];
	for ( k = rows - 2; k >= 0; --k ) {
		result[k] = matrix[k][rows];
		for ( int j = k + 1; j < rows; ++j )
			result[k] -= matrix[k][j] * result[j];
		result[k] /= matrix[k][k];
	}
	return (int)std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
}

int ParallelGaussMethod(double** matrix, const int rows, double* result)
{
	int k;
	auto begin = std::chrono::high_resolution_clock::now();

	for (k = 0; k < rows; ++k)
		cilk_for(int i = k + 1; i < rows; ++i) {
			double koef = -matrix[i][k] / matrix[k][k];
			for (int j = k; j <= rows; ++j)
				matrix[i][j] += koef * matrix[k][j];
		}
	auto end = std::chrono::high_resolution_clock::now();

	result[rows - 1] = matrix[rows - 1][rows] / matrix[rows - 1][rows - 1];
	for (k = rows - 2; k >= 0; --k) {
		cilk::reducer_opadd<double> res(matrix[k][rows]);
		cilk_for(int j = k + 1; j < rows; ++j)
			res -= matrix[k][j] * result[j];
		result[k] = res->get_value() / matrix[k][k];
	}
	return (int)std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
}


int main()
{
	srand( (unsigned) time( 0 ) );
	int i;

	// êîë-âî ñòðîê â ìàòðèöå, ïðèâîäèìîé â êà÷åñòâå ïðèìåðà
	//const int test_matrix_lines = 4;
	const int test_matrix_lines = MATRIX_SIZE;
	double **test_matrix = new double*[test_matrix_lines];
	// öèêë ïî ñòðîêàì
	for ( i = 0; i < test_matrix_lines; ++i )
	{
		// (test_matrix_lines + 1)- êîëè÷åñòâî ñòîëáöîâ â òåñòîâîé ìàòðèöå,
		// ïîñëåäíèé ñòîëáåö ìàòðèöû îòâåäåí ïîä ïðàâûå ÷àñòè óðàâíåíèé, âõîäÿùèõ â ÑËÀÓ
		test_matrix[i] = new double[test_matrix_lines + 1];
	}

	// ìàññèâ ðåøåíèé ÑËÀÓ
	double *result = new double[test_matrix_lines];

	// èíèöèàëèçàöèÿ òåñòîâîé ìàòðèöû
	/*
	test_matrix[0][0] = 2; test_matrix[0][1] = 5;  test_matrix[0][2] = 4;  test_matrix[0][3] = 1;  test_matrix[0][4] = 20;
	test_matrix[1][0] = 1; test_matrix[1][1] = 3;  test_matrix[1][2] = 2;  test_matrix[1][3] = 1;  test_matrix[1][4] = 11;
	test_matrix[2][0] = 2; test_matrix[2][1] = 10; test_matrix[2][2] = 9;  test_matrix[2][3] = 7;  test_matrix[2][4] = 40;
	test_matrix[3][0] = 3; test_matrix[3][1] = 8;  test_matrix[3][2] = 9;  test_matrix[3][3] = 2;  test_matrix[3][4] = 37;
	*/

	InitMatrix(test_matrix);
	int ts = SerialGaussMethod(test_matrix, test_matrix_lines, result);
	int tp = ParallelGaussMethod(test_matrix, test_matrix_lines, result);
	printf("Serial Gauss duration: %d microseconds\n", ts);
	printf("Parallel Gauss duration: %d microseconds\n", tp);
	printf("Boost ratio: %f\n", (float)ts / tp);

	for ( i = 0; i < test_matrix_lines; ++i )
		delete[]test_matrix[i];

	/*
	printf( "Solution:\n" );
	for ( i = 0; i < test_matrix_lines; ++i )
		printf( "x(%d) = %lf\n", i, result[i] );
	*/
	delete[] result;
	return 0;
}
