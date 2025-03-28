# SimRacingUtilities
Assortment of utilities for Sim Racing

### Features
* Automatic window management
* Telemetry simulation for games that do not emit telemetry
* Telemetry (value) scanning and pointer resolution

### How does telemetry simulation work?
Assetto Corsa remote telemetry protocol is simulated by this tool. A "dummy" executable with the name "AssettoCorsa.exe" is launched in order to trigger other sim racing tools such as SimHub to switch to its Assetto Corsa profile.
The user is responsible for providing telemetry information in order to populate the telemetry database which is then fed to subscribers such as SimHub.
The telemetry options are listed in the tree widget while the target executable is the application which you specify with regards to memory reads.
#### NOTE: This feature may NOT work for games with kernel level anti-cheats due to some drivers blocking external memory access for their client applications.

#### The following functions are supported:
* readu8(address, optional ptr_offset1, optional ptr_offset2, ...)
    - reads unsigned char value
* readi8(address, optional ptr_offset1, optional ptr_offset2, ...)
    - reads signed char value
* readu16(address, optional ptr_offset1, optional ptr_offset2, ...)
    - reads a unsigned short value 
* readi16(address, optional ptr_offset1, optional ptr_offset2, ...)
    - ...
* readu32(address, optional ptr_offset1, optional ptr_offset2, ...)
* readi32(address, optional ptr_offset1, optional ptr_offset2, ...)
* readF(address, optional ptr_offset1, optional ptr_offset2, ...)
    - reads a float value
* read[](length, address, optional ptr_offset1, optional ptr_offset2, ...)
    - reads an array of specified length
* map(telemetry index)
    - retrieves the value of the telemetry at the specified index
* any other value is considered a hard value, ie "poop" will assign the value "poop"
* basic arithmetic and logical comparisons are supported for all of the above except for read[]
  - if any of arithmetic or comparisons are used, a constant must be used alongside it (ie readF(0x000)==1)
  - Available functions:
    - "==" - if its equal, then result is 1, otherwise 0
    - "!=" - if its not equal, then result is 1, otherwise 0
    - ">, >=, <, <="  - ...
    - "+,-,*,/"
* For testing purposes, the scan tab accepts commands
  * using "setflag(dev,1)" enables developer mode which allows enumerating all active processes regardless if a window for it exists.
    Additionally, this flag enables the "write" variant of the above function
  

## Window Management Demo


https://github.com/user-attachments/assets/067ea9da-0ae8-4221-b8e9-00c35987297f


## Telemetry Management Demo


https://github.com/user-attachments/assets/49c89507-4d97-47d4-8695-bf3ce6ea37df


## Scanner Demo


https://github.com/user-attachments/assets/d9168795-cc96-4e41-beac-699789891820

