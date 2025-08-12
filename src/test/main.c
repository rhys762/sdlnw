#include <stdio.h>
#include <munit.h>

extern const MunitSuite text_controller_suite;

int main(int argc, char** argv) {
    MunitSuite suites[] = {
        text_controller_suite,
        {0}
    };

    MunitSuite suite = {
        .prefix = "SDLNW ",
        .suites = suites
    };

    return munit_suite_main(&suite, NULL, argc, argv);;
}