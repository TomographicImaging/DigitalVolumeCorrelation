import numpy 
from ccpi.dvc import DVC
from ccpi.dvc import dvcw
import json


controller = DVC()


roi = numpy.loadtxt("central_grid.roi")

print (controller)

pc = dvcw.DataCloud()
pc.loadPointCloudFromNumpy(roi[:10])
pc.organize_cloud(controller.run)

with open('dvc_input.json', "r") as f:
    config = json.load(f) 
print(config)
    
controller.config_run('dvc_input.json')
hd = controller.run.parse_npy(0)
print ("received hd" , hd)
details = eval(hd[0])

controller.run.run_dvc_cmd(pc)