# CS:GO Auto Radar
Automatically make a radar with every compile of a map you do:

Specify the layout in hammer by adding brushes to a visgroup named 'tar_layout', and Auto Radar will do the rest.

![](https://i.imgur.com/jNWULV0.png)

Editable GLSL compositor engine for CS:GO Radars, many features to come.

Notable things up ahead:
- ~~Prop support~~ Done!
- func_instances
- baked lighting & even better AO
- Auto CSG geometry for even better masking

Want to support development? https://www.paypal.me/terri00 :)

# Download:
This tool requires Visual C++ 2017 redistributables, you can donwload them here (both should be installed if Auto Radar won't start):

- https://aka.ms/vs/15/release/vc_redist.x64.exe (64 bit)
- https://aka.ms/vs/15/release/vc_redist.x86.exe (32 bit)

Download the latest release of AutoRadar here: https://github.com/Terri00/MCDV/releases

Run the AutoRadar_installer.exe to copy all the necessary files into the correct locations. (Make sure hammer is closed)

Submit any issues on this git repository, and send suggestions to hgodden00@gmail.com / Terri00#1024 on discord

# Guide:

### Configuration: 
![](https://i.imgur.com/w2M0Ap9.png)

Add a tar_config entity to your map. This defines your radars settings. It is not needed, but allows you to customize it.

#### Examples of the different effects:
![](https://i.imgur.com/tE72qG2.png)

### Detected Visgroups:
![Visgroup example](https://i.imgur.com/fXozJkj.png)

Brushes, displacements, entity brushes and prop_statics which are inside visgroups with these names will change the final radar in different ways. The only one required is `tar_layout` and should define your maps playable space.

| Visgroup name | What it does                                            |
|---------------|---------------------------------------------------------|
| tar_layout   | specifies the layout of the map (the floor)             |
| tar_mask | brushes that should subtract from the layout of the map, use this on walls |
| tar_cover    | brushes that should show up as cover in the radar    |

### Detected Entities:
![](https://i.imgur.com/PyPuPh5.png)

Entities with these classnames get picked up by Auto Radar. They are not required, and the values will otherwise be automatically set.

| Entity name     | What it does                                  |
|-----------------|-----------------------------------------------|
| tar_min | Overrides the minimum height value of the map (place it at the lowest part of your map) |
| tar_max | Overrides the maximum height value (place it at the top of your map) |

### Generating:
![](https://i.imgur.com/Y1l9eDC.png)

The AutoRadar installer set up a new compile profile in the 'expert' mode. Enter the compile mode, and select [TAR] Generate Radar from the list. The hit Go!

### Using this software with Yanzl's Radar graph:
https://github.com/Terri00/CS-GO-Auto-Radar/blob/tavr/radar-graph.md

### Free software used:
- Headers from the amazing STB collection: https://github.com/nothings/stb
- cxxopts: https://github.com/jarro2783/cxxopts

###### Looking for the old height map generator? https://github.com/Terri00/MCDV/tree/master
