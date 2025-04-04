#include <bits/stdc++.h>
#include <fstream>
#include <vector>
#include <array>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <chrono>
#include <iostream>
#include <random>
using namespace std;
using namespace std::chrono;
vector<array<double, 3>> numbers;
vector<int> op_sol;
float op_fit;
vector<int> current_sol;
int tabu_tenure;
float current_fit;
int stopiteration;
random_device rd;
mt19937 gen(rd());
void write_result(string output, float duration, string filename, double op_result)
{
    ofstream file(output, ios::app);
    file << filename << endl;
    file << "optimum solution found:" << "\n";
    for (int i = 0; i < op_sol.size(); i++)
        file << op_sol[i] << "\t";
    file << endl;
    file << "route = " << op_fit << endl;
    file << " Gap = " << ((op_fit - op_result) / op_result) * 100 << "%" << endl;
    file << "running time = " << duration / pow(10, 6) << "seconds" << endl;
    file << "Stop iteration: " << stopiteration;
    file << endl
         << endl;
    file.close();
}
vector<string> readfilename(string &source)
{
    ifstream file(source);
    string file_name;
    vector<string> file_name_list;
    while (file >> file_name)
    {
        file_name_list.push_back(file_name);
    }
    file.close();
    return file_name_list;
}
vector<double> readopresult(string op_result_file)
{
    ifstream file(op_result_file);
    vector<double> op_result;
    double num;
    int point = 0;
    if (!file)
        cout << "Unable to open file";
    else
    {
        while (file >> num)
        {
            op_result.push_back(num);
        }
    }
    file.close();
    return op_result;
}
void readDoublesFromFile(string &filename)
{
    ifstream file(filename);
    array<double, 3> num_arr; // save information about location of a inserted note
    double num;
    if (!file)
        cout << "Unable to open file";
    else
    {
        while (file >> num)
        {
            num_arr[0] = num;
            file >> num_arr[1];
            file >> num_arr[2];
            numbers.push_back(num_arr);
        }
    }
    file.close();
}
double calculateCost(const vector<int> &tour, double **distance)
{
    double totalCost = 0.0;
    int n = tour.size();
    for (int i = 0; i < n - 1; ++i)
    {
        totalCost += distance[tour[i]][tour[i + 1]];
    }
    totalCost += distance[tour[n - 1]][tour[0]];
    return totalCost;
}

void insertCheapest(vector<int> &tour, double **distance, int city)
{
    double minCost = numeric_limits<double>::max();
    int insertPos = 0;
    for (size_t i = 0; i < tour.size() - 1; ++i)
    {
        int from = tour[i];
        int to = tour[i + 1];
        double cost = distance[from][city] + distance[city][to] - distance[from][to];
        if (cost < minCost)
        {
            minCost = cost;
            insertPos = i + 1;
        }
    }
    tour.insert(tour.begin() + insertPos, city);
}

pair<vector<int>, double> cheapestInsertion_init(double **distance, int n)
{
    vector<int> tour = {0};
    for (int i = 1; i < n; ++i)
    {
        insertCheapest(tour, distance, i);
    }
    double Cost = calculateCost(tour, distance);
    return {tour, Cost};
}

vector<int> twoOptMove(const vector<int> &tour, int i, int j)
{
    vector<int> newTour = tour;
    reverse(newTour.begin() + i, newTour.begin() + j + 1);
    return newTour;
}

vector<int> swapCities(const vector<int> &tour, int i, int j)
{
    vector<int> newTour = tour;
    swap(newTour[i], newTour[j]);
    return newTour;
}

vector<int> insertCityMove(const vector<int> &tour, int from, int to)
{
    vector<int> newTour = tour;
    int city = newTour[from];
    newTour.erase(newTour.begin() + from);
    newTour.insert(newTour.begin() + to, city);
    return newTour;
}

