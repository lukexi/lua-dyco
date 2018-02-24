#! #! ../rtlua/lua

local dynamic_c = require 'dynamic_c'

local c_source = [[

int Foo() {
    return 123;
}

]]

library = dynamic_c.create_library("foo", c_source)

print "Press enter to quit"
io.read()
