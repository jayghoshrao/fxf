#include <catch2/catch_test_macros.hpp>
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
