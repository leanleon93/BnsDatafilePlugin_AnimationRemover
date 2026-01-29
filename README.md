# BnS Animation Filter
## A Blade & Soul plugin to hide the skills for selected classes to provide a cleaner screen in combat without hiding party members.

---

## 🚀 **Major Update 3.0: Auto Detect Mode & Enhanced Skill Hiding!** 🚀

> **Auto Detect Mode now automatically identifies your class and hides all skills from other players—including those of the same class—for a cleaner, less cluttered combat view.**  
> _Essential party utilities and buffs (Soulburn, Polarity, Uplift, HMB, Stealth, Projectile Resists, Grabs) remain visible._  
> **This is Enabled by default.** You can deactivate it anytime via the in-game UI.

---

## Ingame Gui
### Auto detect Mode
<img src="./ingame_gui_auto.png" alt="AnimeFilter Ingame GUI" width="800"/>

### Classic Mode
<img src="./ingame_gui.png" alt="AnimeFilter Ingame GUI" width="800"/>

## Features
- (NEO SERVER) Show/hide skills for specific classes.
- (LIVE SERVER) Show/hide skills for specific specializations of classes.
- Preset Profiles
- Switch profiles during gameplay without loading screens.
- Does not hide party buffs or party protection skills. (Soulburn, Polarity, Uplift, HMB, Stealth proc.)
- Does not hide Assassin taxi flower by default. Can be hidden in config.
- Does not hide Warlock TD by default. Can be hidden in config.

### NEO Only Features
- Can completely hide SoulCores in config.
- Does not hide projectile resists by default. Can be hidden in config.
- Does not hide party relevant Grab/Grapple/Fixate animations by default. Can be hidden in config.

### LIVE Only Features
- Does not hide bard tree by default. Can be hidden in config.

## Installation
```
BNSR/
└── Binaries/
    └── Win64/
        ├── plugins/
        │   └── DatafilePluginloader.dll
        └── datafilePlugins/
            └── AnimationFilter.dll
```
- Assumes a basic plugin setup [Bns Unpacked by tomato](https://rentry.co/bns_unpacked)
- Place [ DatafilePluginloader.dll ](https://github.com/leanleon93/BnsPlugin_DatafilePluginloader/releases/latest) in your plugins folder
- Create a datafilePlugins folder next to the plugins folder if it doesnt exist yet
- Place [ AnimationFilter.dll ](https://github.com/leanleon93/BnsDatafilePlugin_AnimationRemover/releases/latest) in your datafilePlugins folder
- Ingame hit the `Insert` key to open the Plugin Ui

## Demo Video
[![Watch the demo](https://img.youtube.com/vi/wuWuUMzoFcw/hqdefault.jpg)](https://www.youtube.com/watch?v=wuWuUMzoFcw)

## Config
- The config file is located at: `Documents/BNS/animfilter_config.xml` or `Documents/BNS/animfilter_config_live.xml`
- On first launch a default config will be created.
- If the Documents directory is not found or is not writable for any reason, the config will silently fall back to `%LOCALAPPDATA%/BnS`
- You can also check the example config file: [animfilter_config.xml](animfilter_config.xml)
- You can add multiple profiles for different scenarios
- The profiles can be edited in-game using the pluginloader gui (insert key)

## Extra options
You can set extra options for a profile.
- Hide Bard tree
- Hide Assassin taxi
- Hide Time Distortion
- Hide Soulcores
- Hide Projectile Resists
- Hide Grab/Grapple/Fixate animations
```xml
<extra_options hideGrabs="true" hideSoulCores="true" hideProjectileResists="true" hideTree="true" hideTaxi="true" hideTD="true" />
```

## Dependencies
- Requires [DatafilePluginloader](https://github.com/leanleon93/BnsPlugin_DatafilePluginloader) to work
