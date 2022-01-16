{ pkgs ? import <unstable> {} }:

let
  shellname = "pico";

  extras = with pkgs; [
    gcc
    cmake # Raspberry Pi Pico SDK
    gcc-arm-embedded # Raspberry Pi Pico SDK
    llvm # Raspberry Pi Pico SDK
    llvmPackages_11.clang
    ccls
    cmake
    cmakeCurses
    platformio
  ];
in
  pkgs.stdenv.mkDerivation {
    name = shellname;
    buildInputs = [
      extras
    ];
    shellHook = ''
      export NIX_SHELL_NAME='${shellname}'
      export PICO_SDK_PATH='/_scratch/picoCode/pico-sdk'
      PICO_EXAMPLES_PATH='FIXME'
      PICO_EXTRAS_PATH='FIXME'
      PICO_PLAYGROUND_PATH='FIXME'
      alias commands='echo "
        mkdir build
        cd build
        cmake ../src -DPICO_SDK_PATH=/path_to_sdk
        make
      "'
    '';
  }
