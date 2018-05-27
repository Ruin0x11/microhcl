# microhcl
C++14 header-only parser for the [Hashicorp Configuration Language](https://www.github.com/hashicorp/hcl).

## Usage
```c++
std::ifstream ifs("foo.hcl");
hcl::ParseResult parseResult = hcl::parse(ifs);

if (!parseResult.valid()) {
    std::cout << pr.errorReason << std::endl;
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
