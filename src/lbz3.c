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
    if(lua_gettop(L)!=2)
    {
        return luaL_error(L, "must have 2 params, data and output_size");
    }
    size_t inpsize;
    const char *inp;
    size_t output_size;
    inp = luaL_checklstring(L, 1, &inpsize);
    output_size = (size_t )luaL_checkinteger(L,2 );
    uint8_t *outbuff = (uint8_t *) lua_newuserdata(L, output_size);

    int bzerr = bz3_decompress((const uint8_t *)inp, outbuff, inpsize, &output_size);
    if (bzerr != BZ3_OK)
    {
        return luaL_error(L, "bz3_decompress() failed with error code %d", bzerr);
    }
    lua_pushlstring(L, (const char *) outbuff, output_size);
    return 1;
}

static luaL_Reg lua_funcs[] = {
        {"bound",      &lbound},
        {"compress",   &lcompress},
        {"decompress", &ldecompress},
        {NULL, NULL}
};

DLLEXPORT int luaopen_bz3(lua_State *L)
{
    luaL_newlib(L, lua_funcs);
    lua_pushstring(L, bz3_version());
    lua_setfield(L, -2, "version");
    return 1;
}