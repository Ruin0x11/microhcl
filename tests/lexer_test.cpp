#include "hcl/hcl.hpp"

#include "thirdparty/catch2/catch.hpp"
#include <map>
#include <sstream>
#include <string>

using namespace hcl;
using namespace hcl::internal;

struct TokenPair {
    TokenType tok;
    std::string text;
};

std::map<std::string, std::vector<TokenPair>> tokenPairs = {
    {"comment", {TokenPair{TokenType::COMMENT, "//"}}}
};
