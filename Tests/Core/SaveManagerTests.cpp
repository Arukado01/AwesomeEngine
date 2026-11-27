#include "Managers/SaveManager.h"
#include "TestSupport.h"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <system_error>

namespace {
  namespace fs = std::filesystem;

  class TempWorkspace {
  private:
    fs::path original_;
    fs::path root_;

  public:
    TempWorkspace() : original_(fs::current_path()) {
      static std::uint64_t counter = 0;
      const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
      root_ = fs::temp_directory_path() /
              ("ae_save_tests_" + std::to_string(stamp) + "_" + std::to_string(++counter));
      fs::create_directories(root_ / "Content");
      fs::current_path(root_);
    }

    ~TempWorkspace() {
      std::error_code ec;
      fs::current_path(original_, ec);
      fs::remove_all(root_, ec);
    }

    fs::path Content() const { return root_ / "Content"; }
  };

  void WriteText(const fs::path &path, std::string_view text) {
    std::ofstream out(path, std::ios::binary);
    out << text;
    EXPECT_TRUE(out.good());
  }

  std::string ReadText(const fs::path &path) {
    std::ifstream in(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in),
                       std::istreambuf_iterator<char>());
  }

  nlohmann::json ReadJson(const fs::path &path) {
    return nlohmann::json::parse(ReadText(path));
  }
} // namespace

int main() {
  // 1. Sin archivo: objeto vacío; Set + destructor crea Save.json.
  {
    TempWorkspace workspace;
    {
      SaveManager saves;
      EXPECT_FALSE(saves.Has("score"));
      EXPECT_TRUE(saves.Get("score", 0) == 0);
      saves.Set("score", 42);
    }

    const auto json = ReadJson(workspace.Content() / "Save.json");
    EXPECT_TRUE(json.contains("score"));
    EXPECT_TRUE(json["score"].get<int>() == 42);
  }

  // 2. Archivo existente: conserva claves y permite modificarlas.
  {
    TempWorkspace workspace;
    WriteText(workspace.Content() / "Save.json",
              R"({"Runner:High Distance":164,"engine.display":"fullscreen"})");

    {
      SaveManager saves;
      EXPECT_TRUE(saves.Has("Runner:High Distance"));
      EXPECT_TRUE(saves.Get("Runner:High Distance", 0) == 164);
      EXPECT_TRUE(saves.Get("engine.display", std::string{}) == "fullscreen");

      saves.Set("Runner:High Distance", 200);
      saves.Set("Tetris:High Score", 5200);
    }

    const auto json = ReadJson(workspace.Content() / "Save.json");
    EXPECT_TRUE(json["Runner:High Distance"].get<int>() == 200);
    EXPECT_TRUE(json["Tetris:High Score"].get<int>() == 5200);
    EXPECT_TRUE(json["engine.display"].get<std::string>() == "fullscreen");
  }

  // 3. Borrar una clave entre varias conserva el resto.
  {
    TempWorkspace workspace;
    WriteText(workspace.Content() / "Save.json", R"({"a":1,"b":2})");

    {
      SaveManager saves;
      saves.Erase("a");
    }

    const auto json = ReadJson(workspace.Content() / "Save.json");
    EXPECT_FALSE(json.contains("a"));
    EXPECT_TRUE(json["b"].get<int>() == 2);
  }

  // 4. Caracterización del bug actual: borrar la última clave no persiste.
  //    F3 cambiará este contrato; entonces esta expectativa se actualizará.
  {
    TempWorkspace workspace;
    WriteText(workspace.Content() / "Save.json", R"({"only":1})");

    {
      SaveManager saves;
      saves.Erase("only");
    }

    const auto json = ReadJson(workspace.Content() / "Save.json");
    EXPECT_TRUE(json.contains("only"));
  }

  // 5. Caracterización del bug actual: Clear tampoco persiste si vacía todo.
  {
    TempWorkspace workspace;
    WriteText(workspace.Content() / "Save.json", R"({"a":1,"b":2})");

    {
      SaveManager saves;
      saves.Clear();
    }

    const auto json = ReadJson(workspace.Content() / "Save.json");
    EXPECT_TRUE(json.contains("a"));
    EXPECT_TRUE(json.contains("b"));
  }

  // 6. Caracterización: un JSON corrupto propaga la excepción de parseo.
  {
    TempWorkspace workspace;
    WriteText(workspace.Content() / "Save.json", "{ invalid");

    bool threw = false;
    try {
      SaveManager saves;
    } catch (const nlohmann::json::parse_error &) {
      threw = true;
    }
    EXPECT_TRUE(threw);
  }

  // 7. Sin archivo y sin claves: el destructor no crea Save.json.
  {
    TempWorkspace workspace;
    {
      SaveManager saves;
      EXPECT_FALSE(saves.Has("missing"));
    }

    EXPECT_FALSE(fs::exists(workspace.Content() / "Save.json"));
  }

  return TestSupport::Result();
}
