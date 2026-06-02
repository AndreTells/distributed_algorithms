{ pkgs ? import <nixpkgs> {} }:
let 
  client = pkgs.stdenv.mkDerivation{
    pname = "mutual_exclusion_client";
    version = "0.1.0";
    src = ../.;

    nativeBuildInputs = [ pkgs.gcc ];

    buildPhase = ''
      g++ ./mutual_exclusion/ressource.cpp \
      ./mutual_exclusion/main_client.cpp -I ./headers \
      -o mutual_exclusion_client.out -pthread
    '';

    installPhase = "
      mkdir -p $out
      mv ./mutual_exclusion_client.out $out
    ";
  };

  server = pkgs.stdenv.mkDerivation{
    pname = "mutual_exclusion_server";
    version = "0.1.0";
    src = ../.;

    nativeBuildInputs = [ pkgs.gcc ];

    buildPhase = ''
      g++ ./mutual_exclusion/main_server.cpp -I ./headers \
      -o mutual_exclusion_server.out -pthread
    '';

    installPhase = "
      mkdir -p $out
      mv ./mutual_exclusion_server.out $out
    ";
  };
in 
{inherit client server;}
