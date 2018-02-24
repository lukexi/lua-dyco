CFLAGS += -fPIC
CFLAGS += -shared -Wl,-undefined,dynamic_lookup
CFLAGS += -Wall
CFLAGS += -I../rtlua/src -I../rtlua/vendor/lua

all: dynamic_c.so

dynamic_c.so: dynamic-c-lua.c dynamic.c compile.c process.c
	clang -o $@ $^ $(CFLAGS)
