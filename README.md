![](library/ETF_Icon.svg)

# Experimental Target Features
This repository provides a MATLAB/Simulink library for experimental features to be used within real-time target applications.
The following features are available:

- **Binary Ring Buffer**: Create multi-file binary ring buffers where filesystem operations are handled by a separate worker thread.
- **Startup File**: Read a file to a buffer at startup and output that buffer during execution.


## How To Use
To use the ETF toolbox, add the `ETF.prj` project file as reference project to your own simulink project.
This will add the `library` and `packages` directories to your project path automatically.
For examples, take a look to the [examples](examples/) directory.


### Initial Setup
You need to build the driver blocks of the toolbox once before use.
If you switch the operating system and/or MATLAB version you may need to rebuild the driver blocks.
Load the `ETF.prj` MATLAB project and run
```
etf.BuildDrivers();
```
