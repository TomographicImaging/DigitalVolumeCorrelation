import dvcw
import numpy

import json

import vtk
import os
from ccpi.viewer.CILViewer2D import Converter

class DVC(object):
    def __init__(self):
        self.run = dvcw.RunControl()
        self.run_keywords = ['subvol_npts', 'subvol_thresh', 'basin_radius',
                             'subvol_size', 'subvol_geom', 'output_filename', 
                             'correlate_filename', 'obj_function', 
                             'interp_type', 'point_cloud_filename', 
                             'rigid_trans', 'num_srch_dof', 'disp_max', 
                             'reference_filename', 'subvol_aspect',
                             'vol_bit_depth','vol_hdr_lngth',
                             'vol_wide','vol_high', 'vol_tall']
        #vol_bit_depth	vol_hdr_lngth	 vol_wide vol_high vol_tall	should be inferred from the input file?
        
    def config_run(self, filename):
        with open(filename, "r") as f:
            config = json.load(f)
            if len(config.keys()) != len(self.run_keywords):
                missing = []
                for k in self.run_keywords:
                    if not k in list(config.keys()):
                        missing.append(k)
                if len(missing) > 0:
                    raise ValueError('Missing Keyword(s): ', missing)
                else:
                    # config the RunControl structure
                    
                    
                    self.run.ref_fname = config['reference_filename']
                    self.run.cor_fname = config['correlate_filename']
                    
                    self.check_input_files()
                    
                    v = config['output_filename']
                    self.run.out_fname = v 
                    self.run.res_fname = v + ".disp"
                    self.run.sta_fname = v + ".stat"
                    
                
                    self.run.pts_fname = config['point_cloud_filename']
                        
                    v = config['subvol_geom']
                    if v.lower() == 'cube':
                        sg = dvcw.Subvol_Type.cube
                    elif v.lower() == 'sphere':
                        sg = dvcw.Subvol_Type.sphere
                    else:
                        #tb = sys.exc_info()[2]
                        raise ValueError(
                           'DVC: Unsupported subvol_geom: {0}'.format(
                                   v
                                   ))
                    self.run.subvol_geom = v.lower()
                    self.run.sub_geo = sg
                    
                    v = config['subvol_thresh']
                    if v.lower() == 'on':
                        m = config['vol_bit_depth']
                        tm = 0
                        tM = numpy.power(2,m, dtype=numpy.float64)
                        v = config['gray_thresh_min']
                        if v >= tm and v < tM:
                            self.run.gray_thresh_min = v
                        else:
                            raise ValueError('Input value {0} out of bounds {1} {2}'
                                             .format(v, tm, tM))
                        v = config['gray_thresh_max']
                        tm = self.run.gray_thresh_min
                        
                        if v >= tm and v < tM:
                            self.run.gray_thresh_max = v
                        else:
                            raise ValueError('Input value {0} out of bounds {1} {2}'
                                             .format(v, tm, tM))
                        v = config['min_vol_fract']
                        tm = 0.
                        tM = 1.
                        if v >= tm and v < tM:
                            self.run.min_vol_fract = v
                        else:
                            raise ValueError('Input value {0} out of bounds {1} {2}'
                                             .format(v, tm, tM))
                    elif v.lower() == 'off':
                        m = config['vol_bit_depth']
                        self.run.gray_thresh_min = 0
                        self.run.gray_thresh_max = numpy.power(2,m, dtype=numpy.float64)
                        self.run.min_vol_fract = 0 
                    else:
                        #tb = sys.exc_info()[2]
                        raise ValueError(
                           'DVC: Unsupported subvol_geom: {0}'.format(
                                   v
                                   ))
                        
                    
                    
                    #required parameters defining the basic the search process
                    v = config['obj_function']
                    if v.lower() == 'sad':
                        sg = dvcw.Objfcn_Type.SAD
                    elif v.lower() == 'ssd':
                        sg = dvcw.Objfcn_Type.SSD
                    elif v.lower() == 'zssd':
                        sg = dvcw.Objfcn_Type.ZSSD
                    elif v.lower() == 'nssd':
                        sg = dvcw.Objfcn_Type.NSSD
                    elif v.lower() == 'znssd':
                        sg = dvcw.Objfcn_Type.ZNSSD
                    else:
                        #tb = sys.exc_info()[2]
                        raise ValueError(
                           'DVC: Unsupported obj_function: {0}'.format(
                                   v
                                   ))
                    self.run.obj_function = v.lower()
                    self.run.obj_fcn = sg
                    
                    v = config[ 'interp_type']
                    if v.lower() == 'nearest':
                        sg = dvcw.Interp_Type.nearest
                    elif v.lower() == 'trilinear':
                        sg = dvcw.Interp_Type.trilinear
                    elif v.lower() == 'tricubic':
                        sg = dvcw.Interp_Type.tricubic
                    else:
                        #tb = sys.exc_info()[2]
                        raise ValueError(
                           'DVC: Unsupported obj_function: {0}'.format(
                                   v
                                   ))
                    self.run.interp_type = v.lower()
                    self.run.int_typ = sg
                    
                    self.run.disp_max = config[ 'disp_max']
                    num_srch_dof	= config['num_srch_dof'] ### 3, 6, or 12
                    if not num_srch_dof in [3,6,12]:
                        raise ValueError('num_srch_dof can only be 3, 6 or 12. Found: ' , num_srch_dof)
                    self.run.num_srch_dof = num_srch_dof
                    
                    # optional parameters
                    if not 'basin_radius' in config.keys():
                        self.run.basin_radius = 2.0
                    else:
                        br = config['basin_radius']
                        if not (br >= 0 and br <= self.run.disp_max):
                            raise ValueError('Basin Radius out of bounds: {0} < {1} < {2}'
                                             .format(0, br, self.run.disp_max))
                    try:
                        rig_trans = numpy.asarray(config['rigid_trans'])
                        vol_size = numpy.asarray( [ config['vol_wide'],
                                                   config['vol_high'],
                                                   config['vol_tall'] ], dtype=numpy.float64)
                        if not (rig_trans < vol_size).all():
                            raise ValueError('rigid_trans out of bounds', 
                                             rig_trans, vol_size)
                        self.run.set_rigid_trans(rig_trans)
                            
                    except KeyError as ke:
                        self.run.set_subvol_aspect(numpy.zeros((3,), 
                                                    dtype=numpy.float64))
                    try:
                        subvol_aspect = numpy.asarray(config['subvol_aspect'])
                        asp_min = 0.1
                        asp_max = 10.
                        inlimits = numpy.asarray([True if i <= asp_max and \
                                        i >= asp_min else False for i in subvol_aspect ]).all()
                        if not inlimits:
                            raise ValueError('subvol_aspect out of bounds', 
                                             subvol_aspect, [asp_min, asp_max])
                        self.run.set_subvol_aspect(subvol_aspect)
                            
                    except KeyError as ke:
                        self.run.set_subvol_aspect(numpy.ones((3,), 
                                                        dtype=numpy.float64))
                    
                    
    def check_input_files(self):
        ref = os.path.abspath(self.run.ref_fname)
        cor = os.path.abspath(self.run.cor_fname)
        
        self.run.ref_fname = "a.npy"
        hd = self.run.understand_npy()
        print ("received hd" , hd)
        npyheader = eval(hd[0])
        shape = npyheader['shape']
        if npyheader['fortran_order']:
            self.run.vol_wide = shape[2]
            self.run.vol_high = shape[1]
            self.run.vol_tall = shape[0]
        else:
            self.run.vol_wide = shape[0]
            self.run.vol_high = shape[1]
            self.run.vol_tall = shape[2]
        
        # read the type of binary data
        # https://docs.scipy.org/doc/numpy/reference/generated/numpy.dtype.html#numpy.dtype
        # Endianness
        little = True if npyheader['descr'][0] == '<' else False
        if little:
            self.run.Endian_Type = dvcw.Endian_Type.little
        else:
            self.run.Endian_Type = dvcw.Endian_Type.big
        # kind (whether (un)signed integer or float )    
        if not (npyheader['descr'][1] == 'i' or npyheader['descr'][1] == 'u'):
            raise TypeError('Input other than unsigned char or unsigned short are not supperted')
        # bit depth only 8 or 16 accepted
        if (npyheader['descr'][2] == '8' or npyheader['descr'][2] == '16' )
            self.run.vol_bit_depth = int(npyheader['descr'][2])
        else:
            raise TypeError('Input bit depth not supported ', npyheader['descr'][2] )
        
        
        # header offset 
        self.run.vol_hdr_lngth = hd[1]
        
        
        
        
        
        
        
                    
    def __str__(self):
        msg = self.run.ref_fname + '\n'
        msg += self.run.cor_fname + '\n'
        msg += self.run.out_fname + '\n'
        msg += self.run.res_fname + '\n'
        msg += self.run.sta_fname + '\n'
        msg += self.run.interp_type + '\n'
        msg += self.run.obj_function + '\n'
        msg += str(self.run.gray_thresh_min) + '\n'
        msg += str(self.run.gray_thresh_max) + '\n'
        msg += str(self.run.min_vol_fract) + '\n'
        
        return msg
                
        
            
            

