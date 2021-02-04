add_executable(
        rfsensor rfsensor.cpp
        rflib rflib.cpp
        )

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(rfsensor pico_stdlib)

# create map/bin/hex file etc.
pico_add_extra_outputs(rfsensor)

# add url via pico_set_program_url
example_auto_set_url(rfsensor)