double twoOptCostUpdate(const vector<int> &tour, int i, int j, double **distance)
{
    int n = tour.size();

    // Get the relevant edges
    int a = tour[(i - 1 + n) % n]; // Previous city to 'i'
    int b = tour[i];               // City at 'i'
    int c = tour[j];               // City at 'j'
    int d = tour[(j + 1) % n];     // Next city to 'j'

    // Compute the delta cost: (new edges) - (old edges)
    double newCost = distance[a][c] + distance[b][d];
    double oldCost = distance[a][b] + distance[c][d];

    return newCost - oldCost;
}

double swapCostUpdate(const vector<int> &tour, int i, int j, double **distance)
{
    int n = tour.size();

    // Get relevant cities and their neighbors
    int a = tour[(i - 1 + n) % n]; // Previous city to 'i'
    int b = tour[i];               // City at 'i'
    int c = tour[(i + 1) % n];     // Next city to 'i'

    int x = tour[(j - 1 + n) % n]; // Previous city to 'j'
    int y = tour[j];               // City at 'j'
    int z = tour[(j + 1) % n];     // Next city to 'j'

    // Calculate cost changes based on swapping cities at 'i' and 'j'
    double oldCost = distance[a][b] + distance[b][c] + distance[x][y] + distance[y][z];
    double newCost = distance[a][y] + distance[y][c] + distance[x][b] + distance[b][z];

    // If i and j are adjacent, we need to avoid double counting an edge
    if (abs(i - j) == 1)
    {
        newCost -= distance[b][c]; // Adjust for overlap
    }

    return newCost - oldCost;
}

double cityInsertCostUpdate(const vector<int> &tour, int from, int to, double **distance)
{
    int n = tour.size();
    if (from == to)
        return 0.0; // No change if positions are the same

    // Get relevant cities around 'from' and 'to'
    int prevFrom = tour[(from - 1 + n) % n];
    int nextFrom = tour[(from + 1) % n];
    int city = tour[from];

    int prevTo = tour[(to - 1 + n) % n];
    int nextTo = tour[to % n];

    // Calculate the old and new costs
    double oldCost = distance[prevFrom][city] + distance[city][nextFrom];
    double newCost = distance[prevFrom][nextFrom] + distance[prevTo][city] + distance[city][nextTo];

    // Special case: if 'from' and 'to' are adjacent
    if (abs(from - to) == 1)
    {
        oldCost = distance[prevFrom][city] + distance[city][nextFrom];
        newCost = distance[prevFrom][nextFrom]; // Overlap handled
    }

    return newCost - oldCost;
}
#include <random> // Thêm thư viện cho random

// Hàm tạo bộ sinh số ngẫu nhiên
std::mt19937 rng(chrono::steady_clock::now().time_since_epoch().count()); // Random seed

// Sinh ngẫu nhiên chỉ số từ 0 đến n-1
int randomIndex(int n)
{
    std::uniform_int_distribution<int> dist(0, n - 1);
    return dist(rng);
}

