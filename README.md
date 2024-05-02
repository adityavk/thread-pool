# Simple Thread Pool Library

The Simple Thread Pool Library is a lightweight header-only C++ library that provides a simple and efficient way to manage and execute tasks in parallel using a thread pool.

## Getting Started
To use the Simple Thread Pool Library, simply include the `SimpleThreadPool.hpp` header file in your project.

```cpp
#include "SimpleThreadPool.hpp"
```

## Example
The following example demonstrates how to use the Simple Thread Pool Library to calculate the sum of the first 1000 natural numbers in parallel.

```cpp
#include <iostream>
#include "SimpleThreadPool.hpp"

int main() {
    // Create a thread pool with 4 threads
    SimpleThreadPool threadPool(4);

    // Define a task that calculates the sum of integers in the range [start, end) modulo (1e9 + 7)
    constexpr auto sumTask = [](std::uint64_t start, std::uint64_t end, std::atomic<int>& result) {
        int sum = 0;
        for (std::uint64_t i = start; i < end; ++i) {
            sum = (sum + i) % 1000000007;
        }
        result = (result + sum) % 1000000007;
    };

    // Queue 100 tasks to calculate the sum of integers in the range [0, 1e9) in parallel
    std::atomic<int> result(0);
    for (std::uint64_t i = 0; i < 100; ++i) {
        threadPool.queue([i, &sumTask, &result] { sumTask(i * 10000000, (i + 1) * 10000000, result); });
    }

    // Wait for all tasks to complete
    threadPool.waitForTasks();
    std::cout << "Sum of integers in the range [0, 1e9) modulo (1e9 + 7) = " << result << std::endl;

    // Stop processing tasks: no more tasks will be queued, but the threads will continue processing the remaining tasks
    // threadPool.stopProcessing();

    return 0;
}
```

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE.md) file for details.
