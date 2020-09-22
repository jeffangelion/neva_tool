# neva_tool
Tool for extracting data from neva.pkg (Evangelion: Jo (PSP) resources file)
#### License: GPLv3+
## BPK0 archive structure
Header           | Body                                            | Footer
-----------------|-------------------------------------------------|-------
`BPK0` + 16 bytes|`BDL0` + 28 bytes + data chunk (could be Zlib-ed)|file list (?)
## neva.pkg info
#### Checksums:
MD5:    `26263287db03caeb0d3fc1393534706f`  
SHA1:   `59d02466f26f9c8711e74187860aac5ebab7bc6d`  
SHA256: `1ebb2c257abc09f8f61eabcc3f589f62609f21eccedd1f4ba0c3252f149e29d8`  
