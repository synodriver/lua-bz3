#include <string.h>
#include "lua.h"
#include "lauxlib.h"

#include "libbz3.h"


#if defined(_WIN32) || defined(_WIN64)
#define DLLEXPORT __declspec(dllexport)
#elif
#define DLLEXPORT
#endif /* _WIN32 */

#define MB (1024 * 1024)

static int
lbz3_gc(lua_State *L)
{
    if (lua_gettop(L) != 1)
    {
        return luaL_error(L, "must have a state");
    }
    struct bz3_state **ud = (struct bz3_state **) luaL_checkudata(L, 1, "bz3.state");
    bz3_free(*ud);
    *ud = NULL;
    return 0;
}

static int
lbz3_last_error(lua_State *L)
{
    if (lua_gettop(L) != 1)
    {
        return luaL_error(L, "must have a state");
    }
    struct bz3_state **ud = (struct bz3_state **) luaL_checkudata(L, 1, "bz3.state");
    lua_pushinteger(L, (lua_Integer) bz3_last_error(*ud));
    return 1;
}

static int
lbz3_strerror(lua_State *L)
{
    if (lua_gettop(L) != 1)
    {
        return luaL_error(L, "must have a state");
    }
    struct bz3_state **ud = (struct bz3_state **) luaL_checkudata(L, 1, "bz3.state");
    lua_pushstring(L, bz3_strerror(*ud));
    return 1;
}

static int
lbz3_encode_block(lua_State *L)
{
    struct bz3_state **ud;
    uint8_t *in;
    size_t in_size;
    size_t buffersize;
    switch (lua_gettop(L))
    {
        case 3:
            ud = (struct bz3_state **) luaL_checkudata(L, 1, "bz3.state");
            in = (uint8_t *) luaL_checklstring(L, 2, &in_size);
            buffersize = (size_t)luaL_checkinteger(L, 3);
            break;
        default:
            return luaL_error(L, "must have a state and input and buffersize");

    }
    uint8_t *buffer = (uint8_t *) lua_newuserdata(L, buffersize);
    memcpy(buffer, in, in_size);
    int32_t newsize;
    if ((newsize = bz3_encode_block(*ud, buffer, (int32_t) in_size)) == -1)
    {
        return luaL_error(L, "Failed to encode a block: %s\n", bz3_strerror(*ud));
    }
    lua_pushlstring(L, (const char *) buffer, (size_t) newsize);
    return 1;

}

static int
lbz3_decode_block(lua_State *L)
{
    struct bz3_state **ud;
    uint8_t *in;
    size_t newsize;
    int32_t oldsize;
    size_t buffersize;
    switch (lua_gettop(L))
    {
        case 4:
            ud = (struct bz3_state **) luaL_checkudata(L, 1, "bz3.state");
            in = (uint8_t *) luaL_checklstring(L, 2, &newsize);
            oldsize = (int32_t) luaL_checkinteger(L, 3);
            buffersize = (size_t)luaL_checkinteger(L, 4);
            break;
        default:
            return luaL_error(L, "must have a state input and oldsize and buffersize");
    }
    uint8_t *buffer = (uint8_t *) lua_newuserdata(L, buffersize);
    memcpy(buffer, in, newsize);
    if (bz3_decode_block(*ud, buffer, buffersize, (int32_t) newsize, oldsize) == -1)
    {
        return luaL_error(L, "Failed to decode a block: %s\n", bz3_strerror(*ud));
    }
    lua_pushlstring(L, (const char *)buffer, (size_t) oldsize);
    return 1;
}

static luaL_Reg lua_bz3_methods[] = {
        {"__gc",         &lbz3_gc},
        {"last_error",   &lbz3_last_error},
        {"strerror",     &lbz3_strerror},
        {"encode_block", &lbz3_encode_block},
        {"decode_block", &lbz3_decode_block},
        {NULL, NULL}
};

static int
lnewstate(lua_State *L)
{
    if (lua_gettop(L) != 1)
    {
        return luaL_error(L, "must have a block size");

    }
    int32_t block_size = (int32_t) luaL_checkinteger(L, 1);
    struct bz3_state *state = bz3_new(block_size);
    if (state == NULL)
    {
        return luaL_error(L, "failed to malloc bz3_state");
    }
    struct bz3_state **ud = lua_newuserdata(L, sizeof(struct bz3_state *));
    *ud = state;
    luaL_setmetatable(L, "bz3.state");
    return 1;
}

static int
lnewbuffer(lua_State *L)
{
    if (lua_gettop(L) != 1)
    {
        return luaL_error(L, "need a size");
    }
    size_t size = (size_t) luaL_checkinteger(L, 1);
    lua_newuserdata(L, size);
    return 1;
}

