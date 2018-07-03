#include "hcl/hcl.hpp"

#include "thirdparty/catch2/catch.hpp"
#include <map>
#include <istream>
#include <sstream>
#include <string>

const std::string fixtureDir = "tests/test-fixtures/parser";

template <typename Map>
bool map_compare (Map const &lhs, Map const &rhs) {
    return lhs.size() == rhs.size()
        && std::equal(lhs.begin(), lhs.end(),
                      rhs.begin());
}

static hcl::Value parse(const std::string& s)
{
    std::stringstream ss(s);
    hcl::internal::Parser p(ss);

    hcl::Value v = p.parse();
    if (p.errorReason().size() != 0) {
        std::cerr << s << std::endl;
        std::cerr << p.errorReason() << std::endl;
    }
    REQUIRE(v.valid());
    return v;
}

static bool parseFails(const std::string& s)
{
    std::stringstream ss(s);
    hcl::internal::Parser p(ss);

    hcl::Value v = p.parse();
    return !v.valid();
}

static bool parseFileFails(const std::string& filename)
{
    std::ifstream is(fixtureDir + "/" + filename);
    REQUIRE(is);
    hcl::internal::Parser p(is);

    hcl::Value v = p.parse();
    return !v.valid();
}

TEST_CASE("parse empty")
{
    hcl::Value v = parse("");

    REQUIRE(v.is<hcl::Object>());
    REQUIRE(0UL == v.size());
}

TEST_CASE("parse comments")
{
    hcl::Value v = parse(
        "# hogehoge\n"
        "# fuga hoge\n");

    REQUIRE(v.is<hcl::Object>());
    REQUIRE(0UL == v.size());
}

TEST_CASE("parse comment and empty line")
{
    hcl::Value v = parse(
        "# hogehoge\n"
        "# fuga hoge\n"
        "\n"
        "\n"
        "# piyo piyo\n");

    REQUIRE(v.is<hcl::Object>());
    REQUIRE(0UL == v.size());
}

TEST_CASE("parse bool")
{
    hcl::Value v = parse(
        "x = true\n"
        "y = false\n");

    REQUIRE(v.get<bool>("x"));
    REQUIRE_FALSE(v.get<bool>("y"));
}

TEST_CASE("parse int")
{
    hcl::Value v = parse(
        "x = 1\n"
        "y = 0\n"
        "z = -1\n");

    REQUIRE(1 == v.get<int>("x"));
    REQUIRE(0 == v.get<int>("y"));
    REQUIRE(-1 == v.get<int>("z"));
}

TEST_CASE("parse float")
{
    hcl::Value v = parse(
        "x = 1.0\n"
        "y = .5\n"
        "z = -124.12\n"
        "w = -0.524\n");

    REQUIRE(1.0 == v.get<double>("x"));
    REQUIRE(0.5 == v.get<double>("y"));
    REQUIRE(-124.12 == v.get<double>("z"));
    REQUIRE(-0.524 == v.get<double>("w"));
}

TEST_CASE("parse empty double quoted string")
{
    hcl::Value v = parse(
        "x = \"\"\n");

    REQUIRE("" == v.get<std::string>("x"));
}

TEST_CASE("parse double quoted string")
{
    hcl::Value v = parse(
        "x = \"hoge\"\n"
        "y = \"hoge \\\"fuga\\\" hoge\"\n"
        "z = \"\\u003F\\U0000003F\"");

    REQUIRE("hoge" == v.get<std::string>("x"));
    REQUIRE("hoge \"fuga\" hoge" == v.get<std::string>("y"));
    REQUIRE("??" == v.get<std::string>("z"));
}

TEST_CASE("parse halfwidth katakana string")
{
    hcl::Value v = parse(
        "x = \"ｴｰﾃﾙ病\"");

    REQUIRE("ｴｰﾃﾙ病" == v.get<std::string>("x"));
}

TEST_CASE("parse ident")
{
    hcl::Value v = parse(
        "x = hoge\n"
        "y = hoge.fuga\n"
        "z = _000.hoge::fuga-piyo");

    REQUIRE("hoge" == v.get<std::string>("x"));
    REQUIRE("hoge.fuga" == v.get<std::string>("y"));
    REQUIRE("_000.hoge::fuga-piyo" == v.get<std::string>("z"));

    REQUIRE(v["x"].isIdent());
    REQUIRE(v["y"].isIdent());
    REQUIRE(v["z"].isIdent());
}

TEST_CASE("parse HIL")
{
    hcl::Value v = parse(
        "x = \"${hoge}\"\n"
        "y = \"${hoge {\\\"fuga\\\"} hoge}\"\n"
        "z = \"${name(hoge)}\"");

    REQUIRE("${hoge}" == v.get<std::string>("x"));
    REQUIRE("${hoge {\"fuga\"} hoge}" == v.get<std::string>("y"));
    REQUIRE("${name(hoge)}" == v.get<std::string>("z"));

    REQUIRE(v["x"].isHil());
    REQUIRE(v["y"].isHil());
    REQUIRE(v["z"].isHil());
}

