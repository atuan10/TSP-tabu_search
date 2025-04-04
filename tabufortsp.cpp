
#include <bits/stdc++.h>
#include <fstream>
#include <vector>
#include <array>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <chrono>
#include <iostream>
using namespace std;
using namespace std::chrono;
vector<array<double, 3>> numbers;
vector<int> op_sol;
float op_fit;
vector<int> current_sol;
vector<array<int, 4>> tabu_list; // tabu_list save 4 nodes of 2 exchange edges : (x,y,z,t)-> edge(x,y) and edge(z,t)
int tabu_tenure;                 // the length of tabu list
float current_fit;
int ite_count = 0;
void write_result(string output, float duration, string filename, double op_result)
{
    ofstream file(output, ios::app);
    file << filename << endl;
    file << "optimum solution found:" << "\n";
    for (int i = 0; i < op_sol.size(); i++)
        file << op_sol[i] << "\t";
    file << endl;
    file << "route = " << 1 / op_fit << endl;
    file << " Gap = " << (1 / (op_fit * op_result) - 1) * 100 << "%" << endl;
    file << "running time = " << duration / pow(10, 6) << "seconds";
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
vector<int> segmentReverse(const vector<int> &tour, int i, int j)
{
    vector<int> newTour = tour;
    reverse(newTour.begin() + i, newTour.begin() + j + 1);
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
    double costRemoved = distance[tour[i]][tour[(i + 1) % n]] +
                         distance[tour[j]][tour[(j + 1) % n]];

    double costAdded = distance[tour[i]][tour[j]] +
                       distance[tour[(i + 1) % n]][tour[(j + 1) % n]];

    return costAdded - costRemoved;
}

double segmentReverseCostUpdate(const vector<int> &tour, int i, int j, double **distance)
{
    int n = tour.size();
    double costRemoved = distance[tour[i]][tour[(i - 1 + n) % n]] +
                         distance[tour[j]][tour[(j + 1) % n]];
    double costAdded = distance[tour[(i - 1 + n) % n]][tour[j]] +
                       distance[tour[i]][tour[(j + 1) % n]];

    return costAdded - costRemoved;
}

double cityInsertCostUpdate(const vector<int> &tour, int from, int to, double **distance)
{
    int n = tour.size();

    if (from == to)
        return 0.0; // No change

    int prevFrom = (from - 1 + n) % n;
    int nextFrom = (from + 1) % n;
    double costRemoved = distance[tour[prevFrom]][tour[from]] +
                         distance[tour[from]][tour[nextFrom]];
    double costAdded = distance[tour[prevFrom]][tour[nextFrom]];
    int prevTo = (to - 1 + n) % n;
    int nextTo = (to + 1) % n;

    costAdded += distance[tour[prevTo]][tour[from]] +
                 distance[tour[from]][tour[nextTo]];

    return costAdded - costRemoved;
}

vector<int> tabuSearch(vector<int> &initialTour, double **distance,
                       int maxIterations, int tabuSize, int maxNoImprovementPerMove)
{
    int n = initialTour.size();
    vector<int> bestTour = initialTour;
    double bestCost = calculateCost(initialTour, distance);
    deque<vector<int>> tabuListTwoOpt, tabuListSegmentReverse, tabuListCityInsert;

    int noImprovement = 0;
    int currentMove = 0;

    for (int iter = 0; iter < maxIterations; ++iter)
    {
        cout << "Iteration " << iter + 1 << ": Current Best Cost = " << bestCost << endl;

        vector<int> bestNeighbor;
        double bestNeighborCost = numeric_limits<double>::max();

        switch (currentMove)
        {
        case 0:
            for (int i = 0; i < n - 1; ++i)
            {
                for (int j = i + 1; j < n; ++j)
                {
                    double deltaCost = twoOptCostUpdate(bestTour, i, j, distance);
                    double newCost = bestCost + deltaCost;

                    if (newCost < bestNeighborCost &&
                        find(tabuListTwoOpt.begin(), tabuListTwoOpt.end(), bestTour) == tabuListTwoOpt.end())
                    {
                        bestNeighbor = twoOptMove(bestTour, i, j);
                        bestNeighborCost = newCost;
                    }
                }
            }
            break;

        case 1:
            for (int i = 0; i < n - 1; ++i)
            {
                for (int j = i + 1; j < n; ++j)
                {
                    double deltaCost = segmentReverseCostUpdate(bestTour, i, j, distance);
                    double newCost = bestCost + deltaCost;

                    if (newCost < bestNeighborCost &&
                        find(tabuListSegmentReverse.begin(), tabuListSegmentReverse.end(), bestTour) == tabuListSegmentReverse.end())
                    {
                        bestNeighbor = segmentReverse(bestTour, i, j);
                        bestNeighborCost = newCost;
                    }
                }
            }
            break;

        case 2:
            for (int from = 0; from < n; ++from)
            {
                for (int to = 0; to < n; ++to)
                {
                    if (from == to)
                        continue;

                    double deltaCost = cityInsertCostUpdate(bestTour, from, to, distance);
                    double newCost = bestCost + deltaCost;

                    if (newCost < bestNeighborCost &&
                        find(tabuListCityInsert.begin(), tabuListCityInsert.end(), bestTour) == tabuListCityInsert.end())
                    {
                        bestNeighbor = insertCityMove(bestTour, from, to);
                        bestNeighborCost = newCost;
                    }
                }
            }
            break;
        }

        if (bestNeighborCost < bestCost)
        {
            bestTour = bestNeighbor;
            bestCost = bestNeighborCost;
            noImprovement = 0;
            if (currentMove == 0)
            {
                tabuListTwoOpt.push_back(bestTour);
                if (tabuListTwoOpt.size() > tabuSize)
                    tabuListTwoOpt.pop_front();
            }
            else if (currentMove == 1)
            {
                tabuListSegmentReverse.push_back(bestTour);
                if (tabuListSegmentReverse.size() > tabuSize)
                    tabuListSegmentReverse.pop_front();
            }
            else if (currentMove == 2)
            {
                tabuListCityInsert.push_back(bestTour);
                if (tabuListCityInsert.size() > tabuSize)
                    tabuListCityInsert.pop_front();
            }
        }
        else
        {
            noImprovement++;
        }

        if (noImprovement >= maxNoImprovementPerMove)
        {
            currentMove = (currentMove + 1) % 3;
            noImprovement = 0;
        }
    }

    return bestTour;
}
int main()
{
    string source = "source.txt";
    string output = "output.txt";
    string op_result_file = "op.txt";

    vector<double> op_result;
    vector<string> file_name_list;
    file_name_list = readfilename(source);
    op_result = readopresult(op_result_file);

    while (!file_name_list.empty())
    {
        string filename = file_name_list[0];
        file_name_list.erase(file_name_list.begin());
        auto start = high_resolution_clock::now();

        readDoublesFromFile(filename);

        double **distance = (double **)malloc(numbers.size() * sizeof(double *));
        for (int i = 0; i < numbers.size(); i++)
        {
            distance[i] = (double *)malloc(numbers.size() * sizeof(double));
        }

        distance_computing(distance);
        current_sol = cheapestInsertion_init(distance, numbers.size());
        current_fit = fitness(distance, current_sol);
        op_sol = tabuSearch({current_sol, current_fit}, distance, 1000, 30, numbers.size());
        for (int i = 0; i < numbers.size(); i++)
            free(distance[i]);
        free(distance);

        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        float time = duration.count();

        write_result(output, time, filename, op_result[0]);
        op_result.erase(op_result.begin());

        // Clear data for next iteration
        numbers.clear();
        op_sol.clear();
        current_sol.clear();
        current_fit = 0;
    }

    return 0;
}
