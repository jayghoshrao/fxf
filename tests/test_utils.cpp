#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include "utils.hpp"

TEST_CASE("trim removes whitespace", "[utils]") {
    SECTION("leading whitespace") {
        CHECK(trim("  hello") == "hello");
        CHECK(trim("\t\nhello") == "hello");
    }

    SECTION("trailing whitespace") {
        CHECK(trim("hello  ") == "hello");
        CHECK(trim("hello\t\n") == "hello");
    }

    SECTION("both sides") {
        CHECK(trim("  hello  ") == "hello");
        CHECK(trim("\t hello \n") == "hello");
    }

    SECTION("no whitespace") {
        CHECK(trim("hello") == "hello");
    }

    SECTION("all whitespace") {
        CHECK(trim("   ") == "");
        CHECK(trim("\t\n") == "");
    }

    SECTION("empty string") {
        CHECK(trim("") == "");
    }
}

TEST_CASE("split_csv_line splits correctly", "[utils]") {
    SECTION("simple split") {
        auto result = split_csv_line("a,b,c", ',');
        REQUIRE(result.size() == 3);
        CHECK(result[0] == "a");
        CHECK(result[1] == "b");
        CHECK(result[2] == "c");
    }

    SECTION("custom delimiter") {
        auto result = split_csv_line("a|b|c", '|');
        REQUIRE(result.size() == 3);
        CHECK(result[0] == "a");
        CHECK(result[1] == "b");
        CHECK(result[2] == "c");
    }

    SECTION("empty fields") {
        auto result = split_csv_line("a,,c", ',');
        REQUIRE(result.size() == 3);
        CHECK(result[0] == "a");
        CHECK(result[1] == "");
        CHECK(result[2] == "c");
    }

    SECTION("single field") {
        auto result = split_csv_line("hello", ',');
        REQUIRE(result.size() == 1);
        CHECK(result[0] == "hello");
    }
}

TEST_CASE("substitute_template replaces placeholders", "[utils]") {
    std::vector<std::string> data = {"apple", "banana", "cherry"};

    SECTION("numbered placeholders") {
        CHECK(substitute_template("{0}", data) == "apple");
        CHECK(substitute_template("{1}", data) == "banana");
        CHECK(substitute_template("{2}", data) == "cherry");
    }

    SECTION("multiple placeholders") {
        CHECK(substitute_template("{0} and {1}", data) == "apple and banana");
    }

    SECTION("repeated placeholders") {
        CHECK(substitute_template("{0} {0}", data) == "apple apple");
    }

    SECTION("all columns placeholder") {
        CHECK(substitute_template("{}", data) == "apple | banana | cherry");
    }

    SECTION("mixed placeholders") {
        CHECK(substitute_template("{} - {0}", data) == "apple | banana | cherry - apple");
    }

    SECTION("no placeholders") {
        CHECK(substitute_template("hello world", data) == "hello world");
    }

    SECTION("invalid index ignored") {
        CHECK(substitute_template("{99}", data) == "{99}");
    }
}

TEST_CASE("SplitCommand parses command strings", "[utils]") {
    SECTION("simple command") {
        auto result = SplitCommand("echo hello world");
        REQUIRE(result.size() == 3);
        CHECK(result[0] == "echo");
        CHECK(result[1] == "hello");
        CHECK(result[2] == "world");
    }

    SECTION("double quoted argument") {
        auto result = SplitCommand("echo \"hello world\"");
        REQUIRE(result.size() == 2);
        CHECK(result[0] == "echo");
        CHECK(result[1] == "hello world");
    }

    SECTION("single quoted argument") {
        auto result = SplitCommand("echo 'hello world'");
        REQUIRE(result.size() == 2);
        CHECK(result[0] == "echo");
        CHECK(result[1] == "hello world");
    }

    SECTION("escaped space") {
        auto result = SplitCommand("echo hello\\ world");
        REQUIRE(result.size() == 2);
        CHECK(result[0] == "echo");
        CHECK(result[1] == "hello world");
    }

    SECTION("mixed quotes") {
        auto result = SplitCommand("cmd 'single' \"double\"");
        REQUIRE(result.size() == 3);
        CHECK(result[0] == "cmd");
        CHECK(result[1] == "single");
        CHECK(result[2] == "double");
    }

    SECTION("empty string") {
        auto result = SplitCommand("");
        CHECK(result.empty());
    }

    SECTION("multiple spaces") {
        auto result = SplitCommand("echo    hello");
        REQUIRE(result.size() == 2);
        CHECK(result[0] == "echo");
        CHECK(result[1] == "hello");
    }
}

TEST_CASE("ExtractFirstURL finds URLs", "[utils]") {
    SECTION("simple URL") {
        CHECK(ExtractFirstURL("visit https://example.com today") == "https://example.com");
    }

    SECTION("URL with path") {
        CHECK(ExtractFirstURL("check https://example.com/path/to/page") == "https://example.com/path/to/page");
    }

    SECTION("URL with query") {
        CHECK(ExtractFirstURL("link: https://example.com?foo=bar") == "https://example.com?foo=bar");
    }

    SECTION("http URL") {
        CHECK(ExtractFirstURL("old http://example.com site") == "http://example.com");
    }

    SECTION("no URL") {
        CHECK(ExtractFirstURL("no url here") == "");
    }

    SECTION("multiple URLs returns first") {
        CHECK(ExtractFirstURL("first https://a.com second https://b.com") == "https://a.com");
    }
}

