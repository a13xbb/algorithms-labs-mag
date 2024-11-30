#include <iostream>
#include <vector>
#include <cmath>
#include <stack>
#include <random>
#include <ctime>
#include <fstream>

class Node {
public:
    int key;
    int value;
    Node* left;
    Node* right;
    Node(int key, int val) : key(key), value(val), left(nullptr), right(nullptr) {}
};


Node* build_tree(const std::vector<int>& arr) {
    Node* root = nullptr;
    std::stack<Node*> st;

    for (int i = 0; i < arr.size(); i++) {
        Node* node = new Node(i, arr[i]);
        Node* last_popped = nullptr;

        int cur_val = node->value;

        while(!st.empty() && st.top()->value > cur_val) {
            last_popped = st.top();
            st.pop();
        }

        node->left = last_popped;

        if (!st.empty()) {
            st.top()->right = node;
        } else {
            root = node;
        }

        st.push(node);
    }

    return root;
}


void check_tree(Node* root) {
    if (root == nullptr) {
        return;
    }

    check_tree(root->left);
    std::cout<<root->key<<' '<<root->value<<'\n';
    check_tree(root->right);
}


void euler_tour(Node* root, std::vector<int>& euler, std::vector<int>& depth, std::vector<int>& first, int cur_depth) {
    if (!root) return;

    euler.push_back(root->key);
    depth.push_back(cur_depth);

    if (first[root->key] == -1) {
        first[root->key] = euler.size() - 1;
    }

    if (root->left) {
        euler_tour(root->left, euler, depth, first, cur_depth + 1);
        euler.push_back(root->key);
        depth.push_back(cur_depth);
    }
    
    if (root->right) {
        euler_tour(root->right, euler, depth, first, cur_depth + 1);
        euler.push_back(root->key);
        depth.push_back(cur_depth);
    }

}

