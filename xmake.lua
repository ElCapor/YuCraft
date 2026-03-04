-- xmake.lua — Minecraft PE Alpha 0.6.1
-- Platforms: Linux (GCC/Clang), Windows (MSVC VS2012+)
-- Usage:
--   xmake                          (release build, current platform)
--   xmake f -m debug               (debug build)
--   xmake build
--   xmake run minecraft-pe
--   xmake project -k vsxmake2022   (generate Visual Studio 2022 solution)
--   xmake project -k vsxmake2019   (generate Visual Studio 2019 solution)

set_project("minecraft-pe-alpha")
set_version("0.6.1")
set_languages("cxx17")

-- Build modes
add_rules("mode.debug", "mode.release")

if is_mode("release") then
    add_defines("NDEBUG")
    set_optimize("faster")   -- -O2
elseif is_mode("debug") then
    add_defines("DEBUG")
    set_optimize("none")     -- -O0
    set_symbols("debug")     -- -g
    if is_plat("linux") then
        add_cxflags("-g3", "-fno-omit-frame-pointer", "-fsanitize=address,undefined")
        add_ldflags("-fsanitize=address,undefined")
    end
end

-- Dependencies (Linux: system package manager / pkg-config)
-- Windows uses vendored libs in handheld/lib/ — no add_requires() needed.
if is_plat("linux") then
    add_requires("libsdl2",          {system = true})
    add_requires("openal-soft",      {system = true})
    add_requires("libpng",           {system = true})
    add_requires("zlib",             {system = true})
    -- epoxy via pkg-config (available as 'libepoxy-dev' / 'libepoxy' on all major distros)
    add_requires("pkgconfig::epoxy", {system = true})
end

-- ============================================================
-- Target 1: raknet (static library)
-- Sources differ per platform:
--   Linux   → handheld/project/lib_projects/raknet/jni/RaknetSources/
--   Windows → handheld/src/raknet/ (embedded alongside game source)
-- ============================================================
target("raknet")
    set_kind("static")

    if is_plat("linux") then
        add_files("handheld/project/lib_projects/raknet/jni/RaknetSources/*.cpp")
        add_includedirs(
            "handheld/project/lib_projects/raknet/jni/RaknetSources"
        )
        add_defines("_LINUX", "LINUX")
        -- Suppress all warnings + allow narrowing/permissive code (legacy raknet)
        add_cxflags("-w", "-fpermissive")
    elseif is_plat("windows") then
        add_files("handheld/src/raknet/*.cpp")
        add_includedirs("handheld/src/raknet", "handheld/src")
        add_defines("WIN32", "_CRT_SECURE_NO_WARNINGS")
        -- Suppress all warnings from vendored raknet (MSVC)
        add_cxflags("/W0")
    end
target_end()

