cmake_minimum_required(VERSION 3.16...4.0)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{DEVKITPRO})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{DEVKITPRO}/cmake/3DS.cmake" CACHE PATH "toolchain file")
    else()
        message(FATAL_ERROR "Please define DEVKITPRO to point to your SDK path!")
    endif()
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

project(minecraftcpp)

set(CTR_APP_NAME "Minecraft PE")
set(CTR_APP_AUTHOR "Your Team")
set(CTR_APP_DESC "Minecraft PE Port for 3DS")
set(CTR_APP_ICON ${CMAKE_CURRENT_LIST_DIR}/icon.png)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall")

set(HANDHELD "${CMAKE_CURRENT_LIST_DIR}/../../src/")
set(ASSETS_DIR "${CMAKE_CURRENT_LIST_DIR}/../../data")

add_definitions(-D__3DS__)

if(DEMO)
    set(CTR_APP_NAME "Minecraft PE Demo")
    add_definitions(-DDEMO_MODE)
endif()
add_compile_definitions(RIP_BACKEND=0x01)
add_compile_options(-O3 -ffast-math -fomit-frame-pointer -fpermissive)

if(PUBLISH)
    add_definitions(-DPUBLISH)
endif()

add_library(raknet
  ${HANDHELD}/raknet/DS_Table.cpp
  ${HANDHELD}/raknet/FormatString.cpp
  ${HANDHELD}/raknet/EncodeClassName.cpp
  ${HANDHELD}/raknet/SuperFastHash.cpp
  ${HANDHELD}/raknet/RakPeer.cpp
  ${HANDHELD}/raknet/DS_BytePool.cpp
  ${HANDHELD}/raknet/DS_ByteQueue.cpp
  ${HANDHELD}/raknet/ReliabilityLayer.cpp
  ${HANDHELD}/raknet/RakNetTransport2.cpp
  ${HANDHELD}/raknet/CheckSum.cpp
  ${HANDHELD}/raknet/TCPInterface.cpp
  ${HANDHELD}/raknet/rdlmalloc.cpp
  ${HANDHELD}/raknet/LocklessTypes.cpp
  ${HANDHELD}/raknet/Itoa.cpp
  ${HANDHELD}/raknet/StringCompressor.cpp
  ${HANDHELD}/raknet/BitStream.cpp
  ${HANDHELD}/raknet/GetTime.cpp
  ${HANDHELD}/raknet/WSAStartupSingleton.cpp
  ${HANDHELD}/raknet/StringTable.cpp
  ${HANDHELD}/raknet/RakString.cpp
  ${HANDHELD}/raknet/RakSleep.cpp
  ${HANDHELD}/raknet/DataCompressor.cpp
  ${HANDHELD}/raknet/CCRakNetSlidingWindow.cpp
  ${HANDHELD}/raknet/SignaledEvent.cpp
  ${HANDHELD}/raknet/SHA1.cpp
  ${HANDHELD}/raknet/PacketizedTCP.cpp
  ${HANDHELD}/raknet/RakNetTypes.cpp
  ${HANDHELD}/raknet/RakNetSocket.cpp
  ${HANDHELD}/raknet/PluginInterface2.cpp
  ${HANDHELD}/raknet/Rand.cpp
  ${HANDHELD}/raknet/RakWString.cpp
  ${HANDHELD}/raknet/SimpleMutex.cpp
  ${HANDHELD}/raknet/SocketLayer.cpp
  ${HANDHELD}/raknet/DS_HuffmanEncodingTree.cpp
  ${HANDHELD}/raknet/RakThread.cpp
  ${HANDHELD}/raknet/LinuxStrings.cpp
  ${HANDHELD}/raknet/RakMemoryOverride.cpp
  ${HANDHELD}/raknet/HTTPConnection.cpp
)

