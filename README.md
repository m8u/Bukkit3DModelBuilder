# Bukkit3DModelBuilder
A combination of C++-written mesh voxelizer and a Bukkit server plugin which allows to build .OBJ models in Minecraft. 
# Installing
You need a Bukkit server running on some UNIX. Then go download the [latest release](https://github.com/m8u/Bukkit3DModelBuilder/releases) and unzip it to your *\<server_directory\>/plugins* as is.
# Usage
Create a *3dmodels* directory in the server root and store your **.obj** models there.

You must be logged as a player on a Minecraft server.

Command: *'/build3dmodel \<modelname\> \<dimension\> \[material\], \[material\], ...'*
  
You can specify as much materials as your **.obj** has, in the same order as your modeling software exports. No **.mtl** needed/supported, you supposed to specify minecraft blocks which look similar to your materials by hand.
