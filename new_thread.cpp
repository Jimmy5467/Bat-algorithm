#include <bits/stdc++.h>
#include <omp.h>
#include <chrono>
using namespace std;
using namespace std::chrono;
struct Task {
    int task_number;
    int processing_time;
};
struct Edge {
    int source_task;
    int destination_task;
    int communication_cost;
};
struct TaskGraph {
    vector<Task> tasks;
    vector<Edge> edges;
};
TaskGraph initializeTaskGraph() {
    TaskGraph graph;
    const int num_tasks = 100;
    const int num_edges = 200;
    srand(time(0));
    for (int i = 0; i < num_tasks; ++i) {
        graph.tasks.push_back({i, rand() % 10 + 1});
    }
    for (int i = 0; i < num_edges; ++i) {
        int source_task = rand() % num_tasks;
        int destination_task = rand() % num_tasks;
        int communication_cost = rand() % 10 + 1;
        graph.edges.push_back({source_task, destination_task, communication_cost});
    }
    return graph;
}
int evaluateSolution(vector<int> solution, TaskGraph graph, int num_processors) {
    vector<int> completion_times(graph.tasks.size(), 0);

    for (int task_idx = 0; task_idx < graph.tasks.size(); task_idx++) {
        int processor_idx = solution[task_idx];
        int task_processing_time = graph.tasks[task_idx].processing_time;

        for (Edge edge : graph.edges) {
            if (edge.destination_task == task_idx) {
                int source_task_completion_time = completion_times[edge.source_task];
                int communication_cost = edge.communication_cost;
                if (solution[edge.source_task] != processor_idx) {
                    source_task_completion_time += communication_cost;
                }
                task_processing_time = max(task_processing_time, source_task_completion_time);
            }
        }
        completion_times[task_idx] = task_processing_time;
    }
    return *max_element(completion_times.begin(), completion_times.end());
}

int main()
{
    srand(time(0)); 
    TaskGraph graph = initializeTaskGraph();
    int num_processors = 20;
    int num_bats = 20;
    int max_iterations = 100;
    int best_fitness = INT_MAX; 
    vector<int> best_solution(graph.tasks.size(), 0);
    double loudness = 0.5;
    double pulse_rate = 0.5;
    double alpha = 0.5;
    double beta = 0.5;
    auto start_time = high_resolution_clock::now();
    vector<vector<int>> population(num_bats, vector<int>(graph.tasks.size(), 0));
    for (int i = 0; i < num_bats; ++i){
        vector<int> processors(num_processors);
        iota(processors.begin(), processors.end(), 0);        
        random_shuffle(processors.begin(), processors.end());
        for (int j = 0; j < graph.tasks.size(); ++j) {
            population[i][j] = processors[j % num_processors]; 
        }
    }
    for (int iter = 0; iter < max_iterations; iter++){
        int local_best_fitness = INT_MAX;

#pragma omp parallel for shared(best_solution, best_fitness, population, graph, num_processors, num_bats) default(none) private(loudness, pulse_rate, alpha, beta) \
    reduction(min : local_best_fitness)
        for (int bat_idx = 0; bat_idx < num_bats; bat_idx++) {
            int fitness = evaluateSolution(population[bat_idx], graph, num_processors);
            if (fitness < local_best_fitness) {
#pragma omp critical
                {
                    if (fitness < best_fitness) {
                        best_solution = population[bat_idx];
                        best_fitness = fitness;
                    }
                }
                local_best_fitness = fitness;
            }
            for (int task_idx = 0; task_idx < graph.tasks.size(); task_idx++) {
                if (rand() / double(RAND_MAX) < pulse_rate) {
                    population[bat_idx][task_idx] = best_solution[task_idx] + alpha * (rand() / double(RAND_MAX) - 0.5);
                }
            }
        }
        loudness *= alpha;
        pulse_rate *= beta;
    }
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);
    cout << "Execution time: " << duration.count() << " milliseconds" << endl;
    cout << "Best solution:" << endl;
    for (int i = 0; i < best_solution.size(); i++) {
        cout << "Task " << i << " assigned to processor " << best_solution[i] << endl;
    }

    return 0;
}
