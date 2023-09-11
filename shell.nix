{ pkgs ? import <unstable> {} }:

let
  shellname = "pico_gradient";
  buildInpts = with pkgs; [
    gcc
    cmake # Raspberry Pi Pico SDK
    gcc-arm-embedded # Raspberry Pi Pico SDK
    llvm # Raspberry Pi Pico SDK
    llvmPackages_11.clang
    ccls
    cmake
    python3
  ];
in
  pkgs.stdenv.mkDerivation {
    name = shellname;
    buildInputs = [ buildInpts ];
    shellHook = ''
      echo -e '\033[1;33m
      Raspberry Pi Pico
      \033[0m'

      # export NIX_SHELL_NAME='${shellname}'
      # if [[ -n $LD_LIBRARY_PATH ]]; then
        # export LD_LIBRARY_PATH="${pkgs.bzip2.out}/lib:$LD_LIBRARY_PATH"
      # else
        # export LD_LIBRARY_PATH="${pkgs.bzip2.out}/lib"
      # fi
      # export LD_LIBRARY_PATH="${pkgs.alsa-lib.out}/lib:$LD_LIBRARY_PATH"
    '';
  }
