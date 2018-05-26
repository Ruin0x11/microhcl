#define CATCH_CONFIG_RUNNER
#include "thirdparty/catch2/catch.hpp"

int main(int argc, char* argv[])
{
    return Catch::Session().run(argc, argv);
}
