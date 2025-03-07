#include <iostream>
#include <unistd.h>

int main()
{
    while (true)
    {
        std::cout << "Hello World" << std::endl;
        sleep(1);
    }
}