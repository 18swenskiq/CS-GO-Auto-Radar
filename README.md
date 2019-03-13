# CS:GO Auto Radar
Automatically make a radar with every compile of a map you do:

Specify the layout in hammer by adding brushes to a visgroup named 'tavr_layout', and Auto Radar will do the rest.

![](https://i.imgur.com/kEkdJND.png)

Editable GLSL compositor engine for CS:GO Radars, many features to come.

Notably:
- Prop support
- Ambient Occlusion
- Boolean geometry for even better path masking

Want to support development? https://www.paypal.me/terri00 :)

# Usage Guide:
This post covers basic usage of Auto Radar: http://www.harrygodden.com/blog/?article=auto-radar-alpha

# Download:
This tool requires Visual C++ 2017 redistributables, you can donwload them here (both should be installed if Auto Radar won't start):
https://aka.ms/vs/15/release/vc_redist.x64.exe (64 bit)

https://aka.ms/vs/15/release/vc_redist.x86.exe (32 bit)

Download the latest release here: https://github.com/Terri00/MCDV/releases

Submit any issues on this git repository, and send suggestions to hgodden00@gmail.com / Terri00#9530 on discord

### Command line options:
```csharp
// Set by hammer =============================================================
-g, --game                  string // (required) The game directory for csgo
-m, --mapfile               string // (required or positional) The mapfile

// User commands =============================================================
-d, --dumpMasks                    // Outputs the masks next to the .exe
-o, --onlyMasks                    // Outputs the masks and does nothing else

-w, --width                 int    // Render width (experimental)
-h, --height                int    // Render height (experimental)
```

### Free software used:
- Headers from the amazing STB collection: https://github.com/nothings/stb
- cxxopts: https://github.com/jarro2783/cxxopts

###### Looking for the old height map generator? https://github.com/Terri00/MCDV/tree/master
