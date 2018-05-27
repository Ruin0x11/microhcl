#include "hcl/hcl.hpp"

#include "thirdparty/catch2/catch.hpp"
#include <map>
#include <istream>
#include <sstream>
#include <string>

const std::string fixtureDir = "tests/test-fixtures/decoding";

template <typename Map>
bool map_compare (Map const &lhs, Map const &rhs) {
    // No predicate needed because there is operator== for pairs already.
    return lhs.size() == rhs.size()
        && std::equal(lhs.begin(), lhs.end(),
                      rhs.begin());
}

static hcl::Value parseFile(const std::string& filename)
{
    std::ifstream is(fixtureDir + "/" + filename);
    REQUIRE(is);
    hcl::internal::Parser p(is);

    hcl::Value v = p.parse();
    if (p.errorReason().size() != 0)
        std::cerr << p.errorReason() << std::endl;
    return v;
}

static bool parseFileFails(const std::string& filename)
{
    std::ifstream is(fixtureDir + "/" + filename);
    REQUIRE(is);
    hcl::internal::Parser p(is);

    hcl::Value v = p.parse();
    return !v.valid();
}

std::map<std::string, hcl::Value> cases = {
    {"basic.hcl",
     hcl::Value(hcl::Object{
             {"foo", "bar"},
             {"bar", "${file(\"bing/bong.txt\")}"}
         })},
    {"basic_squish.hcl",
     hcl::Value(hcl::Object{
             {"foo", "bar"},
             {"bar", "${file(\"bing/bong.txt\")}"},
             {"foo-bar", "baz"}
         })},
    {"empty.hcl",
     hcl::Value(hcl::Object{
             {
                 "resource", hcl::Object{
                     {"foo", hcl::Object{}}
                 }
             }
         })},
    {"tfvars.hcl",
     hcl::Value(hcl::Object{
             {"regularvar", "Should work"},
             {"map.key1", "Value"},
             {"map.key2", "Other value"}
         })},
    {"escape.hcl",
     hcl::Value(hcl::Object{
             {"foo",          "bar\"baz\\n"},
             {"qux",          "back\\slash"},
             {"bar",          "new\nline"},
             {"qax",          R"(slash\:colon)"},
             {"nested",       R"(${HH\\:mm\\:ss})"},
             {"nestedquotes", R"(${"\"stringwrappedinquotes\""})"}
         })},
    {"float.hcl",
     hcl::Value(hcl::Object{
             {"a", 1.02},
             {"b", 2}
         })},
    {"multiline_literal_with_hil.hcl",
     hcl::Value(hcl::Object{
             {"multiline_literal_with_hil", "${hello\n world}"}
         })},
    {"multiline.hcl",
     hcl::Value(hcl::Object{
             {"foo", "bar\nbaz\n"}
         })},
    {"multiline_indented.hcl",
     hcl::Value(hcl::Object{
             {"foo", "  bar\n  baz\n"}
         })},
    {"multiline_no_hanging_indent.hcl",
     hcl::Value(hcl::Object{
             {"foo", "  baz\n    bar\n      foo\n"}
         })},
    {"multiline_no_eof.hcl",
     hcl::Value(hcl::Object{
             {"foo", "bar\nbaz\n"},
             {"key", "value"}
         })},
    {"scientific.hcl",
     hcl::Value(hcl::Object{
             {"a", 1e-10},
             {"b", 1e+10},
             {"c", 1e10},
             {"d", 1.2e-10},
             {"e", 1.2e+10},
             {"f", 1.2e10}
         })},
    {"terraform_heroku.hcl",
     hcl::Value(hcl::Object{
             {"name", "terraform-test-app"},
             {"config_vars", hcl::Object{
                     {"FOO", "bar"}
                 }}
         })},
    {"structure_multi.hcl",
     hcl::Value(hcl::Object{
             {"foo", hcl::Object{
                     {"baz", hcl::Object{
                             {"key", 7}
                         }},
                     {"bar", hcl::Object{
                             {"key", 12}
                         }}
                 }}
         })},
    {"list_of_lists.hcl",
     hcl::Value(hcl::Object{
             {"foo", hcl::List{
                     hcl::List{"foo"}, hcl::List{"bar"}
                 }}
         })},
    {"list_of_maps.hcl",
     hcl::Value(hcl::Object{
             {"foo", hcl::List{
                     hcl::Object{
                         {"somekey1", "someval1"}
                     },
                         hcl::Object{
                             {"somekey2", "someval2"},
                             {"someextrakey", "someextraval"}
                         }
                 }}
         })},
    {"assign_deep.hcl",
     hcl::Value(hcl::Object{
             {"resource", hcl::List{hcl::Object{
                         {"foo", hcl::List{hcl::Object{
                                     {"bar", hcl::Object{}}
                                 }}}
                     }}}
         })},
    {"nested_block_comment.hcl",
     hcl::Value(hcl::Object{
             {"bar", "value"}
         })},
    {"escape_backslash.hcl",
     hcl::Value(hcl::Object{
             {"output", hcl::Object{
                     {"one", R"(${replace(var.sub_domain, ".", "\\.")})"},
                     {"two", R"(${replace(var.sub_domain, ".", "\\\\.")})"},
                     {"many", R"(${replace(var.sub_domain, ".", "\\\\\\\\.")})"}
                 }}
         })},
    {"object_with_bool.hcl",
     hcl::Value(hcl::Object{
             {"path", hcl::Object{
                     {"policy", "write"},
                     {"permissions", hcl::Object{
                             {"bool", hcl::List{false}}
                         }},
                 }}
         })}
};

std::vector<std::string> invalidCases = {
    "multiline_bad.hcl",
    "multiline_literal.hcl",
    "multiline_no_marker.hcl",
    "unterminated_block_comment.hcl",
    "unterminated_brace.hcl",
    "nested_provider_bad.hcl",
    "block_assign.hcl",
    "git_crypt.hcl"
};

TEST_CASE("decode valid structures")
{
    for (const auto& pair : cases) {
        const std::string filename = pair.first;
        SECTION(filename)
        {
            hcl::Value actual = parseFile(filename);
            hcl::Value expected = pair.second;
            REQUIRE(actual.valid());
            std::cout << " ===== Expected ===== " << std::endl;
            std::cout << expected << std::endl;
            std::cout << " =====  Actual  ===== " << std::endl;
            std::cout << actual << std::endl;
            std::cout << " ==================== " << std::endl;
            REQUIRE(map_compare(expected.as<hcl::Object>(), actual.as<hcl::Object>()));
        }
    }
}

TEST_CASE("fail decoding invalid structures")
{
    for (const auto& filename : invalidCases) {
        SECTION(filename)
        {
            REQUIRE(parseFileFails(filename));
        }
    }
}
