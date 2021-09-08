#include "includes/shared.h"
#include "includes/android.h"

void logInfo(std::string message) {
    LOG_INFO("%s", message.c_str());
}

void logError(std::string message) {
    LOG_ERROR("%s", message.c_str());
}

int main(int argc, char* argv []) {

    glm::vec3 a(10);

    return start(argc, argv);
}