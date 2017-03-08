#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <stdlib.h>
#include <queue>
#include <boost/lockfree/queue.hpp>

typedef std::pair<std::vector<int>, std::vector<int>> iso_t;

struct iso_p {
    iso_t* i;
    std::string* s;
};

boost::lockfree::queue<iso_p> maybe_retained(65536);
//std::vector<iso_t*> retained;
//std::mutex cin_mutex;
std::mutex io_mutex;
std::atomic<int> line_count;

std::atomic<iso_t**> retained;
std::atomic<unsigned> retained_count;
std::atomic<unsigned> retained_capacity;

unsigned size_ts;
double perc_ts;

unsigned intersection_size(const std::vector<int>& a, const std::vector<int>& b) {
	unsigned intsz = 0;
	unsigned j = 0;
	for (auto x: a) {
		while (j < b.size() && b[j] < x) j++;
		if (j == b.size()) break;
		if (b[j] == x) intsz++;
	}
	return intsz;
}

bool is_contained_thres(const std::vector<int>& ref, const std::vector<int>& b) {
	if (intersection_size(ref, b) > perc_ts*b.size()) return true;
	return false;
}

bool can_retain(const iso_t& iso) {
	bool left_contained = false;
	bool right_contained = false;
	for (unsigned i=0; i<retained_count; i++) {
		left_contained = left_contained || is_contained_thres(retained[i]->first, iso.first);
		right_contained = right_contained || is_contained_thres(retained[i]->second, iso.second);
		if (left_contained && right_contained) return false;
	}
	return true;
}

void append() {
    bool eof = false;
    bool quit = false;
    while (true) {
		{
            std::unique_lock<std::mutex> stdin_lock(io_mutex);
			if (std::cin.eof()) eof = true;
		}
        iso_p cur;
        bool empty = !maybe_retained.pop(cur);
        if (empty && eof && quit) break;
        if (empty && eof) { 
            std::this_thread::sleep_for(std::chrono::seconds(1));
            quit = true;
            continue;
        }
        if (empty) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        if (!can_retain(*cur.i)) {
            delete cur.i;
            delete cur.s;
            continue;
        }
        {
            std::unique_lock<std::mutex> stdout_lock(io_mutex);
            std::cout << *cur.s << std::endl;
        }
        delete cur.s;
        if (retained_count == retained_capacity) {
            retained_capacity = retained_capacity * 2;
            iso_t** tmp = new iso_t*[retained_capacity];
            for (unsigned i=0; i<retained_count; i++)
                tmp[i] = retained[i];
            retained = tmp;
        }
        retained[retained_count] = cur.i;
        retained_count++;
    }
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


void process() {
	while (true) {
        std::unique_ptr<std::string> line(new std::string);
		{
			std::unique_lock<std::mutex> stdin_lock(io_mutex);
			if (std::cin.eof()) break;
			std::getline(std::cin, *line);
		}
		line_count++;
		std::vector<int> v1, v2;
		unsigned pos = 0;
		int a;
		for (a = getInt(*line, pos); a >= 0; a = getInt(*line, pos)) v1.push_back(a);
		if (a == -2) continue;
		for (a = getInt(*line, pos); a >= 0; a = getInt(*line, pos)) v2.push_back(a);
		if (v1.size() != v2.size()) continue;
		if (v1.size() < size_ts) continue;
		std::sort(v1.begin(), v1.end());
		std::sort(v2.begin(), v2.end());
        std::unique_ptr<iso_t> iso(new iso_t{std::move(v1), std::move(v2)});
		bool ok = can_retain(*iso);
		if (ok) maybe_retained.push({iso.release(), line.release()});
	}
}

int main(int argc, char** argv) {
	if (argc >= 2 && (argv[1] == std::string("-h") || argv[1] == std::string("--help"))) {
		std::cerr << argv[0] << " n_thread retain_sz retain_thres" << std::endl;
		return 1;
	}
	std::ios_base::sync_with_stdio(false);
	unsigned thread_n = argc >= 2 ? atoi(argv[1]) : 1;
	size_ts = argc >= 3 ? atoi(argv[2]) : 10;
	perc_ts = argc >= 4 ? std::stod(argv[3]) : 0.7;
	retained_capacity = 65536;
    retained = new iso_t*[retained_capacity];
    std::vector<std::thread> thrd;
    thrd.emplace_back(append);
	for (unsigned i=0; i<thread_n; i++) thrd.emplace_back(process);
	auto start = std::chrono::steady_clock::now();
	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(5));
		{
            std::unique_lock<std::mutex> stdin_lock(io_mutex);
			if (std::cin.eof()) break;
		}
		auto cur = std::chrono::steady_clock::now();
		double duration = std::chrono::duration_cast<std::chrono::duration<double>>(cur-start).count();
		std::cerr << line_count << " lines in " << duration << " seconds.";
		std::cerr << " Retained " << retained_count << " isomorphisms.";
		std::cerr << " Speed: " << std::fixed << line_count/duration << " lines/sec." << std::endl;
	}
	for (auto& x: thrd) x.join();
	auto cur = std::chrono::steady_clock::now();
	double duration = std::chrono::duration_cast<std::chrono::duration<double>>(cur-start).count();
	std::cerr << line_count << " lines in " << duration << " seconds.";
	std::cerr << " Retained " << retained_count << " isomorphisms.";
	std::cerr << " Speed: " << std::fixed << line_count/duration << " lines/sec." << std::endl;
}
