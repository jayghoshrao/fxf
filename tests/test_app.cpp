#include <catch2/catch_test_macros.hpp>
#include "app.hpp"

// Helper to reset App state between tests
static void ResetAppState() {
    auto& app = App::Instance();
    app.state.lines.data.clear();
    app.controls.filteredIndices.clear();
    app.controls.menuEntries.clear();
    app.controls.selections.clear();
    app.controls.selected = 0;
    app.controls.viewTemplate = "{}";
    // Ensure commands are registered (idempotent - won't double-register)
    app.commands.RegisterDefaultCommands();
}

TEST_CASE("Delete command adjusts selected index", "[app][delete]") {
    ResetAppState();
    auto& app = App::Instance();

    // Setup: 5 rows
    app.state.lines.AddLine("row0", '|');
    app.state.lines.AddLine("row1", '|');
    app.state.lines.AddLine("row2", '|');
    app.state.lines.AddLine("row3", '|');
    app.state.lines.AddLine("row4", '|');

    // Initialize filteredIndices to show all rows
    app.controls.filteredIndices = {0, 1, 2, 3, 4};
    app.ReapplyViewTemplate();

    SECTION("delete last item adjusts selected to new last") {
        app.controls.selected = 4;  // Last item
        REQUIRE(app.controls.filteredIndices.size() == 5);

        app.commands.Execute("delete");

        // After deletion: 4 items (indices 0-3)
        REQUIRE(app.controls.filteredIndices.size() == 4);
        // selected should be adjusted to 3 (new last valid index)
        CHECK(app.controls.selected == 3);
    }

    SECTION("delete middle item keeps selected if still valid") {
        app.controls.selected = 2;  // Middle item
        REQUIRE(app.controls.filteredIndices.size() == 5);

        app.commands.Execute("delete");

        // After deletion: 4 items
        REQUIRE(app.controls.filteredIndices.size() == 4);
        // selected=2 is still valid (now points to what was row3)
        CHECK(app.controls.selected == 2);
    }

    SECTION("delete first item keeps selected at 0") {
        app.controls.selected = 0;
        REQUIRE(app.controls.filteredIndices.size() == 5);

        app.commands.Execute("delete");

        REQUIRE(app.controls.filteredIndices.size() == 4);
        CHECK(app.controls.selected == 0);
    }

    SECTION("delete second-to-last keeps selected") {
        app.controls.selected = 3;  // Second to last
        REQUIRE(app.controls.filteredIndices.size() == 5);

        app.commands.Execute("delete");

        REQUIRE(app.controls.filteredIndices.size() == 4);
        // selected=3 is still valid (now points to what was row4)
        CHECK(app.controls.selected == 3);
    }
}

TEST_CASE("Delete all items results in selected=0", "[app][delete]") {
    ResetAppState();
    auto& app = App::Instance();

    app.state.lines.AddLine("only", '|');
    app.controls.filteredIndices = {0};
    app.ReapplyViewTemplate();
    app.controls.selected = 0;

    app.commands.Execute("delete");

    REQUIRE(app.controls.filteredIndices.empty());
    CHECK(app.controls.selected == 0);
}

TEST_CASE("Commands fail gracefully with empty filter", "[app][delete]") {
    ResetAppState();
    auto& app = App::Instance();

    // No data loaded, filteredIndices is empty
    app.controls.filteredIndices.clear();
    app.controls.selected = 0;

    SECTION("delete returns false with no items") {
        CHECK_FALSE(app.commands.Execute("delete"));
    }

    SECTION("select returns false with no items") {
        CHECK_FALSE(app.commands.Execute("select"));
    }

    SECTION("open returns false with no items") {
        CHECK_FALSE(app.commands.Execute("open"));
    }
}

TEST_CASE("GetOriginalIndex bounds checking", "[app]") {
    ResetAppState();
    auto& app = App::Instance();

    SECTION("empty filteredIndices returns nullopt") {
        app.controls.filteredIndices.clear();
        CHECK_FALSE(app.GetOriginalIndex(0).has_value());
        CHECK_FALSE(app.GetOriginalIndex(100).has_value());
    }

    SECTION("valid index returns correct mapping") {
        app.controls.filteredIndices = {5, 3, 1};  // Display order maps to original indices
        REQUIRE(app.GetOriginalIndex(0).has_value());
        CHECK(*app.GetOriginalIndex(0) == 5);
        REQUIRE(app.GetOriginalIndex(1).has_value());
        CHECK(*app.GetOriginalIndex(1) == 3);
        REQUIRE(app.GetOriginalIndex(2).has_value());
        CHECK(*app.GetOriginalIndex(2) == 1);
    }

    SECTION("out of bounds returns nullopt") {
        app.controls.filteredIndices = {5, 3, 1};
        CHECK_FALSE(app.GetOriginalIndex(3).has_value());
        CHECK_FALSE(app.GetOriginalIndex(100).has_value());
    }
}
