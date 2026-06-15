# How to use
To run this example you must generate the docker image, load it into docker and
then run the compose command. This can be done by:  

## Generate and load docker images

```sh
  nix-build mutual_exclusion_image.nix -A client_image
  docker load < result
```

```sh
  nix-build mutual_exclusion_image.nix -A server_image
  docker load < result
```

## Run docker compose

```sh
  docker compose -f mutual_exclusion_compose.yml up
```
