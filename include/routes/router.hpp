#pragma once

#include <string>

#include "crow/mustache.h"
#include "crow/app.h"

class Router {
    public:
        Router();
        ~Router();
        void route(crow::SimpleApp &app);

    private:
        std::string header_;
        std::string title_;

    private:
        crow::mustache::rendered_template loadHomePage();
};
