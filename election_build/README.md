# How to use
To run this example you must generate the docker image, load it into docker and
then run the compose command. This can be done by:  

## Generate and load docker images

```sh
  nix-build election_image.nix
  docker load < result
```

## Run docker compose

```sh
  docker compose -f election_compose.yml up
```
