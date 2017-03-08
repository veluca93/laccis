#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <memory>
#include <unistd.h>
#include <atomic>
#include <mutex>
#include <chrono>
#include <thread>
#include "lz4io.h"
#include <boost/smart_ptr/detail/spinlock.hpp>

std::atomic<int> line_count;
boost::detail::spinlock io_mutex;

void process(std::string output_folder, int thrn) {
	std::vector<std::unique_ptr<lz4io>> buckets;
    std::vector<FILE*> files;
	std::string line;
	while (true) {
        {
            std::unique_lock<boost::detail::spinlock> stdin_lock(io_mutex);
            std::getline(std::cin, line);
            if (std::cin.eof()) break;
        }
        line_count++;
		unsigned cnt = std::count(line.begin(), line.end(), ',');
		if (cnt%2 != 0) continue;
		cnt /= 2;
		while (buckets.size() <= cnt) {
            FILE* f = fopen(
                (output_folder + "/" +
                std::to_string(buckets.size()) + "_" + std::to_string(thrn) + ".lz4").c_str(),
                "w"
            );
            files.push_back(f);
			buckets.emplace_back(new lz4io(f));
        }
		buckets[cnt]->append(line + "\n");
	}
    buckets.resize(0);
    for (auto x: files) fclose(x);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " output_folder [thread_number]" << std::endl;
        return 1;
    }
    std::ios_base::sync_with_stdio(false);
    std::string output_folder = argv[1];
    unsigned thread_n = argc > 2 ? atoi(argv[2]) : 1;
    std::vector<std::thread> thrd;
    for (unsigned i=0; i<thread_n; i++) thrd.emplace_back(process, output_folder, i);
    auto start = std::chrono::steady_clock::now();
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        {
            std::unique_lock<boost::detail::spinlock> stdin_lock(io_mutex);
            if (std::cin.eof()) break;
        }
        auto cur = std::chrono::steady_clock::now();
        double duration = std::chrono::duration_cast<std::chrono::duration<double>>(cur-start).count();
        std::cerr << "Processed " << line_count << " lines in " << duration << " seconds.";
        std::cerr << " Speed: " << std::fixed << line_count/duration << " lines/sec." << std::endl;
    }
    for (auto& x: thrd) x.join();
    auto cur = std::chrono::steady_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::duration<double>>(cur-start).count();
    std::cerr << "Processed " << line_count << " lines in " << duration << " seconds.";
    std::cerr << " Speed: " << std::fixed << line_count/duration << " lines/sec." << std::endl;
}
