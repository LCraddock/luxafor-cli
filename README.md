## Luxafor CLI utility

The company [Luxafor](https://luxafor.com) sells a little RBG LED "[flag](https://luxafor.com/flag-usb-busylight-availability-indicator/)" that connects to a PC via USB-A, and I love it.

But I wanted a CLI client. So I wrote one. And you can use it, if you'd like.

## Building

These instructions work on both MacOS and Raspian.

### Dependencies

You will need to install the HID API for USB devices and the CMake build system in order to compile this software.

* For MacOS using Homebrew: `brew install hidapi cmake`

* For Raspbian: `sudo apt install libhidapi-dev cmake`

### Get the source

Download the source code (with submodules) with the following:

`gitclone --recurse-submodules http://github.com/mike-rogers/luxafor-cli.git`

### Build using CMake

From within the `luxafor-cli` directory:

```
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
```

## Run the program

To run, from the build directory, `./luxafor blue`. For some systems you might need to use `sudo ./luxafor blue`.

### Simple Commands

Turn the device on with a solid color:
```
./luxafor red              # Solid red
./luxafor green            # Solid green
./luxafor blue             # Solid blue
./luxafor yellow           # Solid yellow
./luxafor magenta          # Solid magenta
./luxafor cyan             # Solid cyan
./luxafor orange           # Solid orange
./luxafor purple           # Solid purple
./luxafor pink             # Solid pink
./luxafor white            # Solid white
./luxafor off              # Turn off the device
./luxafor 0xFF00FF         # Custom hex color
```

### Advanced Commands

The Luxafor Orb supports several lighting effects:

#### Fade Effect
Smoothly transition to a color:
```
./luxafor fade --color red --speed 20
./luxafor fade --color 0x00FF00 --speed 50 --led front
```

#### Strobe Effect
Blink/flash a color:
```
./luxafor strobe --color blue --speed 10 --repeat 5
./luxafor strobe --color red --speed 100 --repeat 0  # Infinite
```

#### Wave Patterns
Display one of 5 built-in wave effects:
```
./luxafor wave --type 1 --color green --speed 50
./luxafor wave --type 3 --color 0xFF00FF --speed 20 --repeat 3
```

#### Built-in Patterns
Use one of 8 pre-programmed patterns:
```
./luxafor pattern --id 7 --repeat 3
./luxafor pattern --id 1 --repeat 0  # Infinite loop
```

### Options

- `--color COLOR`: Color name (red, green, blue, yellow, magenta, cyan, orange, purple, pink, white, off) or hex value (0xRRGGBB or #RRGGBB)
- `--led LED`: Target LED - `all` (default), `front`, or `back`
- `--speed SPEED`: Effect speed (0-255, lower is faster)
- `--repeat COUNT`: Number of repetitions (0-255, 0 = infinite)
- `--type TYPE`: Wave pattern type (1-5)
- `--id ID`: Built-in pattern ID (1-8)

## License

This project has been released with the MIT license.