class SparseTable {
private:
    std::vector<std::vector<int>> table;
    std::vector<int> A;

public:
    SparseTable() {}

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
        if (l < 0 || r >= A.size() || l > r) {
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


std::vector<int> get_block_mins(const std::vector<int>& depth, int block_size, int num_blocks) {
    std::vector<int> block_mins(num_blocks, -1);
    for (int i = 0; i < depth.size(); i++) {
        int block_num = i / block_size;
        if (block_mins[block_num] == -1 || depth[i] < depth[block_mins[block_num]]) {
            block_mins[block_num] = i;
        }
    }
    return block_mins;
}

class LCA {
public:
    std::vector<int> euler;
    std::vector<int> depth;
    std::vector<int> first;

    int block_size;

    std::vector<int> block_mins;

    SparseTable blocks_st;

    std::vector<std::vector<std::vector<int>>> B;

    LCA(Node* root, int n_verts) {
        first.assign(n_verts, -1);

        euler_tour(root, euler, depth, first, 0);

        //1) разбиение массива depth на блоки и вычисление минимума блоков
        block_size = 0.5 * std::__lg(n_verts);
        int num_blocks = (depth.size() + block_size - 1) / block_size;
        block_mins = get_block_mins(depth, block_size, num_blocks);
        std::vector<int> block_mins_values;
        for (int min_idx : block_mins) {
            block_mins_values.push_back(depth[min_idx]);
        }

        //2) nlogn предобработка на массиве минимумов блоков
        blocks_st = SparseTable(block_mins_values);

        //3) квадратичная предобработка на каждом блоке
        B.resize(num_blocks);
        for (int block_idx = 0; block_idx < num_blocks; block_idx++) {
            int start = block_idx * block_size;
            int end = (block_idx + 1) * block_size < depth.size() ?  (block_idx + 1) * block_size : depth.size();
            std::vector<std::vector<int>> cur_block_table(end - start);

            for (int i = start; i < end; i++) {
                for (int j = i; j < end; j++) {
                    //cur_block_table[i][j] - индекс минимума в блоке block_idx на отрезке [i; j]
                    if (i == j) {
                        cur_block_table[i - start].push_back(j);
                    } else {
                        int prev_min = depth[cur_block_table[i - start][j - i - 1]];
                        if (prev_min < depth[j]) {
                            cur_block_table[i - start].push_back(cur_block_table[i - start][j - i - 1]);
                        } else {
                            cur_block_table[i - start].push_back(j);
                        }
                    }
                }
            }

            B[block_idx] = cur_block_table;

        }
    }

    //4 реализация запроса rmq/lca
    int rmq(int l_, int r_) {
        if (l_ < 0 || r_ >= first.size() || l_ > r_) {
            std::cerr<<"Index out of range\n";
            return -1;
        }
        int l = std::min(first[l_], first[r_]);
        int r = std::max(first[l_], first[r_]);

        int block_1 = l / block_size;
        int block_2 = r / block_size;
        if (block_1 == block_2) {
            int left = l - block_1 * block_size;
            int right = r - l;
            return euler[B[block_1][left][right]];
        } else {
            int block_1_min = B[block_1][l - block_1 * block_size][block_size - 1 - (l - block_1 * block_size)];
            int block_2_min = B[block_2][0][r - block_2 * block_size];
            int block12_min = depth[block_1_min] < depth[block_2_min] ? block_1_min : block_2_min;
            if (block_2 - block_1 > 1) {
                int min_between_blocks = block_mins[blocks_st.rmq(block_1 + 1, block_2 - 1)];
                int global_min = depth[block12_min] < depth[min_between_blocks] ? block12_min : min_between_blocks;
                return euler[global_min];
            }
            return euler[block12_min];
        }
    }

    int lca(int l_, int r_) {
        return rmq(l_, r_);
    }
};


// std::vector<int> generate_data

int nlogn_rmq(std::vector<int> vec, int l, int r) {
    SparseTable st = SparseTable(vec);
    return st.rmq(l, r);
}

int lca_rmq(std::vector<int> vec, int l, int r) {
    Node* root = build_tree(vec);
    LCA lca = LCA(root, vec.size());
    return lca.rmq(l, r);
}

int naive_rmq(std::vector<int> vec, int l, int r) {
    int min = vec[l];
    int min_idx = l;
    for (int i = l + 1; i <= r; i++) {
        if (vec[i] < min) {
            min = vec[i];
            min_idx = i;
        }
    }
    return min_idx;
}

std::vector<int> generate_vector(int size) {
    int min_size = 100000, max_size = 100000;
    int min_max_value = 100;

    std::random_device rd;
    std::mt19937 gen(rd());
    // std::uniform_int_distribution<> size_dist(min_size, max_size);
    std::uniform_int_distribution<> value_dist(-min_max_value, min_max_value);

    // int size = size_dist(gen);

    std::vector<int> vec(size);
    for (int i = 0; i < size; ++i) {
        vec[i] = value_dist(gen);
    }

    return vec;
}

int randint(int l, int r) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> int_dist(l, r);

    int num = int_dist(gen);
    return num;
}

void check_answers(int n_iters=100, int vec_size=10000) {
    double naive_sum = 0, nlogn_sum = 0, lca_sum = 0;
    for (int i = 0; i < n_iters; i++) {
        std::vector<int> vec = generate_vector(vec_size);

        int l = randint(0, vec.size() - 1);
        int r = randint(l, vec.size() - 1);

        // std::cout<<l<<' '<<r<<'\n';

        clock_t start = clock();
        int naive_ans = naive_rmq(vec, l, r);
        clock_t end = clock();
        double naive_time = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        naive_sum += naive_time;

        start = clock();
        int nlogn_ans = nlogn_rmq(vec, l, r);
        end = clock();
        double nlogn_time = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        nlogn_sum += nlogn_time;

        start = clock();
        int lca_ans = lca_rmq(vec, l, r);
        end = clock();
        double lca_time = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        lca_sum += lca_time;

        if (nlogn_ans != naive_ans || lca_ans != naive_ans || nlogn_ans != lca_ans) {
            std::cerr<<"Incorrect answers!\n";
            for (auto num : vec) {
                std::cout<<num<<' ';
            }
            std::cout<<'\n';
            std::cout<<l<<' '<<r<<'\n';
            std::cout<<"naive: "<<naive_ans<<'\n';
            std::cout<<"nlogn: "<<nlogn_ans<<'\n';
            std::cout<<"lca_ans: "<<lca_ans<<'\n';
            return;
        }
    }
    std::cout<<"mean naive time: "<<naive_sum / (double)n_iters<<'\n';
    std::cout<<"mean nlogn time: "<<nlogn_sum / (double)n_iters<<'\n';
    std::cout<<"mean lca time: "<<lca_sum / (double)n_iters<<'\n';
}


int main() {
    // std::vector<int> vec = {8, 7, 3, 2, 1, 0, -1, 52};
    // check_answers();
    std::ofstream nlogn_out;
    nlogn_out.open("nlogn_output.txt");
    std::ofstream lca_out;
    lca_out.open("lca_output.txt");

    int max_size = 500000;
    // std::vector<double> nlogn_mean_times;
    // std::vector<double> lca_mean_times;
    for (int vec_size = 500; vec_size <= max_size; vec_size += 500) {
        if (vec_size % 10000 == 0) std::cout<<vec_size<<'\n';
        
        std::vector<int> vec = generate_vector(vec_size);

        int l = randint(0, vec.size() - 1);
        int r = randint(l, vec.size() - 1);

        double nlogn_sum_time = 0;
        int nlogn_ans;
        int n_tries = 1;
        for (int i = 0; i < n_tries; i++) {
            clock_t start = clock();
            nlogn_ans = nlogn_rmq(vec, l, r);
            clock_t end = clock();
            double nlogn_time = static_cast<double>(end - start) / CLOCKS_PER_SEC;
            nlogn_sum_time += nlogn_time;
        }
        double nlogn_mean_time = nlogn_sum_time / n_tries;

        double lca_sum_time = 0;
        int lca_ans;
        for (int i = 0; i < n_tries; i++) {
            clock_t start = clock();
            lca_ans = lca_rmq(vec, l, r);
            clock_t end = clock();
            double lca_time = static_cast<double>(end - start) / CLOCKS_PER_SEC;
            lca_sum_time += lca_time;
        }
        double lca_mean_time = lca_sum_time / n_tries;

        if (nlogn_ans != lca_ans) {
            std::cerr<<"Different answers!";
        }
        nlogn_out<<nlogn_mean_time<<'\n';
        lca_out<<lca_mean_time<<'\n';
        // nlogn_mean_times.push_back(nlogn_mean_time);
        // lca_mean_times.push_back(lca_mean_time);
    }

    // for (const auto& time: nlogn_mean_times) {
    //     nlogn_out<<time<<'\n';
    // }

    // for (const auto& time: lca_mean_times) {
    //     lca_out<<time<<'\n';
    // }
}   