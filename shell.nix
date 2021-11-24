{ pkgs ? import <unstable> {} }:

let
  shellname = "pico";

  extras = with pkgs; [
    gcc
    pkgs.cmake # Raspberry Pi Pico SDK
    pkgs.gcc-arm-embedded # Raspberry Pi Pico SDK
    pkgs.llvm # Raspberry Pi Pico SDK
    llvmPackages_11.clang
    cmake
    cmakeCurses
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
