#ifdef USE_GLAD
#include "opengl_modern.h"
#else
#include "platform_opengl.h"
#endif
#include <iostream>

int main() {
    std::cout << "Testing OpenGL header inclusion..." << std::endl;
    
    GLint major, minor;
    std::cout << "GL_MAJOR_VERSION constant: " << GL_MAJOR_VERSION << std::endl;
    std::cout << "GL_MINOR_VERSION constant: " << GL_MINOR_VERSION << std::endl;
    
    #ifdef _WIN32
    std::cout << "Windows platform detected" << std::endl;
    #endif
    
    std::cout << "Header inclusion test passed!" << std::endl;
    return 0;
}