target_compile_definitions(raknet PRIVATE
  _RAKNET_SUPPORT_ConnectionGraph2=0
  _RAKNET_SUPPORT_DirectoryDeltaTransfer=0
  _RAKNET_SUPPORT_FileListTransfer=0
  _RAKNET_SUPPORT_FullyConnectedMesh2=0
  _RAKNET_SUPPORT_MessageFilter=0
  _RAKNET_SUPPORT_NatPunchthroughClient=0
  _RAKNET_SUPPORT_NatPunchthroughServer=0
  _RAKNET_SUPPORT_NatTypeDetectionClient=0
  _RAKNET_SUPPORT_NatTypeDetectionServer=0
  _RAKNET_SUPPORT_PacketLogger=0
  _RAKNET_SUPPORT_ReadyEvent=0
  _RAKNET_SUPPORT_ReplicaManager3=0
  _RAKNET_SUPPORT_Router2=0
  _RAKNET_SUPPORT_RPC4Plugin=0
  _RAKNET_SUPPORT_TeamBalancer=0
  _RAKNET_SUPPORT_UDPProxyClient=0
  _RAKNET_SUPPORT_UDPProxyCoordinator=0
  _RAKNET_SUPPORT_UDPProxyServer=0
  _RAKNET_SUPPORT_ConsoleServer=0
  _RAKNET_SUPPORT_RakNetTransport=0
  _RAKNET_SUPPORT_TelnetTransport=0
  _RAKNET_SUPPORT_TCPInterface=0
  _RAKNET_SUPPORT_LogCommandParser=0
  _RAKNET_SUPPORT_RakNetCommandParser=0
  _RAKNET_SUPPORT_EmailSender=0
  _RAKNET_SUPPORT_PacketizedTCP=0
  _RAKNET_SUPPORT_TwoWayAuthentication=0
)

