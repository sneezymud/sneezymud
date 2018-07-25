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
1. Assign it to yourself (or another builder)
    ```
    > @set blocka cizra 45660 45664
    > save (or force builder save)
    ```
1. Make changes
    - edit rooms
        ```
        rload 1 (load any halfway built rooms from immortal database
        goto 45664
        redit (describe and connect rooms)
        rsave 1
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
    - add items
        ```
        oedit create
        oedit mod hairball (change name into carrot, add descriptions and stuff)
        oedit save carrot 45664
        ```
    - change items
        ```
        oedit load 45664
        oedit mod carrot
        oedit resave carrot (deletes the original from DB, hope the MUD won't crash just now)
        oedit save carrot 45664
        ```
1. Edit the zonefile -- it's in lib/zonefiles/45660
1. Publish the zone to `sneezy` database
    ```
    low mvroom Cizra 1 45660-45664
    low mvmob Cizra 45664
    low mvobj Cizra 45664
    ```
1. Enable the zone in zonefile (change the 0 to 1) in header
1. Reboot the MUD, watch the zone getting loaded without `rload`
1. Repeat the steps of assigning zone, editing a room, saving to `immortal` and publishing to `sneezy` with an already existing zone to connect your zone to the rest of the world.
1. Your zonefile will clutter up the Git repo. However, to commit it, you also need the room/mob/object dumps from DB. TODO: move them into DB
