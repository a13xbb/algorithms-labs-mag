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

//TODO:
// 1. написать реализацию с общипыванием графа (lowest degree first out)
// 2. написать реализацию с рандомизированным выбором вершины для раскрашивания (брать с наибольшей
// степенью либо где-то рядом)

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

deque<int> degree_sort_verts(vector<unordered_set<int>> &neighbours) {
    vector<pair<int, int>> index_size;
    
    for (int i = 0; i < neighbours.size(); ++i) {
        index_size.push_back({i, neighbours[i].size()});
    }

    sort(index_size.begin(), index_size.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
        return (a.second > b.second);
    });

    deque<int> sorted_verts;
    for (int i = 0; i < index_size.size(); i++) {
        sorted_verts.push_back(index_size[i].first);
    }

    return sorted_verts;
}

deque<int> peel_sort_verts(vector<unordered_set<int>> &neighbours) {
    deque<int> sorted_verts;
    unordered_map<int, int> degree;
    for (int i = 0; i < neighbours.size(); i++) {
        degree[i] = neighbours[i].size();
    }

    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> heap;

    for (const auto& [vert, deg] : degree) {
        heap.push({deg, vert});
    }

    while (!heap.empty()) {
        auto [deg, vert] = heap.top();
        heap.pop();

        if (deg != degree[vert]) {
            continue;
        }

        sorted_verts.push_front(vert);

        for (int v : neighbours[vert]) {
            if (degree[v] > 0) {
                degree[v]--;
                heap.push({degree[v], v});
            }
        }

        degree[vert] = -1;
    }

    return sorted_verts;
}

int color_graph(vector<unordered_set<int>> &color_groups, vector<unordered_set<int>> &neighbours) {
    deque<int> sorted_verts = peel_sort_verts(neighbours);

    //Случайный своп для вершин с одинаковыми степенями
    // srand(time(0));
    // for (size_t i = 0; i < index_size.size();) {
    //     size_t j = i;
    //     while (j < index_size.size() && index_size[j].second == index_size[i].second) {
    //         ++j;
    //     }
    //     if (j - i > 1) {
    //         for (size_t k = i; k < j - 1; ++k) {
    //             if (dis(gen) < 0.5) {
    //                 std::swap(index_size[k], index_size[k + 1]);
    //             }
    //         }
    //     }
    //     i = j;
    // }

    vector<int> colors;
    colors.resize(neighbours.size(), 0);

    int maxcolor = 1;
    while (!sorted_verts.empty()) {
        int cur_vert = sorted_verts.front();
        sorted_verts.pop_front();
        unordered_set<int> not_possible_colors;
        for (auto neighbour : neighbours[cur_vert]) {
            if (colors[neighbour]) {
                not_possible_colors.insert(colors[neighbour]);
            }
        }
        int cur_color = 1;
        while (not_possible_colors.find(cur_color) != not_possible_colors.end()) {
            cur_color++;
        }
        if (cur_color > maxcolor) {
            maxcolor = cur_color;
        }
        colors[cur_vert] = cur_color;
        if (cur_color > color_groups.size()) {
            color_groups.push_back({cur_vert + 1});
        } else {
            color_groups[cur_color - 1].insert(cur_vert + 1);
        }
    }

    return maxcolor;
}

int main()
{
    vector<string> files = { "myciel3.col.txt", "myciel7.col.txt",  "school1.col.txt",
        "school1_nsh.col.txt", "anna.col.txt", "miles1000.col.txt", "miles1500.col.txt",
        "le450_5a.col.txt", "le450_15b.col.txt", "queen11_11.col.txt",};
    
    ofstream fout("color.txt");

    for (auto filename: files) {
        vector<unordered_set<int>> neighbours;
        read_file("./inputs/" + filename, neighbours);

        //DEBUG
        // for (int i = 0; i < neighbours.size(); i++) {
        //     cout<<i<<": ";
        //     for (auto el : neighbours[i]) {
        //         cout<<el<<' ';
        //     }
        //     cout<<'\n';
        // }
        
        vector<unordered_set<int>> color_groups;
        clock_t start = clock();
        int num_colors = color_graph(color_groups, neighbours);
        clock_t end = clock();

        //DEBUG
        // cout<<"Number of colors: "<<num_colors<<'\n';
        // for (auto group : color_groups) {
        //     cout<<'{';
        //     for (auto vert : group) {
        //         cout<<vert<<", ";
        //     }
        //     cout<<"}\n";
        // }
        
        fout << filename << '\n';
        fout << "Colors: " << num_colors << '\n';
        fout << "Time: "<< std::fixed << double(end - start) / CLOCKS_PER_SEC << "\n\n";
    }
}