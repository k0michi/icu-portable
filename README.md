# icu-portable

icu-portable is a CMake wrapper for [icu4c](https://github.com/unicode-org/icu).

## Features

- **CMake-based.** Integrates seamlessly with your CMake projects.
- **No host build required.** Downloads prebuilt ICU data for the target platform, from
  the official ICU releases.

## Installation

To use icu-portable in your CMake project, follow these steps:

1. Add the following to your `CMakeLists.txt`:

```cmake
include(FetchContent)
FetchContent_Declare(
    icu_portable
    GIT_REPOSITORY "https://github.com/k0michi/icu-portable.git"
    GIT_TAG        "main"
)

FetchContent_MakeAvailable(icu_portable)
```

2. Link against the ICU libraries in your target:

```cmake
target_link_libraries(your_target PRIVATE icuuc icui18n icuio)
```

## Requirements

- CMake 3.20 or higher
- C++17 compatible compiler
- Python 3.x (for converting ICU data)

## License

Unlicense (public domain). Note that icu4c is licensed under the ICU License.