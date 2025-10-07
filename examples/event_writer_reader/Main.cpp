#include <R-Engine/Application.hpp>
#include <iostream>

static void hello_word()
{
    std::cout << "Hello, World!" << std::endl;
}

int main(void)
{
    r::Application{}.add_systems<hello_word>(r::Schedule::STARTUP).run();
}
