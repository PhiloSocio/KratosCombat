# Kratos Combat

* **Axe throw and call mechanic.**
  - Throwing axe to looking direction like an aimed magic projectile.
  - Throwed axe's damage calculating with it's attack damage and thrower's one handed skill and archery skill.
  - Throwed axe keeps current enchantment of the axe.
  - Calling axe from it's current location: hitten bone, hitten object, hitten ground position, from havok or while is moving.
  - The axe hits to living targets while arriving.
  - Axe will arrive to your hand with a curved path like God of War.
  - Throwing and arriving speeds are completely editable with ini file.
  
* **Draupnir Spear's throwing and detonating system.**
  - Works same like the axe throwing.
  - Spears stucks their hit positions.
  - Stucked spears explodes one by one after your Draupnir's call move.
  - Explosion system not working with actor targets for now.
  - Explodable spear count editable with ini file.
  
* **Compatible with 1.5.97 - 1.6.x game versions. (Tested with 1.5.97, 1.6.318, 1.6.640)**

## Requirements

* CommonLibSSE NG

Because this uses [CommonLibSSE NG](https://github.com/CharmedBaryon/CommonLibSSE-NG), it supports Skyrim SE, AE, GOG, and VR. 

Hook IDs and offsets must still be found manually for each version.


* [Visual Studio 2022](https://visualstudio.microsoft.com/) (_the free Community edition_)
* [`vcpkg`](https://github.com/microsoft/vcpkg)
  - 1. Clone the repository using git OR [download it as a .zip](https://github.com/microsoft/vcpkg/archive/refs/heads/master.zip)
  - 2. Go into the `vcpkg` folder and double-click on `bootstrap-vcpkg.bat`
  - 3. Edit your system or user Environment Variables and add a new one:
    - Name: `VCPKG_ROOT`  
      Value: `C:\path\to\wherever\your\vcpkg\folder\is`
  - The latest version of vcpkg needs a default repository defined in the json. If you're using an older version of vcpkg, simply delete the default repository definition in `vcpkg-configuration.json`

## Project setup

By default, when this project compiles it will output a `.dll` for your SKSE plugin into the `build/` folder.

If you want to configure this project to output your plugin files
into your Skyrim Special Edition's "`Data`" folder:

- Set the `SKYRIM_FOLDER` environment variable to the path of your Skyrim installation  
  e.g. `C:\Program Files (x86)\Steam\steamapps\common\Skyrim Special Edition`

If you want to configure this project to output your plugin files
into your "`mods`" folder:  
(_for Mod Organizer 2 or Vortex_)

- Set the `SKYRIM_MODS_FOLDER` environment variable to the path of your mods folder:  
  e.g. `C:\Users\<user>\AppData\Local\ModOrganizer\Skyrim Special Edition\mods`  
  e.g. `C:\Users\<user>\AppData\Roaming\Vortex\skyrimse\mods`

## Credits
* [CharmedBayron](https://github.com/CharmedBaryon/CommonLibSSE-NG),
[Ryan McKenzie](https://github.com/Ryan-rsm-McKenzie) and
[powerof3](https://github.com/powerof3) for CommonlibSSE-NG.
* [Mrowr Purr](https://github.com/SkyrimScripting) for his guide videos.
* [Monitor144hz](https://github.com/Monitor144hz) for his CommonlibSSE project template.
* [Ersh](https://github.com/ersh1) for inspiring me with TrueDirectionalMovement mod's projectile stuff.
* [Fenix](https://github.com/fenix31415) for helping to work with projectiles.
