# CS:GO Radar Tools
### Set of tools to help with making radars (new CS:GO style since panorama beta)
Retrieved height information using the tool:

![](https://i.imgur.com/gd1OC8g.png)

# How to use
Everything is operated through a 'console' like interface. Type HELP to get a list of commands and variables you can change.

The primary three you need to know are: OPENBSP, OPENNAV & HELP

![](https://i.imgur.com/vmQ6BDa.gif)

List of commands:

| Commands						             | Info                             |
|----------------------------------|----------------------------------|
| QUIT / EXIT                      | Closes (wow!)                    |
| OPENBSP                          | Opens a BSP file                 |
| OPENNAV                          | Opens a NAV file                 |
| OPEN                             | Open BSP / NAV (1.5.3+)          |
| NAV                              | View nav mesh                    |
| BSP                              | View bsp file                    |
| HELP                             | Helps you out                    |
| PERSPECTIVE / PERSP              | Switches to perspective view     |
| ORTHOGRAPHIC / ORTHO             | Switches to orthographic view    |
| OSIZE / SIZE <int>               | Changes the orthographic scale   |
| LOOKDOWN                         | Sets yaw to 0, pitch to -90      |
| RENDER                           | Renders the view to render.png   |
| RENDERMODE <int>                 | Same as nav/bsp switch           |
| PROJMATRIX <int>                 | Same as persp/ortho              |
| MIN    <int>                     | Minimum levels of height (units) |
| MAX    <int>                     | Maximum levels of height (units) |
| FARZ <int>                       | Far clip plane of the camera     |
| NEARZ <int>                      | Near clip plane of the camera    |

& WASD to fly around, Mouse click and drag to look

# Download:
This tool requires Visual C++ 2017 redistributables, you can donwload them here:
https://aka.ms/vs/15/release/vc_redist.x64.exe (64 bit)

https://aka.ms/vs/15/release/vc_redist.x86.exe (32 bit)

http://www.harrygodden.com/downloads/csgoheightmap1.5.2.zip

Thanks to the patient few for testing.

## Misc... 
Intended for use with [Yanzl](https://github.com/gortnarj)'s radar generator substance graph, example:

![](https://i.imgur.com/gQ2TLRC.png)

Submit any issues on this git repository, and send suggestions to hgodden00@gmail.com / Terri00#9530 on discord
