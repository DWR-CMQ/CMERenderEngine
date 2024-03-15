#include "App.h"

int main(int argc, char** argv)
{
    //auto app = Cme::App::Instance();
    Cme::App app;
    app.Init(false);
    app.Run();

    return 0;
}
