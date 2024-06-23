Code to create a color gradiate between two HSV colors using a ws2812B NeoPixel strip. This is written for use on the raspberry pi pico.

## Steps to build - Ubuntu

```sh
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install doxygen #(required later for pico examples)

ln -s /mnt/d/linux/pico ./pico
cd pico

sudo git clone -b master https://github.com/raspberrypi/pico-sdk.git
sudo git clone -b master https://github.com/raspberrypi/pico-examples.git

cd pico-sdk
sudo git submodule update --init
cd ..
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential
#(answer Y to any prompts)

cd pico-examples
mkdir build
cd build

export PICO_SDK_PATH=../../pico-sdk
cmake ..
cmake ..	#see note 2 below

cd blink
make
make	#see note 2 below
```
