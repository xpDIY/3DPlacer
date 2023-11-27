# This document describe the way to integrate with Openpnp

## General description
In order to integrate with Openpnp, the idea is to provide various G code so that openpnp can use and setup the system. The serial text interface will be used. There are several gcodes currently needed for integration. In the description below, square bracket [] means optional, {} means parameter. 

## System architecture
The system contains openpnp, strip board and components. When user starts job or setup the components in openpnp, ex. feeders, openpnp will fire command (described in section below), then the command string will be passed to strip boards, strip boards will be responbile to pass the strings further to peticular components. The components received the command will then interpret the command and send the response back. The strip board will be used as a tunnel which forward the infomation.
Communication between Openpnp and strip board is serial communication. Strip board can communicate with other strip boards, the protocol used here is I2C. Then under each strip board, there will be multiple components, in order to save space, 1 wire UART is used. The position sensing is achieved by resistor dividing, with 3.3v supply voltage, depend on position, it will sense max 50 positions, each position can have 66 mv gap which gives enough space for detecting the position consistently.

## GCodes
### M115
Firmware information code, this is used to make sure the openpnp can read the firmware therefore can use the I&S (issue and solution) wizard for better support setting up the machine.
  #### Usage
  ```
  M115
  ```
  #### Returns 
  ```
  Firmware name + version
  ```
  #### Example
  ```
  3DPlacer FIRMWARE v.01
  ```

### M888
Used to read back component info, ex. feeder, fiducial, camera etc. If no component id specified, then it will return all component info, components info will be returned as an array of components, each component will be separated by semicolon (;), the line ending will be CRLF.
  If component id specified, then it will return that specific component info
  Information included in the return of gcode contains (for different component, field will be different, the mandatary ones are name, position, type, id)
  
  * **type** - t: type, can be **<fed, fid, cam, chg, stp,lig>**, this can be extended, fed: feeder, fid:fiducial, cam: camera, chg: change station, stp: strip board, lig:light
  * **id** - id: id of component, will be unique accorss all components
  * **position**  - r: row, c: column
  * **dimension** - w: width, l: length, h: height
  * **name** - n: name ```Note: name will always be the last element to avoid for the escaping```
  * **status** - st: status, general ones are **<ok, err>**, different component can have different status, ex. feeder can have **<end, jam>**, light can have **<on,off>**
  * **content** - tw: tape width, ex. 8, 12, 16..
  * **operate offset**: ox: offset x, oy: offset y, this is the offset from contact point of the slot. For feeder, this means pick position, for camera, this means camera center position. For fiducial, this means the fiducial mark position.


 
  #### Component specific field
  * Feeder - feeder can have some specific fields
    * p: pitch, depend on tape inside, it can have 2mm, 4mm, or other pitch
    * ad: tape advance, if not zero, means it is advancing, when finish advancing, it will be reset to 0,  this field can be used to control advacning. ex. ad:1 means advance 1 pitch, see M887 gcode, ad:-1 means travel back 1 pitch position
    * st: status of feeder can be **<ok,err,end,jam>**, ok:good, err:unknown err, end:tap ended, jam: tap jammed
  * Light - light can have some specific status
    * st: status of light can be **<on, off>**. The light can be on or off. The on/off can be used as status, and also as control. See M887 for detial.
   
  #### Usage
  ```
  M888 [C{id}]
  ```
  #### Examples  
  ```
  M888 C12345678; returns t:fed,id:12345678,w:12,l:64,h:43,ox:-5,oy:-2.1,tw:8,st:ok,r:4,c:2,n:100kohm

  ; below command returning all components separated by semicolon (;)
  M888;returns t:fed,id:12345678,w:12,l:64,h:43,ox:-5,oy:-2.1,tw:8,st:ok,r:4,c:2,n:100kohm; t:fed,id:12345679,w:16,l:64,h:43,ox:-5,oy:-4.1,tw:8,st:ok,r:3,c:2,n:100uf; t:fid,id:22222222,w:12,l:12,h:43,r:1,c:1,ox:3,oy:3,n:pri; t:fid,id:33333333,w:12,l:12,h:37,r:10,c:10,ox:3,oy:3,name:sec; t:cam,id:44444444,w:32,l:32,h:43,r:10,c:10,ox:8,oy:8,name:bot; t:lig,id:55555555,r:1,c:1,st:on,n:top; t:lig,id:66666666,r:5,c:5,st:off,n:bot[CRLF]
  ```

### M887
Send command to component. This command can be ued to update component info, ex. name, offset, etc.
  #### usage:
  ```
  M887 C{ID} [field:value]
  ```
  #### examples: 
  ```
  M887 C12345678 ad:2; advacne feeder with ID 12345678 with 2 position
  M887 C87654321 ad:-1; moving back feeder wiht ID 87654321 1 position
  M887 C12345678 ox:-4 n:10kohm ; change offset x to -4mm relative to the center of the contact point
  M887 C55555555 st:off ;switch top light off
  ```

