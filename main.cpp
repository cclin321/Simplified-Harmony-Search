#define _USE_MATH_DEFINES // Enable math constants (e.g., M_PI) in C++

// Standard libraries
#include <iostream>
#include <cstdio>
#include <ctime>
#include <cfloat>
#include <cmath>
#include <vector>

#define INF 1.0e99       // A very large value used for infinity
#define EPS 1.0e-14      // A very small value for tolerance
#define E  2.7182818284590452353602874713526625  // Base of the natural logarithm
#define PI 3.1415926535897932384626433832795029   // Pi constant

// Disable specific compiler warnings (non-portable)
#pragma warning(disable: 4996)
#pragma warning(disable: 6385)

using namespace std;

/************************************************************************/
/*                 Problem and Search Space Settings                    */
/************************************************************************/

const int X_LENGTH = 30; // Problem dimension (i.e., number of variables: x[0], x[1], ..., x[X_LENGTH-1])

// Each variable's lower and upper bounds are stored in X_bound[i][0] and X_bound[i][1]
float X_bound[X_LENGTH][2];

// Global lower and upper bounds for each dimension
const float MIN_X_LB = -100; // Lower bound for x_i
const float MAX_X_UB = 100;  // Upper bound for x_i

/************************************************************************/
/*             Harmony Search (HS) Algorithm Settings                     */
/************************************************************************/

const int MAXIT = 30000000;        // Maximum number of iterations
const int PrintInterval = 1000000; // Interval for printing intermediate results

const float MAX_CPU_TIME = 15;     // Maximum CPU time allowed (in seconds)
const float Interval_TIME = 0.01;  // Time interval for updating best solution records

clock_t totaltime; // Global variable to track the total CPU time used

// Harmony Search parameters for choosing between strategies
const float C_p = 0.9f; // Probability to choose historical memory
const float C_h = 0.1f; // Probability to choose pitch adjustment from historical memory

const float BW = 1.0E-6f; // Bandwidth (step size for pitch adjustment)

/************************************************************************/
/*           Harmony Memory (HM) and Related Data Structures              */
/************************************************************************/

const int HMS = 30; // Harmony memory size (i.e., number of solution vectors maintained)

// The Harmony Memory stores HMS solution vectors, each of dimension X_LENGTH
float HM[HMS][X_LENGTH];
// Array to store the fitness value corresponding to each solution in HM
float f[HMS];

// BEST stores the best solution found at each iteration (last column holds the fitness value)
// Here BEST is allocated with (MAXIT + 1) rows and (X_LENGTH+1) columns.
float(*BEST)[X_LENGTH + 1] = new float[MAXIT + 1][X_LENGTH + 1];

// Array to hold the new candidate solution (x') for each variable
float NCHV[X_LENGTH];

// Vector to record the iterations when an interval update occurs (for printing best cost vs. generation)
vector<int> intervalBest;

// Function prototypes for HS operations
void initialize();                                  // Initialize Harmony Memory (HM)
bool StopCondition();                               // Check the stopping condition (iterations or CPU time)
void memoryConsideration(int varIndex);             // Select value from HM (memory consideration) for x'[varIndex]
void pitchAdjustment(int varIndex);                 // Adjust the candidate value (pitch adjustment) for x'[varIndex]
void randomSelection(int varIndex);                 // Select a random value for x'[varIndex]
void updateHM(float newFit, int& flag);             // Update HM if the new candidate solution is better

// Global variables to record time and iterations when the best solution is found
double runTimeIHS1 = 0;     // Time when the run starts (for calculating time to best solution)
double timeOfBestFound = 0; // Time (in seconds) when the best solution is found

int currentIteration;       // Current iteration count
int iterOfBestFound = 0;    // Iteration number when the best solution was found
int curBest = 0;          // Index in BEST array tracking the number of best updates

/************************************************************************/
/*             Additional Settings for Multiple Runs and Output           */
/************************************************************************/

const int RUNTIMES = 1; // Number of independent runs (can be increased for statistical testing)

