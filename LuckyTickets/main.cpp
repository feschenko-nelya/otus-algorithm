#include <QCoreApplication>

#include <cmath>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include <dirent.h>
#include <mutex>
#include <thread>

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

void printNum(int num, int N)
{
    std::string snum = std::to_string(num);
    short len = snum.length();

    for (short i = 0; i < N - len; ++i)
    {
        snum.insert(0, "0");
    }

    std::cout << snum;
}

void printTicket(int first, int second, int N)
{
    printNum(first, N);
    printNum(second, N);
    std::cout << "\n";
}

void printDuration(std::chrono::steady_clock::time_point startPoint)
{
    auto end = std::chrono::steady_clock::now();
    auto diff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - startPoint);
    auto diff_s = std::chrono::duration_cast<std::chrono::seconds>(end - startPoint);
    auto diff_m = std::chrono::duration_cast<std::chrono::minutes>(end - startPoint);

    std::cout << "Executed time: " << diff_ms.count() << " ms => " << diff_s.count() << " sec => " << diff_m.count() << " min" << std::endl;
}

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

long long calcNumbers(int start, int end, int N)
{
    long long count = (start == 0);

    for (int i = start + count; i < end; ++i)
    {
        int min = 0;
        int max = 0;
        const int sum_i = getSum(i);
        calcMaxMinNum(sum_i, N, max, min);

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

long long globalTicketsCount = 0;
int globalThreadsCount = 0;
int globalMaxThreadsCount = 3;
void threadFun(int begin, int end, int N)
{
    std::cout << "begin thread " << begin << " " << end << std::endl;

    const long long count = calcNumbers(begin, end, N);

    std::mutex m;
    m.lock();
        globalTicketsCount += count;
        --globalThreadsCount;
    m.unlock();

    std::cout << "thread end " << begin << " " << end << std::endl;
}

long long threadCalc(int N)
{
    globalThreadsCount = 0;
    globalTicketsCount = 0;

    const int MAX_NUM = pow(10, N);
    const int DEF_STEP = MAX_NUM / N;

    unsigned int num = 0;
    bool isEnd = false;
    do
    {
        if (!isEnd && (globalThreadsCount < globalMaxThreadsCount))
        {
            const int CUR_STEP = std::fmin(DEF_STEP, MAX_NUM - num);

            ++globalThreadsCount;
            std::thread thread(threadFun, num, num + CUR_STEP, N);
            thread.detach();

            num += CUR_STEP;

            isEnd = (num >= MAX_NUM);
        }

        if (isEnd || (globalThreadsCount >= globalMaxThreadsCount))
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

    } while (!isEnd || (globalThreadsCount > 0));

    return globalTicketsCount;
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

long long calcNumbersFast(int N)
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

long long calcByType(const std::string &type, int N)
{
    if (type == "F")
    {
        return calcNumbersFast(2 * N);
    }

    if (N < 5)
    {
        return calcNumbers(N);
    }

    return threadCalc(N);
}

void test(const std::string &dirPath, const std::string &type)
{
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

                    long long count = calcByType(type, aTest.inValue);

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
    else
    {
        std::cout << "Cant open directory '" << dirPath << "'.\n";
    }
}

// -mode=[C - calc, T - test]
// -type=[S - simple, F - fast]
// -n=[1...9] - N count
// -dir=[path to test files]
// -t=[1...8] - threads count

int main(int argc, char *argv[])
{
    std::string mode("C");
    std::string type("F");
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
            else if (arg.substr(0, 6) == std::string("-type="))
            {
                type = arg.at(arg.length() - 1);
            }
            else if (arg.substr(0, 3) == std::string("-n="))
            {
                N = std::stoi(arg.substr(3));
            }
            else if (arg.substr(0, 5) == std::string("-dir="))
            {
                testDir = arg.substr(5);
            }
            else if (arg.substr(0, 3) == std::string("-t="))
            {
                globalMaxThreadsCount = std::fmin(std::stoi(arg.substr(3)), 8);
            }
        }
    }

    std::cout << "Mode = " << mode << ". N = " << N << ".\n" << std::endl;

    getTen(N);
    if (mode == "C")
    {
        auto start = std::chrono::steady_clock::now();

        long long count = calcByType(type, N);

        printDuration(start);
        std::cout << "Count: " << count << "\n";
    }
    else if (mode == "T")
    {
        test(testDir, type);
    }

    return 0;
}
