# This document describe the way to integrate with Openpnp

## General description
In order to integrate with Openpnp, the idea is to provide various G code so that openpnp can use and setup the system. The serial text interface will be used. There are several gcodes currently needed for integration. In the description below, square bracket [] means optional, {} means parameter.

* M115 - firmware information code, this is used to make sure the openpnp can read the firmware therefore can use the I&S (issue and solution) wizard for better support setting up the machine.
  * Usage: ```M115```
  * Returns: ```Firmware name + version```
  * Example: ```3DPlacer v.01```

* M888 - used to read back component info, ex. feeder, fiducial, camera etc. If no component id specified, then it will return all component info, components info will be returned as an array of components, each component is in separate line, the line ending will be CRLF.
  If component id specified, then it will return that specific component info
  Information included in the return of gcode contains
  * component position (ex. in relation to baseplate, this might need to be interpred by openpnp, pick position offset)
  * component dimension (wxlxh)
  * component content (ex. currently holding component, total num of components etc)
  * component status (is there error or tape is ended)
  * Usage
  ```
  M888 [C{id}]
  ```
  * Examples  
  ```
  M888 C12345678; returns id:12345678,w:12,l:64,h:43,tape:8,type:cas,status:ok,row:4,col:2
  M888;returns 
    type:feeder,id:12345678,w:12,l:64,h:43,tape:8,status:ok,row:4, col:2
    type:feeder,id:87654321,w:16,l:64,h:43,tape:12,status:ended,row:6, col:3
    type:fid,id:22222222,w:12,l:12,h:43,row:1, col:1, name:primary
    type:fid,id:33333333,w:12,l:12,h:40,row:10, col:10,name:secondary
  
  ```

* M887 - Update or set component info
  * usage:```M887 C{ID} [Action]```
  * examples: 
  ```
  M887 C12345678 A2; advacne feeder with ID 12345678 with 2 position
  M887 C87654321 A-1; moving back feeder wiht ID 87654321 1 position
  ```
* M800 - Turn a camera light on/off
  * usage: ```M800 L{ID} P{True:1}{False:0}```
  * examples
  ```
  M800 L1 P1; turn on camera light L1
  M800 L2 P0; turn off camera light L2
  ```
