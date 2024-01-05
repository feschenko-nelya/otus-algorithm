#include <cmath>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include <dirent.h>

// вывод количества времени от точки отсчёта до момента вызова этой функции
void printDuration(std::chrono::steady_clock::time_point startPoint)
{
    auto end = std::chrono::steady_clock::now();
    auto diff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - startPoint);
    auto diff_s = std::chrono::duration_cast<std::chrono::seconds>(end - startPoint);
    auto diff_m = std::chrono::duration_cast<std::chrono::minutes>(end - startPoint);

    std::cout << "Executed time: " << diff_ms.count() << " ms => " << diff_s.count() << " sec => " << diff_m.count() << " min" << std::endl;
}

// алгоритм адаптирован с js. Источник: https://habr.com/ru/articles/266479/
std::vector<long long> getNextArr(const std::vector<long long> &prevArr)
{
    std::vector<long long> res;
    res.reserve(prevArr.size() + 9);

    for (int i = 0; i < prevArr.size() + 9; ++i)
    {
        long long val = 0;

        for (int j = 0; j < 10; ++j)
        {
            const int k = i - j;
            if (k >= 0 && k < prevArr.size())
            {
                val += prevArr[k];
            }
        }

        res.push_back(val);
    }

    return res;
}

long long calcNumbers(int N)
{
    std::vector<long long> arr;
    for (int i = 0; i < 10; ++i)
    {
        arr.push_back(1);
    }

    for (int i = 0; i < N / 2 - 1; ++i)
    {
        arr = getNextArr(arr);
    }

    long long count = 0;
    for (int i = 0; i < arr.size(); ++i)
    {
        count += powl(arr[i], 2);
    }

    return count;
}

void test(const std::string &dirPath)
{
    // - считываются файлы в указанной папке
    // - из файла с расширением .in загружается N
    // - из файла с расширением .out загружается эталонное количество счастливых чисел
    // - считанное N передаётся функции расчёта
    // - полученное значение сверяется с эталонным
    // - выводится результат: что тестировалось и что получилось

    struct TestUnit
    {
        int inValue = -1;
        long long outValue = -1;

        void clear()
        {
            inValue = -1;
            outValue = -1;
        }

        bool isValid()
        {
            return ((inValue >= 0) && (outValue >= 0));
        }

        void addValue(long long value)
        {
            if (isValid())
            {
                throw std::invalid_argument("Test values should be reset.");
            }

            if (inValue < 0)
            {
                inValue = value;
            }
            else
            {
                outValue = value;
            }
        }
    };

    DIR *d = nullptr;
    d = opendir(dirPath.c_str());

    if (d)
    {
        TestUnit aTest;
        struct dirent *entry = nullptr;
        int num = 1;
        while ((entry = readdir(d)))
        {
            if (entry->d_name == std::string(".")
                || entry->d_name == std::string(".."))
            {
                continue;
            }

            std::ifstream file;
            std::string fileName = dirPath;
            if (fileName.back() != '\\')
            {
                fileName.append("\\");
            }
            fileName.append(entry->d_name);
            file.open(fileName);
            if (file.is_open())
            {
                std::string line;
                file >> line;

                long long value = 0;
                std::stringstream sstr (line);
                sstr >> value;

                aTest.addValue(value);
                file.close();

                if (aTest.isValid())
                {
                    auto start = std::chrono::steady_clock::now();

                    std::cout << "Test: " << entry->d_name << std::endl;

                    std::cout << "N: " << aTest.inValue << "\n"
                              << "Expected: " << aTest.outValue << "\n";

                    const long long count = calcNumbers(aTest.inValue);
                    std::cout << "Fact: " << count << "\n";
                    printDuration(start);

                    if (count == aTest.outValue)
                    {
                        std::cout << "Correct";
                    }
                    else
                    {
                        std::cout << "NOT correct";
                    }

                    std::cout << "\n" << std::endl;
                    aTest.clear();
                }
            }
            else
            {
                std::cout << "Could not open: " << fileName << "\n";
            }
        }
        closedir(d);
    }
}

// Параметры командной строки:
// -mode=[C - calc, T - test]
// -n=[1...9] - N count
// -dir=[path to test files]
//
// Например:
//
// -mode=C -n=4
// * будет вызван простой расчёт нужных чисел длиной 8
//
// -mode=T -dir=C:\TestData\
// * будет вызван тест расчёта счастливых чисел

int main(int argc, char *argv[])
{
    std::string mode("C");
    int N = 3;
    std::string testDir("./TestData/");

    if (argc > 1)
    {
        for (int i = 0; i < argc; ++i)
        {
            std::string arg(argv[i]);

            if (arg.substr(0, 6) == std::string("-mode="))
            {
                mode = arg.at(arg.length() - 1);
            }
            else if (arg.substr(0, 3) == std::string("-n="))
            {
                N = std::stoi(arg.substr(3));
            }
            else if (arg.substr(0, 5) == std::string("-dir="))
            {
                testDir = arg.substr(5);
            }
        }
    }

    std::cout << "Mode = " << mode << ". N = " << N << ".\n" << std::endl;

    if (mode == "C")
    {
        auto start = std::chrono::steady_clock::now();

        long long count = calcNumbers(N);

        printDuration(start);
        std::cout << "Count: " << count << "\n";
    }
    else if (mode == "T")
    {
        test(testDir);
    }

    return 0;
}
