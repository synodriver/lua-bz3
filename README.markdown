# simple lua binding for [bzip3](https://github.com/kspalaiologos/bzip3) 's high-level api

### usage

```lua
bz3 = require("bz3")

local data = "1234"
for i=1,7 do
    data = data..data
end
print("bzip3 version "..bz3.version)
print("origin size "..#data)
local compressed = bz3.compress(data, 1000)

print("compressed size "..#compressed)
decompressed = bz3.decompress(compressed, 1000)
assert(decompressed==data)
```