// Enumeration to select different output formats
enum class printStyle {
    printBasic = 0,           // Print basic output: iteration, best fitness, solution values
    printMultiRun,            // Print summary over multiple runs (average, stdDev, etc.)
    printCostVsRun,           // Print best cost per run (stability analysis)
    printCostVsGeneration,    // Print best cost per generation (for convergence plots)
};

printStyle myPrintStyle = printStyle::printBasic; // Choose output format

/************************************************************************/
/*                   Random Number Generator Prototypes                   */
/************************************************************************/

// Returns a random floating-point number in [low, high)
float uniform(float low, float high);

// Returns a random integer in [low, high)
int randint(int low, int high);

/************************************************************************/
/*                      Objective Function (Fitness)                      */
/************************************************************************/

// Sphere function: sum of squares of the variables (minimization problem)
float HSfunc(float* x) {
    float tmp_cost = 0;
    for (int i = 0; i < X_LENGTH; i++) {
        tmp_cost += x[i] * x[i];
    }
    return tmp_cost;
}

/************************************************************************/
/*                             Main Function                              */
/************************************************************************/

int main() {

    int i, j;
    clock_t aveTime = 0; // To accumulate total CPU time over all runs

    // Initialize random seed with current time
    srand((unsigned)time(NULL));

    // Open file for writing problem information
    FILE* fp = fopen("PROBLEM-info.csv", "w");
    fprintf(fp, "Search Space: %f <= x_i <= %f\n", MIN_X_LB, MAX_X_UB);

    // Open file for writing output data
    FILE* SAData = fopen("output.csv", "w");

    // Array to store best fitness from each run
    float ybest_of_each_run[RUNTIMES] = {};
    int sum_num_it = 0; // Total number of iterations over runs

    for (i = 0; i < RUNTIMES; i++) {
        totaltime = clock();   // Record start time for this run
        runTimeIHS1 = clock();   // Record time to later compute best solution time

        // Step 1: Initialize Harmony Memory (generate random solution vectors)
        initialize();
        currentIteration = 0;
        curBest = 0;

        float start = (float)clock() / CLOCKS_PER_SEC;
        int numberInterval = 1;

        // Main optimization loop: continue until stopping conditions are met
        while (StopCondition()) {
            int flag = 0;

            // Generate new candidate solutions using harmony memory considerations
            // Loop over each harmony in the memory (for each trial in the current iteration)
            for (int hms = 1; hms <= HMS; hms++) {
                // Set flag to 1 for the last harmony (indicating final trial for this iteration)
                if (hms == HMS) {
                    flag = 1;
                }

                // For each variable/dimension, generate a candidate value x'
                for (j = 0; j < X_LENGTH; j++) {
                    float r = uniform(0, 1);
                    if (r > C_p) {
                        // Choose a value from historical memory and then adjust it (pitch adjustment)
                        memoryConsideration(j);
                        pitchAdjustment(j);
                    }
                    else if (r > C_h) {
                        // Only choose a value from historical memory
                        memoryConsideration(j);
                    }
                    else {
                        // Randomly select a value within the bounds
                        randomSelection(j);
                    }
                }

                // Evaluate the new candidate solution using the objective function
                float newFitness = HSfunc(NCHV);
                // Update the Harmony Memory if the candidate solution is better
                updateHM(newFitness, flag);
            }

            // Check if enough time has passed to record an interval update
            float end = (float)clock() / CLOCKS_PER_SEC;
            float during = end - start;
            if (during > Interval_TIME * numberInterval) {
                intervalBest.push_back(currentIteration - 1);
                numberInterval++;
            }
            currentIteration++;
        } // End of optimization while-loop

        sum_num_it += currentIteration - 1;

        // Compute run time for the current run and output run summary to file
        totaltime = clock() - totaltime;
        fprintf(fp, "Run #%d: %f seconds\n\t(Iteration of Best Found: %d, Time of Best Found: %f)\n\n",
            i, ((float)totaltime) / CLOCKS_PER_SEC, iterOfBestFound, timeOfBestFound);

        aveTime += totaltime;

        // Output detailed results depending on the selected print style
        if (myPrintStyle == printStyle::printBasic) {
            fprintf(SAData, "Run number: %d, Best values: %g\n", i, BEST[currentIteration - 1][X_LENGTH]);
            // Output best solution vector
            for (j = 0; j < X_LENGTH; j++) {
                fprintf(SAData, "%g ", BEST[currentIteration - 1][j]);
            }
            fprintf(SAData, "\n");
            fprintf(SAData, "-----------------------------------------------------------------------------------------\n");

            // Print best fitness at intervals (for generating a cost vs. generation plot)
            for (j = 0; j < currentIteration - 1; j++) {
                if (j % PrintInterval == 0) {
                    fprintf(SAData, "%d   %g\n", j, BEST[j][X_LENGTH]);
                }
            }
        }
        else if (myPrintStyle == printStyle::printCostVsGeneration) {
            // If printing cost vs. generation, output best cost at recorded intervals
            for (auto item : intervalBest) {
                fprintf(SAData, "%g\n", BEST[item][X_LENGTH]);
            }
            break; // Exit after printing if this style is selected
        }

        // For multi-run or cost vs. run output, store the best fitness of the current run
        if (myPrintStyle == printStyle::printMultiRun || myPrintStyle == printStyle::printCostVsRun) {
            ybest_of_each_run[i] = BEST[currentIteration - 1][X_LENGTH];
        }
    }

    if (myPrintStyle != printStyle::printCostVsGeneration) {
        // Output the average time over all runs
        fprintf(fp, "Average time: %g seconds\n", ((double)aveTime) / CLOCKS_PER_SEC / RUNTIMES);
    }

    // Output summary statistics for multi-run experiments
    if (myPrintStyle == printStyle::printMultiRun) {
        float sum_gbest = 0;
        float sum_gbest_square = 0;
        for (i = 0; i < RUNTIMES; i++) {
            sum_gbest += ybest_of_each_run[i];
            sum_gbest_square += ybest_of_each_run[i] * ybest_of_each_run[i];
        }
        float ave_gbest = sum_gbest / RUNTIMES;
        float stdDev_gbest = sqrt(sum_gbest_square / RUNTIMES - ave_gbest * ave_gbest);
        fprintf(SAData, "\"ActualIteration\",\"aveCost\",\"stdDevCost\"\n");
        fprintf(SAData, "\"%g\",\"%g\",\"%g\"", sum_num_it / float(RUNTIMES), ave_gbest, stdDev_gbest);
    }
    else if (myPrintStyle == printStyle::printCostVsRun) {
        // Output best cost for each run
        for (i = 0; i < RUNTIMES; i++)
            fprintf(SAData, "%g\n", ybest_of_each_run[i]);
    }

    fclose(SAData);
    fclose(fp);

    // Optionally pause system (commented out)
    // system("pause");
    return 0;
}

