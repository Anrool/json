# What
Standard-like JSON header-only library.
It's basically a ``shamefully copy-pasted`` inspired clone of [JSON for Modern C++](https://github.com/nlohmann/json). :bowtie:

# Why
The current C++ standard doesn't have a JSON library, therefore, it looked interesting to explore possible implementations and practice generic programming at the same time.

# Requirements
Standard-compatible compiler with C++11 support.

# Usage approaches
1. Import as CMake project
  - External
```
find_package(anrool_json REQUIRED)
```
   - Internal
```
add_subdirectory(<dir>)
```

and use it as INTERFACE library:
```
add_library(<name> <sources>)
target_link_libraries(<name> PRIVATE anrool_json::anrool_json)
```
2. Clone and append the header folder into target include directories (`git submodule`  can be a good solution).

# CMake build workflow
```
mkdir <build_dir>
cd <build_dir>
cmake <source_dir>
cmake --build <build_dir>
```

# Test
To compile and launch unit tests use the following steps:
```
mkdir <build_dir>
cd <build_dir>
cmake <source_dir> -DANROOL_BUILD_TESTS=ON
cmake --build <build_dir>
(cd test && ctest -VV)
```

# Further steps
- [ ] Create amalgamate
- [ ] Add hashtable support
- [ ] Add tests for most common use cases and fuzz testing
- [ ] Add test coverage
- [ ] Provide package manager support (Conan)
- [ ] Generate documentation
- [ ] Come up with CI integration
- [ ] Invent the way to switch off exceptions
