#include <stdio.h>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <math.h>
using namespace std;

int N;
vector<int>* graph;
vector<int> degeneracy_order;
vector<int> rev_deg_order;

int deg_order() {
    vector<int>* d = new vector<int>[N];
    vector<int> degs(N);
    vector<int> positions(N);
    unsigned degeneracy = 0;
    for (int i=0; i<N; i++) {
        d[graph[i].size()].push_back(i);
        degs[i] = graph[i].size();
        positions[i] = d[degs[i]].size()-1;
    }
    unsigned j = 0;
    for (int i=0; i<N; i++) {
        while (d[j].empty()) j++;
        int v = d[j].back();
        d[j].pop_back();
        positions[v] = -1;
        degeneracy_order.emplace_back(v);
        if (degeneracy < j) {
            degeneracy = j;
        }
        for (auto g: graph[v]) {
            if (positions[g] == -1) continue;
            int& fr = d[degs[g]][positions[g]];
            swap(fr, d[degs[g]].back());
            d[degs[g]].pop_back();
            positions[fr] = positions[g];
            degs[g]--;
            d[degs[g]].push_back(g);
            positions[g] = d[degs[g]].size()-1;
        }
        if (j>0) j--;
    }

    return degeneracy;
}

int nextInt() {
    int n = 0;
    int ch = getchar_unlocked();
    while (ch != EOF && (ch < '0' || ch > '9')) ch = getchar_unlocked();
    if (ch == EOF) return EOF;
    while (ch >= '0' && ch <= '9') {
        n = 10*n + ch - '0';
        ch = getchar_unlocked();
    }
    return n;
}

int main(int argc, char** argv) {
    N = nextInt();
    int M = nextInt();
    graph = new vector<int>[N];
    std::vector<int> labels;
    for (int i=0; i<N; i++) labels.emplace_back(nextInt());
    for (int i=0; i<M; i++) {
        int a = nextInt();
        int b = nextInt();
        if(a == b) continue;
        graph[a].push_back(b);
        graph[b].push_back(a);
    }
    unsigned max_deg = 0;
    for (int i=0; i<N; i++) {
        sort(graph[i].begin(), graph[i].end());
        graph[i].erase(unique(graph[i].begin(), graph[i].end()), graph[i].end());
        if (max_deg < graph[i].size()) max_deg = graph[i].size();
    }
    fprintf(stderr, "Graph size: %d %d\n", N, M);
    fprintf(stderr, "Maximum degree: %u\n", max_deg);
    unsigned degeneracy = deg_order();
    printf("%d\n", degeneracy);
    if (argc != 2) return 0;

    rev_deg_order.resize(N);
    for (int i=0; i<N; i++) rev_deg_order[degeneracy_order[i]] = i;
    std::vector<std::vector<int>> deg_graph(N);
    for (int i=0; i<N; i++)
        for (auto j: graph[i])
            if (rev_deg_order[i] < rev_deg_order[j])
                deg_graph[i].emplace_back(j);
    for (unsigned d=0; d<degeneracy; d++) {
        FILE* f = fopen((argv[1] + "_"s + std::to_string(d+1) + ".txt"s).c_str(), "w");
        fprintf(f, "%d %d\n", N, M);
        for (int i=0; i<N; i++) fprintf(f, "%d ", labels[i]);
        fprintf(f, "\n");
        unsigned tree_count = 0;
        for (int i=0; i<N; i++)
            for (unsigned j=0; j<deg_graph[i].size(); j++)
                if (j == d) {
                    tree_count++;
                    fprintf(f, "%d %d\n", degeneracy_order[i], degeneracy_order[deg_graph[i][j]]);
                }
        fprintf(stderr, "Forest %u size: %u\n", d+1, tree_count);
        for (int i=0; i<N; i++)
            for (unsigned j=0; j<deg_graph[i].size(); j++)
                if (j != d)
                    fprintf(f, "%d %d\n", degeneracy_order[i], degeneracy_order[deg_graph[i][j]]);
        fclose(f);
    }
    return 0;
}
