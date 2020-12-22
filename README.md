# neva_tool
Tool for extracting data from neva.pkg (Evangelion: Jo (PSP) resources file)
#### License: GPLv3+
## BPK0 archive structure
> addresses given for `neva.pkg`
### Header (0x0 - 0x7FF)
1. `BPK0` - 4 bytes
2. *total number of files?* - 4 bytes
3. folder table address (little endian) - 4 bytes
4. folder table offset from file table address (little endian) - 4 bytes
5. file table address (little endian) - 4 bytes

### File entries (0x800 - 0x121EF7FF)
1. `BDL0` - 4 bytes
2. uncompressed file size - 3 bytes
3. `00` - 1 byte
4. compressed file size (if Zlib-ed) - 3 bytes
5. `00` header padding to meet the equation: `x mod 32 = 0`
6. `00` file padding to meet the equation: `x mod 2048 = 0`
> Note: since field length is 3 bytes, maximum chunk size is **16 MiB - 1** (in theory)
### File table (0x121EF800 - 0x12242C0F)
File pointers is splitted by folders. Each folder contains:
1. file pointers
	+ ?? - 4 bytes
	+ *buffer size for file?* (always bigger than file size) - 4 bytes
	+ file size - 4 bytes
	+ file address (little endian) - 4 bytes
2. file pointers names
	+ filename
	+ `00` - 1 byte
	+ `00` padding to meet the equation: `x mod 16 = 0`
### Folder table (0x12242C10 - 0x122451BB)
1. folder names
	+ ?? - 4 bytes
	+ ?? - 4 bytes
	+ folder address - 4 bytes
	+ folder size - 4 bytes
	+ `00000000` - 4 bytes
	+ folder name
	+ `00` folder name padding to meet the equation: `x mod 4 = 0`
		+ if name already fits equation, then add `00000000`

## neva.pkg info

Algo   |Checksum
-------|-----------------------------------------------------------------
MD5    |`26263287db03caeb0d3fc1393534706f`
SHA1   |`59d02466f26f9c8711e74187860aac5ebab7bc6d`
SHA256 |`1ebb2c257abc09f8f61eabcc3f589f62609f21eccedd1f4ba0c3252f149e29d8`
#### Path:
`disk0:/psp_game/usrdir/neva.pkg`
