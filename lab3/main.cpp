#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <time.h>
#include <unordered_set>
#include <algorithm>
#include <random>
#include <deque>
#include <queue>
#include <unordered_map>
using namespace std;


std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(0.0, 1.0);

class QCO {
public:
    vector<int> qco_vec;
    vector<int> tau;
    unordered_map<int, int> idx;
    vector<unordered_set<int>> neighbours;
    int q, c, n;

    QCO(unordered_set<int> clique, vector<unordered_set<int>> neighbours_) {
        n = neighbours_.size();
        q = clique.size();
        c = clique.size();
        neighbours = neighbours_;

        //добавляю в qco все вершины клики
        for (auto it = clique.begin(); it != clique.end(); ++it) {
            qco_vec.push_back(*it);
            idx[*it] = qco_vec.size() - 1;
        }
        //считаю tightness для всех вершин
        tau.resize(neighbours.size());
        for (int i = 0; i < neighbours.size(); i++) {
            tau[i] = 0;
            if (clique.find(i) == clique.end()) {
                for (auto v = clique.begin(); v != clique.end(); ++v) {
                    if (i != *v && neighbours[i].find(*v) == neighbours[i].end()) {
                        tau[i]++;
                    }
                }
            }
        }
        //добавляю в qco кандидатов
        for (int i = 0; i < neighbours.size(); i++) {
            if (clique.find(i) == clique.end() && tau[i] == 0) {
                qco_vec.push_back(i);
                idx[i] = qco_vec.size() - 1;
                c++;
            }
        }
        //добавляю в qco остальные вершины
        for (int i = 0; i < neighbours.size(); i++) {
            if (clique.find(i) == clique.end() && tau[i] != 0) {
                qco_vec.push_back(i);
                idx[i] = qco_vec.size() - 1;
            }
        }
    }

    void swap(int i, int j) {
        if (i < 0 || i > n || j < 0 || j > n) {
            cerr<<"Index error!";
            return;
        }
        int tmp = idx[qco_vec[i]];
        idx[qco_vec[i]] = idx[qco_vec[j]];
        idx[qco_vec[j]] = tmp;

        tmp = qco_vec[i];
        qco_vec[i] = qco_vec[j];
        qco_vec[j] = tmp;
    }

    void insert_to_clique(int vertex) {
        swap(idx[vertex], q++);
        for (int u = 0; u < n; u++) {
            if (u != vertex && neighbours[vertex].find(u) == neighbours[vertex].end()) {
                if (tau[u] == 0) {
                    swap(idx[u], --c);
                }
                tau[u]++;
            }
        }
    }

    void remove_from_clique(int vertex) {
        swap(idx[vertex], --q);
        for (int u = 0; u < n; u++) {
            if (u != vertex && neighbours[vertex].find(u) == neighbours[vertex].end()) {
                if (tau[u] == 1) {
                    swap(idx[u], c++);
                }
                tau[u]--;
            }
        }
    }

    void print_clique() {
        cout<<"Clique:\n";
        for (int i = 0; i < q; i++) {
            cout<<qco_vec[i] + 1<<' ';
        }
        cout<<"\nCandidates:\n";
        for (int i = q; i < c; i++) {
            cout<<qco_vec[i] + 1<<' ';
        }
        cout<<"\nOthers:\n";
        for (int i = c; i < qco_vec.size(); i++) {
            cout<<qco_vec[i] + 1<<' ';
        }
        cout<<'\n';
    }

    void print_indices() {
        for (const auto& p : idx) {
            cout<<p.first<<" : "<<p.second<<'\n';
        }
    }
};

void read_file(string filename, vector<unordered_set<int>> &neighbours) {
    ifstream fin(filename);
    string line;
    int vertices = 0, edges = 0;
    while (getline(fin, line)) {
        if (line[0] == 'c') {
            continue;
        }

        char command;
        stringstream line_input(line);
        if (line[0] == 'p') {
            string type;
            line_input >> command >> type >> vertices >> edges;
            neighbours.resize(vertices);
        } else {
            int vert1, vert2;
            line_input >> command >> vert1 >> vert2;
            neighbours[vert1 - 1].insert(vert2 - 1);
            neighbours[vert2 - 1].insert(vert1 - 1);
        }
    }

}

unordered_set<int> intersection(const unordered_set<int> &set1, const unordered_set<int> &set2) {
    unordered_set<int> intersection_set;
    if (set1.size() < set2.size()) {
        for (int elem : set1) {
            if (set2.find(elem) != set2.end()) {
                intersection_set.insert(elem);
            }
        }
    } else {
        for (int elem : set2) {
            if (set1.find(elem) != set1.end()) {
                intersection_set.insert(elem);
            }
        }
    }

    return intersection_set;
}

