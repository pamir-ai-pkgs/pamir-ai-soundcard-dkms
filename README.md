# Pamir AI Soundcard DKMS Module

This package contains the DKMS (Dynamic Kernel Module Support) modules for the Pamir AI soundcard drivers for Raspberry Pi CM5.

## Overview

The Pamir AI soundcard consists of three main components:

1. **pamir-ai-soundcard** - Main soundcard component driver
2. **pamir-ai-i2c-sound** - I2C driver for TLV320AIC3204 audio codec
3. **pamir-ai-rpi-soundcard** - Standalone RPI soundcard implementation with DAI link

## Features

- **TLV320AIC3204 Audio Codec Support**: Full initialization and control
- **Volume Control**: Hardware volume control (0-100%)
- **Input Gain Control**: Microphone input gain control (0-100%)
- **Register Access**: Direct codec register access via sysfs
- **ALSA Integration**: Standard ALSA sound card interface
- **Device Tree Support**: Configurable via device tree overlays

## Installation

### Prerequisites

- Linux kernel headers for your current kernel
- DKMS package installed
- Root privileges

### Install DKMS Module

```bash
sudo ./install.sh
```

### Enable the Soundcard

1. Add the device tree overlay to `/boot/config.txt`:
   ```
   dtoverlay=pamir-ai-soundcard
   ```

2. Reboot your system

## Usage

### Volume Control

Set volume (0-100%):
```bash
echo 75 | sudo tee /sys/class/i2c-adapter/i2c-*/*/volume_level
```

Get current volume:
```bash
cat /sys/class/i2c-adapter/i2c-*/*/volume_level
```

### Input Gain Control

Set input gain (0-100%):
```bash
echo 60 | sudo tee /sys/class/i2c-adapter/i2c-*/*/input_gain
```

Get current input gain:
```bash
cat /sys/class/i2c-adapter/i2c-*/*/input_gain
```

### Direct Register Access

Write to codec register (page reg value):
```bash
echo "0 65 128" | sudo tee /sys/class/i2c-adapter/i2c-*/*/register_access
```

Read from codec register:
```bash
echo "0 65" | sudo tee /sys/class/i2c-adapter/i2c-*/*/register_access
cat /sys/class/i2c-adapter/i2c-*/*/register_access
```

### ALSA Usage

List available sound cards:
```bash
aplay -l
```

Play audio:
```bash
aplay -D plughw:CARD=snd_pamir_ai_soundcard,DEV=0 test.wav
```

Record audio:
```bash
arecord -D plughw:CARD=snd_pamir_ai_soundcard,DEV=0 -f S16_LE -r 48000 -c 2 recording.wav
```

## Configuration

### Device Tree Properties

The soundcard can be configured via device tree overlay parameters:

```dts
&pamir_ai_sound {
    /* TLV320AIC3204 I2C address (default: 0x18) */
    reg = <0x18>;
    
    /* Enable/disable device */
    status = "okay";
};
```

### Audio Formats Supported

- **Sample Rates**: 48kHz, 96kHz
- **Bit Depths**: 16-bit, 24-bit, 32-bit
- **Channels**: 2 (stereo)
- **Format**: I2S

## Troubleshooting

### Module Not Loading

Check dmesg for errors:
```bash
dmesg | grep -i pamir
```

### No Sound Output

1. Check ALSA mixer settings:
   ```bash
   alsamixer
   ```

2. Verify I2C communication:
   ```bash
   i2cdetect -y 1
   ```

3. Check codec initialization:
   ```bash
   dmesg | grep -i "pamir.*initialization"
   ```

### Volume Issues

- Volume range is 0-100%
- Volume 0 = mute
- Volume 1-20 = low range with DAC attenuation
- Volume 21-60 = medium range
- Volume 61-100 = high range with possible DAC boost

## Uninstallation

```bash
sudo dkms remove pamir-ai-soundcard/2.0.1 --all
```

## Technical Details

### TLV320AIC3204 Initialization

The driver initializes the codec with:
- Software reset
- Clock configuration (NDAC=1, MDAC=2, NADC=1, MADC=4)
- Power management setup
- I/O routing configuration
- Default gain settings

### Volume Control Implementation

The driver uses a two-stage volume control:
1. **DAC Volume Control** (Page 0, Registers 0x41/0x42)
2. **Headphone Amplifier Gain** (Page 1, Registers 0x10-0x13)

### File Structure

- `pamir-ai-soundcard-main.c` - Main soundcard component
- `pamir-ai-i2c-sound-main.c` - I2C codec driver
- `pamir-ai-rpi-soundcard-main.c` - Standalone RPI soundcard
- `dkms.conf` - DKMS configuration
- `Makefile` - Build configuration

## License

GPL v2

## Author

Pamir AI Incorporated - http://www.pamir.ai/