-- ============================================================
-- Target 2: minecraft-pe (executable)
-- ============================================================
target("minecraft-pe")
    set_kind("binary")

    -- Link against our local raknet
    add_deps("raknet")

    -- Shared include directories (both platforms)
    add_includedirs("handheld/src", "handheld/src/client")

    -- --------------------------------------------------------
    -- Linux-specific configuration
    -- --------------------------------------------------------
    if is_plat("linux") then
        add_includedirs("handheld/project/lib_projects/raknet/jni/RaknetSources")
        -- NOTE: do NOT include handheld/lib/include — it contains vendored libpng 1.2.37
        --       headers that clash with the system libpng 1.6.x ABI.
        add_packages("libsdl2", "openal-soft", "libpng", "zlib", "pkgconfig::epoxy")
        add_syslinks("GL", "dl", "pthread")
        add_defines("LINUX", "NO_EGL", "USE_VBO", "POSIX", "_LINUX")
        add_cxflags(
            "-Wall", "-Wextra", "-Wpedantic",
            "-Wno-unused-parameter",
            "-Wno-sign-compare",
            "-Wno-deprecated-declarations",
            "-Wno-missing-field-initializers",
            "-fPIC",
            "-Wno-narrowing",
            "-Wno-reorder",
            "-fvisibility=hidden"
        )

    -- --------------------------------------------------------
    -- Windows-specific configuration (VS2012+)
    -- Uses vendored libs/headers matching the original vcxproj:
    --   ../../lib/include  →  handheld/lib/include
    --   ../../lib/lib      →  handheld/lib/lib
    -- Renderer: OpenGL + GLEW (win32_gl variant)
    -- --------------------------------------------------------
    elseif is_plat("windows") then
        add_includedirs("handheld/lib/include", "handheld/src/raknet")
        add_linkdirs("handheld/lib/lib")
        -- Renderer + networking libs
        add_links("libpng", "glew32", "opengl32")
        add_syslinks(
            "ws2_32",
            "kernel32", "user32", "gdi32", "winspool",
            "comdlg32", "advapi32", "shell32",
            "ole32", "oleaut32", "uuid",
            "odbc32", "odbccp32"
        )
        add_defines("WIN32", "_CRT_SECURE_NO_WARNINGS", "_CONSOLE")
        -- /W3 = warning level 3, /MP = multiprocessor compile, /EHsc = C++ exceptions
        add_cxflags("/W3", "/MP", "/EHsc")
    end

    -- --------------------------------------------------------
    -- Source files (explicit — mirrors CMakeLists.txt / vcxproj exactly)
    -- --------------------------------------------------------
    -- Platform entry point + renderer abstraction layer
    if is_plat("linux") then
        add_files(
            "handheld/src/AppPlatform_linux.cpp",
            "handheld/src/client/renderer/gl.cpp",  -- epoxy-based GL wrapper
            "handheld/src/server/CreatorLevel.cpp"  -- not present in Win32 vcxproj
        )
    elseif is_plat("windows") then
        add_files(
            "handheld/src/AppPlatform_win32.cpp",
            "handheld/src/client/renderer/gles.cpp", -- GLES/GLEW-based wrapper
            -- Additional Windows-only network handlers (NAT, PHP directory server)
            "handheld/src/network/NATPunchHandler.cpp",
            "handheld/src/network/PHPDirectoryServer2.cpp"
        )
    end

    add_files(
        -- Entry points / app
        "handheld/src/main.cpp",
        "handheld/src/NinecraftApp.cpp",
        "handheld/src/Performance.cpp",
        "handheld/src/SharedConstants.cpp",

        -- Platform
        "handheld/src/platform/input/Controller.cpp",
        "handheld/src/platform/input/Keyboard.cpp",
        "handheld/src/platform/input/Mouse.cpp",
        "handheld/src/platform/input/Multitouch.cpp",
        "handheld/src/platform/time.cpp",
        "handheld/src/platform/CThread.cpp",

        -- Client core
        "handheld/src/client/IConfigListener.cpp",
        "handheld/src/client/Minecraft.cpp",
        "handheld/src/client/Options.cpp",
        "handheld/src/client/OptionStrings.cpp",
        "handheld/src/client/OptionsFile.cpp",
        "handheld/src/client/MouseHandler.cpp",

        -- Gamemode
        "handheld/src/client/gamemode/CreatorMode.cpp",
        "handheld/src/client/gamemode/GameMode.cpp",
        "handheld/src/client/gamemode/CreativeMode.cpp",
        "handheld/src/client/gamemode/SurvivalMode.cpp",

        -- GUI components
        "handheld/src/client/gui/components/Button.cpp",
        "handheld/src/client/gui/components/ImageButton.cpp",
        "handheld/src/client/gui/components/GuiElement.cpp",
        "handheld/src/client/gui/components/GuiElementContainer.cpp",
        "handheld/src/client/gui/components/ItemPane.cpp",
        "handheld/src/client/gui/components/InventoryPane.cpp",
        "handheld/src/client/gui/components/LargeImageButton.cpp",
        "handheld/src/client/gui/components/NinePatch.cpp",
        "handheld/src/client/gui/components/RolledSelectionListH.cpp",
        "handheld/src/client/gui/components/RolledSelectionListV.cpp",
        "handheld/src/client/gui/components/ScrolledSelectionList.cpp",
        "handheld/src/client/gui/components/ScrollingPane.cpp",
        "handheld/src/client/gui/components/SmallButton.cpp",
        "handheld/src/client/gui/components/Slider.cpp",
        "handheld/src/client/gui/components/OptionsItem.cpp",
        "handheld/src/client/gui/components/OptionsGroup.cpp",
        "handheld/src/client/gui/components/OptionsPane.cpp",

        -- GUI
        "handheld/src/client/gui/Font.cpp",
        "handheld/src/client/gui/Gui.cpp",
        "handheld/src/client/gui/GuiComponent.cpp",
        "handheld/src/client/gui/Screen.cpp",

        -- Screens
        "handheld/src/client/gui/screens/ScreenChooser.cpp",
        "handheld/src/client/gui/screens/ArmorScreen.cpp",
        "handheld/src/client/gui/screens/ChatScreen.cpp",
        "handheld/src/client/gui/screens/ConfirmScreen.cpp",
        "handheld/src/client/gui/screens/ChestScreen.cpp",
        "handheld/src/client/gui/screens/crafting/PaneCraftingScreen.cpp",
        "handheld/src/client/gui/screens/crafting/StonecutterScreen.cpp",
        "handheld/src/client/gui/screens/crafting/WorkbenchScreen.cpp",
        "handheld/src/client/gui/screens/crafting/CraftingFilters.cpp",
        "handheld/src/client/gui/screens/DeathScreen.cpp",
        "handheld/src/client/gui/screens/ChooseLevelScreen.cpp",
        "handheld/src/client/gui/screens/SimpleChooseLevelScreen.cpp",
        "handheld/src/client/gui/screens/FurnaceScreen.cpp",
        "handheld/src/client/gui/screens/InBedScreen.cpp",
        "handheld/src/client/gui/screens/IngameBlockSelectionScreen.cpp",
        "handheld/src/client/gui/screens/JoinGameScreen.cpp",
        "handheld/src/client/gui/screens/OptionsScreen.cpp",
        "handheld/src/client/gui/screens/PauseScreen.cpp",
        "handheld/src/client/gui/screens/ProgressScreen.cpp",
        "handheld/src/client/gui/screens/RenameMPLevelScreen.cpp",
        "handheld/src/client/gui/screens/SelectWorldScreen.cpp",
        "handheld/src/client/gui/screens/StartMenuScreen.cpp",
        "handheld/src/client/gui/screens/TextEditScreen.cpp",
        "handheld/src/client/gui/screens/touch/TouchIngameBlockSelectionScreen.cpp",
        "handheld/src/client/gui/screens/touch/TouchJoinGameScreen.cpp",
        "handheld/src/client/gui/screens/touch/TouchSelectWorldScreen.cpp",
        "handheld/src/client/gui/screens/touch/TouchStartMenuScreen.cpp",
        "handheld/src/client/gui/screens/UploadPhotoScreen.cpp",

        -- Models
        "handheld/src/client/model/ChickenModel.cpp",
        "handheld/src/client/model/CowModel.cpp",
        "handheld/src/client/model/HumanoidModel.cpp",
        "handheld/src/client/model/PigModel.cpp",
        "handheld/src/client/model/QuadrupedModel.cpp",
        "handheld/src/client/model/SheepModel.cpp",
        "handheld/src/client/model/SheepFurModel.cpp",
        "handheld/src/client/model/geom/Cube.cpp",
        "handheld/src/client/model/geom/ModelPart.cpp",
        "handheld/src/client/model/geom/Polygon.cpp",

        -- Particles
        "handheld/src/client/particle/Particle.cpp",
        "handheld/src/client/particle/ParticleEngine.cpp",

        -- Player
        "handheld/src/client/player/LocalPlayer.cpp",
        "handheld/src/client/player/RemotePlayer.cpp",
        "handheld/src/client/player/input/KeyboardInput.cpp",
        "handheld/src/client/player/input/touchscreen/TouchscreenInput.cpp",

        -- Renderer
        "handheld/src/client/renderer/Chunk.cpp",
        "handheld/src/client/renderer/EntityTileRenderer.cpp",
        "handheld/src/client/renderer/GameRenderer.cpp",
        "handheld/src/client/renderer/ItemInHandRenderer.cpp",
        "handheld/src/client/renderer/LevelRenderer.cpp",
        "handheld/src/client/renderer/RenderChunk.cpp",
        "handheld/src/client/renderer/RenderList.cpp",
        "handheld/src/client/renderer/Tesselator.cpp",
        "handheld/src/client/renderer/Textures.cpp",
        "handheld/src/client/renderer/TileRenderer.cpp",
        "handheld/src/client/renderer/culling/Frustum.cpp",

        -- Entity renderers
        "handheld/src/client/renderer/entity/ArrowRenderer.cpp",
        "handheld/src/client/renderer/entity/ChickenRenderer.cpp",
        "handheld/src/client/renderer/entity/EntityRenderDispatcher.cpp",
        "handheld/src/client/renderer/entity/EntityRenderer.cpp",
        "handheld/src/client/renderer/entity/FallingTileRenderer.cpp",
        "handheld/src/client/renderer/entity/HumanoidMobRenderer.cpp",
        "handheld/src/client/renderer/entity/ItemRenderer.cpp",
        "handheld/src/client/renderer/entity/ItemSpriteRenderer.cpp",
        "handheld/src/client/renderer/entity/MobRenderer.cpp",
        "handheld/src/client/renderer/entity/PaintingRenderer.cpp",
        "handheld/src/client/renderer/entity/PlayerRenderer.cpp",
        "handheld/src/client/renderer/entity/SheepRenderer.cpp",
        "handheld/src/client/renderer/entity/TntRenderer.cpp",
        "handheld/src/client/renderer/entity/TripodCameraRenderer.cpp",
        "handheld/src/client/renderer/ptexture/DynamicTexture.cpp",

        -- Tile entity renderers
        "handheld/src/client/renderer/tileentity/ChestRenderer.cpp",
        "handheld/src/client/renderer/tileentity/SignRenderer.cpp",
        "handheld/src/client/renderer/tileentity/TileEntityRenderDispatcher.cpp",
        "handheld/src/client/renderer/tileentity/TileEntityRenderer.cpp",

        -- Sound
        "handheld/src/client/sound/Sound.cpp",
        "handheld/src/client/sound/SoundEngine.cpp",

        -- Locale / NBT / Network
        "handheld/src/locale/I18n.cpp",
        "handheld/src/nbt/Tag.cpp",
        "handheld/src/network/command/CommandServer.cpp",
        "handheld/src/network/ClientSideNetworkHandler.cpp",
        "handheld/src/network/NetEventCallback.cpp",
        "handheld/src/network/Packet.cpp",
        "handheld/src/network/RakNetInstance.cpp",
        "handheld/src/network/ServerSideNetworkHandler.cpp",

        -- Server
        "handheld/src/server/ServerLevel.cpp",
        "handheld/src/server/ServerPlayer.cpp",

        -- Util
        "handheld/src/util/DataIO.cpp",
        "handheld/src/util/Mth.cpp",
        "handheld/src/util/StringUtils.cpp",
        "handheld/src/util/PerfTimer.cpp",
        "handheld/src/util/PerfRenderer.cpp",

        -- World
        "handheld/src/world/Direction.cpp",
        "handheld/src/world/entity/AgableMob.cpp",
        "handheld/src/world/entity/Entity.cpp",
        "handheld/src/world/entity/EntityFactory.cpp",
        "handheld/src/world/entity/FlyingMob.cpp",
        "handheld/src/world/entity/HangingEntity.cpp",
        "handheld/src/world/entity/Mob.cpp",
        "handheld/src/world/entity/MobCategory.cpp",
        "handheld/src/world/entity/Motive.cpp",
        "handheld/src/world/entity/Painting.cpp",
        "handheld/src/world/entity/PathfinderMob.cpp",
        "handheld/src/world/entity/SynchedEntityData.cpp",
        "handheld/src/world/entity/ai/control/MoveControl.cpp",
        "handheld/src/world/entity/animal/Animal.cpp",
        "handheld/src/world/entity/animal/Chicken.cpp",
        "handheld/src/world/entity/animal/Cow.cpp",
        "handheld/src/world/entity/animal/Pig.cpp",
        "handheld/src/world/entity/animal/Sheep.cpp",
        "handheld/src/world/entity/animal/WaterAnimal.cpp",
        "handheld/src/world/entity/item/FallingTile.cpp",
        "handheld/src/world/entity/item/ItemEntity.cpp",
        "handheld/src/world/entity/item/PrimedTnt.cpp",
        "handheld/src/world/entity/item/TripodCamera.cpp",
        "handheld/src/world/entity/monster/Creeper.cpp",
        "handheld/src/world/entity/monster/Monster.cpp",
        "handheld/src/world/entity/monster/PigZombie.cpp",
        "handheld/src/world/entity/monster/Skeleton.cpp",
        "handheld/src/world/entity/monster/Spider.cpp",
        "handheld/src/world/entity/monster/Zombie.cpp",
        "handheld/src/world/entity/projectile/Arrow.cpp",
        "handheld/src/world/entity/projectile/Throwable.cpp",
        "handheld/src/world/entity/player/Inventory.cpp",
        "handheld/src/world/entity/player/Player.cpp",
        "handheld/src/world/food/SimpleFoodData.cpp",
        "handheld/src/world/inventory/BaseContainerMenu.cpp",
        "handheld/src/world/inventory/ContainerMenu.cpp",
        "handheld/src/world/inventory/FillingContainer.cpp",
        "handheld/src/world/inventory/FurnaceMenu.cpp",
        "handheld/src/world/item/ArmorItem.cpp",
        "handheld/src/world/item/BedItem.cpp",
        "handheld/src/world/item/DyePowderItem.cpp",
        "handheld/src/world/item/Item.cpp",
        "handheld/src/world/item/ItemInstance.cpp",
        "handheld/src/world/item/HangingEntityItem.cpp",
        "handheld/src/world/item/HatchetItem.cpp",
        "handheld/src/world/item/HoeItem.cpp",
        "handheld/src/world/item/PickaxeItem.cpp",
        "handheld/src/world/item/ShovelItem.cpp",
        "handheld/src/world/item/crafting/Recipe.cpp",
        "handheld/src/world/item/crafting/ArmorRecipes.cpp",
        "handheld/src/world/item/crafting/Recipes.cpp",
        "handheld/src/world/item/crafting/FurnaceRecipes.cpp",
        "handheld/src/world/item/crafting/OreRecipes.cpp",
        "handheld/src/world/item/crafting/StructureRecipes.cpp",
        "handheld/src/world/item/crafting/ToolRecipes.cpp",
        "handheld/src/world/item/crafting/WeaponRecipes.cpp",
        "handheld/src/world/level/Explosion.cpp",
        "handheld/src/world/level/Level.cpp",
        "handheld/src/world/level/LightLayer.cpp",
        "handheld/src/world/level/LightUpdate.cpp",
        "handheld/src/world/level/MobSpawner.cpp",
        "handheld/src/world/level/Region.cpp",
        "handheld/src/world/level/TickNextTickData.cpp",
        "handheld/src/world/level/biome/Biome.cpp",
        "handheld/src/world/level/biome/BiomeSource.cpp",
        "handheld/src/world/level/chunk/LevelChunk.cpp",
        "handheld/src/world/level/dimension/Dimension.cpp",
        "handheld/src/world/level/levelgen/CanyonFeature.cpp",
        "handheld/src/world/level/levelgen/DungeonFeature.cpp",
        "handheld/src/world/level/levelgen/LargeCaveFeature.cpp",
        "handheld/src/world/level/levelgen/LargeFeature.cpp",
        "handheld/src/world/level/levelgen/RandomLevelSource.cpp",
        "handheld/src/world/level/levelgen/feature/Feature.cpp",
        "handheld/src/world/level/levelgen/synth/ImprovedNoise.cpp",
        "handheld/src/world/level/levelgen/synth/PerlinNoise.cpp",
        "handheld/src/world/level/levelgen/synth/Synth.cpp",
        "handheld/src/world/level/material/Material.cpp",
        "handheld/src/world/level/pathfinder/Path.cpp",
        "handheld/src/world/level/storage/ExternalFileLevelStorage.cpp",
        "handheld/src/world/level/storage/ExternalFileLevelStorageSource.cpp",
        "handheld/src/world/level/storage/FolderMethods.cpp",
        "handheld/src/world/level/storage/LevelData.cpp",
        "handheld/src/world/level/storage/LevelStorageSource.cpp",
        "handheld/src/world/level/storage/RegionFile.cpp",
        "handheld/src/world/level/tile/BedTile.cpp",
        "handheld/src/world/level/tile/ChestTile.cpp",
        "handheld/src/world/level/tile/CropTile.cpp",
        "handheld/src/world/level/tile/DoorTile.cpp",
        "handheld/src/world/level/tile/EntityTile.cpp",
        "handheld/src/world/level/tile/FurnaceTile.cpp",
        "handheld/src/world/level/tile/GrassTile.cpp",
        "handheld/src/world/level/tile/HeavyTile.cpp",
        "handheld/src/world/level/tile/LightGemTile.cpp",
        "handheld/src/world/level/tile/MelonTile.cpp",
        "handheld/src/world/level/tile/Mushroom.cpp",
        "handheld/src/world/level/tile/NetherReactor.cpp",
        "handheld/src/world/level/tile/NetherReactorPattern.cpp",
        "handheld/src/world/level/tile/StairTile.cpp",
        "handheld/src/world/level/tile/StemTile.cpp",
        "handheld/src/world/level/tile/StoneSlabTile.cpp",
        "handheld/src/world/level/tile/TallGrass.cpp",
        "handheld/src/world/level/tile/Tile.cpp",
        "handheld/src/world/level/tile/TrapDoorTile.cpp",
        "handheld/src/world/level/tile/entity/ChestTileEntity.cpp",
        "handheld/src/world/level/tile/entity/NetherReactorTileEntity.cpp",
        "handheld/src/world/level/tile/entity/SignTileEntity.cpp",
        "handheld/src/world/level/tile/entity/TileEntity.cpp",
        "handheld/src/world/level/tile/entity/FurnaceTileEntity.cpp",
        "handheld/src/world/phys/HitResult.cpp"
    )

    -- --------------------------------------------------------
    -- Post-build: copy data assets next to the binary
    -- The game expects to find ./data/ relative to its working directory.
    -- --------------------------------------------------------
    after_build(function(target)
        import("core.project.config")
        local bindir = path.directory(target:targetfile())
        local src    = path.join(os.projectdir(), "handheld", "data")
        local dst    = path.join(bindir, "data")
        os.cp(src, dst)
        print("Copied handheld/data → " .. dst)
    end)

target_end()
