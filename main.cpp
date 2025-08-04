#include <string>

#include "crow/app.h"

#include "builder.hpp"
#include "router.hpp"

using std::string;

int main() {
    utils::Builder builder;
    builder.convertTsFiles();

    crow::SimpleApp app;

    builder.startFileWatcher();
    builder.enableHotReloading(app);

    Router router;
    router.route(app);

    builder.openBrowser("http://127.0.0.1:3000");
    auto a = app.port(3000).multithreaded().run_async();

    return 0;
}
