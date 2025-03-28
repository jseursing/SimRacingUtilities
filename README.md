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
    == - if its equal, then result is 1, otherwise 0
    != - if its not equal, then result is 1, otherwise 0
    >, >=, <, <=  - ...
    +,-,*,/
* For testing purposes, the scan tab accepts commands
  * using "setflag(dev,1)" enables developer mode which allows enumerating all active processes regardless if a window for it exists.
    Additionally, this flag enables the "write" variant of the above function
  

### Window Management Demo

https://github.com/user-attachments/assets/1c36e100-2f44-40ea-a8d9-b6a5c138e274


### Telemetry Management Demo

https://github.com/user-attachments/assets/b48447b7-aac3-4ce9-bc0b-05ab474c738f

### Scanner Demo

https://github.com/user-attachments/assets/d607dcd6-d799-4663-a633-85ec52286d00

