# Bukkit3DModelBuilder
A combination of C++-written mesh voxelizer and a Bukkit server plugin which allows to build .OBJ models in Minecraft. 
# Installing
You need a Bukkit server running on some UNIX. Then go download the [latest release](https://github.com/m8u/Bukkit3DModelBuilder/releases) and unzip it to your *\<server_directory\>/plugins* as is.
# Usage
Create a directory named *3dmodels* in the server root and store your **.obj** models there **(models must be triangulated!)**.

You must be logged as a player on a Minecraft server.

**Command:** *'/build3dmodel \<modelname\> \<dimension\> \[material\], \[material\], ...'*

**Example:** *'/build3dmodel suzanne.obj 128 STONE, GRASS_BLOCK'*

You can specify as much materials as your **.obj** has, in the same order as your modeling software exports. No **.mtl** needed/supported, you supposed to specify minecraft blocks which look similar to your materials by hand.

# Known problems
- Minecraft client leaves the server while long-time building due to tick timeout.
- Also server itself can crash due to default timeout (60 sec). You can override the timeout in the server properties (set it to -1 to turn it off).
- If building process continues without player on a server, the lighting on the blocks will be broken. For now, you can fix the lighting after building completed using some another bukkit plugin (e.g. [LightCleaner](https://github.com/bergerhealer/Light-Cleaner))
