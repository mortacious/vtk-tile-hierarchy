![VTK Tile Hierarchy](example.png)


# VTK Tile Hierarchy

A custom vtk module that provides classes to load and visualize large out-of-core datasets 
arranged in a multi-resolution spatial data structure (e.g. [potree](https://github.com/potree/potree) datasets).

## Installation

### From source (Python)

```bash
git clone https://github.com/mortacious/vtk-tile-hierarchy.git
cd vtk-tile-hierarchy
python setup.py install
```


### From source (C++)
```bash
git clone https://github.com/mortacious/vtk-tile-hierarchy.git
cd vtk-tile-hierarchy
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make && sudo make install
```

## Acknowledgements
The rendering algorithm used in this package was originally developed by Markus Schütz as part of his 
[thesis](https://www.cg.tuwien.ac.at/research/publications/2016/SCHUETZ-2016-POT/SCHUETZ-2016-POT-thesis.pdf).

