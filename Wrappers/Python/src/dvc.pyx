# distutils: language=c++
from libcpp.string cimport string
from libcpp.vector cimport vector
from libc.stdlib cimport malloc, free
cdef extern from "Point.h":
    cdef cppclass Point:
        Point()
        Point(double, double, double)
        move_to(double , double , double )
        move_by(double , double , double )
        double x()
        double y()
        double z()
        int ix()
        int iy()
        int iz()
        double rx()
        double ry()
        double rz()

cdef extern from "BoundBox.h":
    cdef cppclass BoundBox:
        BoundBox(Point , Point )
        set_center()
        move_to(Point , Point )
        grow_by(double )
        center_on(Point)
        contains(BoundBox)
        Point min()
        Point max()
        Point cen()
        int iwide()
        int ihigh()
        int itall()
        double box_vol() 
cdef extern from "<utility>":
    vector[Point]&& move(vector[Point]&&)

cdef extern from "Utility.h":
    cdef struct RunControl:
        string ref_fname #	 the reference image volume
        string cor_fname #	 the image volume for correlation
        string pts_fname #	 search point cloud data
        string out_fname #	 base name for output files
        string res_fname #	 dvc analysis results, the displacement file
        string sta_fname #	 dvc status file, input echo and run stats
        int vol_bit_depth
        string vol_endian
        int vol_hdr_lngth
        int vol_wide
        int vol_high
        int vol_tall

        string subvol_geom
        int subvol_size
        int subvol_npts
    
        string subvol_thresh
        double gray_thresh_min
        double gray_thresh_max
        double min_vol_fract

        int disp_max
        int num_srch_dof
    
        #Objfcn_Type obj_fcn
        #Interp_Type int_typ
        #Subvol_Type sub_geo
    
        #string obj_function
        #string interp_type

        #vector[double] rigid_trans
        #double basin_radius
        #vector[double] subvol_aspect
        #string fine_srch

cdef extern from "DataCloud.h":
    cdef struct ResultRecord:
        int status              # as defined in Utility.h
        double obj_min			# value at minumum
        vector[double] par_min  #	 parameters at min [ndof]


    cdef cppclass DataCloud:
        DataCloud()
        void organize_cloud(RunControl *run)
        vector[vector[ResultRecord]] results	# [nres][npts]

cdef extern from "Cloud.h":
    cdef cppclass Cloud:
        Cloud()
        vector[Point] ptvect
        AddPoint(Point)
        BoundBox *bbox()
        reset_box(Point , Point )
        echo_summary()
        echo_content()

cdef extern from "InputRead.h":
    cdef cppclass InputRead:
        InputRead()
        int input_file_accessible(string)
        int input_file_read(RunControl*)
        int read_point_cloud(RunControl*, vector[Point] &, vector[int] &)
        string limits_to_string(vector[int] val);
        int echo_input(RunControl *run)

cdef class PyPoint:
    cdef Point *c_point     # hold a C++ instance which we're wrapping
    def __cinit__(self, double xx, double yy, double zz):
        self.c_point = new Point(xx, yy, zz)
    def __dealloc__(self):
        del self.c_point

cdef class PyPoint2:
    cdef Point* thisptr
    cdef PyPointVector vector # keep this alive, since it owns the peak rather that PyPeak2

    def __cinit__(self,PyPointVector vec,idx):
        self.vector = vec
        self.thisptr = &vec.vec[idx]

cdef class PyPointVector:
    cdef vector[Point] vec

    cdef move_from(self, vector[Point]&& move_this):
        self.vec = move(move_this)

    def __getitem__(self,idx):
        return PyPoint(self,idx)

    def __len__(self):
        return self.vec.size()

cdef class PyRunControl:
    cdef RunControl *c_runcontrol
    def __cinit__(self):
        self.c_runcontrol = <RunControl *> malloc(sizeof(RunControl))
    def __dealloc__(self):
        free(self.c_runcontrol)

cdef class PyInputRead:
    cdef InputRead *c_inputread
    def __cinit__(self):
        self.c_inputread = new InputRead()
    def __dealloc__(self):
        del self.c_inputread
    def input_file_accessible(self, string filename):
        self.c_inputread.input_file_accessible(filename)
    def input_file_read(self, PyRunControl rc):
        self.c_inputread.input_file_read(rc.c_runcontrol)
    def read_point_cloud(self, PyRunControl rc,  PyPointVector pv,  search_labels):
        self.c_inputread.read_point_cloud(rc.c_runcontrol, pv.vec, search_labels)
    def limits_to_string(self, vals):
        return self.c_inputread.limits_to_string(vals);

    def echo_input(self, PyRunControl rc):
        self.c_inputread.echo_input(rc.c_runcontrol)

cdef extern from "dvc_cmd.h":
    cpdef int dvc_cmd(string filename)

