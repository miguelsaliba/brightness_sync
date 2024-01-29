# Brightness_sync

CLI utility to change the brightness of all your monitors at the same time.

You can specify in your config file the min and max of each monitor and it will set the brightness as a percentage of that range.

### Dependencies:
- ddcutil
- toml++
- boost

### Usage:

```
brightness_sync [command] [value]

Commands:
   help                    prints this help page
   up                      increases the brightness by 10%
   down                    decreases the brightness by 10%
   s, set VALUE            sets the brightness to the value
   c, change VALUE         changes the brightness by the value
```
