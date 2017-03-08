#include <cstdio>
#include <queue>
#include <vector>
#include <stack>
#include "cuckoo.h"
//#define DEBUG
#ifdef DEBUG
#define DDEBUG
#endif

#include <google/dense_hash_set>
#include <google/dense_hash_map>
using namespace google;

using namespace std;

int N1, N2, M1, M2;

class DSU {
public:
    vector<int> dad;
    vector<int> rank;
    DSU(int N) {
        dad.resize(N);
        rank.resize(N);
        for(int i=0; i<N; i++) dad[i] = i;
    }
    inline int find(int x) {
        if (x == dad[x]) return x;
        return dad[x] = find(dad[x]);
    }
    inline void join(int x, int y) {
        int rx = find(x);
        int ry = find(y);
        if (rx == ry) return;
        if (rank[rx] < rank[ry]) {
            dad[rx] = ry;
            rank[ry] += rank[rx];
        } else {
            dad[ry] = rx;
            rank[rx] += rank[ry];
        }
    }
};


cuckoo_hash_set* g1_orig;
vector<int>* g1_orig_iter;

cuckoo_hash_set* g1;
cuckoo_hash_set* tree;
vector<int>* g1_iter;
vector<int>* tree_iter;
vector<int>* tree_orig;
vector<int> tree_to_orig;
vector<int> parent;
int *c1;
cuckoo_hash_set* g2;
vector<int>* g2_iter;
int *c2;

#ifdef DDEBUG
int indlv = 0;
void pind() {
    for (int i=0; i<2*indlv; i++) putc(' ', stdout);
}
#endif

struct iso_t {
    dense_hash_map<int, int> dir;
    dense_hash_map<int, int> rev;
    iso_t() {
        dir.set_empty_key(-2);
        rev.set_empty_key(-2);
        dir.set_deleted_key(-3);
        rev.set_deleted_key(-3);
    }
    void insert(int a, int b) {
        dir[a] = b;
        rev[b] = a;
    }
    void remove(int a) {
        rev.erase(dir[a]);
        dir.erase(a);
    }
    bool count(int a) {
        return dir.count(a);
    }
    bool count(int a, int b) {
        return dir.count(a) && dir[a] == b;
    }
    bool rcount(int a) {
        return rev.count(a);
    }
    void print(bool end = true) {
        printf("{");
        for (auto x: dir) {
            printf("%d, ", tree_to_orig[x.first]);
        }
        printf("\b\b} -> {");
        for (auto x: dir) {
            printf("%d, ", x.second);
        }
        printf("\b\b}");
        if (end) printf("\n");
        fflush(stdout);
    }
    bool can_add(int a, int b) {
        if (count(a) || rcount(b)) return false;
        for (auto x: g1_iter[a]) {
            if (!count(x)) continue;
            if (!g2[b].count(dir[x]))
                return false;
        }
        for (auto x: g2_iter[b]) {
            if (!rcount(x)) continue;
            if (!g1[a].count(rev[x]))
                return false;
        }
        return true;
    }
};

bool is_parent(iso_t& iso, iso_t& new_iso, int first, int second) {
    priority_queue<pair<int, int>, vector<pair<int, int>>, std::greater<pair<int, int>>> cand;
    for (auto& cur: new_iso.dir) {
        for (auto& a: tree_iter[cur.first]) {
            if (a > first) continue;
            for (auto& b: g2_iter[cur.second]) {
                if (c1[a] != c2[b]) continue;
                if (a == first && b >= second) continue;
                if (!new_iso.can_add(a, b)) continue;
                if (g1[first].count(a) == g2[second].count(b) && first != a && second != b) return false;
                cand.emplace(a, b);
            }
        }
    }
#ifdef DEBUG
    pind();
    printf("cand size: %lu\n", cand.size());
#endif
    iso_t tmp_iso = new_iso;
    while (!cand.empty()) {
        pair<int, int> cnd = cand.top();
        cand.pop();
        if (!tmp_iso.can_add(cnd.first, cnd.second))
            continue;
        tmp_iso.insert(cnd.first, cnd.second);
        if (!iso.count(cnd.first, cnd.second)) {
            return false;
        }
        for (auto& a: tree_iter[cnd.first]) {
            if (a >= first) continue;
            for (auto& b: g2_iter[cnd.second]) {
                if (c1[a] != c2[b]) continue;
                cand.emplace(a, b);
            }
        }
    }
    return true;
}

