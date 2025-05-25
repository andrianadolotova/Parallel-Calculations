#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <iomanip>
#include <algorithm>
using namespace std;
using chrono::high_resolution_clock;
using chrono::duration_cast;
using chrono::nanoseconds;

vector<vector<int>> generate_matrix(int n, int min_val = 0, int max_val = 100) {
    vector<vector<int>> matrix(n, vector<int>(n));
    mt19937 rng(42);
    uniform_int_distribution<int> dist(min_val, max_val);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            matrix[i][j] = dist(rng);
    return matrix;
}

void transpose_single(vector<vector<int>>& matrix) {
    int n = matrix.size();
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            swap(matrix[i][j], matrix[j][i]);
}

void transpose_range(vector<vector<int>>& matrix, int start, int end) {
    int n = matrix.size();
    for (int i = start; i < end; ++i)
        for (int j = i + 1; j < n; ++j)
            swap(matrix[i][j], matrix[j][i]);
}

bool check_transpose(const vector<vector<int>>& orig, const vector<vector<int>>& transposed) {
    int n = orig.size();
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (orig[i][j] != transposed[j][i])
                return false;
    return true;
}

void transpose_multi(vector<vector<int>>& matrix, int num_threads) {
    int n = matrix.size();
    vector<thread> threads;
    int rows_per_thread = n / num_threads;
    int extra = n % num_threads;

    int current_row = 0;
    for (int t = 0; t < num_threads; ++t) {
        int start = current_row;
        int end = start + rows_per_thread + (t < extra ? 1 : 0);
        threads.emplace_back(transpose_range, ref(matrix), start, end);
        current_row = end;
    }
    for (auto& th : threads) th.join();
}

int main() {
    vector<int> sizes = {100, 1000, 10000, 25000};
    vector<int> threads_counts = {1, 4, 8, 16, 32, 64, 128};

    cout << "MatrixSize\tThreads\tTime(sec)\tCorrect\n";
    for (int n : sizes) {
        auto orig = generate_matrix(n);
        auto mat_single = orig;
        auto t1 = high_resolution_clock::now();
        transpose_single(mat_single);
        auto t2 = high_resolution_clock::now();
        double t_linear = duration_cast<nanoseconds>(t2 - t1).count() * 1e-9;
        cout << n << "\t\t1\t" << fixed << setprecision(6) << t_linear
             << "\t\t" << (check_transpose(orig, mat_single) ? "Yes" : "No") << endl;

        for (int num_threads : threads_counts) {
            if (num_threads == 1) continue;
            auto mat_multi = orig;
            auto t3 = high_resolution_clock::now();
            transpose_multi(mat_multi, num_threads);
            auto t4 = high_resolution_clock::now();
            double t_multi = duration_cast<nanoseconds>(t4 - t3).count() * 1e-9;
            cout << n << "\t\t" << num_threads << "\t" << fixed << setprecision(6) << t_multi
                 << "\t\t" << (check_transpose(orig, mat_multi) ? "Yes" : "No") << endl;
        }
        cout << endl;
    }
    return 0;
}
