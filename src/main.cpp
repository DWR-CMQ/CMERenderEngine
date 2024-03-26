#include "App.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(int argc, char** argv)
{
    //auto app = Cme::App::Instance();
    Cme::App app;
    app.Init(false);
    app.Run();

    return 0;
}
