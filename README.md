# neva_tool
Tool for extracting data from neva.pkg (Evangelion: Jo (PSP) resources file)
#### License: GPLv3+
## BPK0 archive structure
### Header (20 bytes)
1. `BPK0`
2. 12 bytes - [TBF](#what-is-tbf)
3. footer address (little endian)

### Body
1. `BDL0`
2. chunk size (or [TBF](#what-is-tbf))
3. `00`
4. chunk size (or `00 00 00`)

### Footer
1. 16 bytes x number of file names in file list
2. file names

> Note: since field length is 3 bytes, maximum chunk size is **16 MiB - 1** (hypothetically)

## neva.pkg info
#### Checksums:
MD5:    `26263287db03caeb0d3fc1393534706f`  
SHA1:   `59d02466f26f9c8711e74187860aac5ebab7bc6d`  
SHA256: `1ebb2c257abc09f8f61eabcc3f589f62609f21eccedd1f4ba0c3252f149e29d8`  
#### Path:
`/psp_game/usrdir/neva.pkg`
###### What is TBF?
This means that I don't know definition of these bytes
