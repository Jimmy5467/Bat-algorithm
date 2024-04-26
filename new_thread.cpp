#include <bits/stdc++.h>
#include <omp.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

// Structure to represent a task
struct Task
{
    int task_number;
    int processing_time;
};

// Structure to represent an edge
struct Edge
{
    int source_task;
    int destination_task;
    int communication_cost;
};

// Structure to represent the task graph
struct TaskGraph
{
    vector<Task> tasks;
    vector<Edge> edges;
};

// Function to initialize the task graph
TaskGraph initializeTaskGraph()
{
    TaskGraph graph;

    // Define the number of tasks and edges
    const int num_tasks = 100;
    const int num_edges = 200;

    // Define the tasks with random processing times
    srand(time(0));
    for (int i = 0; i < num_tasks; ++i)
    {
        graph.tasks.push_back({i, rand() % 10 + 1}); // Random processing time between 1 and 10
    }

    // Define random edges (precedence relations) and communication costs
    for (int i = 0; i < num_edges; ++i)
    {
        int source_task = rand() % num_tasks;
        int destination_task = rand() % num_tasks;
        int communication_cost = rand() % 10 + 1; // Random communication cost between 1 and 10
        graph.edges.push_back({source_task, destination_task, communication_cost});
    }

    return graph;
}

// Function to evaluate solution
int evaluateSolution(vector<int> solution, TaskGraph graph, int num_processors)
{
    vector<int> completion_times(graph.tasks.size(), 0);

    for (int task_idx = 0; task_idx < graph.tasks.size(); task_idx++)
    {
        int processor_idx = solution[task_idx];
        int task_processing_time = graph.tasks[task_idx].processing_time;

        for (Edge edge : graph.edges)
        {
            if (edge.destination_task == task_idx)
            {
                int source_task_completion_time = completion_times[edge.source_task];
                int communication_cost = edge.communication_cost;
                if (solution[edge.source_task] != processor_idx)
                {
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
    srand(time(0)); // Seed for random number generation

    // Example parameters
    TaskGraph graph = initializeTaskGraph();
    int num_processors = 20; // Change the number of processors to the number of tasks
    int num_bats = 20;
    int max_iterations = 100;
    int best_fitness = INT_MAX; // Initialize best fitness outside parallel region

    // Perform bat algorithm
    vector<int> best_solution(graph.tasks.size(), 0);

    // Initialize parameters
    double loudness = 0.5;
    double pulse_rate = 0.5;
    double alpha = 0.5;
    double beta = 0.5;

    // Start timing
    auto start_time = high_resolution_clock::now();

    // Initialize population of bats
    vector<vector<int>> population(num_bats, vector<int>(graph.tasks.size(), 0));
    for (int i = 0; i < num_bats; ++i)
    {
        // Shuffle processor assignment for each bat
        vector<int> processors(num_processors);
        iota(processors.begin(), processors.end(), 0);        // Fill with 0, 1, ..., num_processors - 1
        random_shuffle(processors.begin(), processors.end()); // Shuffle processors randomly
        for (int j = 0; j < graph.tasks.size(); ++j)
        {
            population[i][j] = processors[j % num_processors]; // Assign each task to a randomly shuffled processor
        }
    }

    // Perform iterations outside parallel region
    for (int iter = 0; iter < max_iterations; iter++)
    {
        int local_best_fitness = INT_MAX; // Initialize local best fitness for reduction

#pragma omp parallel for shared(best_solution, best_fitness, population, graph, num_processors, num_bats) default(none) private(loudness, pulse_rate, alpha, beta) \
    reduction(min : local_best_fitness)
        for (int bat_idx = 0; bat_idx < num_bats; bat_idx++)
        {
            // Evaluate fitness of current bat
            int fitness = evaluateSolution(population[bat_idx], graph, num_processors);

            // Update best solution if necessary
            if (fitness < local_best_fitness)
            {
#pragma omp critical
                {
                    if (fitness < best_fitness)
                    {
                        best_solution = population[bat_idx];
                        best_fitness = fitness;
                    }
                }
                local_best_fitness = fitness;
            }

            // Update bat position
            for (int task_idx = 0; task_idx < graph.tasks.size(); task_idx++)
            {
                if (rand() / double(RAND_MAX) < pulse_rate)
                {
                    population[bat_idx][task_idx] = best_solution[task_idx] + alpha * (rand() / double(RAND_MAX) - 0.5);
                }
            }
        }

        // Update loudness and pulse rate
        loudness *= alpha;
        pulse_rate *= beta;
    }

    // End timing
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);

    // Print execution time
    cout << "Execution time: " << duration.count() << " milliseconds" << endl;

    // Print best solution
    cout << "Best solution:" << endl;
    for (int i = 0; i < best_solution.size(); i++)
    {
        cout << "Task " << i << " assigned to processor " << best_solution[i] << endl;
    }

    return 0;
}