void isom(iso_t& iso) {
    int max_first = -1;
    int max_second = -1;
    for (auto& elem: iso.dir)
        if (elem.first > max_first) {
            max_first = elem.first;
            max_second = elem.second;
        }
#ifdef DDEBUG
    pind();
    printf("start ");
    iso.print();
    indlv++;
#endif
    // Complete the current iso
    priority_queue<pair<int, int>, vector<pair<int, int>>, std::greater<pair<int, int>>> cand;
    for (auto& cur: iso.dir) {
        for (auto& a: tree_iter[cur.first]) {
            for (auto& b: g2_iter[cur.second]) {
                if (c1[a] != c2[b]) continue;
                cand.emplace(a, b);
            }
        }
    }
    while (!cand.empty()) {
        pair<int, int> cnd = cand.top();
        cand.pop();
        if (!iso.can_add(cnd.first, cnd.second))
            continue;
        iso.insert(cnd.first, cnd.second);
        for (auto& a: tree_iter[cnd.first]) {
            for (auto& b: g2_iter[cnd.second]) {
                if (c1[a] != c2[b]) continue;
                cand.emplace(a, b);
            }
        }
    }
#ifdef DDEBUG
    pind();
    printf("extended ");
    iso.print();
#endif
    // Start processing the extended iso
    for (auto& elem: iso.dir) {
        for (auto& first: tree_iter[elem.first]) {
            if (first < max_first) continue;
            if (first < elem.first) continue;
#ifdef DDEBUG
            pind();
            printf("t %d %d\n", tree_to_orig[elem.first], tree_to_orig[first]);
#endif
            for (auto& second: g2_iter[elem.second]) {
                if (c1[first] != c2[second]) continue;
                if (max_first == first && max_second >= second) continue;
                if (iso.count(first, second)) continue;
#ifdef DDEBUG
                pind();
                printf("%d %d\n", tree_to_orig[first], second);
#endif
                // Forcibly add (first, second) to iso
                iso_t new_iso;
                for (auto x: iso.dir) {
                    if (x > pair<const int, int>(first, second)) continue;
                    if (g1[first].count(x.first) == g2[second].count(x.second) && first != x.first && second != x.second) {
                        new_iso.insert(x.first, x.second);
                    }
                }
                if (new_iso.dir.size() == 0) continue;
#ifdef DEBUG
                pind();
                printf("new_iso ");
                new_iso.print();
#endif
                // Remove stuff from new_iso to make it connected
                dense_hash_set<int> visited;
                visited.set_empty_key(-2);
                stack<int> dfs_stack;
                dfs_stack.push(first);
                while (!dfs_stack.empty()) {
                    int v = dfs_stack.top();
                    dfs_stack.pop();
                    if (visited.count(v)) continue;
                    visited.insert(v);
                    for (auto x: tree_iter[v])
                        if (new_iso.count(x))
                            dfs_stack.push(x);
                }
                vector<int> to_remove;
                for (auto& x: new_iso.dir)
                    if (!visited.count(x.first))
                        to_remove.push_back(x.first);
                for (auto& x: to_remove)
                    new_iso.remove(x);
#ifdef DEBUG
                pind();
                printf("connect ");
                new_iso.print();
#endif
                if (new_iso.dir.size() == 0) continue;
                // Check if iso is the right parent for new_iso
                if (!is_parent(iso, new_iso, first, second)) continue;
#ifdef DEBUG
                pind();
                printf("is_parent\n");
#endif
/*                // Check if new_iso is maximal
                new_iso.insert(first, second);
                bool is_maximal = true;
                vector<pair<int, int>> m_cand;
                for (auto& cur: new_iso.dir) {
                    for (auto& a: tree_iter[cur.first]) {
                        if (a >= first) continue;
                        for (auto& b: g2_iter[cur.second]) {
                            if (c1[a] != c2[b]) continue;
                            m_cand.emplace_back(a, b);
                        }
                    }
                }
                for (auto& cnd: m_cand) {
                    if (!new_iso.can_add(cnd.first, cnd.second))
                        continue;
                    is_maximal = false;
                    break;
                }
#ifdef DEBUG
                pind();
                printf("is_maximal\n");
#endif
#ifdef DDEBUG
                pind();
                new_iso.print(false);
                printf(" from ");
                iso.print();
#endif
                if (!is_maximal) continue;
*/
                new_iso.insert(first, second);
                isom(new_iso);
            }
        }
    }
#ifndef DDEBUG
    iso.print();
#endif
#ifdef DDEBUG
    indlv--;
    pind();
    printf("stop ");
    iso.print();
#endif
}

