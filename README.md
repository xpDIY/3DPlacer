# 3DPlacer
3DPlacer is an adapter to convert 3D printer into pick and place machine that used 
for mounting SMT component into PCB. The aim is to make life easier for people who DIY their PCB 
and doesn't want to spend >$1k. While cost is low, the project still aim to bring enough accuracy
to mount tiny component (ex. 0201, 0402). Also one main goal is to make conversion simple and straight 
forward so that user can switch between 3D printing and PnP function in couple of minutes.
## Features
* Part rotation
* Airpump connectivity
* Adaptability for different 3D printers
  - Ender3 v2
  - Anycubic Mega SE
  - Ender CR10
  - Kingroon kp3s pro s1
* Compatible with openpnp
* Standard Lego style build plate supporting good extendability
* 30+ feeder can be mounted on 220x220mm plate
* 3D printed nozzle tip and JUKI nozzle tip supported
![3DPlacer for Ender3 v2](https://github.com/xpDIY/3DPlacer/blob/main/pictures/3dplacer_ender3_v2.jpg)

## Software
* Position aware feature supported
* Both controller and feeder firmware included

### Usage

#### Pre-requsite
* DapLink or other programming interface support ARM M0 MCU with SWD interface
* VCCode and clang compiler installed

#### Compile
* Enter sw folder
* make clean
* make

#### Flash
* Make sure programmer is connected correctly to SWD interface
* run command `make flash`

## Contribution

Any contribution will be appreciated!

* If you see any bug or improvement, feel free to create a pull request and explain about the pull request

Thanks and enjoy!

For more info, please refer to https://xpdiy.io
Youtube channel: https://www.youtube.com/@xpdiy


Shield: [![CC BY-NC-SA 4.0][cc-by-nc-sa-shield]][cc-by-nc-sa]

This work is licensed under a
[Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License][cc-by-nc-sa].

[![CC BY-NC-SA 4.0][cc-by-nc-sa-image]][cc-by-nc-sa]

[cc-by-nc-sa]: http://creativecommons.org/licenses/by-nc-sa/4.0/
[cc-by-nc-sa-image]: https://licensebuttons.net/l/by-nc-sa/4.0/88x31.png
[cc-by-nc-sa-shield]: https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg
