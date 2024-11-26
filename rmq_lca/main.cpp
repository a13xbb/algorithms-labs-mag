#include <iostream>
#include <vector>
#include <cmath>

class SparseTable {
private:
    std::vector<std::vector<int>> table;
    std::vector<int> log;

public:
    SparseTable(const std::vector<int>& arr) {
        int n = arr.size();
        int max_log = log2(n) + 1;

        table.resize(n, std::vector<int>(max_log));
        log.resize(n + 1);

        for (int i = 0; i < n; i++) {
            table[i][0] = arr[i];
        }
        
        log[1] = 0;
        for (int i = 2; i < n; i++) {
            log[i] = log[i / 2] + 1;
        }

        for (int j = 1; j < max_log; j++) {
            for (int i = 0; i + (1 << j) <= n; i++) {
                table[i][j] = std::min(table[i][j - 1],
                                       table[i + (1 << ( j- 1))][j - 1]);
            }
        }
    }

    int rmq(int l, int r) {
        int j = std::__lg(r - l + 1);
        return std::min(table[l][j], table[r - (1 << j) + 1][j]);
    }
};



int main() {
    std::vector<int> vec = {2, 8, 4, -1, 55, 3, -10};

    SparseTable st = SparseTable(vec);
    std::cout<<st.rmq(4, 5)<<"\n";
}