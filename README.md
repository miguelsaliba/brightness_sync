# Brightness_sync

CLI utility to change the brightness of all your monitors at the same time.

You can specify in your config file the min and max of each monitor and the brightness will be set as a percentage of that range.

## Installation

### Dependencies:

- [`ddcutil`](https://www.ddcutil.com/install/)
- `toml++`

First, make sure you can run commands with ddcutil (e.g. `ddcutil setvcp 10 50 --display 1`).

Just run `make` then `./brightness_sync [command] [value]`. See usage below.


## Usage:

```
brightness_sync [command] [value]

Commands:
   help                    prints this help page
   up                      increases the brightness by 10%
   down                    decreases the brightness by 10%
   s, set VALUE            sets the brightness to the value
   c, change VALUE         changes the brightness by the value
```

## Configuration

Place your config file in either `$XDG_CONFIG_HOME/brightness.toml` or `$HOME/.config/brightness.toml`

The configuration should be an [array of tables](https://toml.io/en/v1.0.0#array-of-tables) and should look something like this:

```toml
[[display]]
number=1
min=0 # minimum value the brightness will be set to
max=100

[[display]]
number=2
min=10
max=95
```
