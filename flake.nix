{
  description = "C++26 development shell";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        llvm = pkgs.llvmPackages_latest;
        stdenv = if pkgs.stdenv.isDarwin then pkgs.darwin else llvm.libcxxStdenv;
        sdl-runtime = pkgs.lib.optionals (!pkgs.stdenv.isDarwin) ([
          pkgs.wayland
          pkgs.libxkbcommon
          pkgs.libdecor
          pkgs.mesa
          pkgs.vulkan-loader
        ]);
      in
      {
        devShells.default = (pkgs.mkShell.override { inherit stdenv; }) ({
          packages = [
            llvm.clang
            llvm.clang-tools
            pkgs.cmake
            pkgs.ninja
          ] ++ sdl-runtime;

          shellHook = ''
            echo "  $(clang --version | head -n1)"
            echo "  $(cmake --version | head -n1)"
            echo "  ninja version $(ninja --version)"
          '';
        } // pkgs.lib.optionalAttrs (!pkgs.stdenv.isDarwin) {
          LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath sdl-runtime;
          SDL_RENDER_DRIVER = "vulkan";
        });
      }
    );
}
