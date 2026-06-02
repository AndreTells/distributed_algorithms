let 
  pkgs = import <nixpkgs> {config = {}; overlays = [];};
  mutual_exclusion = import ./mutual_exclusion.nix {};

  client_image = pkgs.dockerTools.buildImage {
    name = "mutual_exclusion_client";
    tag = "latest";
    copyToRoot = pkgs.buildEnv {
      name = "image-root";
      paths = [ pkgs.bash mutual_exclusion.client ];
    };

    config = {
      Cmd = ["${pkgs.bash}/bin/bash" "-c" "${mutual_exclusion.client}/mutual_exclusion_client.out $SERVER_IP"];
      Env = [
        "SERVER_IP=127.0.0.1" # considers that the server is in local host by
                              # default
      ];
    };

    meta.description = "Client of the Test of the Mutual Exclusion Algorithm";
  };

  server_image = pkgs.dockerTools.buildImage {
    name = "mutual_exclusion_server";
    tag = "latest";
    copyToRoot = pkgs.buildEnv {
      name = "image-root";
      paths = [ mutual_exclusion.server ];
    };

    config = {
        Cmd = ["${mutual_exclusion.server}/mutual_exclusion_server.out"];
    };

    meta.description = "Server of the Test of the Mutual Exclusion Algorithm";
  };
in 
{
  inherit client_image server_image;
}

