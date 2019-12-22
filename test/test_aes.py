import importlib.util
spec = importlib.util.spec_from_file_location("opendbpy","/home/grads/chhab011/OpenROAD/src/OpenDB/build/src/swig/python/opendbpy.py")
odb = importlib.util.module_from_spec(spec)
spec.loader.exec_module(odb)

db = odb.dbDatabase.create()
odb.odb_import_db(db,"/home/sachin00/chhab011/PDNA/aes_pdn.db")

#sanity check
tech = db.getTech()
layers = tech.getLayers()
for l in layers:
    print("%s"%l.getName())
   
chip = db.getChip()
block = chip.getBlock()
nets = block.getNets()
vdd_nets= []
gnd_nets= []
for net in nets:
    if net.getSigType() == 'POWER': #cpp work with enums
        vdd_nets.append(net)
    if net.getSigType() == 'GROUND': #cpp work with enums
        gnd_nets.append(net)

for vdd_net in vdd_nets: #only 1?
    swires = vdd_net.getSWires() #only 1?
    for swire in swires:
        wires = swire.getWires()
        for wire in wires:
            if wire.isVia():
                via = wire.getBlockVia()
                x,y = wire.getViaXY()
                via_layer = via.getBottomLayer() # convert to routing number
                l = via_layer.getRoutingLevel()
                R = via_layer.getResistance()
                # create nodes l,x,y and l+1,x,y (or find if already exists)
            else:
                layer = wire.getTechLayer()
                R = layer.getResistance()
                length = wire.getLength()
                width = wire.getWidth()
                
