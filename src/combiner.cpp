#include <stdio.h>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <sstream>
#include "sparsehash/dense_hash_map"
#include "sparsehash/dense_hash_set"
#include <unordered_set>

//typedef std::pair<std::map<int, int>, std::map<int, int>> iso_t;

std::vector<google::dense_hash_set<int>> g1, g2;
class iso_t {
	std::map<int, int> vals;
	google::dense_hash_map<int, int> dir;
	google::dense_hash_map<int, int> rev;
public:
	iso_t() {dir.set_empty_key(-1); rev.set_empty_key(-1);}
	void add(int from, int to) {
		vals[from] = to;
		dir[from] = to;
		rev[to] = from;
	}
	bool dir_has(int from) const {return dir.count(from);}
	int dir_get(int from) const {return dir.find(from)->second;}
	bool can_extend(int from, int to) const {
		if (dir.count(from)) return dir.find(from)->second == to;
		if (rev.count(to)) return rev.find(to)->second == from;
		for (auto x: g1[from])
			if (dir.count(x) && !g2[to].count(dir.find(x)->second))
				return false;
		for (auto x: g2[to])
			if (rev.count(x) && !g1[from].count(rev.find(x)->second))
				return false;
		return true;
	}
    bool contains(const iso_t& oth) const {
        for (auto x: oth.get_vals())
            if (!dir.count(x.first) || dir.find(x.first)->second != x.second)
                return false;
        return true;
    }
	void print() const {
		printf("{");
		for (auto x: vals)
			printf("%d, ", x.first);
		printf("\b\b} -> {");
		for (auto x: vals)
			printf("%d, ", x.second);
		printf("\b\b}\n");
	}
	bool operator<(const iso_t& oth) const {
		return vals.size() > oth.vals.size() || (vals.size() == oth.vals.size() && vals < oth.vals);
	}
	const std::map<int, int>& get_vals() const {return vals;}
	size_t size() const {return vals.size();}
	bool operator!=(const iso_t& oth) const {return vals != oth.vals;}
    bool operator==(const iso_t& oth) const {return vals == oth.vals;}
    uint64_t get_hash() const {
        uint64_t hash = 0;
        for (auto x: vals) {
            hash ^= x.first + 0x9e3779b9 + (hash<<6) + (hash>>2);
            hash ^= x.second + 0x9e3779b9 + (hash<<6) + (hash>>2);
        }
        return hash;
    }
};

struct iso_ptr {
    std::shared_ptr<iso_t> ptr;
    iso_ptr(): ptr(nullptr) {}
    iso_ptr(iso_t* ptr): ptr(ptr) {}
    bool operator<(const iso_ptr& oth) const {return *ptr < *oth;}
    bool operator==(const iso_ptr& oth) const {return *ptr == *oth;}
    bool operator!=(const iso_ptr& oth) const {return *ptr != *oth;}
    iso_t& operator*() const {return *ptr;}
    iso_t* operator->() const {return ptr.get();}
};
static const int combine_limit = 20;

namespace std {
    namespace tr1 {
        template<>
        struct hash<iso_ptr> {
            size_t operator()(const iso_ptr& k) const {
                return k->get_hash();
            }
        };
    }
    template<>
    struct hash<iso_ptr> {
        size_t operator()(const iso_ptr& k) const {
            return k->get_hash();
        }
    };
}


google::dense_hash_set<iso_ptr> isos;

void extend(const iso_ptr& iso1, const iso_ptr& iso2, int from, int to) {
	if (iso1->dir_has(from)) return;
	if (!iso1->can_extend(from, to)) return;
	iso1->add(from, to);
	for (auto y: g1[from])
		if (iso2->dir_has(y))
			extend(iso1, iso2, y, iso2->dir_get(y));
}

iso_ptr extend(const iso_ptr& iso1, const iso_ptr& iso2) {
    iso_ptr res(new iso_t(*iso1));
	for (auto x: iso1->get_vals()) 
		for (auto y: g1[x.first])
			if (iso2->dir_has(y))
				extend(res, iso2, y, iso2->dir_get(y));
	return res;
}

void read_graph(char* fin, std::vector<google::dense_hash_set<int>>& g) {
	FILE* f = fopen(fin, "r");
	int N, M;
	fscanf(f, "%d%d", &N, &M);
	g.resize(N);
	for (int i=0; i<N; i++) {
		int a;
		fscanf(f, "%d", &a);
		g[i].set_empty_key(-1);
	}
	for (int i=0; i<M; i++) {
		int a, b;
		fscanf(f, "%d%d", &a, &b);
		g[a].insert(b);
		g[b].insert(a);
	}
	fclose(f);
}

int getInt(const std::string& line, unsigned& pos) {
	int res = 0;
	while (pos < line.size() && line[pos] != '}' && (line[pos] < '0' || line[pos] > '9')) pos++;
	if (pos == line.size()) return -2;
	if (line[pos] == '}') {
		pos++;
		return -1;
	}
	while (pos < line.size() && line[pos] >= '0' && line[pos] <= '9') {
		res = res*10+line[pos] - '0';
		pos++;
	}
	return res; 
}

unsigned bis(const google::dense_hash_set<iso_ptr>& is) {
	unsigned m = 0;
    for (const auto& x: is) m = m < x->size() ? x->size() : m;
    return m;
}

