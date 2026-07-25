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
      in
      {
        devShells.default = (pkgs.mkShell.override { stdenv = llvm.libcxxStdenv; }) {
          packages = [
            llvm.clang
            llvm.clang-tools
            pkgs.cmake
            pkgs.ninja
          ];

          shellHook = ''
            echo "  $(clang --version | head -n1)"
            echo "  $(cmake --version | head -n1)"
            echo "  ninja version $(ninja --version)"
          '';
        };
      }
    );
}
