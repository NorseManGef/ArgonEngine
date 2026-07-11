
#include "ArgonAppFlowControl.h"
#include "plog/Severity.h"
#include <ArgonEngine/ArgonInit.h>
#include <SDL3/SDL.h>
#include <ArgonEngine/Log.h>
#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <cstdio>
ArgonAppFlowControl* controller;
void manual_redraw(){
    if(controller->draw())Argon::swap_buffers();
}
int main(int argc, char** argv){
    Argon::parse_args(argc, argv);
    Argon::initialize_engine("ArgonSoftware","ArgonApp");

    PLOGN << "None test!";
    PLOGF << "Fatal Error test!";
    PLOGE << "Error test!";
    PLOGW << "Warning test!";
    PLOGI << "Info test!";
    PLOGD << "debug test!";
    PLOGV << "verbose test!";

    int run =1;
    Argon::set_manual_redraw(manual_redraw);
    controller=new ArgonAppFlowControl();
    while(run){
        run = Argon::poll_events();
        controller->draw();
        Argon::swap_buffers();
    }
    Argon::terminate_engine();
    delete controller;
    return 0;
}