def distance(coord1, coord2):
    a = numpy.asarray([coord1.x(),coord1.y(),coord1.z()] )
    b = numpy.asarray([coord2.x(),coord2.y(),coord2.z()] )
    
    return numpy.sqrt(sum(numpy.power(a-b,2)))
roi = numpy.loadtxt("central_grid.roi")

np = 2
p = [dvcw.Point(roi[i][1], roi[i][2],roi[i][3]) for i in range(np)]

for i in range(np):
    print ("Point 0: [{0},{1},{2}]".format(
                  p[i].x(),
                  p[i].y(),
                  p[i].z())
    ) 


print ("Points distance: {0}, {1}".format(
        p[0].pt_dist(p[1]), distance(p[0],p[1]))
    )
    
run = dvcw.RunControl()
run.vol_tall = 10
run.ref_fname = 'pippo'
print ("RunControl", run.vol_tall, run.ref_fname)

fnc = dvcw.Objfcn_Type.SAD
run.obj_fcn = fnc

print (fnc, run.obj_fcn)

controller = DVC()
controller.config_run('example.json')

print (controller)

pc = dvcw.DataCloud()
pc.loadPointCloudFromNumpy(roi[:10])
print ("Point read from PointCloud " , pc.getPoint(1))
print ("Point in roi" , roi[1])

a = controller.run.get_rigid_trans()
print ("rigid_trans", a)
controller.run.set_rigid_trans(numpy.asarray([0,1,2], dtype=numpy.float64))
a = controller.run.get_rigid_trans()
print ("rigid_trans", a)

with open('example.json', "r") as f:
    config = json.load(f) 
print (config['rigid_trans'])
        
controller.run.ref_fname = "a.npy"
hd = controller.run.understand_npy()
print ("received hd" , hd)
details = eval(hd[0])

