from distutils.core import setup, Extension

setup(name="pyppm", version="0.0.1",
      ext_modules = [Extension("pyppm", ["pyppm.cpp"])])
