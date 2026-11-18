# scrapcheese

scrapcheese is a small C++ Scrap Mechanic reverse-engineering experiment. Its current code looks up the game's character manager and logs each available character's ID, position and address.

## Build

The repository uses [xmake](https://xmake.io). From the repository root:

```sh
xmake
```

The project contains a game-side module and a small injector project. The code depends on Scrap Mechanic's internal structures and is specific to the game build it was written against.

## Status

This is an exploratory prototype, not a general-purpose mod or supported release.
