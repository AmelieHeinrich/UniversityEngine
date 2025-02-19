//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-03 12:23:11
//

#include "Runtime.hpp"

int main()
{
    ApplicationSpecs specs;
    specs.Width = 1920;
    specs.Height = 1080;
    specs.WindowTitle = "Game Demo";
    specs.ProjectPath = "TestGame.mpj";
    specs.CopyToBackBuffer = true;

    Runtime runtime(specs);
    runtime.Run();
}
