#include <catch2/catch.hpp>
#include <filesystem>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;
using Filepath = std::filesystem::path;

std::string make_preferred(const std::string& path) {
    return Filepath(path).make_preferred().lexically_normal().string();
}

TEST_CASE("ImportResolving") {

    const std::string source1 = R"(
        import "game/classes/Player.wren" for Player
    )";

    const std::string source2 = R"(
        import "classes/Player.wren" for Player
    )";

    const std::string source3 = R"(
        import "classes/Player.wren" for Player
        import "game/classes/Player.wren" for Player
        import "game/../game/classes/Player.wren" for Player
    )";

    const std::string file1 = R"(
        import "../game/classes/Player.wren" for Player
    )";

    const std::string file2 = R"(
        import "Player.wren" for Player
    )";

    const std::string file3 = R"(
        class Player {
        }
    )";

    const std::pair<std::string, std::string> entry1 = {make_preferred("./game/MyGame.wren"), file1};
    const std::pair<std::string, std::string> entry2 = {make_preferred("./game/classes/Other.wren"), file2};
    const std::pair<std::string, std::string> entry3 = {make_preferred("./game/classes/Player.wren"), file3};

    const std::unordered_map<std::string, std::string> filesystem{entry1, entry2, entry3};

    wren::VM vm{{make_preferred("./"), make_preferred("./game")}};

    vm.setPathResolveFunc([&filesystem](const std::vector<std::string>& paths, const std::string& importer,
                                        const std::string& name) -> std::string {
        auto parent = Filepath(importer).parent_path();
        auto relative = (Filepath(parent) / Filepath(name)).lexically_normal().make_preferred().string();

        if (auto it = filesystem.find(relative); it != filesystem.end()) {
            return relative;
        }

        for (const auto& path : paths) {

            auto composed = (Filepath(path) / Filepath(name).lexically_normal().make_preferred()).string();
            if (auto it = filesystem.find(composed); it != filesystem.end()) {
                return composed;
            }
        }

        return make_preferred(name);
    });

    SECTION("Import from cwd") {

        size_t modules_loaded = 0;

        vm.setLoadFileFunc([&filesystem, &modules_loaded](const std::string& path) {
            if (auto it = filesystem.find(path); it != filesystem.end()) {
                modules_loaded += 1;
                return it->second;
            }
            throw wren::NotFound();
        });

        vm.runFromSource("User", source1);
        REQUIRE(modules_loaded == 1);
    }

    SECTION("Import from include dir") {

        size_t modules_loaded = 0;

        vm.setLoadFileFunc([&filesystem, &modules_loaded](const std::string& path) {
            if (auto it = filesystem.find(path); it != filesystem.end()) {
                modules_loaded += 1;
                return it->second;
            }
            throw wren::NotFound();
        });

        vm.runFromSource("User", source2);
        REQUIRE(modules_loaded == 1);
    }

    SECTION("Import from from script") {

        size_t modules_loaded = 0;

        vm.setLoadFileFunc([&filesystem, &modules_loaded](const std::string& path) {
            if (auto it = filesystem.find(path); it != filesystem.end()) {
                modules_loaded += 1;
                return it->second;
            }
            throw wren::NotFound();
        });

        vm.runFromModule("game/classes/Other.wren");
        vm.runFromModule("game/MyGame.wren");

        REQUIRE(modules_loaded == 3);
    }

    SECTION("Same Import Fail") {
        size_t modules_loaded = 0;

        vm.setLoadFileFunc([&filesystem, &modules_loaded](const std::string& path) {
            if (auto it = filesystem.find(path); it != filesystem.end()) {
                modules_loaded += 1;
                return it->second;
            }
            throw wren::NotFound();
        });

        try {
            vm.runFromSource("User", source3);
            REQUIRE(false); // This should fail since source3 imports the same script 3 times
        } catch (...) {
            REQUIRE(vm.getLastError() == "");
        }
    }
}