TEST_CASE("ExtractURLs finds all URLs", "[utils]") {
    SECTION("multiple URLs") {
        auto urls = ExtractURLs("visit https://a.com and https://b.com");
        REQUIRE(urls.size() == 2);
        CHECK(urls[0] == "https://a.com");
        CHECK(urls[1] == "https://b.com");
    }

    SECTION("no URLs") {
        auto urls = ExtractURLs("no urls here");
        CHECK(urls.empty());
    }
}

TEST_CASE("substitute_template_opt replaces placeholders", "[utils]") {
    std::vector<std::string> data = {"apple", "banana", "cherry"};

    SECTION("numbered placeholders") {
        CHECK(substitute_template_opt("{0}", data) == "apple");
        CHECK(substitute_template_opt("{1}", data) == "banana");
        CHECK(substitute_template_opt("{2}", data) == "cherry");
    }

    SECTION("multiple placeholders") {
        CHECK(substitute_template_opt("{0} and {1}", data) == "apple and banana");
    }

    SECTION("repeated placeholders") {
        CHECK(substitute_template_opt("{0} {0}", data) == "apple apple");
    }

    SECTION("all columns placeholder") {
        CHECK(substitute_template_opt("{}", data) == "apple | banana | cherry");
    }

    SECTION("mixed placeholders") {
        CHECK(substitute_template_opt("{} - {0}", data) == "apple | banana | cherry - apple");
    }

    SECTION("no placeholders") {
        CHECK(substitute_template_opt("hello world", data) == "hello world");
    }

    SECTION("invalid index kept as-is") {
        CHECK(substitute_template_opt("{99}", data) == "{99}");
    }

    SECTION("empty data") {
        std::vector<std::string> empty;
        CHECK(substitute_template_opt("{}", empty) == "");
        CHECK(substitute_template_opt("{0}", empty) == "{0}");
    }

    SECTION("single element data") {
        std::vector<std::string> single = {"only"};
        CHECK(substitute_template_opt("{}", single) == "only");
        CHECK(substitute_template_opt("{0}", single) == "only");
    }

    SECTION("non-numeric placeholder kept as-is") {
        CHECK(substitute_template_opt("{abc}", data) == "{abc}");
        CHECK(substitute_template_opt("{0a}", data) == "{0a}");
    }

    SECTION("unclosed brace") {
        CHECK(substitute_template_opt("{0 text", data) == "{0 text");
        CHECK(substitute_template_opt("{ no close", data) == "{ no close");
    }

    SECTION("double-digit index") {
        std::vector<std::string> large(15, "x");
        large[10] = "ten";
        large[14] = "fourteen";
        CHECK(substitute_template_opt("{10}", large) == "ten");
        CHECK(substitute_template_opt("{14}", large) == "fourteen");
    }
}

TEST_CASE("substitute_template and substitute_template_opt produce same results", "[utils]") {
    std::vector<std::string> data = {"apple", "banana", "cherry", "date"};

    std::vector<std::string> templates = {
        "{}",
        "{0}",
        "{1}",
        "{2}",
        "{3}",
        "{0} and {1}",
        "{0} {0} {0}",
        "{} - {0} - {1}",
        "no placeholders",
        "{99}",
        "prefix {0} middle {1} suffix",
        "{}{}{}"
    };

    for (const auto& tmpl : templates) {
        INFO("Template: " << tmpl);
        CHECK(substitute_template(tmpl, data) == substitute_template_opt(tmpl, data));
    }
}

TEST_CASE("substitute_template performance", "[.benchmark]") {
    std::vector<std::string> small_data = {"apple", "banana", "cherry"};
    std::vector<std::string> large_data;
    for (int i = 0; i < 100; ++i) {
        large_data.push_back("item_" + std::to_string(i));
    }

    BENCHMARK("original - simple {0}") {
        return substitute_template("{0}", small_data);
    };

    BENCHMARK("optimized - simple {0}") {
        return substitute_template_opt("{0}", small_data);
    };

    BENCHMARK("original - all columns {}") {
        return substitute_template("{}", small_data);
    };

    BENCHMARK("optimized - all columns {}") {
        return substitute_template_opt("{}", small_data);
    };

    BENCHMARK("original - multiple placeholders") {
        return substitute_template("{0} - {1} - {2}", small_data);
    };

    BENCHMARK("optimized - multiple placeholders") {
        return substitute_template_opt("{0} - {1} - {2}", small_data);
    };

    BENCHMARK("original - mixed {} and {N}") {
        return substitute_template("{} | {0} | {1} | {2}", small_data);
    };

    BENCHMARK("optimized - mixed {} and {N}") {
        return substitute_template_opt("{} | {0} | {1} | {2}", small_data);
    };

    BENCHMARK("original - large data {}") {
        return substitute_template("{}", large_data);
    };

    BENCHMARK("optimized - large data {}") {
        return substitute_template_opt("{}", large_data);
    };

    BENCHMARK("original - large data {50}") {
        return substitute_template("{50}", large_data);
    };

    BENCHMARK("optimized - large data {50}") {
        return substitute_template_opt("{50}", large_data);
    };

    BENCHMARK("original - no placeholders") {
        return substitute_template("static text without any placeholders", small_data);
    };

    BENCHMARK("optimized - no placeholders") {
        return substitute_template_opt("static text without any placeholders", small_data);
    };
}
