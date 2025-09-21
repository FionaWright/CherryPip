#include <iostream>

#include "HelloTriangle.h"

int main() {
    HelloTriangle* helloTriangle = new HelloTriangle();
    helloTriangle->OnInit();

    while (true)
    {
        helloTriangle->OnUpdate();
    }

    return 0;
}
