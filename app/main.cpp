
#include "ArgonAppFlowControl.h"
#include <ArgonEngine/ArgonInit.h>
#include <SDL3/SDL.h>
ArgonAppFlowControl* controller;
void manual_redraw(){
    if(controller->draw())Argon::swap_buffers();
}
int main(int argc, char** argv){
    Argon::initialize_engine("ArgonSoftware","ArgonApp");
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
