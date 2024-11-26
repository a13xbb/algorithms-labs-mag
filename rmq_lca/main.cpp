#include <iostream>
#include <vector>
#include <cmath>

class SparseTable {
private:
    std::vector<std::vector<int>> table;
    std::vector<int> A;

public:
    SparseTable(const std::vector<int>& arr) {
        A = arr;

        int n = A.size();
        int max_log = log2(n) + 1;

        table.resize(n, std::vector<int>(max_log));

        for (int i = 0; i < n; i++) {
            table[i][0] = i;
        }

        for (int j = 1; j < max_log; j++) {
            for (int i = 0; i + (1 << j) <= n; i++) {
                if (A[table[i][j - 1]] <= A[table[i + (1 << (j- 1))][j - 1]]) {
                    table[i][j] = table[i][j - 1];
                } else {
                    table[i][j] = table[i + (1 << (j- 1))][j - 1];
                }
            }
        }
    }

    int rmq(int l, int r) {
        if (l < 0 || r >= A.size()) {
            std::cerr<<"Index out of range\n";
            return -1;
        }
        int j = std::__lg(r - l + 1);
        if (A[table[l][j]] <= A[table[r - (1 << j) + 1][j]]) {
            return table[l][j];
        }
        return table[r - (1 << j) + 1][j];
    }
};



int main() {
    std::vector<int> vec = {2, 8, 4, -1, 55, 3, -10};

    SparseTable st = SparseTable(vec);
    std::cout<<st.rmq(0, 5)<<"\n";
}