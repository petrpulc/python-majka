#!/usr/bin/env python3
"""
Setup script for distutils, module majka.
"""

from distutils.core import setup, Extension

setup(name='majka',
      version='0.6',
      description='Wrapper for Majka morphological analyser',
      long_description='''
Native cPython binding to a C++ implementation of morphological analyser named Majka.

Tags returned from the analyser are also transcribed into a native Python dictionary
to enable a much more Python-like experience without a need to study the documentation.

Based on code of Pavel Smerk and Pavel Rychly, NLP group at MUNI, Czech Republic.

Morphological automatons are distributed separately under different licences.
See http://nlp.fi.muni.cz/ma/''',
      author='Petr Pulc',
      author_email='petrpulc@gmail.com',
      url='https://github.com/petrpulc/python-majka',
      ext_modules=[Extension(name='majka',
                             sources=['majka/majka.cc', 'majkamodule.cpp'],
                             define_macros=[('UTF', 1)],
                             language='c++')],
      classifiers=['Environment :: Plugins',
                   'Intended Audience :: Science/Research',
                   'License :: OSI Approved :: GNU General Public License v2 (GPLv2)',
                   'Programming Language :: C++',
                   'Topic :: Scientific/Engineering :: Information Analysis',
                   'Topic :: Text Processing :: Linguistic']
     )
