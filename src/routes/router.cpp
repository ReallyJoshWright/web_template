#include <string>

#include "crow/mustache.h"
#include "crow/app.h"

#include "router.hpp"

using std::string;

Router::Router() {
    header_ = "Hello";
    title_ = "Hello";
}

Router::~Router() {
}

void Router::route(crow::SimpleApp &app) {
    CROW_ROUTE(app, "/")([this]() {
        return loadHomePage();
    });
}

crow::mustache::rendered_template Router::loadHomePage() {
    auto page = crow::mustache::load("index.html");
    string home_page = crow::mustache::load("home.html").render_string();
    crow::mustache::context ctx({
        {"header", header_},
        {"title", title_},
        {"home_page", home_page}
    });
    
    return page.render(ctx);
}
