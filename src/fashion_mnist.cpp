#include <fstream>
#include <iostream>
#include <limits>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "csv_parser.h"


// Загрузка коэффициентов логистической регрессии из файла (матрица 10x785).
static bool load_logreg_model(const std::string &path, std::vector<std::vector<double>> &W)
{
    std::ifstream fin(path);
    if(fin.is_open() == false)
    {
        return false;
    }
    std::string line;
    W.clear();
    while(std::getline(fin, line))
    {
        if(line.empty()) continue;
        std::istringstream ss(line);
        std::vector<double> row;
        double v;
        while(ss >> v)
        {
            row.push_back(v);
        }
        if(row.empty() == false)
        {
            W.push_back(std::move(row));
        }
    }
    return !W.empty();
}

// Предсказание класса: считаем 10 линейных скорингов (bias + w*x) и берём argmax.
static int predict_logreg(const std::vector<std::vector<double>> &W, const std::vector<double> &x)
{
    int best_class = -1;
    double best_score = -std::numeric_limits<double>::infinity();
    for(size_t k = 0; k < W.size(); ++k)
    {
        const auto &row = W[k];
        double s = 0.0;
        if(!row.empty()) s += row[0];
        const size_t n = std::min(x.size(), row.size() > 0 ? row.size() - 1 : 0);
        for(size_t i = 0; i < n; ++i)
        {
            s += row[i + 1] * x[i];
        }
        if(s > best_score)
        {
            best_score = s;
            best_class = static_cast<int>(k);
        }
    }
    return best_class;
}

int main(int argc, char **argv)
{
    // Вход: test.csv (label + 784 пикселя) и logreg_coef.txt (коэффициенты модели).
    if(argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <test.csv> <logreg_coef.txt>" << std::endl;
        return 1;
    }

    const std::string test_path = argv[1];
    const std::string model_path = argv[2];

    std::vector<std::vector<double>> W;
    if(load_logreg_model(model_path, W) == false)
    {
        std::cerr << "Failed to load model from " << model_path << std::endl;
        return 1;
    }

    std::ifstream fin(test_path);
    if(fin.is_open() == false)
    {
        std::cerr << "Failed to open test data: " << test_path << std::endl;
        return 1;
    }

    std::string line;
    size_t total = 0;
    size_t correct = 0;
    std::vector<double> x(784);

    // Проходим по CSV, парсим строку и считаем accuracy.
    while(std::getline(fin, line))
    {
        int label = -1;
        if(csv_parser::parseRow<double>(line, label, x) == false)
        {
            continue;
        }

        int pred = predict_logreg(W, x);
        if(pred == label)
        {
            ++correct;
        }
        ++total;
    }

    if(total == 0)
    {
        std::cerr << "No valid rows in test data" << std::endl;
        return 1;
    }

    double acc = static_cast<double>(correct) / static_cast<double>(total);
    std::cout << std::fixed << std::setprecision(3) << acc << std::endl;
    return 0;
}