add_executable(${PROJECT_NAME}
  ${HANDHELD}/main.cpp
  ${HANDHELD}/main_nx.h
  ${HANDHELD}/SharedConstants.cpp
  ${HANDHELD}/nbt/Tag.cpp
  ${HANDHELD}/network/ClientSideNetworkHandler.cpp
  ${HANDHELD}/network/Packet.cpp
  ${HANDHELD}/network/command/CommandServer.cpp
  ${HANDHELD}/network/RakNetInstance.cpp
  ${HANDHELD}/network/PHPDirectoryServer2.cpp
  ${HANDHELD}/network/NetEventCallback.cpp
  ${HANDHELD}/network/NATPunchHandler.cpp
  ${HANDHELD}/network/ServerSideNetworkHandler.cpp

  #${HANDHELD}/platform/audio/SoundSystemAL.cpp
  #${HANDHELD}/platform/audio/SoundSystemAL.h

  ${HANDHELD}/platform/CThread.cpp
  ${HANDHELD}/platform/input/Keyboard.cpp
  ${HANDHELD}/platform/input/Mouse.cpp
  ${HANDHELD}/platform/input/Multitouch.cpp
  ${HANDHELD}/platform/input/Controller.cpp
  ${HANDHELD}/platform/time.cpp
  ${HANDHELD}/world/food/SimpleFoodData.cpp
  ${HANDHELD}/world/phys/HitResult.cpp
  ${HANDHELD}/world/inventory/ContainerMenu.cpp
  ${HANDHELD}/world/inventory/FurnaceMenu.cpp
  ${HANDHELD}/world/inventory/BaseContainerMenu.cpp
  ${HANDHELD}/world/inventory/FillingContainer.cpp
  ${HANDHELD}/world/level/pathfinder/Path.cpp
  ${HANDHELD}/world/level/LightUpdate.cpp
  ${HANDHELD}/world/level/levelgen/RandomLevelSource.cpp
  ${HANDHELD}/world/level/levelgen/CanyonFeature.cpp
  ${HANDHELD}/world/level/levelgen/DungeonFeature.cpp
  ${HANDHELD}/world/level/levelgen/feature/Feature.cpp
  ${HANDHELD}/world/level/levelgen/LargeCaveFeature.cpp
  ${HANDHELD}/world/level/levelgen/LargeFeature.cpp
  ${HANDHELD}/world/level/levelgen/synth/Synth.cpp
  ${HANDHELD}/world/level/levelgen/synth/ImprovedNoise.cpp
  ${HANDHELD}/world/level/levelgen/synth/PerlinNoise.cpp
  ${HANDHELD}/world/level/TickNextTickData.cpp
  ${HANDHELD}/world/level/Explosion.cpp
  ${HANDHELD}/world/level/material/Material.cpp
  ${HANDHELD}/world/level/Level.cpp
  ${HANDHELD}/world/level/chunk/LevelChunk.cpp
  ${HANDHELD}/world/level/MobSpawner.cpp
  ${HANDHELD}/world/level/storage/LevelStorageSource.cpp
  ${HANDHELD}/world/level/storage/ExternalFileLevelStorage.cpp
  ${HANDHELD}/world/level/storage/LevelData.cpp
  ${HANDHELD}/world/level/storage/FolderMethods.cpp
  ${HANDHELD}/world/level/storage/ExternalFileLevelStorageSource.cpp
  ${HANDHELD}/world/level/storage/RegionFile.cpp
  ${HANDHELD}/world/level/LightLayer.cpp
  ${HANDHELD}/world/level/tile/StairTile.cpp
  ${HANDHELD}/world/level/tile/FurnaceTile.cpp
  ${HANDHELD}/world/level/tile/DoorTile.cpp
  ${HANDHELD}/world/level/tile/TallGrass.cpp
  ${HANDHELD}/world/level/tile/HeavyTile.cpp
  ${HANDHELD}/world/level/tile/NetherReactor.cpp
  ${HANDHELD}/world/level/tile/StoneSlabTile.cpp
  ${HANDHELD}/world/level/tile/SandTile.cpp
  ${HANDHELD}/world/level/tile/NetherReactorPattern.cpp
  ${HANDHELD}/world/level/tile/CropTile.cpp
  ${HANDHELD}/world/level/tile/Tile.cpp
  ${HANDHELD}/world/level/tile/GrassTile.cpp
  ${HANDHELD}/world/level/tile/Mushroom.cpp
  ${HANDHELD}/world/level/tile/BedTile.cpp
  ${HANDHELD}/world/level/tile/entity/TileEntity.cpp
  ${HANDHELD}/world/level/tile/entity/NetherReactorTileEntity.cpp
  ${HANDHELD}/world/level/tile/entity/SignTileEntity.cpp
  ${HANDHELD}/world/level/tile/entity/FurnaceTileEntity.cpp
  ${HANDHELD}/world/level/tile/entity/ChestTileEntity.cpp
  ${HANDHELD}/world/level/tile/LightGemTile.cpp
  ${HANDHELD}/world/level/tile/EntityTile.cpp
  ${HANDHELD}/world/level/tile/StemTile.cpp
  ${HANDHELD}/world/level/tile/MelonTile.cpp
  ${HANDHELD}/world/level/tile/TrapDoorTile.cpp
  ${HANDHELD}/world/level/tile/ChestTile.cpp
  ${HANDHELD}/world/level/biome/Biome.cpp
  ${HANDHELD}/world/level/biome/BiomeSource.cpp
  ${HANDHELD}/world/level/dimension/Dimension.cpp
  ${HANDHELD}/world/level/Region.cpp
  ${HANDHELD}/world/entity/Entity.cpp
  ${HANDHELD}/world/entity/monster/Creeper.cpp
  ${HANDHELD}/world/entity/monster/Monster.cpp
  ${HANDHELD}/world/entity/monster/PigZombie.cpp
  ${HANDHELD}/world/entity/monster/Spider.cpp
  ${HANDHELD}/world/entity/monster/Skeleton.cpp
  ${HANDHELD}/world/entity/monster/Zombie.cpp
  ${HANDHELD}/world/entity/PathfinderMob.cpp
  ${HANDHELD}/world/entity/MobCategory.cpp
  ${HANDHELD}/world/entity/SynchedEntityData.cpp
  ${HANDHELD}/world/entity/EntityFactory.cpp
  ${HANDHELD}/world/entity/FlyingMob.cpp
  ${HANDHELD}/world/entity/Mob.cpp
  ${HANDHELD}/world/entity/player/Player.cpp
  ${HANDHELD}/world/entity/player/Inventory.cpp
  ${HANDHELD}/world/entity/projectile/Arrow.cpp
  ${HANDHELD}/world/entity/projectile/Throwable.cpp
  ${HANDHELD}/world/entity/animal/Chicken.cpp
  ${HANDHELD}/world/entity/animal/Sheep.cpp
  ${HANDHELD}/world/entity/animal/Animal.cpp
  ${HANDHELD}/world/entity/animal/Pig.cpp
  ${HANDHELD}/world/entity/animal/Cow.cpp
  ${HANDHELD}/world/entity/animal/WaterAnimal.cpp
  ${HANDHELD}/world/entity/HangingEntity.cpp
  ${HANDHELD}/world/entity/ai/control/MoveControl.cpp
  ${HANDHELD}/world/entity/Painting.cpp
  ${HANDHELD}/world/entity/item/PrimedTnt.cpp
  ${HANDHELD}/world/entity/item/FallingTile.cpp
  ${HANDHELD}/world/entity/item/TripodCamera.cpp
  ${HANDHELD}/world/entity/item/ItemEntity.cpp
  ${HANDHELD}/world/entity/Motive.cpp
  ${HANDHELD}/world/entity/AgableMob.cpp
  ${HANDHELD}/world/item/ArmorItem.cpp
  ${HANDHELD}/world/item/HangingEntityItem.cpp
  ${HANDHELD}/world/item/PickaxeItem.cpp
  ${HANDHELD}/world/item/DyePowderItem.cpp
  ${HANDHELD}/world/item/BedItem.cpp
  ${HANDHELD}/world/item/crafting/ToolRecipes.cpp
  ${HANDHELD}/world/item/crafting/StructureRecipes.cpp
  ${HANDHELD}/world/item/crafting/FurnaceRecipes.cpp
  ${HANDHELD}/world/item/crafting/Recipes.cpp
  ${HANDHELD}/world/item/crafting/ArmorRecipes.cpp
  ${HANDHELD}/world/item/crafting/Recipe.cpp
  ${HANDHELD}/world/item/crafting/OreRecipes.cpp
  ${HANDHELD}/world/item/crafting/WeaponRecipes.cpp
  ${HANDHELD}/world/item/Item.cpp
  ${HANDHELD}/world/item/ShovelItem.cpp
  ${HANDHELD}/world/item/ItemInstance.cpp
  ${HANDHELD}/world/item/HatchetItem.cpp
  ${HANDHELD}/world/item/HoeItem.cpp
  ${HANDHELD}/world/Direction.cpp
  ${HANDHELD}/NinecraftApp.cpp
  ${HANDHELD}/util/DataIO.cpp
  ${HANDHELD}/util/PerfTimer.cpp
  ${HANDHELD}/util/StringUtils.cpp
  ${HANDHELD}/util/Mth.cpp
  ${HANDHELD}/util/PerfRenderer.cpp
  ${HANDHELD}/server/ServerPlayer.cpp
  ${HANDHELD}/server/CreatorLevel.cpp
  ${HANDHELD}/server/ArgumentsSettings.cpp
  ${HANDHELD}/server/ServerLevel.cpp
  ${HANDHELD}/Performance.cpp
  ${HANDHELD}/client/Options.cpp
  ${HANDHELD}/client/Minecraft.cpp
  ${HANDHELD}/client/model/PigModel.cpp
  ${HANDHELD}/client/model/ChickenModel.cpp
  ${HANDHELD}/client/model/HumanoidModel.cpp
  ${HANDHELD}/client/model/SheepModel.cpp
  ${HANDHELD}/client/model/SheepFurModel.cpp
  ${HANDHELD}/client/model/CowModel.cpp
  ${HANDHELD}/client/model/geom/Cube.cpp
  ${HANDHELD}/client/model/geom/Polygon.cpp
  ${HANDHELD}/client/model/geom/ModelPart.cpp
  ${HANDHELD}/client/model/QuadrupedModel.cpp
  ${HANDHELD}/client/renderer/GameRenderer.cpp
  ${HANDHELD}/client/renderer/TileRenderer.cpp
  ${HANDHELD}/client/renderer/ptexture/DynamicTexture.cpp
  ${HANDHELD}/client/renderer/culling/Frustum.cpp
  ${HANDHELD}/client/renderer/gles.cpp
  ${HANDHELD}/client/renderer/LevelRenderer.cpp
  ${HANDHELD}/client/renderer/RenderChunk.cpp
  ${HANDHELD}/client/renderer/Tesselator.cpp
  ${HANDHELD}/client/renderer/entity/ChickenRenderer.cpp
  ${HANDHELD}/client/renderer/entity/EntityRenderer.cpp
  ${HANDHELD}/client/renderer/entity/PlayerRenderer.cpp
  ${HANDHELD}/client/renderer/entity/PaintingRenderer.cpp
  ${HANDHELD}/client/renderer/entity/ItemRenderer.cpp
  ${HANDHELD}/client/renderer/entity/FallingTileRenderer.cpp
  ${HANDHELD}/client/renderer/entity/ArrowRenderer.cpp
  ${HANDHELD}/client/renderer/entity/SheepRenderer.cpp
  ${HANDHELD}/client/renderer/entity/TntRenderer.cpp
  ${HANDHELD}/client/renderer/entity/MobRenderer.cpp
  ${HANDHELD}/client/renderer/entity/EntityRenderDispatcher.cpp
  ${HANDHELD}/client/renderer/entity/HumanoidMobRenderer.cpp
  ${HANDHELD}/client/renderer/entity/ItemSpriteRenderer.cpp
  ${HANDHELD}/client/renderer/entity/TripodCameraRenderer.cpp
  ${HANDHELD}/client/renderer/Chunk.cpp
  ${HANDHELD}/client/renderer/ItemInHandRenderer.cpp
  ${HANDHELD}/client/renderer/RenderList.cpp
  ${HANDHELD}/client/renderer/Textures.cpp
  ${HANDHELD}/client/renderer/tileentity/SignRenderer.cpp
  ${HANDHELD}/client/renderer/tileentity/ChestRenderer.cpp
  ${HANDHELD}/client/renderer/tileentity/TileEntityRenderer.cpp
  ${HANDHELD}/client/renderer/tileentity/TileEntityRenderDispatcher.cpp
  ${HANDHELD}/client/renderer/EntityTileRenderer.cpp
  ${HANDHELD}/client/MouseHandler.cpp
  ${HANDHELD}/client/player/input/KeyboardInput.cpp
  ${HANDHELD}/client/player/input/touchscreen/TouchscreenInput.cpp
  ${HANDHELD}/client/player/RemotePlayer.cpp
  ${HANDHELD}/client/player/LocalPlayer.cpp
  ${HANDHELD}/client/OptionsFile.cpp
  ${HANDHELD}/client/OptionStrings.cpp
  ${HANDHELD}/client/sound/SoundEngine.cpp
  ${HANDHELD}/client/sound/Sound.cpp
  ${HANDHELD}/client/gamemode/CreativeMode.cpp
  ${HANDHELD}/client/gamemode/SurvivalMode.cpp
  ${HANDHELD}/client/gamemode/GameMode.cpp
  ${HANDHELD}/client/gamemode/CreatorMode.cpp
  ${HANDHELD}/client/particle/ParticleEngine.cpp
  ${HANDHELD}/client/particle/Particle.cpp
  ${HANDHELD}/client/gui/Screen.cpp
  ${HANDHELD}/client/gui/GuiComponent.cpp
  ${HANDHELD}/client/gui/Gui.cpp
  ${HANDHELD}/client/gui/Font.cpp
  ${HANDHELD}/client/gui/components/OptionsPane.cpp
  ${HANDHELD}/client/gui/components/ScrolledSelectionList.cpp
  ${HANDHELD}/client/gui/components/InventoryPane.cpp
  ${HANDHELD}/client/gui/components/GButton.cpp
  ${HANDHELD}/client/gui/components/TextBox.cpp
  ${HANDHELD}/client/gui/components/ImageButton.cpp
  ${HANDHELD}/client/gui/components/SmallButton.cpp
  ${HANDHELD}/client/gui/components/OptionsGroup.cpp
  ${HANDHELD}/client/gui/components/RolledSelectionListV.cpp
  ${HANDHELD}/client/gui/components/LargeImageButton.cpp
  ${HANDHELD}/client/gui/components/GuiElementContainer.cpp
  ${HANDHELD}/client/gui/components/GuiElement.cpp
  ${HANDHELD}/client/gui/components/RolledSelectionListH.cpp
  ${HANDHELD}/client/gui/components/ScrollingPane.cpp
  ${HANDHELD}/client/gui/components/Slider.cpp
  ${HANDHELD}/client/gui/components/Button.cpp
  ${HANDHELD}/client/gui/components/OptionsItem.cpp
  ${HANDHELD}/client/gui/components/ItemPane.cpp
  ${HANDHELD}/client/gui/components/NinePatch.cpp
  ${HANDHELD}/client/gui/screens/StartMenuScreen.cpp
  ${HANDHELD}/client/gui/screens/InBedScreen.cpp
  ${HANDHELD}/client/gui/screens/IngameBlockSelectionScreen.cpp
  ${HANDHELD}/client/gui/screens/RenameMPLevelScreen.cpp
  ${HANDHELD}/client/gui/screens/touch/TouchJoinGameScreen.cpp
  ${HANDHELD}/client/gui/screens/touch/TouchSelectWorldScreen.cpp
  ${HANDHELD}/client/gui/screens/touch/TouchStartMenuScreen.cpp
  ${HANDHELD}/client/gui/screens/touch/TouchCreateWorldScreen.cpp
  ${HANDHELD}/client/gui/screens/touch/TouchIngameBlockSelectionScreen.cpp
  ${HANDHELD}/client/gui/screens/UploadPhotoScreen.cpp
  ${HANDHELD}/client/gui/screens/ArmorScreen.cpp
  ${HANDHELD}/client/gui/screens/SimpleChooseLevelScreen.cpp
  ${HANDHELD}/client/gui/screens/ChestScreen.cpp
  ${HANDHELD}/client/gui/screens/ConfirmScreen.cpp
  ${HANDHELD}/client/gui/screens/TextEditScreen.cpp
  ${HANDHELD}/client/gui/screens/ScreenChooser.cpp
  ${HANDHELD}/client/gui/screens/crafting/CraftingFilters.cpp
  ${HANDHELD}/client/gui/screens/crafting/StonecutterScreen.cpp
  ${HANDHELD}/client/gui/screens/crafting/WorkbenchScreen.cpp
  ${HANDHELD}/client/gui/screens/crafting/PaneCraftingScreen.cpp
  ${HANDHELD}/client/gui/screens/FurnaceScreen.cpp
  ${HANDHELD}/client/gui/screens/ChatScreen.cpp
  ${HANDHELD}/client/gui/screens/OptionsScreen.cpp
  ${HANDHELD}/client/gui/screens/ProgressScreen.cpp
  ${HANDHELD}/client/gui/screens/DeathScreen.cpp
  ${HANDHELD}/client/gui/screens/ChooseLevelScreen.cpp
  ${HANDHELD}/client/gui/screens/SelectWorldScreen.cpp
  ${HANDHELD}/client/gui/screens/JoinGameScreen.cpp
  ${HANDHELD}/client/gui/screens/PauseScreen.cpp
  ${HANDHELD}/client/IConfigListener.cpp
  ${HANDHELD}/locale/I18n.cpp
        ../../src/main_nx.h
        ../../src/AppPlatform_nx.h
        ../../src/client/player/input/NxInput.h
        ../../src/main_ctr.h
        ../../src/AppPlatform_ctr.h
        ../../src/client/renderer/glctr.h
        ../../src/client/renderer/glctr.h
        ../../src/client/renderer/glctr.cpp
        #../../src/platform/audio/SoundSystemNx.cpp
        #../../src/platform/audio/SoundSystemNx.h
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/include
  $ENV{DEVKITPRO}/portlibs/3ds/include
)

