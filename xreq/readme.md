# xreq - Modern C++ HTTP Client Library

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A simple yet powerful C++17 HTTP client inspired by Python Requests. Built on libcurl with RAII-style management.

## Features

- Chainable API design
- Sync/Async requests
- HTTPS & SSL/TLS
- File uploads
- Cookie persistence
- Header management
- Timeout control
- Cross-platform

## Quick Start

```cpp
#include "xreq.hpp"
#include <iostream>

int main() {
    // Simple GET
    auto res = xreq::Request("https://httpbin.org/get")
        .Param("key", "value")
        .Timeout(5000)
        .Send();

    std::cout << "Status: " << res.status_code << "\n"
              << "Body: " << res.text << std::endl;
    return 0;
}
```

## Installation

### Linux/macOS
```bash
# Debian/Ubuntu
sudo apt install libcurl4-openssl-dev

# macOS
brew install curl
```

### Windows
1. Download [curl for Windows](https://curl.se/windows/)
2. Set include/lib paths in your IDE

## Core API

### Request Builder
| Method | Description |
|--------|-------------|
| `.Header("Key", "Value")` | Add request header |
| `.Param("key", "value")` | URL query parameter |
| `.Timeout(3000)` | Set timeout (ms) |
| `.Body("content", "text/plain")` | Set request body |
| `.File("field", "file.txt")` | Upload file |
| `.CookieFile("cookies.txt")` | Manage cookies |
| `.Insecure()` | Disable SSL verify |
| `.Send()` | Sync request |
| `.SendAsync()` | Async request |

### Response Object
```cpp
struct Response {
    int status_code;       // HTTP status code
    std::string text;      // Response content
    std::map<std::string, std::string> headers; // Headers
};
```

## Advanced Usage

### Async Request
```cpp
auto future = xreq::Request("https://api.example.com/data")
    .SendAsync();

// Do other work...

auto res = future.get();
std::cout << "Async response: " << res.text; 
```

### File Upload
```cpp
xreq::Request("https://httpbin.org/post")
    .File("document", "report.pdf")
    .File("image", "photo.jpg")
    .Send();
```

### Cookie Session
```cpp
// Login and save cookies
xreq::Request("https://example.com/login")
    .CookieFile("session.txt")
    .Body("user=admin&pass=123")
    .Send();

// Reuse cookies
xreq::Request("https://example.com/dashboard")
    .CookieFile("session.txt")
    .Send();
```

## Building

### Command Line
```bash
g++ -std=c++17 -I. main.cpp xreq.cpp -lcurl -pthread -o app
```

### CMake
```cmake
cmake_minimum_required(VERSION 3.14)
project(my_app)

find_package(CURL REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/xreq.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CURL_LIBRARIES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
```

## License
MIT License. See [LICENSE](LICENSE) for details.
