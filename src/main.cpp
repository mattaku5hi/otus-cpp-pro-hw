
#include <cassert>
#include <iostream>
#include <tuple>

#include "matrix.h"


static bool test_suite(void);


static bool test_suite(void)
{
    std::cout << "===== Test suite =====\n"; 
    
    // 1) Матрица с пустым значением 0
    Matrix<int, 0> matrix;

    // 2) Главная диагональ [0,0]..[9,9] значениями 0..9
    for(int i = 0; i < 10; ++i) 
    {
        matrix[i][i] = i; // присвоение 0 освободит [0,0]
    }

    // 3) Побочная диагональ [0,9]..[9,0] значениями 9..0
    for(int i = 0; i < 10; ++i) 
    {
        matrix[i][9 - i] = 9 - i; // присвоение 0 освободит [9,0]
    }

    // 4) Вывести фрагмент [1,1]..[8,8]
    std::cout << "Matrix fragment [1,1]..[8,8]:\n";
    for(int i = 1; i <= 8; ++i) 
    {
        for(int j = 1; j <= 8; ++j)
        {
            int v = matrix[i][j];
            if(j > 1)
            {
                std::cout << ' ';
            }
            std::cout << v;
        }
        std::cout << '\n';
    }

    // 5) Количество занятых ячеек
    std::cout << "Occupied cells amount: " << matrix.size() << '\n'; // ожидаемо 18

    std::cout << "Occupied cells: {x} {y} {value}\n";
    // 6) Вывести все занятые ячейки: x y value
    for(auto t : matrix)
    {
        int x;
        int y;
        int v;
        std::tie(x, y, v) = t;
        std::cout << x << ' ' << y << ' ' << v << '\n';
    }

    // 7) Потоковое присваивание
    ((matrix[100][100] = 314) = 0) = 217;
    assert(matrix[100][100] == 217);
    assert(matrix.size() == 19);

    return true;
}


int main(int, char **)
{ 
    return test_suite();
}
