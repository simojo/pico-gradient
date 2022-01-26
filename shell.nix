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
    '';
  }
