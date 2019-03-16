# CS:GO Auto Radar
Automatically make a radar with every compile of a map you do:

Specify the layout in hammer by adding brushes to a visgroup named 'tavr_layout', and Auto Radar will do the rest.

![](https://i.imgur.com/thAfDqx.png)

Editable GLSL compositor engine for CS:GO Radars, many features to come.

Notable things up ahead:
- Prop support
- baked lighting & even better AO
- Auto CSG geometry for even better masking

Want to support development? https://www.paypal.me/terri00 :)

# Usage Guide:
This guide covers basic setup and usage of Auto Radar: http://www.harrygodden.com/blog/?article=auto-radar-alpha

(Should be setup the same way vvis.exe or vrad.exe are)

# Download:
This tool requires Visual C++ 2017 redistributables, you can donwload them here (both should be installed if Auto Radar won't start):

- https://aka.ms/vs/15/release/vc_redist.x64.exe (64 bit)
- https://aka.ms/vs/15/release/vc_redist.x86.exe (32 bit)

Download the latest release of AutoRadar here: https://github.com/Terri00/MCDV/releases

Submit any issues on this git repository, and send suggestions to hgodden00@gmail.com / Terri00#1024 on discord

# Configuration:
### Command line options:
The following options should be specified on the command line.
```csharp
// Set by hammer =============================================================
-g, --game                  string // (required) The game directory for csgo
-m, --mapfile               string // (required or positional) The mapfile

// User commands =============================================================
-d, --dumpMasks                    // Outputs the masks next to the .exe
-o, --onlyMasks                    // Outputs the masks and does nothing else

-w, --width                 int    // Render width (experimental)
-h, --height                int    // Render height (experimental)

--ao                               // Enables Ambient Occlusion in the radar
--shadows                          // Enables basic traced shadows in the radar
```
Example setup: `AutoRadar.exe -d --ao --shadows -g %1 %2`

Minimum setup: `AutoRadar.exe -g %1 %2` (Basic radar, no shadows or ambient occlusion)

###### Where `%1` is the path to the `/Counter-Strike Global Offensive/csgo/` folder
###### And `%2` is the path to the vmf file you want to compile for.

![None / AO / Shadows](https://i.imgur.com/J1dJkxi.png)

### Detected Visgroups:
Brushes and entity brushes which are inside visgroups with these names will change the final radar in different ways. The only one required is `tavr_layout` and should define your maps playable space.

| Visgroup name | What it does                                            |
|---------------|---------------------------------------------------------|
| tavr_layout   | specifies the layout of the map (the floor)             |
| tavr_negative | brushes that should subtract from the layout of the map |
| tavr_cover    | brushes that should show up as cover in the radar (brushes in this group should also be in tavr_layout)      |

### Detected Entities:
Entities with these classnames get picked up by Auto Radar. They are not required, and the values will otherwise be automatically set.

| Entity name     | What it does                                  |
|-----------------|-----------------------------------------------|
| tavr_height_min | Overrides the minimum height value of the map (place it at the lowest part of your map) |
| tavr_height_max | Overrides the maximum height value (place it at the top of your map) |

### Free software used:
- Headers from the amazing STB collection: https://github.com/nothings/stb
- cxxopts: https://github.com/jarro2783/cxxopts

###### Looking for the old height map generator? https://github.com/Terri00/MCDV/tree/master
