#include "hcl/hcl.hpp"

#include "thirdparty/catch2/catch.hpp"
#include <map>
#include <istream>
#include <sstream>
#include <string>

using namespace hcl;
using namespace hcl::internal;

struct TokenPair {
    TokenType tok;
    std::string text;
};

static std::string f100(100, 'f');

std::map<std::string, std::vector<TokenPair>> tokenPairs = {
    {"operator", {
		{TokenType::LBRACK, "["},
		{TokenType::LBRACE, "{"},
		{TokenType::COMMA, ","},
		{TokenType::PERIOD, "."},
		{TokenType::RBRACK, "]"},
		{TokenType::RBRACE, "}"},
		{TokenType::ASSIGN, "="},
		{TokenType::ADD, "+"},
		{TokenType::SUB, "-"},
	}},
    {"bool", {
		{TokenType::BOOL, "true"},
		{TokenType::BOOL, "false"},
	}},
    {"ident", {
		{TokenType::IDENT, "a"},
		{TokenType::IDENT, "a0"},
		{TokenType::IDENT, "foobar"},
		{TokenType::IDENT, "foo-bar"},
		{TokenType::IDENT, "foo.bar"},
		{TokenType::IDENT, "abc123"},
		{TokenType::IDENT, "LGTM"},
		{TokenType::IDENT, "_"},
		{TokenType::IDENT, "_abc123"},
		{TokenType::IDENT, "abc123_"},
		{TokenType::IDENT, "_abc_123_"},
		{TokenType::IDENT, "_äöü"},
		{TokenType::IDENT, "_本"},
		//{TokenType::IDENT, "äöü"},
		//{TokenType::IDENT, "本"},
		{TokenType::IDENT, "a۰۱۸"},
		{TokenType::IDENT, "foo६४"},
		{TokenType::IDENT, "bar９８７６"},
	}},
    {"heredoc", {
		{TokenType::HEREDOC, "<<EOF\nhello\nworld\nEOF"},
		{TokenType::HEREDOC, "<<EOF123\nhello\nworld\nEOF123"},
	}},
    {"string", {
		{TokenType::STRING, R"(" ")"},
		{TokenType::STRING, R"("a")"},
		{TokenType::STRING, R"("本")"},
		{TokenType::STRING, R"("${file("foo")}")"},
		{TokenType::STRING, R"("${file(\"foo\")}")"},
		//{TokenType::STRING, R"("\a")"},
		//{TokenType::STRING, R"("\b")"},
		//{TokenType::STRING, R"("\f")"},
		{TokenType::STRING, R"("\n")"},
		{TokenType::STRING, R"("\r")"},
		{TokenType::STRING, R"("\t")"},
		//{TokenType::STRING, R"("\v")"},
		{TokenType::STRING, R"("\"")"},
		//{TokenType::STRING, R"("\000")"},
		//{TokenType::STRING, R"("\777")"},
		{TokenType::STRING, R"("\x00")"},
		{TokenType::STRING, R"("\xff")"},
		{TokenType::STRING, R"("\u0000")"},
		{TokenType::STRING, R"("\ufA16")"},
		{TokenType::STRING, R"("\U00000000")"},
		{TokenType::STRING, R"("\U0000ffAB")"},
		{TokenType::STRING, "\"" + f100 + "\""},
	}},
    {"number", {
		{TokenType::NUMBER, "0"},
		{TokenType::NUMBER, "1"},
		{TokenType::NUMBER, "9"},
		{TokenType::NUMBER, "42"},
		{TokenType::NUMBER, "1234567890"},
		{TokenType::NUMBER, "00"},
		{TokenType::NUMBER, "01"},
		{TokenType::NUMBER, "07"},
		{TokenType::NUMBER, "042"},
		{TokenType::NUMBER, "01234567"},
		{TokenType::NUMBER, "0x0"},
		{TokenType::NUMBER, "0x1"},
		{TokenType::NUMBER, "0xf"},
		{TokenType::NUMBER, "0x42"},
		{TokenType::NUMBER, "0x123456789abcDEF"},
		{TokenType::NUMBER, "0x" + f100},
		{TokenType::NUMBER, "0X0"},
		{TokenType::NUMBER, "0X1"},
		{TokenType::NUMBER, "0XF"},
		{TokenType::NUMBER, "0X42"},
		{TokenType::NUMBER, "0X123456789abcDEF"},
		{TokenType::NUMBER, "0X" + f100},
		{TokenType::NUMBER, "-0"},
		{TokenType::NUMBER, "-1"},
		{TokenType::NUMBER, "-9"},
		{TokenType::NUMBER, "-42"},
		{TokenType::NUMBER, "-1234567890"},
		{TokenType::NUMBER, "-00"},
		{TokenType::NUMBER, "-01"},
		{TokenType::NUMBER, "-07"},
		{TokenType::NUMBER, "-29"},
		{TokenType::NUMBER, "-042"},
		{TokenType::NUMBER, "-01234567"},
		{TokenType::NUMBER, "-0x0"},
		{TokenType::NUMBER, "-0x1"},
		{TokenType::NUMBER, "-0xf"},
		{TokenType::NUMBER, "-0x42"},
		{TokenType::NUMBER, "-0x123456789abcDEF"},
		{TokenType::NUMBER, "-0x" + f100},
		{TokenType::NUMBER, "-0X0"},
		{TokenType::NUMBER, "-0X1"},
		{TokenType::NUMBER, "-0XF"},
		{TokenType::NUMBER, "-0X42"},
		{TokenType::NUMBER, "-0X123456789abcDEF"},
		{TokenType::NUMBER, "-0X" + f100},
	}},
    {"float", {
		{TokenType::FLOAT, "0."},
		{TokenType::FLOAT, "1."},
		{TokenType::FLOAT, "42."},
		{TokenType::FLOAT, "01234567890."},
		{TokenType::FLOAT, ".0"},
		{TokenType::FLOAT, ".1"},
		{TokenType::FLOAT, ".42"},
		{TokenType::FLOAT, ".0123456789"},
		{TokenType::FLOAT, "0.0"},
		{TokenType::FLOAT, "1.0"},
		{TokenType::FLOAT, "42.0"},
		{TokenType::FLOAT, "01234567890.0"},
		{TokenType::FLOAT, "0e0"},
		{TokenType::FLOAT, "1e0"},
		{TokenType::FLOAT, "42e0"},
		{TokenType::FLOAT, "01234567890e0"},
		{TokenType::FLOAT, "0E0"},
		{TokenType::FLOAT, "1E0"},
		{TokenType::FLOAT, "42E0"},
		{TokenType::FLOAT, "01234567890E0"},
		{TokenType::FLOAT, "0e+10"},
		{TokenType::FLOAT, "1e-10"},
		{TokenType::FLOAT, "42e+10"},
		{TokenType::FLOAT, "01234567890e-10"},
		{TokenType::FLOAT, "0E+10"},
		{TokenType::FLOAT, "1E-10"},
		{TokenType::FLOAT, "42E+10"},
		{TokenType::FLOAT, "01234567890E-10"},
		{TokenType::FLOAT, "01.8e0"},
		{TokenType::FLOAT, "1.4e0"},
		{TokenType::FLOAT, "42.2e0"},
		{TokenType::FLOAT, "01234567890.12e0"},
		{TokenType::FLOAT, "0.E0"},
		{TokenType::FLOAT, "1.12E0"},
		{TokenType::FLOAT, "42.123E0"},
		{TokenType::FLOAT, "01234567890.213E0"},
		{TokenType::FLOAT, "0.2e+10"},
		{TokenType::FLOAT, "1.2e-10"},
		{TokenType::FLOAT, "42.54e+10"},
		{TokenType::FLOAT, "01234567890.98e-10"},
		{TokenType::FLOAT, "0.1E+10"},
		{TokenType::FLOAT, "1.1E-10"},
		{TokenType::FLOAT, "42.1E+10"},
		{TokenType::FLOAT, "01234567890.1E-10"},
		{TokenType::FLOAT, "-0.0"},
		{TokenType::FLOAT, "-1.0"},
		{TokenType::FLOAT, "-42.0"},
		{TokenType::FLOAT, "-01234567890.0"},
		{TokenType::FLOAT, "-0e0"},
		{TokenType::FLOAT, "-1e0"},
		{TokenType::FLOAT, "-42e0"},
		{TokenType::FLOAT, "-01234567890e0"},
		{TokenType::FLOAT, "-0E0"},
		{TokenType::FLOAT, "-1E0"},
		{TokenType::FLOAT, "-42E0"},
		{TokenType::FLOAT, "-01234567890E0"},
		{TokenType::FLOAT, "-0e+10"},
		{TokenType::FLOAT, "-1e-10"},
		{TokenType::FLOAT, "-42e+10"},
		{TokenType::FLOAT, "-01234567890e-10"},
		{TokenType::FLOAT, "-0E+10"},
		{TokenType::FLOAT, "-1E-10"},
		{TokenType::FLOAT, "-42E+10"},
		{TokenType::FLOAT, "-01234567890E-10"},
		{TokenType::FLOAT, "-01.8e0"},
		{TokenType::FLOAT, "-1.4e0"},
		{TokenType::FLOAT, "-42.2e0"},
		{TokenType::FLOAT, "-01234567890.12e0"},
		{TokenType::FLOAT, "-0.E0"},
		{TokenType::FLOAT, "-1.12E0"},
		{TokenType::FLOAT, "-42.123E0"},
		{TokenType::FLOAT, "-01234567890.213E0"},
		{TokenType::FLOAT, "-0.2e+10"},
		{TokenType::FLOAT, "-1.2e-10"},
		{TokenType::FLOAT, "-42.54e+10"},
		{TokenType::FLOAT, "-01234567890.98e-10"},
		{TokenType::FLOAT, "-0.1E+10"},
		{TokenType::FLOAT, "-1.1E-10"},
		{TokenType::FLOAT, "-42.1E+10"},
		{TokenType::FLOAT, "-01234567890.1E-10"},
        }}
};

void testTokenList(const std::string& name)
{
    for(const auto& tokenPair : tokenPairs.at(name))
    {
        SECTION(tokenPair.text)
        {
            std::stringstream ss(tokenPair.text);
            Lexer lexer(ss);

            Token t = lexer.nextToken();

            REQUIRE(t.type() == tokenPair.tok);
        }
    }
}

TEST_CASE("test operator")
{
    testTokenList("operator");
}

TEST_CASE("test bool")
{
    testTokenList("bool");
}

TEST_CASE("test ident")
{
    testTokenList("ident");
}

TEST_CASE("test heredoc")
{
    testTokenList("heredoc");
}

TEST_CASE("test string")
{
    testTokenList("string");
}

TEST_CASE("test number")
{
    testTokenList("number");
}

TEST_CASE("test float")
{
    testTokenList("float");
}

TEST_CASE("test real world example")
{
    std::stringstream ss(R"(# This comes from Terraform, as a test
	variable "foo" {
	    default = "bar"
	    description = "bar"
	}

	provider "aws" {
	  access_key = "foo"
	  secret_key = "${replace(var.foo, ".", "\\.")}"
	}

	resource "aws_security_group" "firewall" {
	    count = 5
	}

	resource aws_instance "web" {
	    ami = "${var.foo}"
	    security_groups = [
	        "foo",
	        "${aws_security_group.firewall.foo}"
	    ]

	    network_interface {
	        device_index = 0
	        description = <<EOF
Main interface
EOF
	    }

		network_interface {
	        device_index = 1
	        description = <<-EOF
			Outer text
				Indented text
			EOF
		}
	})");
    Lexer lexer(ss);
    static std::vector<TokenPair> literals = {
        //{TokenType::COMMENT, )"// This comes from Terraform, as a test)"},
		{TokenType::IDENT, R"(variable)"},
		{TokenType::STRING, R"("foo")"},
		{TokenType::LBRACE, R"({)"},
		{TokenType::IDENT, R"(default)"},
		{TokenType::ASSIGN, R"(=)"},
		{TokenType::STRING, R"("bar")"},
		{TokenType::IDENT, R"(description)"},
		{TokenType::ASSIGN, R"(=)"},
		{TokenType::STRING, R"("bar")"},
		{TokenType::RBRACE, R"(})"},
		{TokenType::IDENT, R"(provider)"},
		{TokenType::STRING, R"("aws")"},
		{TokenType::LBRACE, R"({)"},
		{TokenType::IDENT, R"(access_key)"},
		{TokenType::ASSIGN, R"(=)"},
		{TokenType::STRING, R"("foo")"},
		{TokenType::IDENT, R"(secret_key)"},
		{TokenType::ASSIGN, R"(=)"},
		{TokenType::STRING, R"("${replace(var.foo, ".", "\\.")}")"},
		{TokenType::RBRACE, R"(})"},
		{TokenType::IDENT, R"(resource)"},
		{TokenType::STRING, R"("aws_security_group")"},
		{TokenType::STRING, R"("firewall")"},
		{TokenType::LBRACE, R"({)"},
		{TokenType::IDENT, R"(count)"},
		{TokenType::ASSIGN, R"(=)"},
		{TokenType::NUMBER, R"(5)"},
		{TokenType::RBRACE, R"(})"},
		{TokenType::IDENT, R"(resource)"},
		{TokenType::IDENT, R"(aws_instance)"},
		{TokenType::STRING, R"("web")"},
		{TokenType::LBRACE, R"({)"},
		{TokenType::IDENT, R"(ami)"},
		{TokenType::ASSIGN, R"(=)"},
		{TokenType::STRING, R"("${var.foo}")"},
		{TokenType::IDENT, R"(security_groups)"},
		{TokenType::ASSIGN, R"(=)"},
		{TokenType::LBRACK, R"([)"},
		{TokenType::STRING, R"("foo")"},
		{TokenType::COMMA, R"(,)"},
		{TokenType::STRING, R"("${aws_security_group.firewall.foo}")"},
		{TokenType::RBRACK, R"(])"},
		{TokenType::IDENT, R"(network_interface)"},
		{TokenType::LBRACE, R"({)"},
		{TokenType::IDENT, R"(device_index)"},
		{TokenType::ASSIGN, R"(=)"},
		{TokenType::NUMBER, R"(0)"},
		{TokenType::IDENT, R"(description)"},
		{TokenType::ASSIGN, R"(=)"},
		{TokenType::HEREDOC, "<<EOF\nMain interface\nEOF\n"},
		{TokenType::RBRACE, R"(})"},
		{TokenType::IDENT, R"(network_interface)"},
		{TokenType::LBRACE, R"({)"},
		{TokenType::IDENT, R"(device_index)"},
		{TokenType::ASSIGN, R"(=)"},
		{TokenType::NUMBER, R"(1)"},
		{TokenType::IDENT, R"(description)"},
		{TokenType::ASSIGN, R"(=)"},
		{TokenType::HEREDOC, "<<-EOF\n\t\t\tOuter text\n\t\t\t\tIndented text\n\t\t\tEOF\n"},
		{TokenType::RBRACE, R"(})"},
		{TokenType::RBRACE, R"(})"},
		{TokenType::END_OF_FILE, R"()"},
    };

    int i = 0;
    for(const auto& tokenPair : literals)
    {
        std::cout << i << ": \"" << tokenPair.text << "\" " << static_cast<int>(tokenPair.tok) << std::endl;
        Token t = lexer.nextToken();

        REQUIRE(t.type() == tokenPair.tok);
        i++;
    }
}
