#查找该目录下的所有源文件，并保存到SRC_LIST
aux_source_directory(. SRC_LIST)
add_compile_definitions(THREADED)
add_library(mprpc ${SRC_LIST})
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)