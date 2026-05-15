#include "directory_walker.hpp"
#include "media_watcher.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

class TempDirFixture : public ::testing::Test {
protected:
  fs::path temp_dir = "./tmp";
  std::unordered_map<std::string, std::string> extension_category = {
      {".mp3", "audio"},   {".wav", "audio"},  {".mov", "video"},
      {".jpeg", "images"}, {".png", "images"},
  };

  void SetUp() override {
    fs::remove_all(temp_dir);
    fs::create_directories(temp_dir);
  }

  void TearDown() override { fs::remove_all(temp_dir); }

  void CreateFile(const fs::path &relative_path,
                  const std::string &content = "") {
    fs::path full_path = temp_dir / relative_path;
    fs::create_directories(full_path.parent_path());
    std::ofstream file(full_path);
    if (!content.empty()) {
      file << content;
    }
    file.close();
  }

  void DeleFiles() {}
};

TEST_F(TempDirFixture, DirectoryWalkerBasicFileDiscovery) {
  CreateFile("audio/song1.mp3", "MP3_DATA");
  CreateFile("audio/song2.wav", "WAV_DATA");
  CreateFile("video/movie.mov", "MOV_DATA");
  CreateFile("images/photo1.jpeg", "JPEG_DATA");
  CreateFile("images/photo2.png", "PNG_DATA");

  DirectoryWalker walker(temp_dir.string(), extension_category);
  auto result = walker.GetFilenamesDevidedByCategory();

  EXPECT_TRUE(result.contains("audio"));
  EXPECT_TRUE(result.contains("video"));
  EXPECT_TRUE(result.contains("images"));

  EXPECT_EQ(result["audio"].size(), 2);
  EXPECT_EQ(result["video"].size(), 1);
  EXPECT_EQ(result["images"].size(), 2);

  auto audio_files = result["audio"];
  EXPECT_TRUE(std::find(audio_files.begin(), audio_files.end(), "song1.mp3") !=
              audio_files.end());
  EXPECT_TRUE(std::find(audio_files.begin(), audio_files.end(), "song2.wav") !=
              audio_files.end());
}

TEST_F(TempDirFixture, DirectoryWalkerNestedDirectories) {
  CreateFile("audio/subfolder/deep/song1.mp3", "MP3_DATA");
  CreateFile("audio/subfolder/song2.wav", "WAV_DATA");
  CreateFile("video/subfolder/deep/movie.mov", "MOV_DATA");

  DirectoryWalker walker(temp_dir.string(), extension_category);
  auto result = walker.GetFilenamesDevidedByCategory();

  EXPECT_EQ(result["audio"].size(), 2);
  EXPECT_EQ(result["video"].size(), 1);
}

TEST_F(TempDirFixture, DirectoryWalkerNoOrphanedFiles) {
  CreateFile("audio/song1.mp3", "MP3_DATA");
  CreateFile("video/movie.mov", "MOV_DATA");
  CreateFile("other/random.txt", "TXT_DATA");

  DirectoryWalker walker(temp_dir.string(), extension_category);
  auto result = walker.GetFilenamesDevidedByCategory();

  for (const auto &[category, files] : result) {
    for (const auto &file : files) {
      EXPECT_NE(file, "random.txt");
    }
  }

  int total_files = 0;
  for (const auto &[category, files] : result)
    total_files += files.size();

  EXPECT_EQ(total_files, 2);
}

TEST_F(TempDirFixture, MediaWatcherCreatesResultFile) {
  CreateFile("audio/song1.mp3", "MP3_DATA");
  CreateFile("audio/song2.wav", "WAV_DATA");
  CreateFile("video/movie.mov", "MOV_DATA");

  fs::path old_cwd = fs::current_path();
  fs::current_path(temp_dir);

  MediaWatcher watcher(temp_dir.string(), extension_category, 450, false, true);
  auto original_cout_buffer = std::cout.rdbuf();
  {
    std::stringstream ss{};
    std::cout.rdbuf(ss.rdbuf());

    std::thread watcher_thread([&watcher]() { watcher.Start(); });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    watcher.Stop();
    watcher_thread.join();
    std::string s = ss.str();
    std::cout.rdbuf(original_cout_buffer);
  }

  fs::path result_file = "./result.json";
  EXPECT_TRUE(fs::exists(result_file)) << "result.json should be created";

  fs::current_path(old_cwd);
}

TEST_F(TempDirFixture, MediaWatcherResultJsonContainsAllFiles) {
  CreateFile("audio/song1.mp3", "MP3_DATA");
  CreateFile("audio/song2.wav", "WAV_DATA");
  CreateFile("video/movie.mov", "MOV_DATA");
  CreateFile("images/photo1.jpeg", "JPEG_DATA");

  DirectoryWalker walker(temp_dir.string(), extension_category);
  auto result = walker.GetFilenamesDevidedByCategory();

  EXPECT_TRUE(result.contains("audio"));
  EXPECT_TRUE(result.contains("video"));
  EXPECT_TRUE(result.contains("images"));

  EXPECT_EQ(result["audio"].size(), 2);
  EXPECT_EQ(result["video"].size(), 1);
  EXPECT_EQ(result["images"].size(), 1);

  json j = result;
  EXPECT_TRUE(j.contains("audio"));
  EXPECT_TRUE(j.contains("video"));
  EXPECT_TRUE(j.contains("images"));
  EXPECT_EQ(j["audio"].size(), 2);
  EXPECT_EQ(j["video"].size(), 1);
  EXPECT_EQ(j["images"].size(), 1);
}
