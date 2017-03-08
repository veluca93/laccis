#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <mutex>
#include <boost/smart_ptr/detail/spinlock.hpp>

std::thread printer;
int interval;
std::atomic<int> line_count;
std::atomic<bool> input_end;
std::unordered_map<int, int> sizes;
boost::detail::spinlock io_mutex;

void print_status() {
    std::ios_base::sync_with_stdio(true);
    auto start = std::chrono::steady_clock::now();
    while (!input_end) {
        std::chrono::duration<long int> time_elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now()-start);
        {
            std::unique_lock<boost::detail::spinlock> io_lock(io_mutex);
            std::cout << std::setw(10) << time_elapsed.count() << " " << std::setw(10) << line_count;
            if (isatty(fileno(stdout))) {
                std::cout << "\r" << std::flush;
            } else {
                std::cout << std::endl;
            }
	}
        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }
    std::chrono::duration<long int> time_elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now()-start);
    {
        std::cout << std::setw(10) << time_elapsed.count() << " " << std::setw(10) << line_count;
        if (isatty(fileno(stdout))) {
            std::cout << "\r" << std::flush;
        } else {
            std::cout << std::endl;
        }
    }
}

void print_sizes(std::ostream& out) {
    out << "\nSizes:\n";
    std::vector<std::pair<int, int>> counts;
    for (auto x: sizes) counts.push_back(x);
    std::sort(counts.begin(), counts.end());
    for (auto x: counts)
        out << std::setw(10) << x.first << " " << std::setw(10) << x.second << std::endl;
}

void usr1_handler(int) {
    print_sizes(std::cerr);
    std::cerr << std::endl;
    std::cerr << std::endl;
}

void cleanup() {
    input_end = true; 
    printer.join();
    print_sizes(std::cout);
}

void sg_handler(int signal) {
    exit(0);
}

int main(int argc, char** argv) {
    interval = argc < 2 ? 1 : atoi(argv[1]);
    printer = std::thread(&print_status);
    atexit(cleanup);
    signal(SIGINT, sg_handler);
    signal(SIGTERM, sg_handler);
    signal(SIGPIPE, sg_handler);
    signal(SIGUSR1, usr1_handler);
    std::string line;
    std::ios_base::sync_with_stdio(false);
    while (true) {
        {
            std::unique_lock<boost::detail::spinlock> io_lock(io_mutex);
            if (getline(std::cin, line).eof()) break;
        }
        int cur_size = std::count(line.begin(), line.end(), ',')/2;
        sizes[cur_size]++;
        line_count++;
    }
}