int select_random_candidate(const unordered_set<int> &candidates, const vector<unordered_set<int>> &neighbours,
const vector<double> &probs, unordered_map<int, int> in_tabu_list = {}) {
    int max_degree = -1;
    int selected_vert = 0;
    for (int vert : candidates) {
        int cur_vert_degree = neighbours[vert].size();
        if (cur_vert_degree > max_degree && !in_tabu_list[vert]) {
            max_degree = cur_vert_degree;
            selected_vert = vert;
        }
    }

    for (int vert : candidates) {
        int diff = max_degree - neighbours[vert].size();
        if ((diff < probs.size()) && (dis(gen) <= probs[diff]) && (!in_tabu_list[vert])) {
            selected_vert = vert;
            break;
        }
    }

    if (max_degree == -1) {
        return -1;
    }
    return selected_vert;
}

int select_biggest_intersection_candidate(const unordered_set<int> &candidates, const vector<unordered_set<int>> &neighbours,
const unordered_set<int> &clique_members) {
    int best_vert = -1;
    int max_intersection = 0;
    double prob = 0.8;
    for (auto vert : candidates) {
        if (clique_members.find(vert) == clique_members.end()) {
            int int_sz = intersection(candidates, neighbours[vert]).size();
            if (int_sz > max_intersection && dis(gen) <= prob) {
                max_intersection = int_sz;
                best_vert = vert;
            }
        }
    }

    return best_vert;
}

int find_clique(unordered_set<int> &clique_members, vector<unordered_set<int>> &neighbours, string heuristic) {
    vector<double> probs = {0.6, 0.4, 0.3, 0.2, 0.1, 0.05, 0.03, 0.02};
    int max_degree = 0;
    int first_vert = 0;
    for (int i = 0; i < neighbours.size(); i++) {
        if (neighbours[i].size() > max_degree) {
            max_degree = neighbours[i].size();
            first_vert = i;
        }
    }

    for (int i = 0; i < neighbours.size(); i++) {
        int diff = max_degree - neighbours[i].size();
        if ((diff < probs.size()) && (dis(gen) <= probs[diff])) {
            first_vert = i;
        }
    }

    // int first_vert = rand() % neighbours.size();

    clique_members.insert(first_vert);
    // cout<<first_vert<<endl;
    unordered_set<int> candidates = neighbours[first_vert];
    while (!candidates.empty()) {
        // cout<<candidates.size()<<'\n';
        int cur_vert;
        if (heuristic == "random") {
            cur_vert = select_random_candidate(candidates, neighbours, probs);
        } else {
            cur_vert = select_biggest_intersection_candidate(candidates, neighbours, clique_members);
        }
        
        if (cur_vert == -1) {
            break;
        }
        clique_members.insert(cur_vert);
        candidates = intersection(candidates, neighbours[cur_vert]);
    }

    return clique_members.size();
}

unordered_set<int> generate_start_solution(vector<unordered_set<int>>& neighbours, int n_iters = 30) {
    unordered_set<int> best_clique_members;
    int best_size = -1;

    for (int i = 0; i < n_iters; i++) {
        unordered_set<int> clique_members;
        int clique_size = find_clique(clique_members, neighbours, "random");
        if (clique_size > best_size) {
            best_size = clique_size;
            best_clique_members = clique_members;
        }
    }
    return best_clique_members;
}

void remove_from_clique(int vert, QCO& qco, deque<int>& tabu_deleted, unordered_map<int, int> in_tabu_deleted) {
    qco.remove_from_clique(vert);
    tabu_deleted.push_back(vert);
    in_tabu_deleted[vert] = 1;
    in_tabu_deleted[tabu_deleted.front()] = 0;
    tabu_deleted.pop_front();
}

void insert_to_clique(int vert, QCO& qco, deque<int>& tabu_added, unordered_map<int, int> in_tabu_added) {
    qco.insert_to_clique(vert);
    tabu_added.push_back(vert);
    in_tabu_added[vert] = 1;
    in_tabu_added[tabu_added.front()] = 0;
    tabu_added.pop_front();
}

