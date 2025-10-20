function BuildDrivers(generateSimulinkBlocks)
    %ETF.BuildDrivers Build or rebuild the driver blocks for the ETF toolbox.
    % 
    % PARAMETERS
    % generateSimulinkBlocks ... True if simulink blocks should be generated in a new simulink model. Default value is false.
    %
    % DETAILS
    % This MATLAB function generates all S-functions and compiles the corresponding mex binaries for the Simulink library.

    arguments
        generateSimulinkBlocks (1,1) logical = false
    end

    % print banner
    fprintf('\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n');
    fprintf(' ________  _________  ________\n');
    fprintf('|_   __  ||  _   _  ||_   __  |\n');
    fprintf('  | |_ \\_||_/ | | \\_|  | |_ \\_|\n');
    fprintf('  |  _| _     | |      |  _|\n');
    fprintf(' _| |__/ |   _| |_    _| |_\n');
    fprintf('|________|  |_____|  |_____|\n\n');
    fprintf(' Experimental Target Features\n');
    fprintf(' %s\n', char(strjoin(string(etf.GetVersion()),'.')));
    fprintf('~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n');

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
    % Driver: Startup File
    % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    def = legacy_code('initialize');
    def.SFunctionName           = 'SFunctionETFStartupFile';
    def.StartFcnSpec            = 'void ETFDriver_StartupFileInitialize(void** work1, uint8 p1[], uint32 p2, uint32 p3)';
    def.TerminateFcnSpec        = 'void ETFDriver_StartupFileTerminate(void* work1)';
    def.OutputFcnSpec           = 'void ETFDriver_StartupFileStep(void* work1, uint8 y1[p3], uint32 y2[1], uint32 p3)';
    def.HeaderFiles             = {'ETFDriver_StartupFile.hpp'};
    def.SourceFiles             = [{'ETFDriver_StartupFile.cpp'}, sourceFiles];
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
    % generate SFunctions
    fprintf('generate S-functions: ');
    legacy_code('sfcn_cmex_generate', defs);
    fprintf('done\n');

    % Compile
    cflags    = '-Wall -Wextra -mtune=native';
    cxxflags  = '-Wall -Wextra -mtune=native -std=c++20';
    ldflags   = '-Wall -Wextra -mtune=native -std=c++20';
    libraries = {'-L/usr/lib','-L/usr/local/lib','-lstdc++','-lpthread'};
    legacy_code('compile', defs, [{['CFLAGS=$CFLAGS ',cflags],['CXXFLAGS=$CXXFLAGS ',cxxflags],['LINKFLAGS=$LINKFLAGS ',ldflags]},libraries]);

    % generate TLC
    fprintf('\ngenerate TLC: ');
    legacy_code('sfcn_tlc_generate', defs);
    fprintf('done\n');

    % generate RTWMAKECFG
    fprintf('generate rtwmakecfg: ');
    legacy_code('rtwmakecfg_generate', defs);
    fprintf('done\n');

    % Generate Simulink blocks (not required, all blocks are already in the library)
    if(generateSimulinkBlocks)
        fprintf('generate simulink blocks: ');
        legacy_code('slblock_generate', defs);
        fprintf('done\n');
    end

    % navigate back to current working directory
    cd(currentWorkingDirectory);

    % print footer
    fprintf('\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n');
    fprintf(' ETF DRIVER BUILD COMPLETED\n');
    fprintf('~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n');
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