TEST_CASE("fail parsing invalid HIL")
{
    REQUIRE(parseFails("x = ${hoge}"));
    REQUIRE(parseFails("x = \"${{hoge}\""));
    REQUIRE(parseFails("x = \"${{hoge}\"\n"));
}

TEST_CASE("parse heredoc")
{
    hcl::Value v = parse(
        "hoge = <<EOF\nHello\nWorld\nEOF\n"
        "fuga = <<FOO123\n\thoge\n\tfuga\nFOO123\n"
        "piyo = <<-EOF\n\t\t\tOuter text\n\t\t\t\tIndented text\n\t\t\tEOF\n"
        );
    REQUIRE("Hello\nWorld\n" == v.get<std::string>("hoge"));
    REQUIRE("\thoge\n\tfuga\n" == v.get<std::string>("fuga"));
    REQUIRE("Outer text\n\tIndented text\n" == v.get<std::string>("piyo"));
}


TEST_CASE("parse indented heredoc")
{
    hcl::Value v = parse(
        "hoge = <<-EOF\n    Hello\n      World\n    EOF\n");
    REQUIRE("Hello\n  World\n" == v.get<std::string>("hoge"));
}

TEST_CASE("parse indented heredoc with hanging indent")
{
    hcl::Value v = parse(
        "hoge = <<-EOF\n    Hello\n  World\n             EOF\n");
    REQUIRE("    Hello\n  World\n" == v.get<std::string>("hoge"));
}

TEST_CASE("parse empty single quoted string")
{
    hcl::Value v = parse(
        "x = ''\n");
    REQUIRE("" == v.get<std::string>("x"));
}

TEST_CASE("parse single quoted string")
{
    hcl::Value v = parse(
        "x = 'foo bar \"foo bar\"'\n");

    REQUIRE("foo bar \"foo bar\"" == v.get<std::string>("x"));
}

TEST_CASE("parse list")
{
    hcl::Value v = parse(
        "x = [1, 2, 3]\n"
        "y = []\n"
        "z = [\"\", \"\", ]\n"
        "w = [1, \"string\", <<EOF\nheredoc contents\nEOF]");

    const hcl::Value& x = v.get<hcl::List>("x");
    REQUIRE(3UL == x.size());
    REQUIRE(1 == x.get<int>(0));
    REQUIRE(2 == x.get<int>(1));
    REQUIRE(3 == x.get<int>(2));

    const hcl::Value& y = v.get<hcl::List>("y");
    REQUIRE(0UL == y.size());

    const hcl::Value& z = v.get<hcl::List>("z");
    REQUIRE(2UL == z.size());
    REQUIRE("" == z.get<std::string>(0));
    REQUIRE("" == z.get<std::string>(1));

    const hcl::Value& w = v.get<hcl::List>("w");
    REQUIRE(3UL == w.size());
    REQUIRE(1 == w.get<int>(0));
    REQUIRE("string" == w.get<std::string>(1));
    REQUIRE("heredoc contents\n" == w.get<std::string>(2));
}

TEST_CASE("fail parsing invalid list")
{
    REQUIRE(parseFails("w = 1, \"string\", <<EOF\nheredoc contents\nEOF"));
}

TEST_CASE("parse list of maps")
{
    hcl::Value v = parse(R"(
foo = [
  {key = "hoge"},
  {key = "fuga", key2 = "piyo"},
]
)");

    const hcl::Value& foo = v.get<hcl::List>("foo");
    REQUIRE(2UL == foo.size());

    const hcl::Value& first = foo.get<hcl::Object>(0);
    REQUIRE(1UL == first.size());
    REQUIRE("hoge" == first.get<std::string>("key"));

    const hcl::Value& second = foo.get<hcl::Object>(1);
    REQUIRE(2UL == second.size());
    REQUIRE("fuga" == second.get<std::string>("key"));
    REQUIRE("piyo" == second.get<std::string>("key2"));
}

TEST_CASE("parse leading comment in list")
{
    hcl::Value v = parse(R"(foo = [
1,
# bar
2,
3,
],
)");

    const hcl::Value& foo = v.get<hcl::List>("foo");
    REQUIRE(3UL == foo.size());
    REQUIRE(1 == foo.get<int>(0));
    REQUIRE(2 == foo.get<int>(1));
    REQUIRE(3 == foo.get<int>(2));
}