int tabu_search(vector<unordered_set<int>>& neighbours, unordered_set<int>& best_solution,
                int del_tenure, int add_tenure, int max_no_improvement_iters, int num_iters) {

    vector<double> probs = {0.7, 0.5, 0.3, 0.2};

    best_solution = generate_start_solution(neighbours, 1);
    int best_size = best_solution.size();

    for (int _ = 0; _ < num_iters; _++) {

        unordered_set<int> start_solution = generate_start_solution(neighbours, 1);
        // best_solution = {0, 1, 2};
        // return best_size;
        QCO qco = QCO(start_solution, neighbours);

        deque<int> tabu_deleted, tabu_added;
        for (int i = 0; i < del_tenure; i++) {
            tabu_deleted.push_back(-1);
        }
        for (int i = 0; i < add_tenure; i++) {
            tabu_added.push_back(-1);
        }

        unordered_map<int, int> in_tabu_deleted, in_tabu_added;

        int no_improvement_iters = 0;
        while(no_improvement_iters <= max_no_improvement_iters) {
            no_improvement_iters++;

            bool moved_or_swapped = false;
            //move neighborhood
            while (qco.c > qco.q) {
                unordered_set<int> candidates;
                for (int i = qco.q; i < qco.c; i++) {
                    int vert = qco.qco_vec[i];
                    candidates.insert(vert);
                }
                int new_vert = select_random_candidate(candidates, neighbours, probs);
                if (new_vert != -1) {
                    qco.insert_to_clique(new_vert);
                    moved_or_swapped = true;
                    if (qco.q > best_size) {
                        no_improvement_iters = 0;
                        best_size = qco.q;
                        best_solution.clear();
                        for (int i = 0; i < qco.q; i++) {
                            best_solution.insert(qco.qco_vec[i]);
                        }
                    }
                }
            }

            //1-2 swap
            for (int i = 0; i < qco.q; i++) {
                int k = qco.qco_vec[i];
                if (in_tabu_added[k]) continue;

                vector<int> knn;
                for (int j = qco.c; j < qco.qco_vec.size(); j++) {
                    int u = qco.qco_vec[j];
                    if (u != k && neighbours[k].find(u) == neighbours[k].end() && qco.tau[u] == 1) {
                        knn.push_back(u);
                    }
                }
                if (knn.size() < 2) continue;

                int chosen_vert1 = -1, chosen_vert2 = -1;
                bool found_verts_flag = false;
                for (int j = 0; j < knn.size() - 1; j++) {
                    for (int m = j + 1; m < knn.size(); m++) {
                        int vert1 = knn[j], vert2 = knn[m];
                        if (neighbours[vert1].find(vert2) != neighbours[vert1].end()
                            && !in_tabu_deleted[vert1] && !in_tabu_deleted[vert2]) {
                            chosen_vert1 = vert1;
                            chosen_vert2 = vert2;
                            found_verts_flag = true;
                            break;
                        }
                    }
                    if (found_verts_flag) break;
                }

                if (chosen_vert1 == -1 && chosen_vert2 == -1) {
                    continue;
                }

                // qco.remove_from_clique(k);
                // qco.insert_to_clique(chosen_vert1);
                // qco.insert_to_clique(chosen_vert2);
                moved_or_swapped = true;

                remove_from_clique(k, qco, tabu_deleted, in_tabu_deleted);
                insert_to_clique(chosen_vert1, qco, tabu_added, in_tabu_added);
                insert_to_clique(chosen_vert2, qco, tabu_added, in_tabu_added);
                if (qco.q > best_size) {
                    no_improvement_iters = 0;
                    best_size = qco.q;
                    best_solution.clear();
                    for (int i = 0; i < qco.q; i++) {
                        best_solution.insert(qco.qco_vec[i]);
                    }
                }
            }

            //1-1 swap
            if (!moved_or_swapped) {
                for (int i = 0; i < qco.q; i++) {
                    int k = qco.qco_vec[i];
                    if (in_tabu_added[k]) continue;

                    unordered_set<int> knn;
                    for (int j = qco.c; j < qco.qco_vec.size(); j++) {
                        int u = qco.qco_vec[j];
                        if (u != k && neighbours[k].find(u) == neighbours[k].end() && qco.tau[u] == 1) {
                            knn.insert(u);
                        }
                    }
                    if (knn.size() == 0) continue;

                    int new_vert = select_random_candidate(knn, neighbours,
                    {0.3, 0.5, 0.8, 0.8, 0.8}, in_tabu_deleted);
                    if (new_vert == -1) continue;

                    remove_from_clique(k, qco, tabu_deleted, in_tabu_deleted);
                    insert_to_clique(new_vert, qco, tabu_added, in_tabu_added);
                }
            }
            

        }

    }

    return best_size;    

}

int main()
{
    srand(time(0));
    vector<string> files = {"brock200_1", "brock200_2", "brock200_3", "brock200_4",
    "brock400_1", "brock400_2", "brock400_3", "brock400_4",
    "C125.9", "gen200_p0.9_44", "gen200_p0.9_55", "hamming8-4",
    "johnson16-2-4", "johnson8-2-4", "keller4", "MANN_a27",
    "MANN_a9", "p_hat1000-1", "p_hat1000-2", "p_hat1500-1",
    "p_hat300-3", "p_hat500-3", "san1000", "sanr200_0.9", 
    "sanr400_0.7"};
    // vector<string> files = {"test"};
    // vector<string> files = {"brock400_3"};
    
    ofstream fout("clique.txt");

    for (auto filename: files) {
        cout<<filename<<'\n';
        vector<unordered_set<int>> neighbours;
        read_file("../lab2/inputs/" + filename + ".clq", neighbours);

        //DEBUG
        // for (int i = 0; i < neighbours.size(); i++) {
        //     cout<<i<<": ";
        //     for (auto el : neighbours[i]) {
        //         cout<<el<<' ';
        //     }
        //     cout<<'\n';
        // }
        
        clock_t start = clock();
        
        unordered_set<int> clique;
        int best_size = tabu_search(neighbours, clique,
        floor((float)neighbours.size() * 0.2),
        floor((float)neighbours.size() * 0.08), 200, 100);

        clock_t end = clock();
        
        fout << filename << '\n';
        fout << "Clique size: " << best_size << '\n';
        fout << "Time: "<< std::fixed << double(end - start) / CLOCKS_PER_SEC << "\n\n";
    }
}