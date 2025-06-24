# Make Roanoke Work

## Organization

- `bring_up`: Instructions and resources for BeepBeep bring-up (after assembly)
- `card_test`: micropython test code to verify wiegand interpretation
- `docs`: Relevant documents, photos, images, etc. 
- `wiegand_sig_gen`: RP2040-based wiegand signal generator
- `cheepcheep`: A C-based rewrite of the beepbeep firmware

## Set up

Any binary files are tracked via git-lfs. To access:

1. Install git-lfs:

```
sudo apt install git-lfs
git lfs install
```

2. Pull the lfs objects:

```
git lfs pull
```
