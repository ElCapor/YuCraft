cmake_minimum_required(VERSION 3.20)

# ── Paths ──────────────────────────────────────────────────────────────────
# This file lives at  handheld/project/linux/CMakeLists.txt
# Source root is      handheld/src/
# RakNet sources are  handheld/project/lib_projects/raknet/jni/RaknetSources/

set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../src")
set(RAKNET_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/../lib_projects/raknet/jni/RaknetSources")

# ── Includes ───────────────────────────────────────────────────────────────
include("${CMAKE_SOURCE_DIR}/cmake/CompilerFlags.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/FindDependencies.cmake")

# ── RakNet static library ──────────────────────────────────────────────────
# Build raknet WITHOUT the -m32 flag that the vendored CMakeLists.txt forces.
file(GLOB RAKNET_SOURCES "${RAKNET_SRC}/*.cpp")

add_library(raknet STATIC ${RAKNET_SOURCES})
target_include_directories(raknet PUBLIC "${RAKNET_SRC}")
target_compile_options(raknet PRIVATE
    -w              # suppress all warnings (legacy code)
    -fpermissive    # downgrade ill-formed narrowing to warning (C++11 braced-init)
)
target_compile_definitions(raknet PRIVATE
    _LINUX
    LINUX
)

# ── Minecraft client sources ───────────────────────────────────────────────
# Derived from handheld/project/raspberry/Makefile with these changes:
#   • Removed main_rpi.cpp  (main_linux.h is included by main.cpp via #ifdef LINUX)
#   • Replaced gles.cpp with gl.cpp  (desktop GL uses glGenBuffers, not fake IDs)
#   • Added AppPlatform_linux.cpp  (new Linux AppPlatform implementation)

set(MCPE_SOURCES
    "${SRC}/main.cpp"
    "${SRC}/AppPlatform_linux.cpp"
    "${SRC}/platform/input/Controller.cpp"
    "${SRC}/platform/input/Keyboard.cpp"
    "${SRC}/platform/input/Mouse.cpp"
    "${SRC}/platform/input/Multitouch.cpp"
    "${SRC}/platform/time.cpp"
    "${SRC}/platform/CThread.cpp"
    "${SRC}/NinecraftApp.cpp"
    "${SRC}/Performance.cpp"
    "${SRC}/SharedConstants.cpp"
    "${SRC}/client/IConfigListener.cpp"
    "${SRC}/client/Minecraft.cpp"
    "${SRC}/client/Options.cpp"
    "${SRC}/client/OptionStrings.cpp"
    "${SRC}/client/OptionsFile.cpp"
    "${SRC}/client/MouseHandler.cpp"
    "${SRC}/client/gamemode/CreatorMode.cpp"
    "${SRC}/client/gamemode/GameMode.cpp"
    "${SRC}/client/gamemode/CreativeMode.cpp"
    "${SRC}/client/gamemode/SurvivalMode.cpp"
    "${SRC}/client/gui/components/Button.cpp"
    "${SRC}/client/gui/components/ImageButton.cpp"
    "${SRC}/client/gui/components/GuiElement.cpp"
    "${SRC}/client/gui/components/GuiElementContainer.cpp"
    "${SRC}/client/gui/components/ItemPane.cpp"
    "${SRC}/client/gui/components/InventoryPane.cpp"
    "${SRC}/client/gui/components/LargeImageButton.cpp"
    "${SRC}/client/gui/components/NinePatch.cpp"
    "${SRC}/client/gui/components/RolledSelectionListH.cpp"
    "${SRC}/client/gui/components/RolledSelectionListV.cpp"
    "${SRC}/client/gui/components/ScrolledSelectionList.cpp"
    "${SRC}/client/gui/components/ScrollingPane.cpp"
    "${SRC}/client/gui/components/SmallButton.cpp"
    "${SRC}/client/gui/components/Slider.cpp"
    "${SRC}/client/gui/components/OptionsItem.cpp"
    "${SRC}/client/gui/components/OptionsGroup.cpp"
    "${SRC}/client/gui/components/OptionsPane.cpp"
    "${SRC}/client/gui/Font.cpp"
    "${SRC}/client/gui/Gui.cpp"
    "${SRC}/client/gui/GuiComponent.cpp"
    "${SRC}/client/gui/Screen.cpp"
    "${SRC}/client/gui/screens/ScreenChooser.cpp"
    "${SRC}/client/gui/screens/ArmorScreen.cpp"
    "${SRC}/client/gui/screens/ChatScreen.cpp"
    "${SRC}/client/gui/screens/ConfirmScreen.cpp"
    "${SRC}/client/gui/screens/ChestScreen.cpp"
    "${SRC}/client/gui/screens/crafting/PaneCraftingScreen.cpp"
    "${SRC}/client/gui/screens/crafting/StonecutterScreen.cpp"
    "${SRC}/client/gui/screens/crafting/WorkbenchScreen.cpp"
    "${SRC}/client/gui/screens/crafting/CraftingFilters.cpp"
    "${SRC}/client/gui/screens/DeathScreen.cpp"
    "${SRC}/client/gui/screens/ChooseLevelScreen.cpp"
    "${SRC}/client/gui/screens/SimpleChooseLevelScreen.cpp"
    "${SRC}/client/gui/screens/FurnaceScreen.cpp"
    "${SRC}/client/gui/screens/InBedScreen.cpp"
    "${SRC}/client/gui/screens/IngameBlockSelectionScreen.cpp"
    "${SRC}/client/gui/screens/JoinGameScreen.cpp"
    "${SRC}/client/gui/screens/OptionsScreen.cpp"
    "${SRC}/client/gui/screens/PauseScreen.cpp"
    "${SRC}/client/gui/screens/ProgressScreen.cpp"
    "${SRC}/client/gui/screens/RenameMPLevelScreen.cpp"
    "${SRC}/client/gui/screens/SelectWorldScreen.cpp"
    "${SRC}/client/gui/screens/StartMenuScreen.cpp"
    "${SRC}/client/gui/screens/TextEditScreen.cpp"
    "${SRC}/client/gui/screens/touch/TouchIngameBlockSelectionScreen.cpp"
    "${SRC}/client/gui/screens/touch/TouchJoinGameScreen.cpp"
    "${SRC}/client/gui/screens/touch/TouchSelectWorldScreen.cpp"
    "${SRC}/client/gui/screens/touch/TouchStartMenuScreen.cpp"
    "${SRC}/client/gui/screens/UploadPhotoScreen.cpp"
    "${SRC}/client/model/ChickenModel.cpp"
    "${SRC}/client/model/CowModel.cpp"
    "${SRC}/client/model/HumanoidModel.cpp"
    "${SRC}/client/model/PigModel.cpp"
    "${SRC}/client/model/QuadrupedModel.cpp"
    "${SRC}/client/model/SheepModel.cpp"
    "${SRC}/client/model/SheepFurModel.cpp"
    "${SRC}/client/model/geom/Cube.cpp"
    "${SRC}/client/model/geom/ModelPart.cpp"
    "${SRC}/client/model/geom/Polygon.cpp"
    "${SRC}/client/particle/Particle.cpp"
    "${SRC}/client/particle/ParticleEngine.cpp"
    "${SRC}/client/player/LocalPlayer.cpp"
    "${SRC}/client/player/RemotePlayer.cpp"
    "${SRC}/client/player/input/KeyboardInput.cpp"
    "${SRC}/client/player/input/touchscreen/TouchscreenInput.cpp"
    "${SRC}/client/renderer/Chunk.cpp"
    "${SRC}/client/renderer/EntityTileRenderer.cpp"
    "${SRC}/client/renderer/GameRenderer.cpp"
    "${SRC}/client/renderer/ItemInHandRenderer.cpp"
    "${SRC}/client/renderer/LevelRenderer.cpp"
    "${SRC}/client/renderer/RenderChunk.cpp"
    "${SRC}/client/renderer/RenderList.cpp"
    "${SRC}/client/renderer/Tesselator.cpp"
    "${SRC}/client/renderer/Textures.cpp"
    "${SRC}/client/renderer/TileRenderer.cpp"
    "${SRC}/client/renderer/gl.cpp"           # Desktop GL (not gles.cpp)
    "${SRC}/client/renderer/culling/Frustum.cpp"
    "${SRC}/client/renderer/entity/ArrowRenderer.cpp"
    "${SRC}/client/renderer/entity/ChickenRenderer.cpp"
    "${SRC}/client/renderer/entity/EntityRenderDispatcher.cpp"
    "${SRC}/client/renderer/entity/EntityRenderer.cpp"
    "${SRC}/client/renderer/entity/FallingTileRenderer.cpp"
    "${SRC}/client/renderer/entity/HumanoidMobRenderer.cpp"
    "${SRC}/client/renderer/entity/ItemRenderer.cpp"
    "${SRC}/client/renderer/entity/ItemSpriteRenderer.cpp"
    "${SRC}/client/renderer/entity/MobRenderer.cpp"
    "${SRC}/client/renderer/entity/PaintingRenderer.cpp"
    "${SRC}/client/renderer/entity/PlayerRenderer.cpp"
    "${SRC}/client/renderer/entity/SheepRenderer.cpp"
    "${SRC}/client/renderer/entity/TntRenderer.cpp"
    "${SRC}/client/renderer/entity/TripodCameraRenderer.cpp"
    "${SRC}/client/renderer/ptexture/DynamicTexture.cpp"
    "${SRC}/client/renderer/tileentity/ChestRenderer.cpp"
    "${SRC}/client/renderer/tileentity/SignRenderer.cpp"
    "${SRC}/client/renderer/tileentity/TileEntityRenderDispatcher.cpp"
    "${SRC}/client/renderer/tileentity/TileEntityRenderer.cpp"
    "${SRC}/client/sound/Sound.cpp"
    "${SRC}/client/sound/SoundEngine.cpp"
    "${SRC}/locale/I18n.cpp"
    "${SRC}/nbt/Tag.cpp"
    "${SRC}/network/command/CommandServer.cpp"
    "${SRC}/network/ClientSideNetworkHandler.cpp"
    "${SRC}/network/NetEventCallback.cpp"
    "${SRC}/network/Packet.cpp"
    "${SRC}/network/RakNetInstance.cpp"
    "${SRC}/network/ServerSideNetworkHandler.cpp"
    "${SRC}/server/CreatorLevel.cpp"
    "${SRC}/server/ServerLevel.cpp"
    "${SRC}/server/ServerPlayer.cpp"
    "${SRC}/util/DataIO.cpp"
    "${SRC}/util/Mth.cpp"
    "${SRC}/util/StringUtils.cpp"
    "${SRC}/util/PerfTimer.cpp"
    "${SRC}/util/PerfRenderer.cpp"
    "${SRC}/world/Direction.cpp"
    "${SRC}/world/entity/AgableMob.cpp"
    "${SRC}/world/entity/Entity.cpp"
    "${SRC}/world/entity/EntityFactory.cpp"
    "${SRC}/world/entity/FlyingMob.cpp"
    "${SRC}/world/entity/HangingEntity.cpp"
    "${SRC}/world/entity/Mob.cpp"
    "${SRC}/world/entity/MobCategory.cpp"
    "${SRC}/world/entity/Motive.cpp"
    "${SRC}/world/entity/Painting.cpp"
    "${SRC}/world/entity/PathfinderMob.cpp"
    "${SRC}/world/entity/SynchedEntityData.cpp"
    "${SRC}/world/entity/ai/control/MoveControl.cpp"
    "${SRC}/world/entity/animal/Animal.cpp"
    "${SRC}/world/entity/animal/Chicken.cpp"
    "${SRC}/world/entity/animal/Cow.cpp"
    "${SRC}/world/entity/animal/Pig.cpp"
    "${SRC}/world/entity/animal/Sheep.cpp"
    "${SRC}/world/entity/animal/WaterAnimal.cpp"
    "${SRC}/world/entity/item/FallingTile.cpp"
    "${SRC}/world/entity/item/ItemEntity.cpp"
    "${SRC}/world/entity/item/PrimedTnt.cpp"
    "${SRC}/world/entity/item/TripodCamera.cpp"
    "${SRC}/world/entity/monster/Creeper.cpp"
    "${SRC}/world/entity/monster/Monster.cpp"
    "${SRC}/world/entity/monster/PigZombie.cpp"
    "${SRC}/world/entity/monster/Skeleton.cpp"
    "${SRC}/world/entity/monster/Spider.cpp"
    "${SRC}/world/entity/monster/Zombie.cpp"
    "${SRC}/world/entity/projectile/Arrow.cpp"
    "${SRC}/world/entity/projectile/Throwable.cpp"
    "${SRC}/world/entity/player/Inventory.cpp"
    "${SRC}/world/entity/player/Player.cpp"
    "${SRC}/world/food/SimpleFoodData.cpp"
    "${SRC}/world/inventory/BaseContainerMenu.cpp"
    "${SRC}/world/inventory/ContainerMenu.cpp"
    "${SRC}/world/inventory/FillingContainer.cpp"
    "${SRC}/world/inventory/FurnaceMenu.cpp"
    "${SRC}/world/item/ArmorItem.cpp"
    "${SRC}/world/item/BedItem.cpp"
    "${SRC}/world/item/DyePowderItem.cpp"
    "${SRC}/world/item/Item.cpp"
    "${SRC}/world/item/ItemInstance.cpp"
    "${SRC}/world/item/HangingEntityItem.cpp"
    "${SRC}/world/item/HatchetItem.cpp"
    "${SRC}/world/item/HoeItem.cpp"
    "${SRC}/world/item/PickaxeItem.cpp"
    "${SRC}/world/item/ShovelItem.cpp"
    "${SRC}/world/item/crafting/Recipe.cpp"
    "${SRC}/world/item/crafting/ArmorRecipes.cpp"
    "${SRC}/world/item/crafting/Recipes.cpp"
    "${SRC}/world/item/crafting/FurnaceRecipes.cpp"
    "${SRC}/world/item/crafting/OreRecipes.cpp"
    "${SRC}/world/item/crafting/StructureRecipes.cpp"
    "${SRC}/world/item/crafting/ToolRecipes.cpp"
    "${SRC}/world/item/crafting/WeaponRecipes.cpp"
    "${SRC}/world/level/Explosion.cpp"
    "${SRC}/world/level/Level.cpp"
    "${SRC}/world/level/LightLayer.cpp"
    "${SRC}/world/level/LightUpdate.cpp"
    "${SRC}/world/level/MobSpawner.cpp"
    "${SRC}/world/level/Region.cpp"
    "${SRC}/world/level/TickNextTickData.cpp"
    "${SRC}/world/level/biome/Biome.cpp"
    "${SRC}/world/level/biome/BiomeSource.cpp"
    "${SRC}/world/level/chunk/LevelChunk.cpp"
    "${SRC}/world/level/dimension/Dimension.cpp"
    "${SRC}/world/level/levelgen/CanyonFeature.cpp"
    "${SRC}/world/level/levelgen/DungeonFeature.cpp"
    "${SRC}/world/level/levelgen/LargeCaveFeature.cpp"
    "${SRC}/world/level/levelgen/LargeFeature.cpp"
    "${SRC}/world/level/levelgen/RandomLevelSource.cpp"
    "${SRC}/world/level/levelgen/feature/Feature.cpp"
    "${SRC}/world/level/levelgen/synth/ImprovedNoise.cpp"
    "${SRC}/world/level/levelgen/synth/PerlinNoise.cpp"
    "${SRC}/world/level/levelgen/synth/Synth.cpp"
    "${SRC}/world/level/material/Material.cpp"
    "${SRC}/world/level/pathfinder/Path.cpp"
    "${SRC}/world/level/storage/ExternalFileLevelStorage.cpp"
    "${SRC}/world/level/storage/ExternalFileLevelStorageSource.cpp"
    "${SRC}/world/level/storage/FolderMethods.cpp"
    "${SRC}/world/level/storage/LevelData.cpp"
    "${SRC}/world/level/storage/LevelStorageSource.cpp"
    "${SRC}/world/level/storage/RegionFile.cpp"
    "${SRC}/world/level/tile/BedTile.cpp"
    "${SRC}/world/level/tile/ChestTile.cpp"
    "${SRC}/world/level/tile/CropTile.cpp"
    "${SRC}/world/level/tile/DoorTile.cpp"
    "${SRC}/world/level/tile/EntityTile.cpp"
    "${SRC}/world/level/tile/FurnaceTile.cpp"
    "${SRC}/world/level/tile/GrassTile.cpp"
    "${SRC}/world/level/tile/HeavyTile.cpp"
    "${SRC}/world/level/tile/LightGemTile.cpp"
    "${SRC}/world/level/tile/MelonTile.cpp"
    "${SRC}/world/level/tile/Mushroom.cpp"
    "${SRC}/world/level/tile/NetherReactor.cpp"
    "${SRC}/world/level/tile/NetherReactorPattern.cpp"
    "${SRC}/world/level/tile/StairTile.cpp"
    "${SRC}/world/level/tile/StemTile.cpp"
    "${SRC}/world/level/tile/StoneSlabTile.cpp"
    "${SRC}/world/level/tile/TallGrass.cpp"
    "${SRC}/world/level/tile/Tile.cpp"
    "${SRC}/world/level/tile/TrapDoorTile.cpp"
    "${SRC}/world/level/tile/entity/ChestTileEntity.cpp"
    "${SRC}/world/level/tile/entity/NetherReactorTileEntity.cpp"
    "${SRC}/world/level/tile/entity/SignTileEntity.cpp"
    "${SRC}/world/level/tile/entity/TileEntity.cpp"
    "${SRC}/world/level/tile/entity/FurnaceTileEntity.cpp"
    "${SRC}/world/phys/HitResult.cpp"
)

# ── Executable ────────────────────────────────────────────────────────────
add_executable(minecraft-pe ${MCPE_SOURCES})

# ── Include directories ────────────────────────────────────────────────────
target_include_directories(minecraft-pe PRIVATE
    "${SRC}"
    "${SRC}/client"              # for #include "Minecraft.h" style includes
    "${RAKNET_SRC}"
    # NOTE: do NOT add handheld/lib/include here — it contains libpng 1.2.37
    # headers that conflict with the system 1.6.x ABI (png_struct layout
    # changed, corrupting row reads). System headers arrive via PNG::PNG and
    # ZLIB::ZLIB imported targets in target_link_libraries below.
)

# ── Compile definitions ────────────────────────────────────────────────────
target_compile_definitions(minecraft-pe PRIVATE
    LINUX               # triggers #include "main_linux.h" in main.cpp
    NO_EGL              # SDL2 owns the GL context; App::swapBuffers() is no-op
    USE_VBO             # enable VBO rendering path in gl.cpp / gles.h
    POSIX               # generic POSIX flag used in platform/*.cpp
    _LINUX              # used by some RakNet headers
)

# ── Link libraries ─────────────────────────────────────────────────────────
# SDL2 target name may vary by SDL2 CMake config version.
if(TARGET SDL2::SDL2)
    set(SDL2_TARGET SDL2::SDL2)
elseif(TARGET SDL2::SDL2-static)
    set(SDL2_TARGET SDL2::SDL2-static)
else()
    set(SDL2_TARGET PkgConfig::SDL2)
endif()

target_link_libraries(minecraft-pe PRIVATE
    raknet
    ${SDL2_TARGET}
    OpenGL::GL
    PkgConfig::epoxy
    OpenAL::OpenAL
    PNG::PNG
    ZLIB::ZLIB
    Threads::Threads
    ${CMAKE_DL_LIBS}
)

# ── Compiler flags ─────────────────────────────────────────────────────────
mcpe_set_compiler_flags(minecraft-pe)

# ── Post-build: copy game data next to the binary ─────────────────────────
add_custom_command(TARGET minecraft-pe POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo
        "-- Copying game data to $<TARGET_FILE_DIR:minecraft-pe>/data/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/handheld/data"
        "$<TARGET_FILE_DIR:minecraft-pe>/data"
    COMMENT "Copying game assets"
)

# ── Install rules ─────────────────────────────────────────────────────────
install(TARGETS minecraft-pe DESTINATION bin)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/handheld/data/"
        DESTINATION share/minecraft-pe/data)
