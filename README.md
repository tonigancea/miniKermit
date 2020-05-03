# miniKermit

## What is the Kermit protocol?
Kermit is an ARQ (Automatic Repeat Request) protocol that supports file transfer. Each wrong-recieved packet or unrecieved packet is retransmitted. The important payload is surrounded with control fields and each packet must be confirmed.  

This project is written in `.c`.

## How to run it
1. **Build** the _environment_:  
`cd ./link_emulator`  
`make` (building)  
`cd ..` (returning to the main folder)

2. **Build** the _sender_ and _receiver_ binaries:  
`make` (in the main folder)

3. **Start** the experiment. The experiment is sending `file1` `file2` and `file3`. You can change what files are sent by modifying accordingly inside the `run_experiment` script:  
 `./run_experiment`
 
4. Enjoy the transfer!

## Keep on coding! :)
