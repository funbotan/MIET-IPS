#### №1

```
Generated matrix:
1.000000 4.000000 2.000000 
4.000000 2.000000 5.000000 

Average values in rows:
Row 0: 2.333333
Row 1: 3.666667

Average values in columns:
Column 0: 2.500000
Column 1: 3.000000
Column 2: 3.500000
```

Очевидно, программа работоспособна.

#### №2

Введем параллелизацию:

```
void FindAverageValues( eprocess_type proc_type, double** matrix, const size_t numb_rows, const size_t numb_cols, double* average_vals )
{
	switch ( proc_type )
	{
		case eprocess_type::by_rows:
		{
			cilk_for ( size_t i = 0; i < numb_rows; ++i )
			{
				cilk::reducer_opadd<double> sum(0.0);
				cilk_for ( size_t j = 0; j < numb_cols; ++j )
				{
					sum += matrix[i][j];
				}
				average_vals[i] = sum.get_value() / numb_cols;
			}
			break;
		}
		case eprocess_type::by_cols:
		{
			cilk_for ( size_t j = 0; j < numb_cols; ++j )
			{
				cilk::reducer_opadd<double> sum(0.0);
				cilk_for( size_t i = 0; i < numb_rows; ++i )
				{
					sum += matrix[i][j];
				}
				average_vals[j] = sum.get_value() / numb_rows;
			}
			break;
		}
		default:
		{
			throw("Incorrect value for parameter 'proc_type' in function FindAverageValues() call!");
		}
	}
}
```

Корректность результата не нарушилась:

```
Generated matrix:
1.000000 3.000000 3.000000 
1.000000 2.000000 4.000000 

Average values in rows:
Row 0: 2.333333
Row 1: 2.333333

Average values in columns:
Column 0: 1.000000
Column 1: 2.500000
Column 2: 3.500000
```

#### №3

Intel Parallel Inspector > Detect leaks:

![Inspector](https://raw.githubusercontent.com/funbotan/MIET-IPS/master/lab3/inspector1.png)

Intel Parallel Inspector > Detect memory problems:

![Inspector](https://raw.githubusercontent.com/funbotan/MIET-IPS/master/lab3/inspector2.png)

Intel Parallel Inspector > Locate memory problems:

![Inspector](https://raw.githubusercontent.com/funbotan/MIET-IPS/master/lab3/inspector3.png)

Обнаружена утечка памяти: не освобождается память, выделенная для тестовой матрицы. Хотя в реальности это не приведет к проблемам, т.к. матрица создается однократно и ОС автоматически очистит пространство процесса при его завершении.

Intel Parallel Inspector > Detect deadlocks:

![Inspector](https://raw.githubusercontent.com/funbotan/MIET-IPS/master/lab3/inspector4.png)

Проблем с конфликтами потоков не обнаружено.

#### №4

Исправим утечку памяти:

```
delete[] average_vals_in_cols;
delete[] average_vals_in_rows;

for (size_t i = 0; i < numb_rows; ++i)
	delete[] matrix[i];
delete[] matrix;
```

![Inspector](https://raw.githubusercontent.com/funbotan/MIET-IPS/master/lab3/inspector5.png)

Больше проблем ни в одном из режимов нет.