void build_tree() {
// Da qui
    stack<std::pair<int, int>> dfs_stack;
    for (int i=0; i<N1; i++) {
        dfs_stack.emplace(i, i);
        while (!dfs_stack.empty()) {
            pair<int, int> p = dfs_stack.top();
            dfs_stack.pop();
            if (parent[p.first] != -1) continue;
            parent[p.first] = p.second;
            tree_to_orig.push_back(p.first);
            for (auto x: tree_orig[p.first])
                dfs_stack.emplace(x, p.first);
        }
    }
// a qui
// calcola in parent[i] il parent di i nell'albero
// e in tree_to_orig la corrispondenza [etichetta_di_i_nell'albero] -> i
    vector<int> orig_to_tree(N1);
    for (int i=0; i<N1; i++)
        orig_to_tree[tree_to_orig[i]] = i;
    for (int i=0; i<N1; i++)
        if (parent[i] != i) {
            tree[orig_to_tree[i]].insert(orig_to_tree[parent[i]]);
            tree[orig_to_tree[parent[i]]].insert(orig_to_tree[i]);
            tree_iter[orig_to_tree[parent[i]]].push_back(orig_to_tree[i]);
            tree_iter[orig_to_tree[i]].push_back(orig_to_tree[parent[i]]);
#ifdef DDEBUG
            printf("%d %d -- %d %d\n", i, parent[i], orig_to_tree[i], orig_to_tree[parent[i]]);
#endif
        }
    for (int i=0; i<N1; i++)
        for (auto x: g1_orig_iter[i]) {
            g1[orig_to_tree[i]].insert(orig_to_tree[x]);
            g1[orig_to_tree[x]].insert(orig_to_tree[i]);
            g1_iter[orig_to_tree[i]].push_back(orig_to_tree[x]);
            g1_iter[orig_to_tree[x]].push_back(orig_to_tree[i]);
        }
    delete[] tree_orig;
    delete[] g1_orig;
    delete[] g1_orig_iter;
    vector<int> c1_tmp(N1);
    for (int i=0; i<N1; i++) c1_tmp[i] = c1[i];
    for (int i=0; i<N1; i++)
        c1[i] = c1_tmp[tree_to_orig[i]];
}

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s g1 g2\n", argv[0]);
        return 1;
    }
    FILE* f1 = fopen(argv[1], "r");
    fscanf(f1, "%d%d", &N1, &M1);
    g1 = new cuckoo_hash_set[N1];
    g1_orig = new cuckoo_hash_set[N1];
    parent.resize(N1, -1);
    tree = new cuckoo_hash_set[N1];
    tree_iter = new vector<int>[N1];
    g1_iter = new vector<int>[N1];
    g1_orig_iter = new vector<int>[N1];
    c1 = new int[N1];
    DSU dsu(N1);
    tree_orig = new vector<int>[N1];
    for (int i=0; i<N1; i++) fscanf(f1, "%d", c1+i);
    for (int i=0; i<M1; i++) {
        int a, b;
        fscanf(f1, "%d%d", &a, &b);
        g1_orig[a].insert(b);
        g1_orig[b].insert(a);
        g1_orig_iter[a].push_back(b);
        g1_orig_iter[b].push_back(a);
        if (dsu.find(a) != dsu.find(b)) {
            tree_orig[a].push_back(b);
            tree_orig[b].push_back(a);
            dsu.join(a, b);
        }
    }
    FILE* f2 = fopen(argv[2], "r");
    fscanf(f2, "%d%d", &N2, &M2);
    g2 = new cuckoo_hash_set[N2];
    g2_iter = new vector<int>[N2];
    c2 = new int[N2];
    for (int i=0; i<N2; i++) fscanf(f2, "%d", c2+i);
    for (int i=0; i<M2; i++) {
        int a, b;
        fscanf(f2, "%d%d", &a, &b);
        g2[a].insert(b);
        g2[b].insert(a);
        g2_iter[a].push_back(b);
        g2_iter[b].push_back(a);
    }
    build_tree();
    for (int i=0; i<N1; i++) {
        for (int j=0; j<N2; j++) {
            if (c1[i] != c2[j]) continue;
            bool has_back_black = false;
            for (auto& x: tree_iter[i]) {
                if (x > i) continue;
                for (auto& y: g2_iter[j])
                    if (c1[x] == c2[y]) {
                        has_back_black = true;
                        break;
                    }
                if (has_back_black) break;
            }
            if (has_back_black) continue;
            iso_t iso;
            iso.insert(i, j);
            isom(iso);
        }
    }
}
