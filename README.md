# Minecraft PE for the Nintendo Switch!

to build it, make sure you have the latest DevkitPro from devkitpro.org

```
cmake -DPUBLISH=on -B build -S handheld/project/nx
cd build
make -j$(nproc)
```

If you want to build the demo; you can add the ``-DDEMO=on`` flag to the cmake line.

# GamePlay
[![NX-GamePlay](https://img.youtube.com/vi/tUwJesK0Fgc/0.jpg)](https://www.youtube.com/watch?v=tUwJesK0Fgc)
[![PSV-GamePlay](https://img.youtube.com/vi/88zhe_yYPjo/0.jpg)](https://www.youtube.com/watch?v=88zhe_yYPjo)
# Credits
   - Olebeck (graphics, sound, networking)
   - Li (controls, refining options, menuing)
   - Koutsie (original options menu)
   - PVR_PSP2 developers (GrapheneCt) 
   - Silica (PSV-Port)
   - efimandreev0 (NX-Port)
# Other information:
 - NRO is just install-and-play, no additional setup should be required.
 - should be able to play local multiplayer over local wifi, and even cross-play with android/ios (armv7 only!) running official APK and also PSV.
 - savedata (worlds, options , etc) are stored in sdmc:/switch/minecraftpe/
 - as this is a source-port and cannot be easily adapted to any other version
