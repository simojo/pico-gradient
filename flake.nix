{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/678bc0c5b84ecadaa66d8567363eccef0d558e9c";
  };
  outputs = { self, nixpkgs }:
  let
    system = "x86_64-linux";
    pkgs = import nixpkgs {
      inherit system;
      overlays = [];
    };
    ldPath = with pkgs; pkgs.lib.makeLibraryPath [];
  in {
    devShells.${system}.default = pkgs.mkShell {
      buildInputs = with pkgs; [
        gcc
        cmake # Raspberry Pi Pico SDK
        gcc-arm-embedded # Raspberry Pi Pico SDK
        llvm # Raspberry Pi Pico SDK
        llvmPackages.clang
        ccls
        cmake
        python3
      ];
      shellHook = ''
        echo -e '\033[1;33m
        Raspberry Pi Pico
        \033[0m'
        if [[ $LD_LIBRARY_PATH == "" ]]; then
          export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${ldPath}"
        else
          export LD_LIBRARY_PATH="${ldPath}"
        fi

        # %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        # %%%% NOTE: this is to help our .clangd file pick up on where the system %%%%
        # %%%% headers are. Otherwise, warnings litter the code                   %%%%
        # %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        export CLANGD_INCLUDE_1="$(arm-none-eabi-gcc -print-file-name=)include"
        export CLANGD_INCLUDE_2="$(arm-none-eabi-gcc -print-file-name=)include-fixed"
        export CLANGD_INCLUDE_3="$(arm-none-eabi-gcc -print-file-name=)../../../../arm-none-eabi/include"
        cat > .clangd <<EOF
        CompileFlags:
          Add: [
            -isystem,
            $CLANGD_INCLUDE_1,
            -isystem,
            $CLANGD_INCLUDE_2,
            -isystem,
            $CLANGD_INCLUDE_3
          ]
        EOF
      '';
    };
  };
}