static int
lbound(lua_State *L)
{
    if (lua_gettop(L) != 1)
    {
        return luaL_error(L, "only one param is needed");
    }
    size_t inpsize = luaL_checkinteger(L, 1);
    lua_pushinteger(L, (lua_Integer) bz3_bound(inpsize));
    return 1;
}

static int
lversion(lua_State *L)
{
    if (lua_gettop(L) != 0)
    {
        return luaL_error(L, "no arguments");
    }
    lua_pushstring(L, bz3_version());
    return 1;
}

// string output_size block_size
static int
lcompress(lua_State *L)
{
    size_t inpsize;
    const char *inp;
    size_t output_size;
    uint32_t block_size;
    switch (lua_gettop(L))
    {
        case 1:
        {
            inp = luaL_checklstring(L, 1, &inpsize);
            output_size = bz3_bound(inpsize);
            block_size = MB;
            break;
        }
        case 2:
        {
            inp = luaL_checklstring(L, 1, &inpsize);
            output_size = (size_t) luaL_checkinteger(L, 2);
            block_size = MB;
            break;
        }
        case 3:
        {
            inp = luaL_checklstring(L, 1, &inpsize);
            output_size = (size_t) luaL_checkinteger(L, 2);
            block_size = (uint32_t) luaL_checkinteger(L, 3);
            break;
        }
        default:
        {
            return luaL_error(L, "wrong params. data [output_buff_size [block_size]]");
        }
    }
    uint8_t *outbuff = (uint8_t *) lua_newuserdata(L, output_size);
    int bzerr = bz3_compress(block_size, (const uint8_t *) inp, outbuff, inpsize, &output_size);
    if (bzerr != BZ3_OK)
    {
        return luaL_error(L, "bz3_compress() failed with error code %d", bzerr);
    }
    lua_pushlstring(L, (const char *) outbuff, output_size);
    return 1;
}

// string output_size
static int
ldecompress(lua_State *L)
{
    if (lua_gettop(L) != 2)
    {
        return luaL_error(L, "must have 2 params, data and output_size");
    }
    size_t inpsize;
    const char *inp;
    size_t output_size;
    inp = luaL_checklstring(L, 1, &inpsize);
    output_size = (size_t) luaL_checkinteger(L, 2);
    uint8_t *outbuff = (uint8_t *) lua_newuserdata(L, output_size);

    int bzerr = bz3_decompress((const uint8_t *) inp, outbuff, inpsize, &output_size);
    if (bzerr != BZ3_OK)
    {
        return luaL_error(L, "bz3_decompress() failed with error code %d", bzerr);
    }
    lua_pushlstring(L, (const char *) outbuff, output_size);
    return 1;
}

static int
lmin_memory_needed(lua_State* L)
{
    if (lua_gettop(L) != 1)
    {
        return luaL_error(L, "must have a block_size");
    }
    int32_t block_size = (int32_t)luaL_checkinteger(L, 1);
    lua_pushinteger(L, (lua_Integer)bz3_min_memory_needed(block_size));
    return 1;
}

static int
lorig_size_sufficient_for_decode(lua_State* L)
{
    if (lua_gettop(L) != 2)
    {
        return luaL_error(L, "must have a block and origin size");
    }
    size_t block_size;
    const uint8_t * block = (const uint8_t *)luaL_checklstring(L, 1, &block_size);
    int32_t orig_size = (int32_t)luaL_checkinteger(L, 2);
    lua_pushinteger(L, (lua_Integer)bz3_orig_size_sufficient_for_decode(block, block_size, orig_size));
    return 1;
}

static luaL_Reg lua_funcs[] = {
        {"newstate",   &lnewstate},
        {"newbuffer",  &lnewbuffer},
        {"version",    &lversion},
        {"bound",      &lbound},
        {"compress",   &lcompress},
        {"decompress", &ldecompress},
        {"min_memory_needed", &lmin_memory_needed},
        {"orig_size_sufficient_for_decode", &lorig_size_sufficient_for_decode},
        {NULL, NULL}
};

DLLEXPORT int luaopen_bz3(lua_State *L)
{
    if (!luaL_newmetatable(L, "bz3.state"))
    {
        return luaL_error(L, "bz3.state already in register");
    }
    lua_pushvalue(L, -1); // mt mt
    lua_setfield(L, -2, "__index"); // mt
    luaL_setfuncs(L, lua_bz3_methods, 0); // mt

    luaL_newlib(L, lua_funcs);
    lua_pushstring(L, bz3_version());
    lua_setfield(L, -2, "version");
    return 1;
}