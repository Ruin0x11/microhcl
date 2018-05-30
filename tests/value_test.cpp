#include "hcl/hcl.hpp"

#include "thirdparty/catch2/catch.hpp"
#include <map>
#include <iostream>
#include <istream>
#include <sstream>
#include <string>

bool map_compare(hcl::Object const &actual, hcl::Object const &expected) {
    bool result =  actual.size() == expected.size()
        && std::equal(actual.begin(), actual.end(),
                      expected.begin());
    if(!result) {
        std::cout << " ===== Expected ===== " << std::endl;
        std::cout << expected << std::endl;
        std::cout << " =====  Actual  ===== " << std::endl;
        std::cout << actual << std::endl;
        std::cout << " ==================== " << std::endl;
    }
    return result;
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

TEST_CASE("boolean")
{
    const hcl::Value t(true);
    const hcl::Value f(false);

    REQUIRE(t.is<bool>());
    REQUIRE(f.is<bool>());

    REQUIRE(t.as<bool>());
    REQUIRE_FALSE(f.as<bool>());

    hcl::Value v;
    v = true;
    REQUIRE(v.is<bool>());
    REQUIRE(v.as<bool>());

    v = false;
    REQUIRE(v.is<bool>());
    REQUIRE_FALSE(v.as<bool>());

    hcl::Value x = t;
    REQUIRE(x.is<bool>());
    REQUIRE(x.as<bool>());
}

TEST_CASE("int")
{
    const hcl::Value zero(0);
    const hcl::Value one(1);
    const hcl::Value mone(-1);

    REQUIRE(zero.is<int>());
    REQUIRE(one.is<int>());
    REQUIRE(mone.is<int>());

    REQUIRE(0 == zero.as<int>());
    REQUIRE(1 == one.as<int>());
    REQUIRE(-1 == mone.as<int>());

    hcl::Value v;
    v = 100;
    REQUIRE(v.is<int>());
    REQUIRE(100 == v.as<int>());
}

TEST_CASE("int64_t")
{
    const hcl::Value zero(static_cast<int64_t>(0));
    const hcl::Value one(static_cast<int64_t>(1));
    const hcl::Value mone(static_cast<int64_t>(-1));

    REQUIRE(zero.is<int64_t>());
    REQUIRE(one.is<int64_t>());
    REQUIRE(mone.is<int64_t>());

    // We accept is<int>() also.
    REQUIRE(zero.is<int>());
    REQUIRE(one.is<int>());
    REQUIRE(mone.is<int>());

    REQUIRE(0 == zero.as<int64_t>());
    REQUIRE(1 == one.as<int64_t>());
    REQUIRE(-1 == mone.as<int64_t>());

    hcl::Value v;
    v = static_cast<int64_t>(100);
    REQUIRE(v.is<int64_t>());
    REQUIRE(100 == v.as<int64_t>());
}

TEST_CASE("double")
{
    const hcl::Value zero(0.0);
    const hcl::Value one(1.0);
    const hcl::Value mone(-1.0);

    REQUIRE(zero.is<double>());
    REQUIRE(one.is<double>());
    REQUIRE(mone.is<double>());

    REQUIRE(0.0 == zero.as<double>());
    REQUIRE(1.0 == one.as<double>());
    REQUIRE(-1.0 == mone.as<double>());

    hcl::Value v;
    v = 100.0;
    REQUIRE(v.is<double>());
    REQUIRE(100.0 == v.as<double>());
}

TEST_CASE("doubleWrite")
{
    {
        const hcl::Value v(1.0);
        std::ostringstream ss;
        v.write(&ss);
        REQUIRE("1.000000" == ss.str());
    }

    {
        const hcl::Value v(10000000.0);
        std::ostringstream ss;
        v.write(&ss);
        REQUIRE("10000000.000000" == ss.str());
    }

    {
        const hcl::Value v(123456.789123);
        std::ostringstream ss;
        v.write(&ss);
        REQUIRE("123456.789123" == ss.str());
    }
}

TEST_CASE("string")
{
    hcl::Value v1(std::string("foo"));
    REQUIRE(v1.is<std::string>());
    REQUIRE("foo" == v1.as<std::string>());

    v1 = "test";
    REQUIRE(v1.is<std::string>());
    REQUIRE("test" == v1.as<std::string>());

    v1 = std::string("kotori");
    REQUIRE(v1.is<std::string>());
    REQUIRE("kotori" == v1.as<std::string>());

    hcl::Value v2("foo");
    REQUIRE(v2.is<std::string>());
    REQUIRE("foo" == v2.as<std::string>());

}

TEST_CASE("bool_array")
{
    hcl::Value v((hcl::List()));
    v.push(false);
    v.push(true);

    std::vector<bool> vs = v.as<std::vector<bool>>();
    REQUIRE(2U == vs.size());
    REQUIRE_FALSE(vs[0]);
    REQUIRE(vs[1]);

    REQUIRE(v.is<std::vector<bool>>());
    REQUIRE_FALSE(v.is<std::vector<int>>());
    REQUIRE_FALSE(v.is<std::vector<int64_t>>());
    REQUIRE_FALSE(v.is<std::vector<double>>());
    REQUIRE_FALSE(v.is<std::vector<std::string>>());
    REQUIRE_FALSE(v.is<std::vector<hcl::List>>());
    REQUIRE_FALSE(v.is<std::vector<hcl::Object>>());
}

TEST_CASE("int_array")
{
    hcl::Value v((hcl::List()));
    v.push(0);
    v.push(1);

    std::vector<int> vs = v.as<std::vector<int>>();
    REQUIRE(2U == vs.size());
    REQUIRE(0 == vs[0]);
    REQUIRE(1 == vs[1]);

    std::vector<int64_t> vs2 = v.as<std::vector<int64_t>>();
    REQUIRE(2U == vs2.size());
    REQUIRE(0 == vs2[0]);
    REQUIRE(1 == vs2[1]);

    REQUIRE_FALSE(v.is<std::vector<bool>>());
    REQUIRE(v.is<std::vector<int>>());
    REQUIRE(v.is<std::vector<int64_t>>());
    REQUIRE_FALSE(v.is<std::vector<double>>());
    REQUIRE_FALSE(v.is<std::vector<std::string>>());
    REQUIRE_FALSE(v.is<std::vector<hcl::List>>());
    REQUIRE_FALSE(v.is<std::vector<hcl::Object>>());
}

TEST_CASE("double_array")
{
    hcl::Value v((hcl::List()));
    v.push(0.0);
    v.push(1.0);

    std::vector<double> vs = v.as<std::vector<double>>();
    REQUIRE(2U == vs.size());
    REQUIRE(0.0 == vs[0]);
    REQUIRE(1.0 == vs[1]);

    REQUIRE_FALSE(v.is<std::vector<bool>>());
    REQUIRE_FALSE(v.is<std::vector<int>>());
    REQUIRE_FALSE(v.is<std::vector<int64_t>>());
    REQUIRE(v.is<std::vector<double>>());
    REQUIRE_FALSE(v.is<std::vector<std::string>>());
    REQUIRE_FALSE(v.is<std::vector<hcl::List>>());
    REQUIRE_FALSE(v.is<std::vector<hcl::Object>>());
}

TEST_CASE("string_array")
{
    hcl::Value v((hcl::List()));
    v.push("foo");
    v.push("bar");

    std::vector<std::string> vs = v.as<std::vector<std::string>>();
    REQUIRE(2U == vs.size());
    REQUIRE("foo" == vs[0]);
    REQUIRE("bar" == vs[1]);

    REQUIRE_FALSE(v.is<std::vector<bool>>());
    REQUIRE_FALSE(v.is<std::vector<int>>());
    REQUIRE_FALSE(v.is<std::vector<int64_t>>());
    REQUIRE_FALSE(v.is<std::vector<double>>());
    REQUIRE(v.is<std::vector<std::string>>());
    REQUIRE_FALSE(v.is<std::vector<hcl::List>>());
    REQUIRE_FALSE(v.is<std::vector<hcl::Object>>());
}

TEST_CASE("array_array")
{
    hcl::Value v((hcl::List()));
    v.push(v);

    std::vector<hcl::List> vs = v.as<std::vector<hcl::List>>();
    REQUIRE(1U == vs.size());

    REQUIRE_FALSE(v.is<std::vector<bool>>());
    REQUIRE_FALSE(v.is<std::vector<int>>());
    REQUIRE_FALSE(v.is<std::vector<int64_t>>());
    REQUIRE_FALSE(v.is<std::vector<double>>());
    REQUIRE_FALSE(v.is<std::vector<std::string>>());
    REQUIRE(v.is<std::vector<hcl::List>>());
    REQUIRE_FALSE(v.is<std::vector<hcl::Object>>());
}

TEST_CASE("table_array")
{
    hcl::Value v((hcl::List()));
    v.push(hcl::Object());

    std::vector<hcl::Object> vs = v.as<std::vector<hcl::Object>>();
    REQUIRE(1U == vs.size());

    REQUIRE_FALSE(v.is<std::vector<bool>>());
    REQUIRE_FALSE(v.is<std::vector<int>>());
    REQUIRE_FALSE(v.is<std::vector<int64_t>>());
    REQUIRE_FALSE(v.is<std::vector<double>>());
    REQUIRE_FALSE(v.is<std::vector<std::string>>());
    REQUIRE_FALSE(v.is<std::vector<hcl::List>>());
    REQUIRE(v.is<std::vector<hcl::Object>>());
}

TEST_CASE("table")
{
    hcl::Value v;

    // If we call set() to null value, the value will become table automatically.
    v.set("key1", 1);
    v.set("key2", 2);

    REQUIRE(1 == v.get<int>("key1"));
    REQUIRE(2 == v.get<int>("key2"));
}

TEST_CASE("table2")
{
    hcl::Value v;

    v.set("key1.key2", 1);
    REQUIRE(1 == v.find("key1.key2")->as<int>());
}

TEST_CASE("table3")
{
    hcl::Value ary;
    ary.push(0);
    ary.push(1);
    ary.push(2);

    hcl::Value v;
    v.set("key", ary);

    std::vector<int> vs = v.get<std::vector<int>>("key");
    REQUIRE(3U == vs.size());
    REQUIRE(0 == vs[0]);
    REQUIRE(1 == vs[1]);
    REQUIRE(2 == vs[2]);
}

TEST_CASE("tableErase")
{
    hcl::Value v;
    v.set("key1.key2", 1);

    REQUIRE(v.erase("key1.key2"));
    REQUIRE(nullptr == v.find("key1.key2"));

    REQUIRE_FALSE(v.has("key1.key2"));
}

TEST_CASE("number")
{
    hcl::Value v(1);
    REQUIRE(v.isNumber());
    REQUIRE(1.0 == v.asNumber());

    v = 2.5;
    REQUIRE(v.isNumber());
    REQUIRE(2.5 == v.asNumber());

    v = false;
    REQUIRE_FALSE(v.isNumber());
}

TEST_CASE("tableFind")
{
    hcl::Value v;
    v.set("foo", 1);

    hcl::Value* x = v.find("foo");
    REQUIRE(x != nullptr);
    *x = 2;

    REQUIRE(2 == v.find("foo")->as<int>());
}

TEST_CASE("tableHas")
{
    hcl::Value v;
    v.set("foo", 1);
    REQUIRE(v.has("foo"));
    REQUIRE_FALSE(v.has("bar"));
}

TEST_CASE("merge")
{
    hcl::Value v1;
    hcl::Value v2;

    v1.set("foo.foo", 1);
    v1.set("foo.bar", 2);
    v1.set("bar", 3);

    v2.set("foo.bar", 4);
    v2.set("foo.baz", 5);
    v2.set("bar", 6);

    REQUIRE(v1.merge(v2));

    REQUIRE(6 == v1.get<int>("bar"));
    REQUIRE(1 == v1.get<int>("foo.foo"));
    REQUIRE(4 == v1.get<int>("foo.bar"));
    REQUIRE(5 == v1.get<int>("foo.baz"));
}

TEST_CASE("arrayFind")
{
    hcl::Value v;
    v.push(1);

    hcl::Value* x = v.find(0);
    REQUIRE(x != nullptr);
    *x = 2;

    REQUIRE(2 == v.find(0)->as<int>());
}

TEST_CASE("keyParsing")
{
    hcl::Value v;
    v.set("_0000.0000", 1);
    REQUIRE(1 == v.get<int>("_0000.0000"));
}

TEST_CASE("comparing")
{
    hcl::Value n1, n2;
    hcl::Value b1(true), b2(false), b3(true);
    hcl::Value i1(1), i2(2), i3(1);
    hcl::Value d1(1.0), d2(2.0), d3(1.0);
    hcl::Value s1("foo"), s2("bar"), s3("foo");
    hcl::Value a1((hcl::List())), a2((hcl::List())), a3((hcl::List()));
    a1.push(1);
    a2.push(2);
    a3.push(1);

    hcl::Value t1((hcl::Object())), t2((hcl::Object())), t3((hcl::Object()));
    t1.set("k1", "v1");
    t2.set("k2", "v2");
    t3.set("k1", "v1");

    REQUIRE(n1 == n2);
    REQUIRE(b1 == b3);
    REQUIRE(i1 == i3);
    REQUIRE(d1 == d3);
    REQUIRE(s1 == s3);
    REQUIRE(t1 == t3);

    REQUIRE(b1 != b2);
    REQUIRE(i1 != i2);
    REQUIRE(d1 != d2);
    REQUIRE(s1 != s2);
    REQUIRE(t1 != t2);

    REQUIRE(i1 != d1);
}

TEST_CASE("operatorBox")
{
    hcl::Value v;
    v["key"] = "value";
    v["foo.bar"] = "foobar";
    v.setChild("foo", "bar");

    REQUIRE("value" == v.findChild("key")->as<std::string>());
    REQUIRE("foobar" == v.findChild("foo.bar")->as<std::string>());
    REQUIRE("bar" == v["foo"].as<std::string>());
}

TEST_CASE("operatorBoxList")
{
    hcl::Value v;
    v.push("value");
    v.push("foobar");

    REQUIRE("value" == v.get<std::string>(0));
    REQUIRE("foobar" == v.get<std::string>(1));
    REQUIRE("value" == v[0].as<std::string>());
    REQUIRE("foobar" == v[1].as<std::string>());
    REQUIRE_THROWS(v[2]);
}

TEST_CASE("test shares key with")
{
    SECTION("non-object")
    {
        const hcl::Value a = hcl::Value(hcl::Object{
                {"foo", "bar"}
            });
        const hcl::Value b = false;
        REQUIRE_FALSE(a.sharesKeyWith(b));
        REQUIRE_FALSE(b.sharesKeyWith(a));
    }
    SECTION("no sharing")
    {
        const hcl::Value a = hcl::Value(hcl::Object{
                {"foo", "bar"}
            });
        const hcl::Value b = hcl::Value(hcl::Object{
                {"bar", "foo"}
            });

        REQUIRE_FALSE(a.sharesKeyWith(b));
        REQUIRE_FALSE(b.sharesKeyWith(a));
    }
    SECTION("same level")
    {
        const hcl::Value a = hcl::Value(hcl::Object{
                {"foo", "bar"}
            });
        const hcl::Value b = hcl::Value(hcl::Object{
                {"bar", "foo"},
                {"foo", "baz"},
            });

        REQUIRE(a.sharesKeyWith(b));
        REQUIRE(b.sharesKeyWith(a));
    }
    SECTION("no detection of nested")
    {
        const hcl::Value a = hcl::Value(hcl::Object{
                {"foo", hcl::Object{
                        {"baz", "hoge"}
                    }},
            });
        const hcl::Value b = hcl::Value(hcl::Object{
                {"bar", hcl::Object{
                        {"baz", "piyo"}
                    }},
            });

        REQUIRE_FALSE(a.sharesKeyWith(b));
        REQUIRE_FALSE(b.sharesKeyWith(a));
    }
}

TEST_CASE("test merge objects")
{
    SECTION("assign non-object to non-object")
    {
        hcl::Value v = hcl::Value(hcl::Object{
                {"foo", 42}
            });
        hcl::Value m = hcl::Value("bar");
        v.mergeObjects({"foo"}, m);

        const hcl::Value expected = hcl::Value(hcl::Object{
                {"foo", hcl::List{42, "bar"}}
            });

        REQUIRE(map_compare(v.as<hcl::Object>(), expected.as<hcl::Object>()));
    }
    SECTION("assign non-object to object")
    {
        hcl::Value v = hcl::Value(hcl::Object{
                {"foo", hcl::Object{
                        {"name", "putit"}
                    }}
            });
        hcl::Value m = 42;
        v.mergeObjects({"foo"}, m);

        const hcl::Value expected = hcl::Value(hcl::Object{
                {"foo", hcl::List{
                        hcl::Object{
                            {"name", "putit"}
                        },
                            42}
                }});

        REQUIRE(map_compare(v.as<hcl::Object>(), expected.as<hcl::Object>()));
    }
    SECTION("merge object with object")
    {
        hcl::Value v = hcl::Value(hcl::Object{
                {"foo", hcl::Object{
                        {"name", "putit"}
                    }}
            });
        hcl::Value m = hcl::Object{
            {"color", "white"},
            {"hp", 100}
        };
        v.mergeObjects({"foo"}, m);

        const hcl::Value expected = hcl::Value(hcl::Object{
                {"foo", hcl::Object{
                        {"name", "putit"},
                        {"color", "white"},
                        {"hp", 100}
                    }}
            });

        REQUIRE(map_compare(v.as<hcl::Object>(), expected.as<hcl::Object>()));
    }
    SECTION("expand non-objects into list")
    {
        hcl::Value v = hcl::Value(hcl::Object{
                {"foo", "bar"}
            });
        hcl::Value m = "baz";
        v.mergeObjects({"foo"}, m);

        const hcl::Value expected = hcl::Value(hcl::Object{
                {"foo", hcl::List{"bar", "baz"}}
            });

        REQUIRE(map_compare(v.as<hcl::Object>(), expected.as<hcl::Object>()));
    }
    SECTION("expand objects into list")
    {
        hcl::Value v = hcl::Value(hcl::Object{
                {"foo", hcl::Object{
                        {"name", "putit"}
                    }}
            });
        hcl::Value m = hcl::Object{
                {"name", "snail"}
        };
        v.mergeObjects({"foo"}, m);

        const hcl::Value expected = hcl::Value(hcl::Object{
                {"foo", hcl::List{
                        hcl::Object{
                            {"name", "putit"}
                        },
                        hcl::Object{
                            {"name", "snail"}
                        }
                    }}
            });

        REQUIRE(map_compare(v.as<hcl::Object>(), expected.as<hcl::Object>()));
    }
    SECTION("add non-object to list")
    {
        hcl::Value v = hcl::Value(hcl::Object{
                {"foo", hcl::List{"bar", "baz"}}
            });
        hcl::Value m = 42;
        v.mergeObjects({"foo"}, m);

        const hcl::Value expected = hcl::Value(hcl::Object{
                {"foo", hcl::List{"bar", "baz", 42}}
            });

        REQUIRE(map_compare(v.as<hcl::Object>(), expected.as<hcl::Object>()));
    }
    SECTION("add object to list")
    {
        hcl::Value v = hcl::Value(hcl::Object{
                {"foo", hcl::List{
                        hcl::Object{
                            {"name", "putit"}
                        }
                    }}
            });
        hcl::Value m = hcl::Object{
                {"color", "white"}
        };
        v.mergeObjects({"foo"}, m);

        const hcl::Value expected = hcl::Value(hcl::Object{
                {"foo", hcl::List{
                        hcl::Object{
                            {"name", "putit"}
                        },
                        hcl::Object{
                            {"color", "white"}
                        }
                    }}
            });

        REQUIRE(map_compare(v.as<hcl::Object>(), expected.as<hcl::Object>()));
    }
    SECTION("add list to list")
    {
        hcl::Value v = hcl::Value(hcl::Object{
                {"foo", hcl::List{"bar", "baz"}}
            });
        hcl::Value m = hcl::List{"hoge", "fuga"};
        v.mergeObjects({"foo"}, m);

        const hcl::Value expected = hcl::Value(hcl::Object{
                {"foo", hcl::List{
                        {"bar", "baz", hcl::List{"hoge", "fuga"}},
                    }}
            });

        REQUIRE(map_compare(v.as<hcl::Object>(), expected.as<hcl::Object>()));
    }
}

TEST_CASE("test merging of object lists")
{
    hcl::Value a = parse(R"(
chara putit { name = "putit" }
chara yeek  { name = "yeek"  }
)");
    hcl::Value b = parse(R"(
chara snail { name = "snail" }
chara shade { name = "shade" }
)");

    a.merge(b);
    const hcl::Value expected = hcl::Value(hcl::Object{
                {"chara", hcl::Object{
                        {"putit", hcl::Object{
                                {"name", "putit"}
                            }},
                        {"yeek", hcl::Object{
                                {"name", "yeek"}
                            }},
                        {"snail", hcl::Object{
                                {"name", "snail"}
                            }},
                        {"shade", hcl::Object{
                                {"name", "shade"}
                            }},
                    }
                }});
    REQUIRE(map_compare(a.as<hcl::Object>(), expected.as<hcl::Object>()));
}

TEST_CASE("test merging of object list and single object")
{
    hcl::Value a = parse(R"(
chara putit { name = "putit" }
)");
    hcl::Value b = parse(R"(
chara  { name = "foo" }
chara yeek  { name = "yeek"  }
)");

    a.merge(b);
    const hcl::Value expected = hcl::Value(hcl::Object{
            {"chara", hcl::Object{
                    {"name", "foo"},
                    {"putit", hcl::Object{
                            {"name", "putit"}
                        }},
                    {"yeek", hcl::Object{
                            {"name", "yeek"}
                        }},
                        }}
        });
    REQUIRE(map_compare(a.as<hcl::Object>(), expected.as<hcl::Object>()));
}


TEST_CASE("fail indexing non-list by index")
{
    hcl::Value v = 1;
    REQUIRE_THROWS(v[0]);
}

TEST_CASE("fail indexing non-object by string")
{
    hcl::Value v = 1;
    REQUIRE_THROWS(v["foo"]);
}
