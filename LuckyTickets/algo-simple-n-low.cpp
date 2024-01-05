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

// кеш степеней 10
int getTen(int N)
{
    static std::vector<int> tens;

    if (tens.empty())
    {
        for (int i = 0; i <= N; ++i)
        {
            tens.push_back(pow(10, i));
        }
    }

    return tens[N];
}

// сумма всех цифр в числе
int getSum(int num)
{
    short sum = 0;

    do
    {
        sum += (num % 10);
        num = num / 10;
    } while (num > 0);

    return sum;
}

// проверка числа на соответствие нужной сумме цифр
bool checkSum(int leftSum, int num)
{
    short sum = num % 10;

    if (sum > leftSum)
        return false;

    do
    {
        num = num / 10;
        sum += (num % 10);
    } while (num > 0);

    return (sum == leftSum);
}

// расчёт диапазона чисел - от какого до какого искать счастливые билеты
void calcMaxMinNum(int num, int N, int &resMax, int &resMin)
{
    const int cN = N;
    resMax = 0;
    resMin = 0;

    while (num - 9 > 0)
    {
        resMax += 9 * getTen(N - 1);
        resMin += 9 * getTen(cN - N);

        num -= 9;
        --N;
    }

    resMin += num * getTen(cN - N);
    resMax += num * getTen(N - 1);
}

// сам перебор счастливых чисел в определенном диапазоне
long long calcNumbers(int start, int end, int N)
{
    long long count = (start == 0);

    for (int i = start + count; i < end; ++i)
    {
        int min = 0;
        int max = 0;
        const int sum_i = getSum(i);
        calcMaxMinNum(sum_i, N, max, min);

        // если расчёт долгий, то вывести значения, на которых происходит расчёт,
        // чтобы было понятно, что программа не зависла
        if (i % getTen(std::fmax(4, N / 2)) == 0)
        {
            std::cout << "working " << " i=" << i << " sum=" << sum_i
                      << " min=" << min << " max=" << max << std::endl;
        }

        for (int j = min; j <= max; j += 9)
        {
            if (checkSum(sum_i, j))
            {
                ++count;
            }
        }
    }

    return count;
}

long long calcNumbers(int N)
{
    const long long maxNum = pow(10, N);

    return calcNumbers(0, maxNum, N);
}

void test(const std::string &dirPath)
{
    // - считываются файлы в указанной папке
    // - из файла с расширением .in загружается N
    // - из файла с расширение .out загружается эталонное количество счастливых чисел
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

// до N = 6 включая можно быстро увидеть результат
// выше - нет

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

    // инициализация кеша степеней 10
    getTen(N);

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
