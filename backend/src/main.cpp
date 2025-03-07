#include <string>
#include <cstdio>
#include "crow.h"
#include "lib.h"

int main()
{
    crow::SimpleApp app; // define your crow application

    // define your endpoint at the root directory
    CROW_ROUTE(app, "/")([]()
                         { 
                            std::ostringstream oss;
    oss << "Hello world, foo is " << foo() << "!";
    return oss.str(); });

    // set the port, set the app to run on multiple threads, and run the app
    app.port(8080).concurrency(4).run();
}