{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  nativeBuildInputs = with pkgs.buildPackages; [
    boost libmysqlclient scons gtest pkg-config libxcrypt
  ];
}
