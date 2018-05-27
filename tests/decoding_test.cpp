#include "hcl/hcl.hpp"

#include "thirdparty/catch2/catch.hpp"
#include <map>
#include <istream>
#include <sstream>
#include <string>

const std::string fixtureDir = "tests/test-fixtures/decoding";

template <typename Map>
bool map_compare (Map const &lhs, Map const &rhs) {
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
             {"nested",       R"(${HH\:mm\:ss})"},
             {"nestedquotes", R"(${""stringwrappedinquotes""})"}
         })},
    {"float.hcl",
     hcl::Value(hcl::Object{
             {"a", 1.02},
             {"b", 2}
         })},
    {"multiline_literal_with_hil.hcl",
     hcl::Value(hcl::Object{
             {"multiline_literal_with_hil", "${hello\n  world}"}
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
                     {"bar", hcl::Object{
                             {"key", 12}
                         }},
                     {"baz", hcl::Object{
                             {"key", 7}
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
    // {"nested_block_comment.hcl",
    //  hcl::Value(hcl::Object{
    //          {"bar", "value"}
    //      })},
    {"escape_backslash.hcl",
     hcl::Value(hcl::Object{
             {"output", hcl::Object{
                     {"one", R"(${replace(var.sub_domain, ".", "\.")})"},
                     {"two", R"(${replace(var.sub_domain, ".", "\\.")})"},
                     {"many", R"(${replace(var.sub_domain, ".", "\\\\.")})"}
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
         })},
    {"list_of_nested_object_lists.hcl",
     hcl::Value(hcl::Object{
             {"variable", hcl::List{
                     hcl::Object{
                         {"foo", hcl::Object{
                                 {"default", "bar"},
                                 {"description", "bar"}
                             }},
                         {"amis", hcl::Object{
                                 {"default", hcl::Object{
                                         {"east", "foo"}
                                     }}
                             }}
                     },
                         hcl::Object{
                             {"foo", hcl::Object{
                                     {"hoge", "fuga"}
                                 }}
                         }
                 }}
         })}
};

std::vector<std::string> invalidCases = {
    "multiline_bad.hcl",
    "multiline_literal.hcl",
    "multiline_literal_single_quoted.hcl",
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

TEST_CASE("decode flat map")
{
    const std::string filename = "structure_flatmap.hcl";
    hcl::Value actual = parseFile(filename);
    hcl::Value expected = hcl::Value(hcl::Object{
            {"foo", hcl::Object{
                    {"foo", "bar"},
                    {"key", 7},
                }}
        });
    REQUIRE(actual.valid());
    REQUIRE(map_compare(expected.as<hcl::Object>(), actual.as<hcl::Object>()));
}

TEST_CASE("decode flat structure")
{
    const std::string filename = "flat.hcl";
    hcl::Value value = parseFile(filename);
    REQUIRE(value.valid());

    REQUIRE("bar" == value["foo"].as<std::string>());
    REQUIRE(7 == value["Key"].as<int>());
    REQUIRE_THROWS(value["Key"].as<std::string>());
    REQUIRE_THROWS(value["Key"].as<double>());
    REQUIRE_THROWS(value["foo"].as<bool>());
}

TEST_CASE("decode array structure")
{
    const std::string filename = "decode_policy.hcl";
    hcl::Value value = parseFile(filename);
    REQUIRE(value.valid());

    REQUIRE("read" == value["key"][""]["policy"].as<std::string>());
    REQUIRE("write" == value["key"]["foo/"]["policy"].as<std::string>());
    REQUIRE("read" == value["key"]["foo/bar/"]["policy"].as<std::string>());
    REQUIRE("deny" == value["key"]["foo/bar/baz"]["policy"].as<std::string>());
}

TEST_CASE("decode slice structure")
{
    const std::string filename = "slice_expand.hcl";
    hcl::Value value = parseFile(filename);
    REQUIRE(value.valid());

    REQUIRE("value" == value["service"]["my-service-0"]["key"].as<std::string>());
    REQUIRE("value" == value["service"]["my-service-1"]["key"].as<std::string>());
}


TEST_CASE("decode map structure")
{
    const std::string filename = "decode_tf_variable.hcl";
    hcl::Value value = parseFile(filename);
    REQUIRE(value.valid());

    REQUIRE("bar" == value["variable"]["foo"]["default"].as<std::string>());
    REQUIRE("bar" == value["variable"]["foo"]["description"].as<std::string>());
    REQUIRE("foo" == value["variable"]["amis"]["default"]["east"].as<std::string>());
}

TEST_CASE("decode top level keys")
{
    const std::string filename = "top_level_keys.hcl";
    hcl::Value value = parseFile(filename);
    REQUIRE(value.valid());

    REQUIRE("blah" == value["template"][0]["source"].as<std::string>());
    REQUIRE("blahblah" == value["template"][1]["source"].as<std::string>());
}
