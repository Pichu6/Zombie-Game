add_executable(zs_gen_cli main.cpp)
set_target_properties(zs_gen_cli PROPERTIES OUTPUT_NAME zs_gen)
target_link_libraries(zs_gen_cli PRIVATE zs_gen)
