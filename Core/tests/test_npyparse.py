
try:
    import numpy as np
except ImportError as e:
    has_numpy = False
else:
    has_numpy = True

import tempfile, os

import unittest
import subprocess

class TestNpyParse(unittest.TestCase):
    def setUp(self):
        tempfiles = []
        for dtype in [np.uint8, np.int8, np.uint16, np.int16]:
            v = np.ones((2, 3,4), dtype=dtype)

            fd, path = tempfile.mkstemp(suffix=".npy")
            np.save(path, v)
            tempfiles.append((fd, path))
        self.tempfiles = tempfiles
    def tearDown(self):
        for fd, path in getattr(self, "tempfiles", []):
            try:
                os.close(fd)
                os.remove(path)
            except OSError:
                pass
    
    @unittest.skipIf(not has_numpy, "NumPy is not installed")
    def test_numpy_header_read(self):
        import numpy.lib.format as fmt
        for fd, path in getattr(self, "tempfiles", []):
            with open(path, 'rb') as f:
                major, minor = fmt.read_magic(f)
                if major == 1:
                    shape, fortran_order, dtype = fmt.read_array_header_1_0(f)
                elif major == 2:
                    shape, fortran_order, dtype = fmt.read_array_header_2_0(f)
                else:
                    raise ValueError(f"Unsupported NPY version {major}.{minor}")
                
                header_len = f.tell() - (10 if major == 1 else 12)
                
                is_big_endian = 0 if dtype.str[0] == '<' else 1
                
                if dtype.str[2] == '1':
                    bit = 8
                elif dtype.str[2] == '2':
                    bit = 16
                else:
                    bit = 0
                    
                if major == 1:
                    data_offset = 10 + header_len
                elif major == 2:
                    data_offset = 12 + header_len
                else:
                    data_offset = 0
                    
                is_fortran = 1 if fortran_order else 0
                
                padded_shape = list(shape) + [1]*(3-len(shape))
                X, Y, Z = padded_shape[0], padded_shape[1], padded_shape[2]
                
                expected = f"{path} {X} {Y} {Z} {is_big_endian} {bit} {is_fortran} {header_len} {major} {minor} {data_offset}"
                
            exe_path = os.environ.get("TEST_NPYPARSE_EXE", "./test_npyparse")
            result = subprocess.run([exe_path, path], capture_output=True, text=True)
            print(f"Expected: {expected}")
            print(f"Got: {result.stdout.strip()}")
            self.assertEqual(expected, result.stdout.strip())
