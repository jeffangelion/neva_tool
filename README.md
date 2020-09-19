# neva tool
Tool for extracting data from neva.pkg (Evangelion: Jo (PSP) resources file)
## BPK0 archive structure
Header           | Body                                            | Footer
-----------------|-------------------------------------------------|-------
`BPK0` + 16 bytes|`BDL0` + 28 bytes + data chunk (could be Zlib-ed)|file list (?)
#### License: GPLv3+