TEST_CASE("parse comment in list")
{
    hcl::Value v = parse(R"(foo = [
1,
2, # bar
3,
],
)");

    const hcl::Value& foo = v.get<hcl::List>("foo");
    REQUIRE(3UL == foo.size());
    REQUIRE(1 == foo.get<int>(0));
    REQUIRE(2 == foo.get<int>(1));
    REQUIRE(3 == foo.get<int>(2));
}

TEST_CASE("parse empty object type")
{
    hcl::Value v = parse(R"(
foo = {}
)");

    const hcl::Value& foo = v.get<hcl::Object>("foo");
    REQUIRE(0UL == foo.size());

}

TEST_CASE("parse simple object type")
{
    hcl::Value v = parse(R"(
foo = {
    bar = "hoge"
}
)");

    const hcl::Value& foo = v.get<hcl::Object>("foo");
    REQUIRE(1UL == foo.size());
    REQUIRE("hoge" == foo.get<std::string>("bar"));
}

TEST_CASE("parse object type with two fields")
{
    hcl::Value v = parse(R"(
foo = {
    bar = "hoge"
    baz = ["piyo"]
}
)");

    const hcl::Value& foo = v.get<hcl::Object>("foo");
    REQUIRE(2UL == foo.size());
    REQUIRE("hoge" == foo.get<std::string>("bar"));

    const hcl::Value& baz = foo.get<hcl::List>("baz");
    REQUIRE(1UL == baz.size());
    REQUIRE("piyo" == baz.get<std::string>(0));
}

TEST_CASE("parse object type with nested empty map")
{
    hcl::Value v = parse(R"(
foo = {
    bar = {}
}
)");

    const hcl::Value& foo = v.get<hcl::Object>("foo");
    REQUIRE(1UL == foo.size());

    const hcl::Value& bar = foo.get<hcl::Object>("bar");
    REQUIRE(0UL == bar.size());
}

TEST_CASE("parse object type with nested empty map and value")
{
    hcl::Value v = parse(R"(
foo = {
    bar = {}
    foo = true
}
)");

    const hcl::Value& foo = v.get<hcl::Object>("foo");
    REQUIRE(2UL == foo.size());

    const hcl::Value& bar = foo.get<hcl::Object>("bar");
    REQUIRE(0UL == bar.size());

    REQUIRE(true == foo.get<bool>("foo"));
}

TEST_CASE("parse object keys")
{
    std::vector<std::string> valid = {
		R"(foo {})",
		R"(foo = {})",
		R"(foo = bar)",
		R"(foo = 123)",
		R"(foo = "${var.bar}")",
		R"("foo" {})",
		R"("foo" = {})",
		R"("foo" = "${var.bar}")",
		R"(foo bar {})",
		R"(foo "bar" {})",
		R"("foo" bar {})",
		R"(foo bar baz {})"
    };

    for (const auto& i : valid) {
        SECTION(i)
        {
            REQUIRE_FALSE(parseFails(i));
        }
    }
}

TEST_CASE("fail parsing invalid keys")
{
    std::vector<std::string> invalid = {
        "foo 12 {}",
        "foo bar = {}",
        "foo []",
        "12 {}"
    };

    for (const auto& i : invalid) {
        SECTION(i)
        {
            REQUIRE(parseFails(i));
        }
    }
}

TEST_CASE("parse nested keys")
{
    hcl::Value v = parse(R"(foo "bar" baz { hoge = "piyo" })");

    const hcl::Value& foo = v.get<hcl::Object>("foo");
    const hcl::Value& bar = foo.get<hcl::Object>("bar");
    const hcl::Value& baz = bar.get<hcl::Object>("baz");
    REQUIRE("piyo" == baz.get<std::string>("hoge"));
}

TEST_CASE("parse multiple same nested keys")
{
    hcl::Value v = parse(R"(
foo bar { hoge = "piyo", hogera = "fugera" }
foo bar { hoge = "fuge" }
foo bar { hoge = "baz" }
)");

    const hcl::Value& foo = v.get<hcl::List>("foo");

    const hcl::Value& a = foo.get<hcl::Object>(0);
    const hcl::Value& b = foo.get<hcl::Object>(1);
    const hcl::Value& c = foo.get<hcl::Object>(2);

    const hcl::Value& barA = a.get<hcl::Object>("bar");
    REQUIRE("piyo" == barA.get<std::string>("hoge"));
    REQUIRE("fugera" == barA.get<std::string>("hogera"));

    const hcl::Value& barB = b.get<hcl::Object>("bar");
    REQUIRE("fuge" == barB.get<std::string>("hoge"));

    const hcl::Value& barC = c.get<hcl::Object>("bar");
    REQUIRE("baz" == barC.get<std::string>("hoge"));
}

