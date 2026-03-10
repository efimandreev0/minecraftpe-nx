# Minecraft PE for the PlayStation Vita!

to build it, make sure you have the latest VitaSDK from vitasdk.org

```
cmake -DPUBLISH=on -B build -S handheld/project/vita
cd build
make -j$(nproc)
```

If you want to build the demo; you can add the ``-DDEMO=on`` flag to the cmake line.


Credits:

# Development work:
   - Olebeck (graphics, sound, networking)
   - Li (controls, refining options, menuing)
   - Koutsie (original options menu concept)
   
	PVR_PSP2 maintainers.
	The Vita Scene
