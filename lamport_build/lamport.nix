{ pkgs ? import <nixpkgs> {} }:
let 
  client = pkgs.stdenv.mkDerivation{
    pname = "lamport_client";
    version = "0.1.0";
    src = ../.;

    nativeBuildInputs = [ pkgs.gcc ];

    buildPhase = ''
      g++ ./lamport/logical_clock.cpp \
      ./lamport/timed_messaging.cpp \
      ./lamport/main_client.cpp -I ./headers \
      -o logical_clock_client.out
    '';

    installPhase = "
      mkdir -p $out
      mv ./logical_clock_client.out $out
    ";
  };

  server = pkgs.stdenv.mkDerivation{
    pname = "lamport_server";
    version = "0.1.0";
    src = ../.;

    nativeBuildInputs = [ pkgs.gcc ];

    buildPhase = ''
      g++ ./lamport/logical_clock.cpp \
      ./lamport/timed_messaging.cpp \
      ./lamport/main_server.cpp -I ./headers \
      -o logical_clock_server.out
    '';

    installPhase = "
      mkdir -p $out
      mv ./logical_clock_server.out $out
    ";
  };
in 
{inherit client server;}
