#include <filesystem>
#include <cstdlib>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <map>

#include "crow/app.h"

#include "builder.hpp"

namespace fs = std::filesystem;

using std::string;
using std::system;
using std::thread;
using std::map;

namespace utils {
    ///////////////////////////////////////////////////////////////////////////
    // constructors
    ///////////////////////////////////////////////////////////////////////////
    Builder::Builder() {
        dirs_to_watch_ = {"templates", "static"};
    }

    ///////////////////////////////////////////////////////////////////////////
    // destructor
    ///////////////////////////////////////////////////////////////////////////
    Builder::~Builder() {
    }

    ///////////////////////////////////////////////////////////////////////////
    // public member functions
    ///////////////////////////////////////////////////////////////////////////
    void Builder::convertTsFiles() {
        fs::path js_dir = "static/js";
        fs::path tsdir = "static/ts";
    
        if (!fs::exists(js_dir)) {
            try {
                fs::create_directories(js_dir);
                CROW_LOG_INFO << "Created output directory: "
                              << js_dir.string();
            } catch (const fs::filesystem_error &e) {
                CROW_LOG_ERROR << "Error creating output directory "
                               << js_dir.string() << ": " << e.what();
            }
        }
    
        CROW_LOG_INFO << "Starting TypeScript transpilation...";
        bool all_succesfull = true;
    
        try {
            for (const auto &entry : fs::recursive_directory_iterator(tsdir)) {
                if (entry.is_regular_file()) {
                    fs::path input_file_path = entry.path();
                    if (input_file_path.extension() == ".ts") {
                        fs::path output_file_path = js_dir / input_file_path
                            .stem().string().append(".js");
    
                        string compile = "tsc --target ES6 "
                            + input_file_path.string()
                            + " --outFile "
                            + output_file_path.string()
                            + " --module system";
    
                        CROW_LOG_INFO << "Transpiling: "
                                      << input_file_path.filename().string();
                        CROW_LOG_INFO << "Command: " << compile;
    
                        int result = system(compile.c_str());
    
                        if (result != 0) {
                            CROW_LOG_ERROR << "Failed to transpile "
                                           << input_file_path.filename()
                                                .string()
                                           << ". Exit code: " << result;
    
                            all_succesfull = false;
                        }
                    }
                }
            }
        } catch (const fs::filesystem_error &e) {
            CROW_LOG_ERROR << "Error accessing TypeScript directory "
                           << tsdir.string() << ": " << e.what();
        }
    
        if (all_succesfull) {
            CROW_LOG_INFO << "TypeScript transpilation successful!";
        } else {
            CROW_LOG_ERROR << "TypeScript transpilation failed";
            CROW_LOG_ERROR
                << "Please ensure TypeScript (tsc) is installed globally";
            CROW_LOG_ERROR
                << "You might need to install it: npm install -g typescript";
        }
    }

    void Builder::startFileWatcher() {
        thread file_watcher_thread(&Builder::startFileWatcherInternal, this);

        file_watcher_thread.detach();
    }

    void Builder::enableHotReloading(crow::SimpleApp &app) {
        CROW_WEBSOCKET_ROUTE(app, "/ws")
            .onopen([&](crow::websocket::connection &conn) {
                CROW_LOG_INFO << "New websocket connection";
                std::lock_guard<std::mutex> _(mtx_);
                users_.insert(&conn);
            })
            .onclose([&](
                crow::websocket::connection &conn,
                const string &reason,
                unsigned short code
            ) {
                CROW_LOG_INFO << reason;
                CROW_LOG_INFO << "Websocket connection closed " << code;
                std::lock_guard<std::mutex> _(mtx_);
                users_.erase(&conn);
            })
            .onmessage([&](
                crow::websocket::connection &/*conn*/,
                const string &data,
                bool is_binary
            ) {
                CROW_LOG_INFO << data;
                CROW_LOG_INFO << is_binary;
            });
    }

    void Builder::openBrowser(const string &url) {
        #ifdef _WIN32
            string command = "start " + url;
        #elif __APPLE__
            string command = "open " + url;
        #else
            string command = "xdg-open " + url;
        #endif
            system(command.c_str());
    }

    ///////////////////////////////////////////////////////////////////////////
    // private member functions
    ///////////////////////////////////////////////////////////////////////////
    void Builder::populate_file_map(
        const fs::path &dir_path,
        map<fs::path, fs::file_time_type> &file_map
    ) {
        if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
            return;
        }
    
        for (const auto &entry : fs::recursive_directory_iterator(dir_path)) {
            if (fs::is_regular_file(entry.status())) {
                if (entry.path().string().starts_with("static/js")) {
                    continue;
                }
                file_map[entry.path()] = fs::last_write_time(entry.path());
            }
        }
    }

    void Builder::startFileWatcherInternal() {
        for (const auto &dir : dirs_to_watch_) {
            populate_file_map(dir, watched_files_);
        }
    
        CROW_LOG_INFO << "Watching "
                      << watched_files_.size()
                      << " files for changes";
    
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            bool file_changed = false;
            bool ts_file_changed = false;
    
            for (auto &pair : watched_files_) {
                try {
                    auto current_write_time = fs::last_write_time(pair.first);
                    if (current_write_time != pair.second) {
                        CROW_LOG_INFO << "File changed! " << pair.first;
                        if (pair.first.string().starts_with("static/ts")) {
                            ts_file_changed = true;
                        }
                        pair.second = current_write_time;
                        file_changed = true;
                    }
                } catch (const fs::filesystem_error &e) {
                    CROW_LOG_ERROR << "Filesystem error: " << e.what();
                }
            }
    
            if (file_changed) {
                if (ts_file_changed) {
                    convertTsFiles();
                }
                std::lock_guard<std::mutex> _(mtx_);
                for (auto &user : users_) {
                    user->send_text("reload");
                }
            }
        }
    }
}
