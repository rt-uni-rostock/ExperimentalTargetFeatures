![](library/ETF_Icon.svg)

# Experimental Target Features
This repository provides a MATLAB/Simulink library for experimental features to be used within real-time target applications.
The following features are available:

- **Binary Ring Buffer**: Create multi-file binary ring buffers where filesystem operations are handled by a separate worker thread.


## How To Use
To use the ETF toolbox, add the `ETF.prj` project file as reference project to your own simulink project.
This will add the `library` and `packages` directories to your project path automatically.
For examples, take a look to the [examples](examples/) directory.


### Initial Setup
Depending on your operating system and MATLAB version you may need to rebuild the driver blocks of the toolbox.
Load this project in MATLAB and run
```
etf.BuildDrivers();
```
