from setuptools import setup, Extension

ext = Extension(
    '_parqueadero',
    sources=[
        'src/cpp/parqueadero.cpp',
        'src/cpp/parqueadero_wrap.cxx'
    ],
    include_dirs=['src/cpp'],
    extra_compile_args=['-std=c++11'],
)

setup(name='parqueadero', ext_modules=[ext])