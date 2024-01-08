import cadquery as cq


w=310 # total width of the plate
d=230 # total depth of the plate
pitch = 8.0 # pitch of the bumps
rBot=8 # bottom rect section row, for saving printing material
cBot=8 # bottom rect section column, for saving prining material
botFactor=0.8 # controls the space between bottom rect
lclip=3 # number of space to place the clip
wclip=3 # number of space to place the clip
toClip=True # to allow clipping space to be created
toCutBottom=True # to cut the bottom to save material
componentHole=True #to provide component hole or not
#
# Lego Brick Constants-- these make a Lego brick a Lego :)
#
clearance = 0.1
bumpDiam = 4.8
bumpHeight = 4
height = 3.2
componentHoleDepth=1.5 # the depth to hold the comopnent in the back of PCB
cutBottomHeight=0

rBotPitch = w/rBot
cBotPitch = d/cBot

wbumps = int(w//pitch)     # number of bumps long
lbumps = int(d//pitch)     # number of bumps wide

lbumpSize=(d-(lbumps)*pitch)/2
wbumpSize=(w-(wbumps)*pitch)/2

t = (pitch - (2 * clearance) - bumpDiam) / 2.0
postDiam = pitch - t  # works out to 6.5
total_length = d #lbumps*pitch - 2.0*clearance
total_width = w #wbumps*pitch - 2.0*clearance

# make the base
s = cq.Workplane("XY").box(total_length, total_width, height)
if toCutBottom:
    toCut = cq.Workplane("XY").workplane(offset=-height/2-1).rarray(rBotPitch,cBotPitch,rBot,cBot).rect(rBotPitch*botFactor,cBotPitch*botFactor).extrude(cutBottomHeight+1)
    s=s.cut(toCut)
# make the bumps on the top
s = (s.faces(">Z").workplane().
    rarray(pitch, pitch, lbumps, wbumps, True).circle(bumpDiam / 2.0)
    .extrude(bumpHeight))#.edges(">Z").fillet(0.3)
#comopnent holder
if componentHole:
    ch = cq.Workplane("XY").workplane(offset=height/2).rarray(pitch, pitch, lbumps+1, wbumps+1, True).rect(bumpDiam,bumpDiam).extrude(-componentHoleDepth)#.edges(">Z").fillet(0.3)
    s = s.cut(ch)


if toClip:
    toCut = cq.Workplane("XY").workplane(offset=height/2).rarray((d-16-lbumpSize)/(lclip-1),(w-wbumpSize),lclip,2).rect(16+lbumpSize,8+wbumpSize).extrude(2*height)
    s = s.cut(toCut)

    toCut = cq.Workplane("XY").workplane(offset=height/2).rarray(d-8-lbumpSize,(w-16-wbumpSize)/(wclip-1),2,wclip).rect(8+lbumpSize,16+wbumpSize).extrude(2*height)
    s = s.cut(toCut)

show_object(s)