const std::vector<iso_ptr>& get_biggest(const google::dense_hash_set<iso_ptr>& s) {
    static std::vector<iso_ptr> vec;
    vec.clear();
    for (const auto& x: s) vec.push_back(x);
    nth_element(vec.begin(), vec.begin()+combine_limit, vec.end());
    if (vec.size() > combine_limit)
        vec.resize(combine_limit);
    return vec;
}

struct ptrhash {
    size_t operator()(const iso_ptr& k) const {
        return (((size_t) k.ptr.get()) ^ 0xdeadbaba) * 0xdeadbaba;
    }
};

struct ptreq {
    size_t operator()(const iso_ptr& a, const iso_ptr& b) const {
        return a.ptr == b.ptr;
    }
};

std::vector<google::dense_hash_set<iso_ptr, ptrhash, ptreq>> node_isos; 
//std::vector<google::dense_hash_set<iso_ptr, ptrhash, ptreq>> node_new_isos; 

int main(int argc, char** argv) {
	if (argc < 3) {
		fprintf(stderr, "Usage: %s graph1 graph2 [isomorphisms=stdin [smallest = 10]]\n", argv[0]);
		return 1;
	}
	read_graph(argv[1], g1);
	read_graph(argv[2], g2);
	std::istream* fin;
	if (argc > 3) fin = new std::ifstream(argv[3]);
	else fin = &std::cin;
	unsigned smallest = argc > 4 ? atoi(argv[4]) : 0;
	google::dense_hash_set<iso_ptr> new_isos;

    iso_ptr empty = new iso_t;
    iso_ptr deleted = new iso_t;
    deleted->add(g1.size(), g2.size());
    for (unsigned i=0; i<g1.size(); i++) {
        node_isos.emplace_back();
        node_isos.back().set_empty_key(empty);
        node_isos.back().set_deleted_key(deleted);
    }
    isos.set_empty_key(empty);
    isos.set_deleted_key(deleted);
    new_isos.set_empty_key(empty);

	std::string line;
	while (!getline(*fin, line).eof()) {
		std::vector<int> v1, v2;
		unsigned pos = 0;
		int a;
		for (a = getInt(line, pos); a >= 0; a = getInt(line, pos)) v1.push_back(a);
		if (a == -2) continue;
		for (a = getInt(line, pos); a >= 0; a = getInt(line, pos)) v2.push_back(a);
		if (v1.size() != v2.size()) continue;
		if (v1.size() < smallest) continue;
		iso_ptr tmp(new iso_t);
		for (unsigned i=0; i<v1.size(); i++) {
			tmp->add(v1[i], v2[i]);
            node_isos[v1[i]].insert(tmp);
        }
		new_isos.insert(tmp);
	}
	int iter = 0;
	bool changed = true;
	fprintf(stderr, "Start: %zu isomorphisms. Biggest isomorphism is %u.\n", isos.size()+new_isos.size(), std::max(bis(isos), bis(new_isos)));
	while (changed) {
        for (auto& x: node_isos) x.clear();
        for (auto x: isos) {
            for (auto y: x->get_vals())
                node_isos[y.first].insert(x);
        }
        for (auto x: new_isos) {
            for (auto y: x->get_vals())
                node_isos[y.first].insert(x);
        }
		changed = false;
		google::dense_hash_set<iso_ptr> tmp_isos;
		google::dense_hash_set<iso_ptr> extended_isos;
        tmp_isos.set_empty_key(empty);
        extended_isos.set_empty_key(empty);
		for (const auto& x: get_biggest(new_isos)) {
            google::dense_hash_set<iso_ptr, ptrhash, ptreq> candidates;
            candidates.set_empty_key(empty);
            //for (auto y: isos) candidates.insert(y);
            //for (auto y: new_isos) new_candidates.insert(y);
            for (auto y: x->get_vals())
                for (auto z: g1[y.first])
                    for (auto w: node_isos[z])
                        candidates.insert(w);
            //fprintf(stderr, "%zu\n", candidates.size());
			for (const auto& y: candidates) {
                /*if (x->contains(*y)) {
                    extended_isos.insert(y);
                    continue;
                }*/
				const auto& temp = extend(x, y);
				if (!isos.count(temp) && !new_isos.count(temp)) {
                    tmp_isos.insert(temp);
                    if (temp != x) extended_isos.insert(x);
                }
                const auto& temp2 = extend(y, x);
				if (!isos.count(temp2) && !new_isos.count(temp2)) {
                    tmp_isos.insert(temp2);
                    if (temp2 != y) extended_isos.insert(y);
                }
			}
		}
		if (!tmp_isos.empty()) changed = true;
        for (auto x: new_isos) isos.insert(x);
		new_isos.clear();
		std::swap(new_isos, tmp_isos);
		iter++;
		for (const auto& x: extended_isos) isos.erase(x);
		fprintf(stderr, "Iteration %d, %zu isomorphisms (%zu new). Biggest isomorphism is %u.\n", iter, isos.size()+new_isos.size(), new_isos.size(), std::max(bis(isos), bis(new_isos)));
	}
    std::vector<iso_ptr> res;
	for (const auto& x: isos) res.push_back(x);
    std::sort(res.begin(), res.end());
    for (const auto& x: res) x->print();
}
