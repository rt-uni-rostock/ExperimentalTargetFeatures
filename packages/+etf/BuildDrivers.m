function BuildDrivers()
    %ETF.BuildDrivers Build or rebuild the driver blocks for the ETF toolbox.
    % 
    % DETAILS
    % This MATLAB function generates all S-functions and compiles the corresponding mex binaries for the Simulink library.

    fprintf('\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n B U I L D   S I M U L I N K - D R I V E R S\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n');

    % navigate to drivers source directory
    currentWorkingDirectory = pwd();
    thisDirectory = extractBefore(mfilename('fullpath'),strlength(mfilename('fullpath')) - strlength(mfilename) + 1);
    driverDirectory = fullfile(thisDirectory,'..','..','library','source');
    cd(driverDirectory);

    % find source files and generate specifications for all drivers
    sourceFiles = FindSourceFiles();
    defs = [];

    % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    % Driver: Binary Ring Buffer
    % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    def = legacy_code('initialize');
    def.SFunctionName           = 'SFunctionETFBinaryRingBuffer';
    def.StartFcnSpec            = 'void ETFDriver_BinaryRingBufferInitialize(void** work1, uint8 p1[], uint32 p2, uint32 p3, uint32 p4, uint32 p5, int32 p6)';
    def.TerminateFcnSpec        = 'void ETFDriver_BinaryRingBufferTerminate(void* work1)';
    def.OutputFcnSpec           = 'void ETFDriver_BinaryRingBufferStep(void* work1, uint8 y1[1], uint32 y2[1], uint8 u1[], uint8 u2)';
    def.HeaderFiles             = {'ETFDriver_BinaryRingBuffer.hpp'};
    def.SourceFiles             = [{'ETFDriver_BinaryRingBuffer.cpp'}, sourceFiles];
    def.IncPaths                = {'etf'};
    def.SrcPaths                = {'etf'};
    def.LibPaths                = {''};
    def.HostLibFiles            = {};
    def.Options.language        = 'C++';
    def.Options.useTlcWithAccel = false;
    def.SampleTime              = 'inherited';
    defs = [defs; def];


    % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    % Compile and generate all required files
    % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    % Generate SFunctions
    legacy_code('sfcn_cmex_generate', defs);

    % Compile
    cflags    = '-Wall -Wextra -mtune=native';
    cxxflags  = '-Wall -Wextra -mtune=native -std=c++20';
    ldflags   = '-Wall -Wextra -mtune=native -std=c++20';
    libraries = {'-L/usr/lib','-L/usr/local/lib','-lstdc++','-lpthread'};
    legacy_code('compile', defs, [{['CFLAGS=$CFLAGS ',cflags],['CXXFLAGS=$CXXFLAGS ',cxxflags],['LINKFLAGS=$LINKFLAGS ',ldflags]},libraries]);

    % Generate TLC
    legacy_code('sfcn_tlc_generate', defs);

    % Generate RTWMAKECFG
    legacy_code('rtwmakecfg_generate', defs);

    % Generate Simulink blocks (not required, all blocks are already in the library)
    % legacy_code('slblock_generate', defs);

    % navigate back to current working directory
    fprintf('\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n');
    cd(currentWorkingDirectory);
end

% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% PRIVATE HELPER FUNCTIONS
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function sourceFiles = FindSourceFiles()
    thisDirectory = extractBefore(mfilename('fullpath'),strlength(mfilename('fullpath')) - strlength(mfilename) + 1);
    listings = dir(fullfile(thisDirectory,'etf','*'));
    listings = listings(~[listings.isdir]);
    sourceFiles = {};
    for i = 1:numel(listings)
        if(endsWith(listings(i).name,'.c') || endsWith(listings(i).name,'.cpp'))
            sourceFiles = [sourceFiles, {listings(i).name}];
        end
    end
end

