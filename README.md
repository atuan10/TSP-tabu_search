# TSP-tabu_search

This project implements the **Tabu Search metaheuristic** to solve the **Travelling Salesman Problem (TSP)**. The TSP is a classic optimization problem that aims to find the shortest possible route visiting a set of cities exactly once and returning to the origin city.

## 📁 Project Structure

| File                         | Description |
|-----------------------------|-------------|
| `tabufortsp.cpp`            | Main program to run the Tabu Search for TSP. |
| `move2_2_tabu_search.c++`   | Implements a 2-2 move (swap two city pairs) neighborhood. |
| `changeifnotimprove.cpp`    | Heuristic: change only if it does not worsen the solution. |
| `compete.cpp`               | Compares performance between different heuristics/methods. |
| `cycling.cpp`               | Detects and handles cycling in the search. |
| `random.cpp`                | Generates a random initial tour. |
| `randomv2.cpp`              | Improved or alternative random tour generation. |
| `1_move`                    | Possibly a 1-move neighborhood implementation. |
| `README.md`                 | Project description and usage instructions. |

## 🚀 Features

- Basic and advanced Tabu Search moves (1-move, 2-2 swap, etc.)
- Tabu list to avoid cycling
- Random initialization strategies
- Optional diversification and intensification strategies

## 🛠 How to Build

This project is written in **C++**. Use any standard compiler such as `g++`.

Example:

```bash
g++ tabufortsp.cpp -o tsp_solver
./tsp_solver
