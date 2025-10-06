#include <R-Engine/Application.hpp>
#include <iostream>

static void hello_word()
{
    std::cout << "Hello, World!" << std::endl;
}

int main(void)
{
    r::Application{}.add_systems(r::Schedule::STARTUP, hello_word).run();
}
