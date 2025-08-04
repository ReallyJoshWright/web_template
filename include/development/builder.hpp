#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <mutex>
#include <set>
#include <map>

#include "crow/app.h"

namespace utils {
    class Builder {
        public:
            Builder();
            ~Builder();
            void convertTsFiles();
            void startFileWatcher();
            void enableHotReloading(crow::SimpleApp &app);
            void openBrowser(const std::string &url);

        private:
            std::set<crow::websocket::connection*> users_;
            std::mutex mtx_;
            std::vector<std::string> dirs_to_watch_;
            std::map<std::filesystem::path, std::filesystem::file_time_type>
                watched_files_;

        private:
            void startFileWatcherInternal();
            void populate_file_map(
                const std::filesystem::path &dir_path,
                std::map<std::filesystem::path,
                         std::filesystem::file_time_type> &file_map
            );
    };
}
