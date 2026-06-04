let 
  pkgs = import <nixpkgs> {config = {}; overlays = [];};
  election = import ./election.nix {};
in 
pkgs.dockerTools.buildImage {
    name = "election_host";
    tag = "latest";
    copyToRoot = pkgs.buildEnv {
      name = "image-root";
      paths = [ pkgs.bash election ];
    };

    config = {
      Cmd = ["${pkgs.bash}/bin/bash" "-c" "${election}/election_host.out $NETWORK_SIZE $HOST_ID $NETWORK_IPS"];
      Env = [
        "NETWORK_SIZE=1"
        "HOST_ID=0"
        "NETWORK_IPS=\"127.0.0.1\"" # considers that the server is in local host by
                                # default
      ];
    };

    meta.description = "One Host in the Test of the Election Algorithm";
  }

