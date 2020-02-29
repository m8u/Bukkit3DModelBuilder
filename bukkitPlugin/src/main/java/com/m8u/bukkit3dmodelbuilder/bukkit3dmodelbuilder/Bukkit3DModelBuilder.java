package com.m8u.bukkit3dmodelbuilder.bukkit3dmodelbuilder;

import org.bukkit.Location;
import org.bukkit.Material;
import org.bukkit.World;
import org.bukkit.block.Block;
import org.bukkit.command.Command;
import org.bukkit.command.CommandSender;
import org.bukkit.entity.Player;
import org.bukkit.plugin.java.JavaPlugin;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.*;
import java.util.List;
import java.util.logging.Logger;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public final class Bukkit3DModelBuilder extends JavaPlugin {

    Logger logger = getLogger();
    String serverDirectory = new File(".").getAbsolutePath();

    @Override
    public void onEnable() {
        //getLogger().info("Hello Bukkitttttttttttttt!");
    }

    @Override
    public void onDisable() {

    }

    @Override
    public boolean onCommand(CommandSender sender, Command cmd, String label, String[] args) {
        if (cmd.getName().equalsIgnoreCase("build3dmodel")) {

            int dimension = 16;
            Material material = Material.STONE;
            boolean noMaterialSpecified = false;
            boolean validArgs = true;

            ArrayList<Material> randomMaterials = new ArrayList<Material>();
            for (Material mat : Material.values()) {
                if (mat.isBlock() && mat.name().toLowerCase().contains("glass")) randomMaterials.add(mat);
            }

            Random randomMachine = new Random();

            Map<Character, Material> materialsDict = new HashMap<Character, Material>();
            for (char mapKey = 'a'; mapKey <= 'z'; mapKey++) {
                materialsDict.put(mapKey, Material.STONE);
            }

            switch (args.length) {
                case 0:
                case 1:
                    sender.sendMessage("§o§cerror: not enough arguments");
                    validArgs = false;
                    break;
                case 2:
                    try {
                        dimension = Integer.parseInt(args[1]);
                        noMaterialSpecified = true;
                    } catch (Exception e) {
                        sender.sendMessage("§o§cerror: some of arguments are invalid");
                        validArgs = false;
                    }
                    break;
                case 3:
                    try {
                        dimension = Integer.parseInt(args[1]);
                        material = Material.getMaterial(args[2].toUpperCase());
                        //logger.info(material.name());
                    } catch (Exception e) {
                        sender.sendMessage("§o§cerror: some of arguments are invalid");
                        validArgs = false;
                    }
                    break;
                default:
                    try {
                        dimension = Integer.parseInt(args[1]);
                        char mapKeyToReplace = 'a';
                        for (int i = 2; i < args.length; i++) {
                            materialsDict.replace(mapKeyToReplace, Material.getMaterial(args[i].toUpperCase()));
                            mapKeyToReplace++;
                        }
                        //for ( Map.Entry<Character, Material> entry : materialsDict.entrySet() ) {
                        //    logger.info(entry.getKey()+": "+entry.getValue());
                        //}
                    } catch (Exception e) {
                        sender.sendMessage("§o§cerror: some of arguments are invalid");
                        validArgs = false;
                    }
                    break;
            }

            if (!validArgs) {
                return false;
            }

            Location playerLocation = ((Player) sender).getLocation();

            World world = playerLocation.getWorld();
            double playerX = playerLocation.getBlockX(),
                    playerY = playerLocation.getBlockY(),
                    playerZ = playerLocation.getBlockZ();
            double blockX = 0, blockY = 0, blockZ = 0;

            float yaw = Math.abs(playerLocation.getYaw());
            int[] blockCoordParams = new int[4];
            if ((yaw >= 315 && yaw < 360) || (yaw >= 0 && yaw < 45)) {
                //sender.sendMessage("south");
                blockCoordParams = new int[] {-1, -1, 0, 2};
            } else if ((yaw >= 45 && yaw < 135)) {
                //sender.sendMessage("west");
                blockCoordParams = new int[] {-1, 1, 2, 0};
            } else if ((yaw >= 135 && yaw < 225)) {
                //sender.sendMessage("north");
                blockCoordParams = new int[] {1, 1, 0, 2};
            } else if ((yaw >= 225 && yaw < 315)) {
                //sender.sendMessage("east");
                blockCoordParams = new int[] {1, -1, 2, 0};
            }

            sender.sendMessage("§oprocessing...");

            ProcessBuilder processBuilder = new ProcessBuilder();
            processBuilder.command("bash", "-c", serverDirectory+"/plugins/Bukkit3DModelBuilder/./voxelize "
                                                +serverDirectory+"/3dmodels/"+args[0]+" "+dimension);

            try {
                Process process = processBuilder.start();
                BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));

                String line;
                int x = 0, y = 0, z = 0;
                double[] coordsToSetblock;
                Location blockLocation;
                while ((line = reader.readLine()) != null) {
                    if (line.equals("")) continue;
                    for (x = 0; x < dimension; x++) {
                        if (line.charAt(x) != '0') {
                            coordsToSetblock = new double[] {x - dimension/2.0, y - dimension/2.0, z - dimension/2.0};
                            blockX = playerX + blockCoordParams[0]*(coordsToSetblock[blockCoordParams[2]]);
                            blockY = playerY + coordsToSetblock[1];
                            blockZ = playerZ + blockCoordParams[1]*(coordsToSetblock[blockCoordParams[3]]);

                            blockLocation = new Location(world, blockX, blockY, blockZ);
                            Block block = world.getBlockAt(blockLocation);

                            if (noMaterialSpecified) {
                                material = randomMaterials.get(randomMachine.nextInt(randomMaterials.size()));
                                block.setType(material);
                            } else {
                                try {
                                    block.setType(materialsDict.get(line.charAt(x)));
                                } catch (Exception e) {}
                            }
                        }
                    }
                    y++;
                    if (y == dimension) {
                        z++; y = 0;
                    }
                }
                int exitCode = process.waitFor();
            } catch (IOException | InterruptedException e) {
                e.printStackTrace();
            }

            sender.sendMessage("§ocomplete!");
            return true;
        }
        return false;
    }

    @Override
    public List<String> onTabComplete(CommandSender sender, Command cmd, String alias, String[] args) {
        if (cmd.getName().equalsIgnoreCase("build3dmodel")) {
            //logger.info(Arrays.toString(args));
            switch (args.length) {
                case 0:
                    return new ArrayList<>();
                case 1:
                    ArrayList<String> objFiles = new ArrayList<String>();
                    try (Stream<Path> walk = Files.walk(Paths.get(serverDirectory+"/3dmodels/"))) {

                        List<String> result = walk.map(Path::toString)
                                .filter(f -> f.endsWith(".obj")).collect(Collectors.toList());

                        for (String filename : result) {
                            String[] splitted = filename.split("/");
                            if (splitted[splitted.length-1].startsWith(args[0].toLowerCase())) {
                                objFiles.add(splitted[splitted.length-1]);
                            }
                        }

                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    return objFiles;
                case 2:
                    ArrayList<String> dimension = new ArrayList<String>();
                    dimension.add("<int dimension>");
                    return dimension;
                default:
                    ArrayList<String> materials = new ArrayList<String>();
                    String[] splitted = cmd.toString().split(" ");
                    for (Material material : Material.values()) {
                        if (material.isBlock() && material.name().startsWith(args[args.length-1].toUpperCase())) {
                            materials.add(material.name().toLowerCase());
                        }
                    }
                    return materials;
            }
        }
        return null;
    }
}
