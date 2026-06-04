{ pkgs ? import <nixpkgs> {} }:
pkgs.stdenv.mkDerivation{
  pname = "election_host";
  version = "0.1.0";
  src = ../.;

  nativeBuildInputs = [ pkgs.gcc ];

  buildPhase = ''
    g++ ./election/main_host.cpp -I ./headers \
    -o election_host.out -pthread
  '';

  installPhase = "
    mkdir -p $out
    mv ./election_host.out $out
  ";
}
