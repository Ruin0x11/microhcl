# microhcl [![AppVeyor Build Status][appveyor-build-status-svg]][appveyor-build-status] [![Travis CI Build Status][travis-build-status-svg]][travis-build-status]
C++14 header-only parser for the [Hashicorp Configuration Language](https://www.github.com/hashicorp/hcl).

## Requirements
- C++14-compliant compiler
- CMake >= 2.8.12

## Usage
```c++
std::ifstream ifs("foo.hcl");
hcl::ParseResult parseResult = hcl::parse(ifs);

if (!parseResult.valid()) {
    std::cout << parseResult.errorReason << std::endl;
    return;
}

const hcl::Value& value = parseResult.value;
```

## Running the tests
```
mkdir out/Debug
cd out/Debug
cmake ../../tests
make
./test_runner
```

## Incompatibilities
- Block comments are unsupported.
- Negative float numbers without a leading 0 are not recognized.
- Some unprintable escape sequences (like `\a`) are not recognized.
- Object items can be output in any order when writing. (can be turned off by passing `MICROHCL_USE_MAP` as a compile definition)
- Comments are not preserved when writing.
- There are currently no implicit type conversions. The type retrieved must be the one specified in the original HCL code.

<!-- Badges -->
[appveyor-build-status]: https://ci.appveyor.com/project/Ruin0x11/microhcl/branch/master
[appveyor-build-status-svg]: https://ci.appveyor.com/api/projects/status/fi2su01yo2eah7wf/branch/master?svg=true
[travis-build-status]: https://travis-ci.org/Ruin0x11/microhcl?branch=master
[travis-build-status-svg]: https://travis-ci.org/Ruin0x11/microhcl.svg?branch=master
