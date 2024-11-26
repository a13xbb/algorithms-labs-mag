#include <iostream>
#include <vector>
#include <cmath>
#include <stack>

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

    LCA(Node* root, int n_verts) {
        euler_tour(root, euler, depth, first, 0);

        //1) разбиение массива depth на блоки и вычисление минимума блоков
        int block_size = 0.5 * std::__lg(n_verts);
        int num_blocks = (depth.size() + block_size - 1) / block_size;
        std::vector<int> block_mins = get_block_mins(depth, block_size, num_blocks);

        //TODO:
        // 2) Выполнить nlogn RMQ на массиве минимумов блоков
        // 3) В каждом блоке Bi вычислить аргмины на отрезках [1; j], [j+1; b]
        // 4) Сделать квадратичную предобработку для каждого блока Bi
        // 5) Реализовать запрос rmq/lca
    }
};


int main() {
    std::vector<int> vec = {8, 7, 3, 20, 2, 17, 5, 21, 11, 12,25, 25, 25, 25, 25, 25, 25, 25, 25, 25};
    // Node* root = build_tree(vec);
    // std::vector<int> euler, depth;
    // std::vector<int> first(vec.size(), -1);
    // euler_tour(root, euler, depth, first, 0);

    // int n_verts = vec.size();
    // int block_size = 0.5 * std::__lg(n_verts);
    // int num_blocks = euler.size() / block_size;
    // num_blocks = euler.size() % block_size == 0 ? num_blocks : num_blocks + 1;
    // std::vector<int> block_mins(num_blocks);
    // std::cout<<n_verts<<' '<<euler.size()<<'\n';
    // std::cout<<block_size<<' '<<num_blocks<<'\n';
    // std::cout<<std::__lg(31)<<'\n';
}   