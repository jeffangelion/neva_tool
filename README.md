# neva_tool
Tool for extracting data from neva.pkg (Evangelion: Jo (PSP) resources file)
#### License: GPLv3+
## BPK0 archive structure
> Note: addresses and offsets are always little endian
### Header
|Offset|Size (in bytes)|Purpose                                    |
|------|---------------|-------------------------------------------|
|0     |4              |File signature (`"BPK0"`)                  |
|4     |4              |(?) Total number of files                  |
|8     |4              |Folder table address                       |
|12    |4              |Folder table offset from file table address|
|16    |4              |File table address                         |
|20    |2028           |Padding                                    |

### File entries
|Offset|Size (in bytes)|Purpose                                    |
|------|---------------|-------------------------------------------|
|0     |4              |File entry signature (`"BDL0"`)            |
|4     |4              |File entry size                            |
|8     |4              |File entry size (if was compressed by Zlib)|
|12    |20             |Padding                                    |
|32    |N              |File entry itself                          |
|32+N  |M              |Padding (mod 2048 = 0)                     |

### File table
#### File entry pointers
|Offset|Size (in bytes)|Purpose                  |
|------|---------------|-------------------------|
|0     |4              |Offset to file entry name|
|4     |4              |(?) Buffer size for file |
|8     |4              |File entry size          |
|12    |4              |File entry address       |
#### File entry names
|Offset|Size (in bytes)|Purpose             |
|------|---------------|--------------------|
|0     |N              |File entry name     |
|N     |M              |Padding (mod 16 = 0)|

> This table needs more explanation
### Folder entries table
|Offset|Size (in bytes)|Purpose               |
|------|---------------|----------------------|
|0     |2              |n1|
|2     |2              |n2|
|4     |2              |Folder record size    |
|6     |2              |Number of file entries|
|8     |4              |Folder entry address  |
|12    |4              |Folder entry size     |
|16    |4              |Divider (`0x00000000`)|
|20    |N              |Folder entry name     |
|20+N  |1+M            |Padding (mod 4 = 0)   |

#### Pseudocode explanation of `n1` and `n2`:
```
if n1 <= folderRecordSize and n2 == 0 then
    print No subfolders
else if n1 == folderRecordSize and n2 == folderRecordSize then
    print No subfolders
else if n1 > folderRecordSize and n2 == folderRecordSize then
    print Next (n1 - n2) bytes are subfolders
else if n1 == 0 and n2 == folderRecordSize then
    print Is subfolder
```
