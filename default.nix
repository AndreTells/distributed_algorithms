{ pkgs ? import <nixpkgs> {} }:
let 
  system = pkgs.stdenv.mkDerivation{
    pname = "lab2";
    version = "0.1.0";
    src = ./.;

    nativeBuildInputs = [ pkgs.gcc ];

    buildPhase = ''
    '';

    installPhase = "mkdir -p $out";
  };

  test = pkgs.stdenv.mkDerivation{
    pname = "lab2-tests";
    version = "0.1.0";
    src = ./.;

    nativeBuildInputs = [ pkgs.gcc ];

    buildInputs = [ pkgs.cunit ];

    doCheck = true;
    buildPhase = ''
      g++ ./test/logical_clock_unit_test.cpp \
      ./src/simple_logical_clock/simple_logical_clock.cpp -lcunit -I ./headers \
      -o simple_logical_clock_unit_test.out

      g++ ./test/logical_clock_unit_test.cpp \
      ./src/logical_clock/logical_clock.cpp -lcunit -I ./headers \
      -o logical_clock_unit_test.out
    '';

    checkPhase = ''
      ./simple_logical_clock_unit_test.out
      ./logical_clock_unit_test.out
    '';

    installPhase = "mkdir -p $out";
  };
in {inherit system test;}
