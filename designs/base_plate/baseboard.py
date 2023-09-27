import cadquery as cq


w=234 # total width of the plate
d=234 # total depth of the plate
pitch = 8.0 # pitch of the bumps
rBot=8 # bottom rect section row, for saving printing material
cBot=8 # bottom rect section column, for saving prining material
botFactor=0.8 # controls the space between bottom rect
lclip=3 # number of space to place the clip
wclip=3 # number of space to place the clip
toClip=True # to allow clipping space to be created
toCutBottom=True # to cut the bottom to save material
#
# Lego Brick Constants-- these make a Lego brick a Lego :)
#
clearance = 0.1
bumpDiam = 4.8
bumpHeight = 3
height = 3.2

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
    toCut = cq.Workplane("XY").workplane(offset=-height/2-1).rarray(rBotPitch,cBotPitch,rBot,cBot).rect(rBotPitch*botFactor,cBotPitch*botFactor).extrude(height)
    s=s.cut(toCut)
# make the bumps on the top
s = (s.faces(">Z").workplane().
    rarray(pitch, pitch, lbumps, wbumps, True).circle(bumpDiam / 2.0)
    .extrude(bumpHeight))#.edges(">Z").fillet(0.3)

if toClip:
    toCut = cq.Workplane("XY").workplane(offset=height/2).rarray((w-8-wbumpSize),(d-16-lbumpSize)/(lclip-1),2,lclip).rect(8+wbumpSize,16+lbumpSize).extrude(2*height)
    s = s.cut(toCut)

    toCut = cq.Workplane("XY").workplane(offset=height/2).rarray((w-16-wbumpSize)/(wclip-1),d-8-lbumpSize,wclip,2).rect(16+wbumpSize,8+lbumpSize).extrude(2*height)
    s = s.cut(toCut)

show_object(s)
