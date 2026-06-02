let 
  pkgs = import <nixpkgs> {config = {}; overlays = [];};
  lamport = import ./lamport.nix {};

  client_image = pkgs.dockerTools.buildImage {
    name = "lamport_client";
    tag = "latest";
    copyToRoot = pkgs.buildEnv {
      name = "image-root";
      paths = [ pkgs.bash lamport.client ];
    };

    config = {
      Cmd = ["${pkgs.bash}/bin/bash" "-c" "${lamport.client}/logical_clock_client.out $SERVER_IP"];
      Env = [
        "SERVER_IP=127.0.0.1" # considers that the server is in local host by
                              # default
      ];
    };

    meta.description = "Client of the Test of the Lamport Logical Clock Algorithm";
  };

  server_image = pkgs.dockerTools.buildImage {
    name = "lamport_server";
    tag = "latest";
    copyToRoot = pkgs.buildEnv {
      name = "image-root";
      paths = [ lamport.server ];
    };

    config = {
        Cmd = ["${lamport.server}/logical_clock_server.out"];
    };

    meta.description = "Server of the Test of the Lamport Logical Clock Algorithm";
  };
in 
{
  inherit client_image server_image;
}

