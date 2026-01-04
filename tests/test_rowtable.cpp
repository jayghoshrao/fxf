#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <filesystem>
#include "RowTable.hpp"

TEST_CASE("RowTable basic operations", "[rowtable]") {
    RowTable table;

    SECTION("AddLine adds parsed row") {
        table.AddLine("a|b|c", '|');
        REQUIRE(table.data.size() == 1);
        REQUIRE(table.data[0].size() == 3);
        CHECK(table.data[0][0] == "a");
        CHECK(table.data[0][1] == "b");
        CHECK(table.data[0][2] == "c");
    }

    SECTION("multiple AddLine calls") {
        table.AddLine("a|b", '|');
        table.AddLine("c|d", '|');
        REQUIRE(table.data.size() == 2);
    }

    SECTION("operator[] access") {
        table.AddLine("x|y|z", '|');
        CHECK(table[0][0] == "x");
        CHECK(table[0][1] == "y");
        CHECK(table[0][2] == "z");
    }
}

TEST_CASE("RowTable GetRow", "[rowtable]") {
    RowTable table;
    table.AddLine("a|b|c", '|');
    table.AddLine("d|e|f", '|');

    SECTION("valid index") {
        auto row = table.GetRow(0);
        REQUIRE(row.size() == 3);
        CHECK(row[0] == "a");
    }

    SECTION("invalid index returns empty") {
        auto row = table.GetRow(999);
        CHECK(row.empty());
    }
}

TEST_CASE("RowTable GetJoinedRow", "[rowtable]") {
    RowTable table;
    table.AddLine("a|b|c", '|');

    SECTION("default separator") {
        CHECK(table.GetJoinedRow(0) == "a b c");
    }

    SECTION("custom separator") {
        CHECK(table.GetJoinedRow(0, ", ") == "a, b, c");
    }

    SECTION("invalid index returns empty") {
        CHECK(table.GetJoinedRow(999) == "");
    }
}

TEST_CASE("RowTable Erase", "[rowtable]") {
    RowTable table;
    table.AddLine("a|b", '|');
    table.AddLine("c|d", '|');
    table.AddLine("e|f", '|');

    SECTION("erase middle") {
        table.Erase(1);
        REQUIRE(table.data.size() == 2);
        CHECK(table.data[0][0] == "a");
        CHECK(table.data[1][0] == "e");
    }

    SECTION("erase first") {
        table.Erase(0);
        REQUIRE(table.data.size() == 2);
        CHECK(table.data[0][0] == "c");
    }

    SECTION("erase invalid index does nothing") {
        table.Erase(999);
        CHECK(table.data.size() == 3);
    }
}

TEST_CASE("RowTable GetMenuEntries", "[rowtable]") {
    RowTable table;
    table.AddLine("apple|red", '|');
    table.AddLine("banana|yellow", '|');

    SECTION("all columns template") {
        auto entries = table.GetMenuEntries("{}");
        REQUIRE(entries.size() == 2);
        CHECK(entries[0] == "apple | red");
        CHECK(entries[1] == "banana | yellow");
    }

    SECTION("specific column template") {
        auto entries = table.GetMenuEntries("{0}");
        REQUIRE(entries.size() == 2);
        CHECK(entries[0] == "apple");
        CHECK(entries[1] == "banana");
    }

    SECTION("custom template") {
        auto entries = table.GetMenuEntries("{0} is {1}");
        REQUIRE(entries.size() == 2);
        CHECK(entries[0] == "apple is red");
        CHECK(entries[1] == "banana is yellow");
    }
}

TEST_CASE("RowTable Substitute", "[rowtable]") {
    RowTable table;
    table.AddLine("foo|bar|baz", '|');

    SECTION("substitute with template") {
        CHECK(table.Substitute("{0}-{1}", 0) == "foo-bar");
    }

    SECTION("invalid index returns empty") {
        CHECK(table.Substitute("{0}", 999) == "");
    }
}

TEST_CASE("RowTable Load", "[rowtable]") {
    RowTable table;
    std::string testFile = "/tmp/fxf_test_file.txt";

    SECTION("load existing file") {
        {
            std::ofstream out(testFile);
            out << "a|b|c\n";
            out << "d|e|f\n";
        }

        auto result = table.Load(testFile, '|');
        CHECK(result.has_value());
        REQUIRE(table.data.size() == 2);
        CHECK(table.data[0][0] == "a");
        CHECK(table.data[1][0] == "d");

        std::filesystem::remove(testFile);
    }

    SECTION("load nonexistent file returns error") {
        auto result = table.Load("/nonexistent/path/file.txt", '|');
        CHECK(!result.has_value());
        CHECK(result.error().find("Failed to open") != std::string::npos);
    }

    SECTION("load clears existing data") {
        table.AddLine("old|data", '|');

        {
            std::ofstream out(testFile);
            out << "new|data\n";
        }

        table.Load(testFile, '|');
        REQUIRE(table.data.size() == 1);
        CHECK(table.data[0][0] == "new");

        std::filesystem::remove(testFile);
    }
}
