dune_symlink_to_source_files(FILES n1.input)

dumux_add_test(NAME lswi-n1
              LABELS porousmediumflow 2pnc
              SOURCES lswi-n.cc
              COMPILE_DEFINITIONS DUMUX_ENABLE_OLD_PROPERTY_MACROS=0 NUM_PARTICLES=1
              COMPILE_FLAGS -Wno-deprecated-declarations -I${CMAKE_SOURCE_DIR}/examples/lswi-n
              COMMAND ${CMAKE_SOURCE_DIR}/bin/testing/runtest.py
              CMD_ARGS  --script fuzzy
                        --command "${CMAKE_CURRENT_BINARY_DIR}/lswi-n1 n1.input" )


