# This document describe the way to integrate with Openpnp

## General description
In order to integrate with Openpnp, the idea is to provide various G code so that openpnp can use and setup the system. The serial text interface will be used. There are 2 gcodes currently needed for integration.

* M115 - firmware information code, this is used to make sure the openpnp can read the firmware therefore can use the I&S (issue and solution) wizard for better support setting up the machine.
  * Usage: ```M115```
  * Returns: ```Firmware name + version```
  * Example: ```3DPlacer v.01```

* M888 (TBD) - used to read back all the feeder info. The feeder info will be returned as an array in JSON format. Information included in the return of gcode contains
  * feeder position (in relation to baseplate, this might need to be interpred by openpnp)
  * feeder dimension (wxlxh)
  * feeder content (tape width, pitch, rotation in tape)
  * feeder status (is there error or tape is ended)
  * Examples
  
  ```
  M888;returns 
    {
      feeders:
      [
        {id:12345678,w:12,l:64,h:43,tape:8,type:cas,status:ok,row:4, col:2},
        {id:87654321,w:16,l:64,h:43,tape:12,type:cas,status:ended,row:6, col:3}
      ],
      fiducials:
      [
        {id:22222222,w:12,l:12,h:43,row:1, col:1, name:primary},
        {id:22222222,w:12,l:12,h:40,row:10, col:10,name:secondary}
      ]
      ... //other types components
    }

  ```

* M887 - Advance tape for n position
  * usage:```M887 F{ID} A{Integer}```
  * examples: 
  ```
  M887 F12345678 A2; advacne feeder with ID 12345678 with 2 position
  M887 F87654321 A-1; moving back feeder wiht ID 87654321 1 position
  ```
* M800 - Turn a camera light on/off
  * usage: ```M800 L{ID} P{True:1}{False:0}```
  * examples
  ```
  M800 L1 P1; turn on camera light L1
  M800 L2 P0; turn off camera light L2
  ```
  

