ls test-luac.lua ../rtlua/vendor/lua/loadlib.c | entr -rcs "(cd ../rtlua && rm -f lua && make) && ./test-luac.lua"
