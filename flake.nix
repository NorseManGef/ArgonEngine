{
  description = "A Nix-flake-based C/C++ development environment";

  inputs.nixpkgs.url = "https://flakehub.com/f/NixOS/nixpkgs/0.1.*.tar.gz";

  outputs = { self, nixpkgs }:
    let
      supportedSystems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
      forEachSupportedSystem = f: nixpkgs.lib.genAttrs supportedSystems (system: f {
        pkgs = import nixpkgs { inherit system; config.allowUnfree = true; };
      });
    in
    {
      devShells = forEachSupportedSystem ({ pkgs }: {
        default = pkgs.mkShell.override
          {
            # Override stdenv in order to change compiler:
            # stdenv = pkgs.clangStdenv;
          }
          {
            packages = with pkgs; [
              clang-tools
              cmake
              codespell
              conan
              cppcheck
              doxygen
              gtest
              lcov
              vcpkg
              vcpkg-tool
              gnumake
              bear
              ninja

              dbus
              pkg-config
              udev
              libevdev

              vulkan-loader

              libGL
              libx11
              libxcursor
              libxext
              xorg.libXinerama
              libxrandr
              xorg.libXi
              xorg.libXScrnSaver
              xorg.libXtst
              libxcb
              libxkbcommon

              mesa

              wayland
              wayland-scanner
              libffi
              egl-wayland

              pipewire
              pulseaudio

              #for generator scripts
              python3
            ] ++ (if system == "aarch64-darwin" then [ ] else [ gdb ]);

            VULKAN_PATH = "${pkgs.lib.getLib pkgs.vulkan-loader}/lib/libvulkan.so";
            X11XCB_PATH = "${pkgs.lib.getLib pkgs.libx11}/lib/libX11-xcb.so";

            ZENITY_PATH = "${pkgs.lib.getExe pkgs.zenity}";
          };
      });
    };
}