/************************************************************************/
/*                        Helper Functions                                */
/************************************************************************/

// Returns a random floating-point number in [low, high)
float uniform(float low, float high) {
    return float(rand()) / RAND_MAX * (high - low) + low;
}

// Returns a random integer in [low, high)
int randint(int low, int high) {
    return rand() % (high - low) + low;
}

// Initialize the search space bounds and the Harmony Memory (HM)
void initialize() {
    int i;
    // Initialize bounds for each variable
    for (i = 0; i < X_LENGTH; i++) {
        X_bound[i][0] = MIN_X_LB;
        X_bound[i][1] = MAX_X_UB;
    }

    // Generate initial random solutions for HM
    for (i = 0; i < HMS; i++) {
        // For the first two dimensions, use integer random selection
        for (int j = 0; j < 2; j++) {
            HM[i][j] = float(randint(int(X_bound[j][0]), int(X_bound[j][1]) + 1));
        }
        // For the remaining dimensions, use continuous uniform random selection
        for (int j = 2; j < X_LENGTH; j++) {
            HM[i][j] = uniform(X_bound[j][0], X_bound[j][1]);
        }
        // Compute and store the fitness of the generated solution
        f[i] = HSfunc(HM[i]);
    }
}

// Check if the stopping condition (maximum iterations or CPU time) has been reached
bool StopCondition() {
    bool flag = true;
    if (currentIteration > MAXIT)
        flag = false;
    if (((float)clock() - totaltime) / CLOCKS_PER_SEC > MAX_CPU_TIME)
        flag = false;
    return flag;
}

