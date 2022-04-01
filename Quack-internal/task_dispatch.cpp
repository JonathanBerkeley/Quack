#include "pch.hpp"
#include "task_dispatch.hpp"
#include "config.hpp"
#include "memory_scanner.hpp"


using namespace std::chrono_literals;
using namespace cfg;
namespace chrono = std::chrono;

using Task = std::pair<chrono::seconds, std::function<void(void)>>;

// todo: Develop this functionality
void TaskDispatch(int) {
    // Set up randomness
    std::random_device rd{};
    std::mt19937 mt{rd()};
    std::uniform_int_distribution<> dist(0, 10);

    std::vector<Task> tasks;
}