TEST_CASE("parse multiple nested keys")
{
    hcl::Value v = parse(R"(
foo "bar" baz { hoge = "piyo" }
foo "bar" { hoge = "piyo" }
foo { hoge = "piyo" }
foo hogera { hoge = "piyo" }
)");

    const hcl::Value& foo = v.get<hcl::List>("foo");

    const hcl::Value& a = foo.get<hcl::Object>(0);
    const hcl::Value& b = foo.get<hcl::Object>(1);
    const hcl::Value& c = foo.get<hcl::Object>(2);
    const hcl::Value& d = foo.get<hcl::Object>(3);

    const hcl::Value& barA = a.get<hcl::Object>("bar");
    const hcl::Value& bazA = barA.get<hcl::Object>("baz");
    REQUIRE("piyo" == bazA.get<std::string>("hoge"));

    const hcl::Value& barB = b.get<hcl::Object>("bar");
    REQUIRE("piyo" == barB.get<std::string>("hoge"));

    REQUIRE("piyo" == c.get<std::string>("hoge"));

    const hcl::Value& hogeraD = d.get<hcl::Object>("hogera");
    REQUIRE("piyo" == hogeraD.get<std::string>("hoge"));
}

TEST_CASE("parse nested assignment to string and ident")
{
    hcl::Value v = parse(R"(
foo "bar" baz { "hoge" = fuge }
"foo" bar baz { hogera = "fugera" }
)");
    const hcl::Value& foo = v.get<hcl::List>("foo");

    const hcl::Value& a = foo.get<hcl::Object>(0);
    const hcl::Value& b = foo.get<hcl::Object>(1);

    const hcl::Value& barA = a.get<hcl::Object>("bar");
    const hcl::Value& bazA = barA.get<hcl::Object>("baz");
    REQUIRE("fuge" == bazA.get<std::string>("hoge"));

    const hcl::Value& barB = b.get<hcl::Object>("bar");
    const hcl::Value& bazB = barB.get<hcl::Object>("baz");
    REQUIRE("fugera" == bazB.get<std::string>("hogera"));
}

TEST_CASE("parse nested assignment with object")
{
    hcl::Value v = parse(R"(
foo = 6
foo "bar" { hoge = "piyo" }
)");

    const hcl::Value& foo = v.get<hcl::List>("foo");

    REQUIRE(6 == foo.get<int>(0));

    const hcl::Value& b = foo.get<hcl::Object>(1);
    const hcl::Value& bar = b.get<hcl::Object>("bar");
    REQUIRE("piyo" == bar.get<std::string>("hoge"));
}

TEST_CASE("parse comment group")
{
    std::vector<std::string> valid = {
        "# Hello\n# World",
        "# Hello\r\n# Windows",
    };

    for (const auto& i : valid) {
        REQUIRE_FALSE(parseFails(i));
    }
}

TEST_CASE("parse comment after line")
{
    hcl::Value v = parse(
        "x = 1 # hogehoge");

    REQUIRE(1 == v.get<int>("x"));
}

TEST_CASE("parse official HCL tests")
{
    std::vector<std::pair<std::string, bool>> files = {
        {
            "assign_colon.hcl",
            true,
        },
        // {
        //     "comment.hcl",
        //     false,
        // },
        // {
        //     "comment_crlf.hcl",
        //     false,
        // },
        {
            "comment_lastline.hcl",
            false,
        },
        {
            "comment_single.hcl",
            false,
        },
        {
            "empty.hcl",
            false,
        },
        {
            "list_comma.hcl",
            false,
        },
        {
            "multiple.hcl",
            false,
        },
        {
            "object_list_comma.hcl",
            false,
        },
        {
            "structure.hcl",
            false,
        },
        {
            "structure_basic.hcl",
            false,
        },
        {
            "structure_empty.hcl",
            false,
        },
        {
            "complex.hcl",
            false,
        },
        {
            "complex_crlf.hcl",
            false,
        },
        {
            "types.hcl",
            false,
        },
        {
            "array_comment.hcl",
            false,
        },
        {
            "array_comment_2.hcl",
            true,
        },
        {
            "missing_braces.hcl",
            true,
        },
        {
            "unterminated_object.hcl",
            true,
        },
        {
            "unterminated_object_2.hcl",
            true,
        },
        {
            "key_without_value.hcl",
            true,
        },
        {
            "object_key_without_value.hcl",
            true,
        },
        {
            "object_key_assign_without_value.hcl",
            true,
        },
        {
            "object_key_assign_without_value2.hcl",
            true,
        },
        {
            "object_key_assign_without_value3.hcl",
            true,
        },
        {
            "git_crypt.hcl",
            true,
        }
    };

    for (const auto& pair : files) {
        const std::string filename = pair.first;
        SECTION(filename)
        {
            bool shouldFail = pair.second;
            REQUIRE(parseFileFails(filename) == shouldFail);
        }
    }
}
