add_library(can buf.c msg.c net.c socket.c vci.c)
target_include_directories(can PRIVATE Lely)