// Cập nhật trong thuật toán Tabu Search
vector<int> tabuSearch(vector<int> &initialTour, double **distance, int maxIterations,
                       int tabuSize, int maxNoImprovementPerMove)
{
    int n = initialTour.size();
    vector<int> bestTour = initialTour;
    double bestCost = calculateCost(initialTour, distance);

    deque<pair<int, int>> tabuListTwoOpt, tabuListSwap, tabuListInsert;
    int noImprovement = 0, countTostop = 0;
    vector<int> curBestTour = bestTour;

    for (int iter = 0; iter < maxIterations; ++iter)
    {
        cout << "Iteration " << iter + 1 << ": Current Best Cost = " << bestCost << endl;

        vector<int> bestNeighbor;
        double bestNeighborCost = numeric_limits<double>::max();
        bool improved = false;
        int bestMoveType = -1;
        int city1 = -1, city2 = -1;
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                int currentMove = randomIndex(3);
                vector<int> neighbor;
                double newCost;

                switch (currentMove)
                {
                case 0: // 2-opt
                    neighbor = twoOptMove(curBestTour, i, j);
                    newCost = calculateCost(neighbor, distance);

                    if (find(tabuListTwoOpt.begin(), tabuListTwoOpt.end(), make_pair(i, j)) != tabuListTwoOpt.end() && newCost >= bestCost)
                        continue;

                    break;

                case 1: // Swap move
                    neighbor = swapCities(curBestTour, i, j);
                    newCost = calculateCost(neighbor, distance);

                    if (find(tabuListSwap.begin(), tabuListSwap.end(), make_pair(i, j)) != tabuListSwap.end() && newCost >= bestCost)
                        continue;

                    break;

                case 2: // Insert city move
                    if (i == j)
                        continue;

                    neighbor = insertCityMove(curBestTour, i, j);
                    newCost = calculateCost(neighbor, distance);

                    if (find(tabuListInsert.begin(), tabuListInsert.end(), make_pair(i, j)) != tabuListInsert.end() && newCost >= bestCost)
                        continue;

                    break;
                }

                if (newCost < bestNeighborCost)
                {
                    bestNeighbor = neighbor;
                    bestNeighborCost = newCost;
                    bestMoveType = currentMove;
                    city1 = i;
                    city2 = j;
                    improved = true;
                }
            }
        }

        curBestTour = bestNeighbor;
        if (bestNeighborCost < bestCost)
        {
            bestTour = bestNeighbor;
            bestCost = bestNeighborCost;
            noImprovement = 0;
        }
        else
        {
            ++noImprovement;
            ++countTostop;
        }

        switch (bestMoveType)
        {
        case 0: // 2-opt move
            tabuListTwoOpt.emplace_back(city1, city2);
            if (tabuListTwoOpt.size() > tabuSize)
                tabuListTwoOpt.pop_front();
            break;

        case 1: // Swap move
            tabuListSwap.emplace_back(city1, city2);
            if (tabuListSwap.size() > tabuSize)
                tabuListSwap.pop_front();
            break;

        case 2: // Insert move
            tabuListInsert.emplace_back(city1, city2);
            if (tabuListInsert.size() > tabuSize)
                tabuListInsert.pop_front();
            break;
        }

        if (noImprovement > 200)
        {
            cout << "Stopping search due to lack of improvement." << endl;
            break;
        }
        cout << "Current Best Tour: ";
        for (int city : bestTour)
        {
            cout << city << " ";
        }
        cout << "\n";
    }
    stopiteration = countTostop;
    return bestTour;
}

void distance_computing(double **distance)
{
    for (int i = 0; i < numbers.size(); i++)
    {
        for (int j = 0; j < numbers.size(); j++)
            distance[i][j] = sqrt(pow(numbers[i][1] - numbers[j][1], 2) + pow(numbers[i][2] - numbers[j][2], 2));
    }
}
int main()
{
    string source = "source.txt";
    string output = "outputrandom1.txt";
    string op_result_file = "op.txt";
    vector<double> op_result;
    vector<string> file_name_list;
    file_name_list = readfilename(source);
    op_result = readopresult(op_result_file);
    while (!file_name_list.empty())
    {

        string filename;
        filename = file_name_list[0];
        file_name_list.erase(file_name_list.begin());
        auto start = high_resolution_clock::now();
        readDoublesFromFile(filename);
        double **distance = (double **)malloc(numbers.size() * sizeof(double *));
        for (int i = 0; i < numbers.size(); i++)
        {
            distance[i] = (double *)malloc(numbers.size() * sizeof(double));
        }
        int n = numbers.size();
        distance_computing(distance);
        op_sol = cheapestInsertion_init(distance, n).first;
        current_sol = op_sol;
        op_fit = current_fit = cheapestInsertion_init(distance, n).second;
        op_sol = tabuSearch(op_sol, distance, n * 10, (int)sqrt(n * 10), 40);
        op_fit = calculateCost(op_sol, distance);
        for (int i = 0; i < numbers.size(); i++)
            free(distance[i]);
        free(distance);
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        float time = duration.count();
        write_result(output, time, filename, op_result[0]);
        op_result.erase(op_result.begin());
        numbers.clear();
        op_sol.clear();
        op_fit = 0;
        current_sol.clear();
        current_fit = 0;
        stopiteration = 0;
    }
    return 0;
}