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
const vector<double> &probs) {
    int max_degree = -1;
    int selected_vert = 0;
    for (int vert : candidates) {
        int cur_vert_degree = neighbours[vert].size();
        if (cur_vert_degree > max_degree) {
            max_degree = cur_vert_degree;
            selected_vert = vert;
        }
    }

    for (int vert : candidates) {
        int diff = max_degree - neighbours[vert].size();
        if ((diff < probs.size()) && (dis(gen) <= probs[diff])) {
            selected_vert = vert;
            break;
        }
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
    
    ofstream fout("clique.txt");

    for (auto filename: files) {
        cout<<filename<<'\n';
        vector<unordered_set<int>> neighbours;
        read_file("./inputs/" + filename + ".clq", neighbours);

        //DEBUG
        // for (int i = 0; i < neighbours.size(); i++) {
        //     cout<<i<<": ";
        //     for (auto el : neighbours[i]) {
        //         cout<<el<<' ';
        //     }
        //     cout<<'\n';
        // }
        
        clock_t start = clock();

        unordered_set<int> best_clique_members;
        int best_size = -1;
        int n_iters = 30;

        for (int i = 0; i < n_iters; i++) {
            unordered_set<int> clique_members;
            int clique_size = find_clique(clique_members, neighbours, "intersection");
            if (clique_size > best_size) {
                best_size = clique_size;
                best_clique_members = clique_members;
            }
        }

        n_iters = 5000;

        for (int i = 0; i < n_iters; i++) {
            unordered_set<int> clique_members;
            int clique_size = find_clique(clique_members, neighbours, "random");
            if (clique_size > best_size) {
                best_size = clique_size;
                best_clique_members = clique_members;
            }
        }

        clock_t end = clock();
        
        fout << filename << '\n';
        fout << "Clique size: " << best_size << '\n';
        fout << "Time: "<< std::fixed << double(end - start) / CLOCKS_PER_SEC << "\n\n";
    }
}