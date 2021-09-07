#include "includes/shared.h"
#include "includes/android.h"

int main(int argc, char* argv []) {

    glm::vec3 a(10);

    LOG_ERROR("BAAAAAAAA is %f", a.x);

    return start(argc, argv);
}
