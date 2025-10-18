print("This is a native Lua print statement.")

Log.info("Hello from Lua! This is an info message.")
Log.warn("This is a warning from the script.")
Log.debug("This is a debug message.")

local player_name = "Yutsuna"
local score = 100

Log.info(string.format("Player '%s' scored %d points!", player_name, score))
Log.error("This is a test error message from test_log.lua")
