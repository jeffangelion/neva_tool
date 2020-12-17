all:
	gcc neva_tool.c -o neva_tool -lm
static:
	gcc -static neva_tool.c -o neva_tool -lm
