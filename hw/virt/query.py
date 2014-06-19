import ctypes
import sys

lib = ctypes.CDLL('./libvirtdisplay.so')
getDisplay = lib.getDisplay
getDisplay.restype = ctypes.c_void_p
getDisplay.argtypes = []

getExtCodes = lib.getExtCodes
getExtCodes.restype = ctypes.c_void_p
getExtCodes.argtypes = [ctypes.c_void_p]

createOutput = lib.createOutput
createOutput.restype = ctypes.c_bool
createOutput.argtypes = [ctypes.c_void_p, ctypes.c_char_p]

if __name__ == '__main__':
    display = getDisplay()
    if not display:
        sys.exit('cannot open display')
    codes = getExtCodes(display)
    if not codes:
        sys.exit('failed to initialize extension')
    ret = createOutput(display, codes, "foobar")
    if not ret:
        sys.exit('createOutput returned error')