target_link_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/lib
  $ENV{DEVKITPRO}/portlibs/3ds/lib
)

target_link_libraries(${PROJECT_NAME} PUBLIC
  #GLESv1
  #GL
  #picaGL
  GLASS
  kazmath
  kygx
  rip
  ctru_old
  citro3d
  citro2d 
  png
  z
  m
  raknet
)

set(ROMFS_STAGING_DIR "${CMAKE_CURRENT_BINARY_DIR}/romfs_staging")

add_custom_target(prepare_romfs ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${ROMFS_STAGING_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_DIR}/images ${ROMFS_STAGING_DIR}/images
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_DIR}/fonts ${ROMFS_STAGING_DIR}/fonts
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_DIR}/lang ${ROMFS_STAGING_DIR}/lang
)
add_dependencies(${PROJECT_NAME} prepare_romfs)

ctr_generate_smdh(${PROJECT_NAME}.smdh
  NAME ${CTR_APP_NAME}
  DESCRIPTION ${CTR_APP_DESC}
  AUTHOR ${CTR_APP_AUTHOR}
  #ICON ${CTR_APP_ICON}
)

ctr_create_3dsx(${PROJECT_NAME}
  #SMDH ${PROJECT_NAME}.smdh
  #ROMFS ${ROMFS_STAGING_DIR}
)