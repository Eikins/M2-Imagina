 
 
 
    // Initialize lua state
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

    ...

    // Execute Player Script
	if (CheckLua(L, luaL_dofile(L, "Scripts/PlayerHealthUpdate.lua")))
	{
		lua_getglobal(L, "player_health");
		if (lua_isnumber(L, -1))
		{
			float playerHealthResult = (float)lua_tonumber(L, -1);
            // Internal 
			OnPlayerHealthResult(playerHealthResult);
		}
	}

	...

	lua_close(L);