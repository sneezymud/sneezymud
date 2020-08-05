with import <nixpkgs> {};
pkgs.mkShell {
  # git for getting commit hash, python for unittesting
  buildInputs = [ stdenv scons boost mariadb-connector-c python git ];
}