// The following functions (LineVal_down, LineVal_up, ExpVal_down) are helper functions
// that compute linearly or exponentially decreasing/increasing values based on the current iteration.
// They are defined here but not used in this code. They can be removed if not needed.

float LineVal_down(float min, float max, int generation, bool flag) {
    float val;
    if (flag)
        val = (min - max) / (MAXIT)*generation + max;
    else
        val = max;
    return val;
}

float LineVal_up(float min, float max, int generation, bool flag) {
    float val;
    if (flag)
        val = (max - min) / (MAXIT)*generation + min;
    else
        val = min;
    return val;
}

float ExpVal_down(float min, float max, int generation, bool flag) {
    float val;
    float c = log(min / max) / MAXIT;
    if (flag)
        val = max * exp(c * generation);
    else
        val = max;
    return val;
}

// Select a value for x'[varIndex] from a randomly chosen solution in HM (memory consideration)
void memoryConsideration(int varIndex) {
    NCHV[varIndex] = HM[randint(0, HMS)][varIndex];
}

// Adjust the candidate solution's variable (pitch adjustment) by adding/subtracting a small random value
void pitchAdjustment(int varIndex) {
    float randVal = uniform(0, 1);
    float temp = NCHV[varIndex];

    if (uniform(0, 1) < 0.5) {
        temp += randVal * BW;
        if (temp < X_bound[varIndex][1])
            NCHV[varIndex] = temp;
    }
    else {
        temp -= randVal * BW;
        if (temp > X_bound[varIndex][0])
            NCHV[varIndex] = temp;
    }
}

// Randomly select a value for x'[varIndex] within the allowed bounds
void randomSelection(int varIndex) {
    NCHV[varIndex] = uniform(X_bound[varIndex][0], X_bound[varIndex][1]);
}

// Update the Harmony Memory (HM) with the new candidate solution if it has better fitness
void updateHM(float newFitness, int& flag) {
    // Find the worst solution in HM (the one with the highest fitness value)
    float worst = f[0];
    int worstIndex = 0;
    int i;
    for (i = 0; i < HMS; i++) {
        if (f[i] > worst) {
            worst = f[i];
            worstIndex = i;
        }
    }

    // If the new candidate solution is better than the worst in HM, replace it
    if (newFitness < worst) {
        for (int k = 0; k < X_LENGTH; k++) {
            HM[worstIndex][k] = NCHV[k];
        }
        f[worstIndex] = newFitness;
    }

    // Find the best solution in HM (the one with the lowest fitness value)
    float best = f[0];
    int bestIndex = 0;
    for (i = 0; i < HMS; i++) {
        if (f[i] < best) {
            best = f[i];
            bestIndex = i;
        }
    }

    // If flag is set (indicating the final trial in this iteration),
    // record the current best solution in the BEST array
    if (flag) {
        for (int k = 0; k < X_LENGTH; k++) {
            BEST[curBest][k] = HM[bestIndex][k];
        }
        BEST[curBest][X_LENGTH] = best; // Store the fitness value in the last column
        curBest++;
    }

    // If the new best solution is better than the previous best (and flag is set),
    // update the iteration count and time when the best solution was found.
    if (best < BEST[curBest][X_LENGTH] && flag) {
        iterOfBestFound = currentIteration;
        timeOfBestFound = (clock() - runTimeIHS1) / 1000.0; // Note: This may need adjustment using CLOCKS_PER_SEC for precision.
    }
    // Reset flag for next use
    flag = 0;
}
