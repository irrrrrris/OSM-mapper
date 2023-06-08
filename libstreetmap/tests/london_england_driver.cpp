#include <iostream>
#include <string>
#include <random>
#include <UnitTest++/UnitTest++.h>

#include "StreetsDatabaseAPI.h"
#include "m1.h"
#include "m2.h"

namespace ezgl {
	extern void set_disable_event_loop (bool new_setting);
}


int main(int argc, char** argv) {
    //Disable interactive graphics
    ezgl::set_disable_event_loop(true);

    std::string map_name = "/cad2/ece297s/public/maps/london_england.streets.bin";
    bool load_success = loadMap(map_name);

    if(!load_success) {
        std::cout << "ERROR: Could not load map file: '" << map_name << "'!";
        std::cout << " Subsequent tests will likely fail." << std::endl;
        //Don't abort tests, since we still want to show that all
        //tests fail.
    }

    //Run the unit tests
    int num_failures = UnitTest::RunAllTests();

    return num_failures;
}
