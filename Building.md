How to build
==========

1. Pick a zone to work on, either an existing one or a new one. To list existing zones,
    ```
    > show zones
    ```
    To create a new zone,
    ```
    > zonefile new 5 Cizra - Temple of Stupidity
    Success! new zone: Cizra - Temple of Stupidity with vnums from 45660 to 45664
    ```
1. Assign it to yourself (or a builder)
    ```
    > @set blocka cizra 45660 45664
    > save
    ```
1. Load any work in progress from `immortal` database to live MUD
    ```
    > rload 1
    ```
1. Make changes
    - edit rooms
        ```
        goto 45664
        redit (describe and connect rooms)
        ```
    - add mobs
        ```
        show mob rabbit
        load mob 44784
        medit mod rabbit (change name to Lagomorph, describe appropriately)
        medit save lagomorph 45664
        ```
    - change mobs
        ```
        medit load 45664
        medit mod lagomorph (do further changes)
        medit save lagomorph 45664
        ```
    - TODO: add items
1. Save the state from live MUD to `immortal` database
    ```
    > rsave 1
    ```
1. Edit the zonefile -- it's in lib/zonefiles/45660
1. Publish the zone to `sneezy` database
    ```
    $ docker exec -it sneezy /bin/sh
    (docker) ~/sneezymud-docker/sneezymud/code $ objs/sqladdwld Cizra 1 45660-45664
    (docker) ~/sneezymud-docker/sneezymud/code $ objs/sqladdmobs Cizra 45664
    ```
1. Enable the zone in zonefile (change the 0 to 1) in header
1. Reboot the MUD, watch the zone getting loaded without `rload`
1. Repeat the steps of assigning zone, editing a room, saving to `immortal` and publishing to `sneezy` with an already existing zone to connect your zone to the rest of the world.
1. Your zonefile will clutter up the Git repo. TODO: move them into DB
