// include: shell.js
// The Module object: Our interface to the outside world. We import
// and export values on it. There are various ways Module can be used:
// 1. Not defined. We create it here
// 2. A function parameter, function(moduleArg) => Promise<Module>
// 3. pre-run appended it, var Module = {}; ..generated code..
// 4. External script tag defines var Module.
// We need to check if Module already exists (e.g. case 3 above).
// Substitution will be replaced with actual code on later stage of the build,
// this way Closure Compiler will not mangle it (e.g. case 4. above).
// Note that if you want to run closure, and also to use Module
// after the generated code, you will need to define   var Module = {};
// before the code. Then that object will be used in the code, and you
// can continue to use Module afterwards as well.
var Module = typeof Module != 'undefined' ? Module : {};

// Determine the runtime environment we are in. You can customize this by
// setting the ENVIRONMENT setting at compile time (see settings.js).

// Attempt to auto-detect the environment
var ENVIRONMENT_IS_WEB = typeof window == 'object';
var ENVIRONMENT_IS_WORKER = typeof WorkerGlobalScope != 'undefined';
// N.b. Electron.js environment is simultaneously a NODE-environment, but
// also a web environment.
var ENVIRONMENT_IS_NODE = typeof process == 'object' && typeof process.versions == 'object' && typeof process.versions.node == 'string' && process.type != 'renderer';
var ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER;

if (ENVIRONMENT_IS_NODE) {

}

// --pre-jses are emitted after the Module integration code, so that they can
// refer to Module (if they choose; they can also define Module)
// include: C:\Users\Admin\AppData\Local\Temp\tmpa3dy03o8.js

  Module['expectedDataFileDownloads'] ??= 0;
  Module['expectedDataFileDownloads']++;
  (() => {
    // Do not attempt to redownload the virtual filesystem data when in a pthread or a Wasm Worker context.
    var isPthread = typeof ENVIRONMENT_IS_PTHREAD != 'undefined' && ENVIRONMENT_IS_PTHREAD;
    var isWasmWorker = typeof ENVIRONMENT_IS_WASM_WORKER != 'undefined' && ENVIRONMENT_IS_WASM_WORKER;
    if (isPthread || isWasmWorker) return;
    var isNode = typeof process === 'object' && typeof process.versions === 'object' && typeof process.versions.node === 'string';
    function loadPackage(metadata) {

      var PACKAGE_PATH = '';
      if (typeof window === 'object') {
        PACKAGE_PATH = window['encodeURIComponent'](window.location.pathname.substring(0, window.location.pathname.lastIndexOf('/')) + '/');
      } else if (typeof process === 'undefined' && typeof location !== 'undefined') {
        // web worker
        PACKAGE_PATH = encodeURIComponent(location.pathname.substring(0, location.pathname.lastIndexOf('/')) + '/');
      }
      var PACKAGE_NAME = 'main.data';
      var REMOTE_PACKAGE_BASE = 'main.data';
      var REMOTE_PACKAGE_NAME = Module['locateFile'] ? Module['locateFile'](REMOTE_PACKAGE_BASE, '') : REMOTE_PACKAGE_BASE;
var REMOTE_PACKAGE_SIZE = metadata['remote_package_size'];

      function fetchRemotePackage(packageName, packageSize, callback, errback) {
        if (isNode) {
          require('fs').readFile(packageName, (err, contents) => {
            if (err) {
              errback(err);
            } else {
              callback(contents.buffer);
            }
          });
          return;
        }
        Module['dataFileDownloads'] ??= {};
        fetch(packageName)
          .catch((cause) => Promise.reject(new Error(`Network Error: ${packageName}`, {cause}))) // If fetch fails, rewrite the error to include the failing URL & the cause.
          .then((response) => {
            if (!response.ok) {
              return Promise.reject(new Error(`${response.status}: ${response.url}`));
            }

            if (!response.body && response.arrayBuffer) { // If we're using the polyfill, readers won't be available...
              return response.arrayBuffer().then(callback);
            }

            const reader = response.body.getReader();
            const iterate = () => reader.read().then(handleChunk).catch((cause) => {
              return Promise.reject(new Error(`Unexpected error while handling : ${response.url} ${cause}`, {cause}));
            });

            const chunks = [];
            const headers = response.headers;
            const total = Number(headers.get('Content-Length') ?? packageSize);
            let loaded = 0;

            const handleChunk = ({done, value}) => {
              if (!done) {
                chunks.push(value);
                loaded += value.length;
                Module['dataFileDownloads'][packageName] = {loaded, total};

                let totalLoaded = 0;
                let totalSize = 0;

                for (const download of Object.values(Module['dataFileDownloads'])) {
                  totalLoaded += download.loaded;
                  totalSize += download.total;
                }

                Module['setStatus']?.(`Downloading data... (${totalLoaded}/${totalSize})`);
                return iterate();
              } else {
                const packageData = new Uint8Array(chunks.map((c) => c.length).reduce((a, b) => a + b, 0));
                let offset = 0;
                for (const chunk of chunks) {
                  packageData.set(chunk, offset);
                  offset += chunk.length;
                }
                callback(packageData.buffer);
              }
            };

            Module['setStatus']?.('Downloading data...');
            return iterate();
          });
      };

      function handleError(error) {
        console.error('package error:', error);
      };

      var fetchedCallback = null;
      var fetched = Module['getPreloadedPackage'] ? Module['getPreloadedPackage'](REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE) : null;

      if (!fetched) fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE, (data) => {
        if (fetchedCallback) {
          fetchedCallback(data);
          fetchedCallback = null;
        } else {
          fetched = data;
        }
      }, handleError);

    function runWithFS(Module) {

      function assert(check, msg) {
        if (!check) throw msg + new Error().stack;
      }
Module['FS_createPath']("/", "GameData", true, true);
Module['FS_createPath']("/GameData", "Fonts", true, true);
Module['FS_createPath']("/GameData", "Maps", true, true);
Module['FS_createPath']("/GameData/Maps", "autosave", true, true);
Module['FS_createPath']("/GameData", "Shaders", true, true);
Module['FS_createPath']("/GameData", "Sounds", true, true);
Module['FS_createPath']("/GameData/Sounds", "Dog", true, true);
Module['FS_createPath']("/GameData", "Textures", true, true);
Module['FS_createPath']("/GameData/Textures", "brushes", true, true);
Module['FS_createPath']("/GameData/Textures/brushes", "Ground", true, true);
Module['FS_createPath']("/GameData/Textures/brushes", "Metal", true, true);
Module['FS_createPath']("/GameData/Textures/brushes", "Wall", true, true);
Module['FS_createPath']("/GameData/Textures/brushes", "Water", true, true);
Module['FS_createPath']("/GameData/Textures/brushes", "Wood", true, true);
Module['FS_createPath']("/GameData/Textures/brushes", "delvenPack", true, true);
Module['FS_createPath']("/GameData/Textures/brushes", "libreQuake", true, true);
Module['FS_createPath']("/GameData/Textures/brushes/libreQuake", "lq_conc", true, true);
Module['FS_createPath']("/GameData/Textures/brushes/libreQuake", "lq_dev", true, true);
Module['FS_createPath']("/GameData/Textures/brushes/libreQuake", "lq_flesh", true, true);
Module['FS_createPath']("/GameData/Textures/brushes/libreQuake", "lq_greek", true, true);
Module['FS_createPath']("/GameData/Textures/brushes/libreQuake", "lq_health_ammo", true, true);
Module['FS_createPath']("/GameData/Textures/brushes/libreQuake", "lq_legacy", true, true);
Module['FS_createPath']("/GameData/Textures/brushes/libreQuake", "lq_liquidsky", true, true);
Module['FS_createPath']("/GameData/Textures/brushes/libreQuake", "lq_mayan", true, true);
Module['FS_createPath']("/GameData/Textures/brushes/libreQuake", "lq_medieval", true, true);
Module['FS_createPath']("/GameData/Textures/brushes/libreQuake", "lq_metal", true, true);
Module['FS_createPath']("/GameData/Textures/brushes/libreQuake", "lq_palette", true, true);
Module['FS_createPath']("/GameData/Textures/brushes/libreQuake", "lq_props", true, true);
Module['FS_createPath']("/GameData/Textures/brushes/libreQuake", "lq_tech", true, true);
Module['FS_createPath']("/GameData/Textures/brushes/libreQuake", "lq_terra", true, true);
Module['FS_createPath']("/GameData/Textures/brushes/libreQuake", "lq_utility", true, true);
Module['FS_createPath']("/GameData/Textures/brushes/libreQuake", "lq_wood", true, true);
Module['FS_createPath']("/GameData/Textures/brushes", "tormentPack", true, true);
Module['FS_createPath']("/GameData/Textures", "particles", true, true);

      /** @constructor */
      function DataRequest(start, end, audio) {
        this.start = start;
        this.end = end;
        this.audio = audio;
      }
      DataRequest.prototype = {
        requests: {},
        open: function(mode, name) {
          this.name = name;
          this.requests[name] = this;
          Module['addRunDependency'](`fp ${this.name}`);
        },
        send: function() {},
        onload: function() {
          var byteArray = this.byteArray.subarray(this.start, this.end);
          this.finish(byteArray);
        },
        finish: function(byteArray) {
          var that = this;
          // canOwn this data in the filesystem, it is a slide into the heap that will never change
          Module['FS_createDataFile'](this.name, null, byteArray, true, true, true);
          Module['removeRunDependency'](`fp ${that.name}`);
          this.requests[this.name] = null;
        }
      };

      var files = metadata['files'];
      for (var i = 0; i < files.length; ++i) {
        new DataRequest(files[i]['start'], files[i]['end'], files[i]['audio'] || 0).open('GET', files[i]['filename']);
      }

      function processPackageData(arrayBuffer) {
        assert(arrayBuffer, 'Loading data file failed.');
        assert(arrayBuffer.constructor.name === ArrayBuffer.name, 'bad input to processPackageData');
        var byteArray = new Uint8Array(arrayBuffer);
        var curr;
        // Reuse the bytearray from the XHR as the source for file reads.
          DataRequest.prototype.byteArray = byteArray;
          var files = metadata['files'];
          for (var i = 0; i < files.length; ++i) {
            DataRequest.prototype.requests[files[i].filename].onload();
          }          Module['removeRunDependency']('datafile_main.data');

      };
      Module['addRunDependency']('datafile_main.data');

      Module['preloadResults'] ??= {};

      Module['preloadResults'][PACKAGE_NAME] = {fromCache: false};
      if (fetched) {
        processPackageData(fetched);
        fetched = null;
      } else {
        fetchedCallback = processPackageData;
      }

    }
    if (Module['calledRun']) {
      runWithFS(Module);
    } else {
      (Module['preRun'] ??= []).push(runWithFS); // FS is not initialized yet, wait for it
    }

    }
    loadPackage({"files": [{"filename": "/GameData/Fonts/Kingthings_Calligraphica_2.ttf", "start": 0, "end": 29804}, {"filename": "/GameData/Maps/autosave/test.1.map", "start": 29804, "end": 45755}, {"filename": "/GameData/Maps/autosave/test.2.map", "start": 45755, "end": 61803}, {"filename": "/GameData/Maps/autosave/test.3.map", "start": 61803, "end": 77961}, {"filename": "/GameData/Maps/autosave/test2.1.map", "start": 77961, "end": 734529}, {"filename": "/GameData/Maps/e1m1.map", "start": 734529, "end": 10746058}, {"filename": "/GameData/Maps/e1m1.obj", "start": 10746058, "end": 20591079}, {"filename": "/GameData/Maps/test.map", "start": 20591079, "end": 20607223}, {"filename": "/GameData/Maps/test.mtl", "start": 20607223, "end": 20607500}, {"filename": "/GameData/Maps/test.obj", "start": 20607500, "end": 20626016}, {"filename": "/GameData/Maps/test2.map", "start": 20626016, "end": 21283891}, {"filename": "/GameData/Maps/test2.mtl", "start": 21283891, "end": 21284586}, {"filename": "/GameData/Maps/test2.obj", "start": 21284586, "end": 22187883}, {"filename": "/GameData/Shaders/default_pixel.frag", "start": 22187883, "end": 22193711}, {"filename": "/GameData/Shaders/default_vertex.vert", "start": 22193711, "end": 22195914}, {"filename": "/GameData/Shaders/empty_pixel.frag", "start": 22195914, "end": 22195981}, {"filename": "/GameData/Shaders/fullscreen_vertex.vert", "start": 22195981, "end": 22196185}, {"filename": "/GameData/Shaders/instanced_bilboard_vertex.vert", "start": 22196185, "end": 22198124}, {"filename": "/GameData/Shaders/solidRed_pixel.frag", "start": 22198124, "end": 22198293}, {"filename": "/GameData/Shaders/texture_pixel.frag", "start": 22198293, "end": 22198481}, {"filename": "/GameData/Shaders/ui.vert", "start": 22198481, "end": 22198784}, {"filename": "/GameData/Shaders/ui_flatcolor.frag", "start": 22198784, "end": 22198920}, {"filename": "/GameData/Shaders/ui_textured.frag", "start": 22198920, "end": 22199177}, {"filename": "/GameData/Sounds/Dog/Death.wav", "start": 22199177, "end": 22272563, "audio": 1}, {"filename": "/GameData/Sounds/mew.wav", "start": 22272563, "end": 22345949, "audio": 1}, {"filename": "/GameData/Textures/M_Shotgun_Base_Color.png", "start": 22345949, "end": 22721091}, {"filename": "/GameData/Textures/arms.png", "start": 22721091, "end": 22761335}, {"filename": "/GameData/Textures/brushes/Ground/grass.png", "start": 22761335, "end": 22765119}, {"filename": "/GameData/Textures/brushes/Metal/metal1.png", "start": 22765119, "end": 23189175}, {"filename": "/GameData/Textures/brushes/Wall/brickWall1.png", "start": 23189175, "end": 23197391}, {"filename": "/GameData/Textures/brushes/Wall/brickWall2.png", "start": 23197391, "end": 23205515}, {"filename": "/GameData/Textures/brushes/Wall/brickWall3.png", "start": 23205515, "end": 23213398}, {"filename": "/GameData/Textures/brushes/Water/Water1_t.png", "start": 23213398, "end": 23526499}, {"filename": "/GameData/Textures/brushes/Wood/wood1.png", "start": 23526499, "end": 23874851}, {"filename": "/GameData/Textures/brushes/__TB_empty.png", "start": 23874851, "end": 23875609}, {"filename": "/GameData/Textures/brushes/brick.png", "start": 23875609, "end": 24388523}, {"filename": "/GameData/Textures/brushes/brickPBR.png", "start": 24388523, "end": 25318655}, {"filename": "/GameData/Textures/brushes/brickPBR_orm.png", "start": 25318655, "end": 25743506}, {"filename": "/GameData/Textures/brushes/bricks.png", "start": 25743506, "end": 25754426}, {"filename": "/GameData/Textures/brushes/cat.png", "start": 25754426, "end": 26003864}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door1a.png", "start": 26003864, "end": 26011648}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door1b.png", "start": 26011648, "end": 26020838}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door1c.png", "start": 26020838, "end": 26028689}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door1d.png", "start": 26028689, "end": 26037861}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door2a.png", "start": 26037861, "end": 26045585}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door2b.png", "start": 26045585, "end": 26054710}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door2c.png", "start": 26054710, "end": 26062738}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door2d.png", "start": 26062738, "end": 26071966}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door3a.png", "start": 26071966, "end": 26076114}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door3b.png", "start": 26076114, "end": 26080892}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door3c.png", "start": 26080892, "end": 26085042}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door3d.png", "start": 26085042, "end": 26089834}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door4a.png", "start": 26089834, "end": 26093955}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door4b.png", "start": 26093955, "end": 26098746}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door4c.png", "start": 26098746, "end": 26103016}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door4d.png", "start": 26103016, "end": 26107852}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric1a.png", "start": 26107852, "end": 26113228}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric1b.png", "start": 26113228, "end": 26119356}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric1c.png", "start": 26119356, "end": 26125650}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric1d.png", "start": 26125650, "end": 26129286}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric1e.png", "start": 26129286, "end": 26132972}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric2a.png", "start": 26132972, "end": 26137697}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric2b.png", "start": 26137697, "end": 26143148}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric2c.png", "start": 26143148, "end": 26148826}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric2d.png", "start": 26148826, "end": 26152112}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric2e.png", "start": 26152112, "end": 26155473}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric3a.png", "start": 26155473, "end": 26160427}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric3b.png", "start": 26160427, "end": 26166098}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric3c.png", "start": 26166098, "end": 26171923}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric3d.png", "start": 26171923, "end": 26175303}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric3e.png", "start": 26175303, "end": 26178745}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric4a.png", "start": 26178745, "end": 26188163}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric4b.png", "start": 26188163, "end": 26198128}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric4c.png", "start": 26198128, "end": 26208257}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric4d.png", "start": 26208257, "end": 26213781}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric4e.png", "start": 26213781, "end": 26219351}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric5a.png", "start": 26219351, "end": 26228418}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric5b.png", "start": 26228418, "end": 26238065}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric5c.png", "start": 26238065, "end": 26247865}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric5d.png", "start": 26247865, "end": 26253275}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric5e.png", "start": 26253275, "end": 26258709}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric6a.png", "start": 26258709, "end": 26265843}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric6b.png", "start": 26265843, "end": 26273571}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric6c.png", "start": 26273571, "end": 26281466}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric6d.png", "start": 26281466, "end": 26285866}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric6e.png", "start": 26285866, "end": 26290324}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground1a.png", "start": 26290324, "end": 26298246}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground1b.png", "start": 26298246, "end": 26307082}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground1c.png", "start": 26307082, "end": 26316578}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground1d.png", "start": 26316578, "end": 26326621}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground2a.png", "start": 26326621, "end": 26336489}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground2b.png", "start": 26336489, "end": 26345396}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground2c.png", "start": 26345396, "end": 26355121}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground2d.png", "start": 26355121, "end": 26365542}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground3a.png", "start": 26365542, "end": 26375601}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground3b.png", "start": 26375601, "end": 26385313}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground3c.png", "start": 26385313, "end": 26394749}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground4a.png", "start": 26394749, "end": 26404940}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground4b.png", "start": 26404940, "end": 26415390}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground4c.png", "start": 26415390, "end": 26425331}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litmetal1a.png", "start": 26425331, "end": 26429359}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litmetal1b.png", "start": 26429359, "end": 26434071}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litmetal1c.png", "start": 26434071, "end": 26438152}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litmetal2a.png", "start": 26438152, "end": 26442205}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litmetal2b.png", "start": 26442205, "end": 26446964}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litmetal2c.png", "start": 26446964, "end": 26450835}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litstone1a.png", "start": 26450835, "end": 26454213}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litstone1b.png", "start": 26454213, "end": 26458426}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litstone1c.png", "start": 26458426, "end": 26462057}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litstone2a.png", "start": 26462057, "end": 26466854}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litstone2b.png", "start": 26466854, "end": 26472380}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litstone2c.png", "start": 26472380, "end": 26476623}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalgen1.png", "start": 26476623, "end": 26482448}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalgen2.png", "start": 26482448, "end": 26489328}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan1a.png", "start": 26489328, "end": 26497386}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan1b.png", "start": 26497386, "end": 26504856}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan1c.png", "start": 26504856, "end": 26514058}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan2a.png", "start": 26514058, "end": 26522385}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan2b.png", "start": 26522385, "end": 26529797}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan2c.png", "start": 26529797, "end": 26538996}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan3a.png", "start": 26538996, "end": 26547603}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan3b.png", "start": 26547603, "end": 26555830}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan3c.png", "start": 26555830, "end": 26565132}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan4a.png", "start": 26565132, "end": 26573893}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan4b.png", "start": 26573893, "end": 26582218}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan4c.png", "start": 26582218, "end": 26591673}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip1a.png", "start": 26591673, "end": 26598317}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip1b.png", "start": 26598317, "end": 26604829}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip2a.png", "start": 26604829, "end": 26611962}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip2b.png", "start": 26611962, "end": 26618980}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip3a.png", "start": 26618980, "end": 26626251}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip3b.png", "start": 26626251, "end": 26633329}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip4a.png", "start": 26633329, "end": 26640897}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip4b.png", "start": 26640897, "end": 26648252}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm1a.png", "start": 26648252, "end": 26655250}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm1b.png", "start": 26655250, "end": 26661783}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm1c.png", "start": 26661783, "end": 26669971}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm2a.png", "start": 26669971, "end": 26677468}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm2b.png", "start": 26677468, "end": 26684056}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm2c.png", "start": 26684056, "end": 26692317}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm3a.png", "start": 26692317, "end": 26699593}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm3b.png", "start": 26699593, "end": 26706519}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm3c.png", "start": 26706519, "end": 26714514}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm4a.png", "start": 26714514, "end": 26721394}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm4b.png", "start": 26721394, "end": 26728102}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm4c.png", "start": 26728102, "end": 26735911}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm5.png", "start": 26735911, "end": 26739570}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm6.png", "start": 26739570, "end": 26743293}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr1a.png", "start": 26743293, "end": 26750085}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr1b.png", "start": 26750085, "end": 26758456}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr1c.png", "start": 26758456, "end": 26766126}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr1d.png", "start": 26766126, "end": 26774243}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr1e.png", "start": 26774243, "end": 26782135}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr2a.png", "start": 26782135, "end": 26790326}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr2b.png", "start": 26790326, "end": 26799838}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr2c.png", "start": 26799838, "end": 26808740}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr2d.png", "start": 26808740, "end": 26817991}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr2e.png", "start": 26817991, "end": 26827062}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr3a.png", "start": 26827062, "end": 26835193}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr3b.png", "start": 26835193, "end": 26844903}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr3c.png", "start": 26844903, "end": 26853875}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr3d.png", "start": 26853875, "end": 26863349}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr3e.png", "start": 26863349, "end": 26872576}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr4a.png", "start": 26872576, "end": 26881082}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr4b.png", "start": 26881082, "end": 26889542}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr4c.png", "start": 26889542, "end": 26898206}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr5a.png", "start": 26898206, "end": 26905385}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr5b.png", "start": 26905385, "end": 26912513}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr5c.png", "start": 26912513, "end": 26920653}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr6a.png", "start": 26920653, "end": 26929329}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr6b.png", "start": 26929329, "end": 26937744}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr6c.png", "start": 26937744, "end": 26946477}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slategen1.png", "start": 26946477, "end": 26952438}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slategen2.png", "start": 26952438, "end": 26959997}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk1a.png", "start": 26959997, "end": 26965947}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk1b.png", "start": 26965947, "end": 26971763}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk1c.png", "start": 26971763, "end": 26977707}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk1d.png", "start": 26977707, "end": 26983353}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk2a.png", "start": 26983353, "end": 26992225}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk2b.png", "start": 26992225, "end": 27000899}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk2c.png", "start": 27000899, "end": 27009696}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk2d.png", "start": 27009696, "end": 27017992}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk3a.png", "start": 27017992, "end": 27026439}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk3b.png", "start": 27026439, "end": 27035086}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk4a.png", "start": 27035086, "end": 27041705}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk4b.png", "start": 27041705, "end": 27048391}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk4c.png", "start": 27048391, "end": 27055008}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk4d.png", "start": 27055008, "end": 27061622}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk4e.png", "start": 27061622, "end": 27068206}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk4f.png", "start": 27068206, "end": 27074563}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk5a.png", "start": 27074563, "end": 27084040}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk5b.png", "start": 27084040, "end": 27093552}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk5c.png", "start": 27093552, "end": 27103050}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk5d.png", "start": 27103050, "end": 27112523}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk5e.png", "start": 27112523, "end": 27121914}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk5f.png", "start": 27121914, "end": 27131051}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk6a.png", "start": 27131051, "end": 27139976}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk6b.png", "start": 27139976, "end": 27149201}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr1a.png", "start": 27149201, "end": 27154631}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr1b.png", "start": 27154631, "end": 27161053}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr1c.png", "start": 27161053, "end": 27168076}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr2a.png", "start": 27168076, "end": 27175964}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr2b.png", "start": 27175964, "end": 27184779}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr2c.png", "start": 27184779, "end": 27194060}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr3a.png", "start": 27194060, "end": 27199763}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr3b.png", "start": 27199763, "end": 27206241}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr3c.png", "start": 27206241, "end": 27213233}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr4a.png", "start": 27213233, "end": 27221625}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr4b.png", "start": 27221625, "end": 27230694}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr4c.png", "start": 27230694, "end": 27239794}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr5a.png", "start": 27239794, "end": 27245884}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr5b.png", "start": 27245884, "end": 27252297}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr5c.png", "start": 27252297, "end": 27259215}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr6a.png", "start": 27259215, "end": 27267760}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr6b.png", "start": 27267760, "end": 27276580}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr6c.png", "start": 27276580, "end": 27285864}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonegen1.png", "start": 27285864, "end": 27290958}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonegen2.png", "start": 27290958, "end": 27298388}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonestep1a.png", "start": 27298388, "end": 27303831}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonestep1b.png", "start": 27303831, "end": 27309371}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonestep2a.png", "start": 27309371, "end": 27317220}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonestep2b.png", "start": 27317220, "end": 27324970}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm1a.png", "start": 27324970, "end": 27330260}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm1b.png", "start": 27330260, "end": 27336360}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm1c.png", "start": 27336360, "end": 27341936}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm2a.png", "start": 27341936, "end": 27349682}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm2b.png", "start": 27349682, "end": 27358512}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm2c.png", "start": 27358512, "end": 27366820}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm3a.png", "start": 27366820, "end": 27372120}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm3b.png", "start": 27372120, "end": 27378193}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm4a.png", "start": 27378193, "end": 27386015}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm4b.png", "start": 27386015, "end": 27394546}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm5.png", "start": 27394546, "end": 27397703}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm6.png", "start": 27397703, "end": 27402058}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonewaf1a.png", "start": 27402058, "end": 27407669}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonewaf1b.png", "start": 27407669, "end": 27413286}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonewaf2a.png", "start": 27413286, "end": 27421159}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonewaf2b.png", "start": 27421159, "end": 27429268}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood1a.png", "start": 27429268, "end": 27433967}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood1b.png", "start": 27433967, "end": 27438926}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood2a.png", "start": 27438926, "end": 27445622}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood2b.png", "start": 27445622, "end": 27452579}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood3a.png", "start": 27452579, "end": 27457783}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood3b.png", "start": 27457783, "end": 27463241}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood4a.png", "start": 27463241, "end": 27470073}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood4b.png", "start": 27470073, "end": 27477118}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5a.png", "start": 27477118, "end": 27483131}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5b.png", "start": 27483131, "end": 27489966}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5c.png", "start": 27489966, "end": 27496976}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5d.png", "start": 27496976, "end": 27504566}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5e.png", "start": 27504566, "end": 27513699}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5f.png", "start": 27513699, "end": 27522728}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5g.png", "start": 27522728, "end": 27531462}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5h.png", "start": 27531462, "end": 27540190}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6a.png", "start": 27540190, "end": 27547820}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6b.png", "start": 27547820, "end": 27556268}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6c.png", "start": 27556268, "end": 27563836}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6d.png", "start": 27563836, "end": 27572233}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6e.png", "start": 27572233, "end": 27581990}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6f.png", "start": 27581990, "end": 27591936}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6g.png", "start": 27591936, "end": 27601139}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6h.png", "start": 27601139, "end": 27610578}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_woodgen1.png", "start": 27610578, "end": 27615125}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_woodgen2.png", "start": 27615125, "end": 27621818}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry1a.png", "start": 27621818, "end": 27629605}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry1b.png", "start": 27629605, "end": 27636752}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry1c.png", "start": 27636752, "end": 27641022}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry1d.png", "start": 27641022, "end": 27645032}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry2a.png", "start": 27645032, "end": 27652171}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry2b.png", "start": 27652171, "end": 27658730}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry2c.png", "start": 27658730, "end": 27662551}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry2d.png", "start": 27662551, "end": 27666189}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry3a.png", "start": 27666189, "end": 27673318}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry3b.png", "start": 27673318, "end": 27679904}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry3c.png", "start": 27679904, "end": 27683877}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry3d.png", "start": 27683877, "end": 27687649}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry4a.png", "start": 27687649, "end": 27698531}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry4b.png", "start": 27698531, "end": 27707369}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry4c.png", "start": 27707369, "end": 27712871}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry4d.png", "start": 27712871, "end": 27717581}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry5a.png", "start": 27717581, "end": 27728165}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry5b.png", "start": 27728165, "end": 27736791}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry5c.png", "start": 27736791, "end": 27742100}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry5d.png", "start": 27742100, "end": 27746650}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry6a.png", "start": 27746650, "end": 27755499}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry6b.png", "start": 27755499, "end": 27762777}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry6c.png", "start": 27762777, "end": 27767395}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry6d.png", "start": 27767395, "end": 27771407}, {"filename": "/GameData/Textures/brushes/foil.png", "start": 27771407, "end": 28028625}, {"filename": "/GameData/Textures/brushes/grass.png", "start": 28028625, "end": 28156750}, {"filename": "/GameData/Textures/brushes/hole_t.png", "start": 28156750, "end": 28159179}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_1.png", "start": 28159179, "end": 28172292}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_10.png", "start": 28172292, "end": 28186406}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_2.png", "start": 28186406, "end": 28200447}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_3.png", "start": 28200447, "end": 28214957}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_4.png", "start": 28214957, "end": 28229343}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_5.png", "start": 28229343, "end": 28243792}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_6.png", "start": 28243792, "end": 28258409}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_7.png", "start": 28258409, "end": 28273006}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_8.png", "start": 28273006, "end": 28287354}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_9.png", "start": 28287354, "end": 28301512}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_a1.png", "start": 28301512, "end": 28315960}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_1.png", "start": 28315960, "end": 28325388}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_10.png", "start": 28325388, "end": 28334837}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_2.png", "start": 28334837, "end": 28344231}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_3.png", "start": 28344231, "end": 28353876}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_4.png", "start": 28353876, "end": 28363383}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_5.png", "start": 28363383, "end": 28372888}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_6.png", "start": 28372888, "end": 28382401}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_7.png", "start": 28382401, "end": 28391946}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_8.png", "start": 28391946, "end": 28401356}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_9.png", "start": 28401356, "end": 28410893}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_a1.png", "start": 28410893, "end": 28419923}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_1.png", "start": 28419923, "end": 28427787}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_10.png", "start": 28427787, "end": 28436723}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_2.png", "start": 28436723, "end": 28445117}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_3.png", "start": 28445117, "end": 28455344}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_4.png", "start": 28455344, "end": 28464623}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_5.png", "start": 28464623, "end": 28474353}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_6.png", "start": 28474353, "end": 28483913}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_7.png", "start": 28483913, "end": 28492738}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_8.png", "start": 28492738, "end": 28501519}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_9.png", "start": 28501519, "end": 28510995}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_a1.png", "start": 28510995, "end": 28519402}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_1.png", "start": 28519402, "end": 28525561}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_10.png", "start": 28525561, "end": 28531921}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_2.png", "start": 28531921, "end": 28538186}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_3.png", "start": 28538186, "end": 28545112}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_4.png", "start": 28545112, "end": 28551602}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_5.png", "start": 28551602, "end": 28558195}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_6.png", "start": 28558195, "end": 28564719}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_7.png", "start": 28564719, "end": 28571127}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_8.png", "start": 28571127, "end": 28577410}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_9.png", "start": 28577410, "end": 28583993}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_a1.png", "start": 28583993, "end": 28590062}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc5_1.png", "start": 28590062, "end": 28598824}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc5_10.png", "start": 28598824, "end": 28608439}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc5_2.png", "start": 28608439, "end": 28617755}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc5_8.png", "start": 28617755, "end": 28626835}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc5_9.png", "start": 28626835, "end": 28636549}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc5_a1.png", "start": 28636549, "end": 28644924}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_1.png", "start": 28644924, "end": 28651337}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_10.png", "start": 28651337, "end": 28657746}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_2.png", "start": 28657746, "end": 28664074}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_3.png", "start": 28664074, "end": 28670935}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_4.png", "start": 28670935, "end": 28677543}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_5.png", "start": 28677543, "end": 28684219}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_6.png", "start": 28684219, "end": 28690993}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_7.png", "start": 28690993, "end": 28697696}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_8.png", "start": 28697696, "end": 28704192}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_9.png", "start": 28704192, "end": 28710814}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_a1.png", "start": 28710814, "end": 28717458}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_1.png", "start": 28717458, "end": 28725248}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_10.png", "start": 28725248, "end": 28732926}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_2.png", "start": 28732926, "end": 28740552}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_3.png", "start": 28740552, "end": 28748496}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_4.png", "start": 28748496, "end": 28756264}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_5.png", "start": 28756264, "end": 28764088}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_6.png", "start": 28764088, "end": 28772087}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_7.png", "start": 28772087, "end": 28780067}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_8.png", "start": 28780067, "end": 28787891}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_9.png", "start": 28787891, "end": 28795691}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_a1.png", "start": 28795691, "end": 28803424}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr1_1.png", "start": 28803424, "end": 28805756}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr1_2.png", "start": 28805756, "end": 28809020}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr1_3.png", "start": 28809020, "end": 28812560}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr1_4.png", "start": 28812560, "end": 28815337}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr1_5.png", "start": 28815337, "end": 28818933}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr1_6.png", "start": 28818933, "end": 28821989}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_1.png", "start": 28821989, "end": 28824099}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_2.png", "start": 28824099, "end": 28827312}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_3.png", "start": 28827312, "end": 28830932}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_4.png", "start": 28830932, "end": 28834330}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_5.png", "start": 28834330, "end": 28837690}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_6.png", "start": 28837690, "end": 28840889}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_7.png", "start": 28840889, "end": 28844024}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_8.png", "start": 28844024, "end": 28846965}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/clip.png", "start": 28846965, "end": 28847471}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_blue_a.png", "start": 28847471, "end": 28848027}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_blue_b.png", "start": 28848027, "end": 28848582}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_blue_c.png", "start": 28848582, "end": 28849137}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_brown_a.png", "start": 28849137, "end": 28849692}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_brown_b.png", "start": 28849692, "end": 28850247}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_brown_c.png", "start": 28850247, "end": 28850802}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_green_a.png", "start": 28850802, "end": 28851358}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_green_b.png", "start": 28851358, "end": 28851913}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_green_c.png", "start": 28851913, "end": 28852468}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_grey_a.png", "start": 28852468, "end": 28853021}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_grey_b.png", "start": 28853021, "end": 28853577}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_grey_c.png", "start": 28853577, "end": 28854132}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_olive_a.png", "start": 28854132, "end": 28854687}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_olive_b.png", "start": 28854687, "end": 28855241}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_olive_c.png", "start": 28855241, "end": 28855796}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_orange_a.png", "start": 28855796, "end": 28856352}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_orange_b.png", "start": 28856352, "end": 28856907}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_orange_c.png", "start": 28856907, "end": 28857462}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_pink_a.png", "start": 28857462, "end": 28858018}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_pink_b.png", "start": 28858018, "end": 28858574}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_pink_c.png", "start": 28858574, "end": 28859129}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_purple_a.png", "start": 28859129, "end": 28859685}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_purple_b.png", "start": 28859685, "end": 28860240}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_purple_c.png", "start": 28860240, "end": 28860795}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_red_a.png", "start": 28860795, "end": 28861346}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_red_b.png", "start": 28861346, "end": 28861897}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_red_c.png", "start": 28861897, "end": 28862448}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_tan_a.png", "start": 28862448, "end": 28863004}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_tan_b.png", "start": 28863004, "end": 28863560}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_tan_c.png", "start": 28863560, "end": 28864115}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_yellow_a.png", "start": 28864115, "end": 28864671}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_yellow_b.png", "start": 28864671, "end": 28865227}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_yellow_c.png", "start": 28865227, "end": 28865781}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_blue_a.png", "start": 28865781, "end": 28866598}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_blue_b.png", "start": 28866598, "end": 28867416}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_blue_c.png", "start": 28867416, "end": 28868234}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_brown_a.png", "start": 28868234, "end": 28869056}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_brown_b.png", "start": 28869056, "end": 28869879}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_brown_c.png", "start": 28869879, "end": 28870701}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_green_a.png", "start": 28870701, "end": 28871522}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_green_b.png", "start": 28871522, "end": 28872344}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_green_c.png", "start": 28872344, "end": 28873166}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_grey_a.png", "start": 28873166, "end": 28873976}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_grey_b.png", "start": 28873976, "end": 28874789}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_grey_c.png", "start": 28874789, "end": 28875603}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_olive_a.png", "start": 28875603, "end": 28876421}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_olive_b.png", "start": 28876421, "end": 28877236}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_olive_c.png", "start": 28877236, "end": 28878053}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_orange_a.png", "start": 28878053, "end": 28878876}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_orange_b.png", "start": 28878876, "end": 28879699}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_orange_c.png", "start": 28879699, "end": 28880519}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_pink_a.png", "start": 28880519, "end": 28881342}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_pink_b.png", "start": 28881342, "end": 28882164}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_pink_c.png", "start": 28882164, "end": 28882986}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_purple_a.png", "start": 28882986, "end": 28883808}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_purple_b.png", "start": 28883808, "end": 28884630}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_purple_c.png", "start": 28884630, "end": 28885451}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_red_a.png", "start": 28885451, "end": 28886261}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_red_b.png", "start": 28886261, "end": 28887071}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_red_c.png", "start": 28887071, "end": 28887882}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_tan_a.png", "start": 28887882, "end": 28888702}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_tan_b.png", "start": 28888702, "end": 28889526}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_tan_c.png", "start": 28889526, "end": 28890347}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_yellow_a.png", "start": 28890347, "end": 28891168}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_yellow_b.png", "start": 28891168, "end": 28891990}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_yellow_c.png", "start": 28891990, "end": 28892809}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/hint.png", "start": 28892809, "end": 28893675}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/hintskip.png", "start": 28893675, "end": 28894568}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/key_gold_1.png", "start": 28894568, "end": 28895323}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/key_silver_1.png", "start": 28895323, "end": 28896077}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/light_fbr.png", "start": 28896077, "end": 28896993}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/origin.png", "start": 28896993, "end": 28897476}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/plus_0_button_fbr.png", "start": 28897476, "end": 28897675}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/plus_0_shoot_fbr.png", "start": 28897675, "end": 28897954}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/plus_1_button_fbr.png", "start": 28897954, "end": 28898155}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/plus_1_shoot_fbr.png", "start": 28898155, "end": 28898436}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/plus_a_button_fbr.png", "start": 28898436, "end": 28898637}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/plus_a_shoot_fbr.png", "start": 28898637, "end": 28898924}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/skip.png", "start": 28898924, "end": 28899412}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/sky_dev.png", "start": 28899412, "end": 28901384}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/sky_dev.png.bak", "start": 28901384, "end": 28913738}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/sky_dev_day_fbr.png", "start": 28913738, "end": 28917564}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/sky_dev_void.png", "start": 28917564, "end": 28918475}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_blood1.png", "start": 28918475, "end": 28918837}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_lava1.png", "start": 28918837, "end": 28919208}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_lavaskip.png", "start": 28919208, "end": 28920321}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_slime1.png", "start": 28920321, "end": 28920691}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_slimeskip.png", "start": 28920691, "end": 28921764}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_smile.png", "start": 28921764, "end": 28922244}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_teleport.png", "start": 28922244, "end": 28922568}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_water1.png", "start": 28922568, "end": 28922940}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_water2.png", "start": 28922940, "end": 28923309}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_waterskip.png", "start": 28923309, "end": 28925049}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/trigger.png", "start": 28925049, "end": 28925546}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_blue_a.png", "start": 28925546, "end": 28926226}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_blue_b.png", "start": 28926226, "end": 28926906}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_blue_c.png", "start": 28926906, "end": 28927586}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_brown_a.png", "start": 28927586, "end": 28928268}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_brown_b.png", "start": 28928268, "end": 28928950}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_brown_c.png", "start": 28928950, "end": 28929632}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_green_a.png", "start": 28929632, "end": 28930314}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_green_b.png", "start": 28930314, "end": 28930996}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_green_c.png", "start": 28930996, "end": 28931678}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_grey_a.png", "start": 28931678, "end": 28932352}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_grey_b.png", "start": 28932352, "end": 28933028}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_grey_c.png", "start": 28933028, "end": 28933705}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_olive_a.png", "start": 28933705, "end": 28934385}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_olive_b.png", "start": 28934385, "end": 28935062}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_olive_c.png", "start": 28935062, "end": 28935741}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_orange_a.png", "start": 28935741, "end": 28936423}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_orange_b.png", "start": 28936423, "end": 28937105}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_orange_c.png", "start": 28937105, "end": 28937786}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_pink_a.png", "start": 28937786, "end": 28938468}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_pink_b.png", "start": 28938468, "end": 28939150}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_pink_c.png", "start": 28939150, "end": 28939832}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_purple_a.png", "start": 28939832, "end": 28940515}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_purple_b.png", "start": 28940515, "end": 28941197}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_purple_c.png", "start": 28941197, "end": 28941879}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_red_a.png", "start": 28941879, "end": 28942552}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_red_b.png", "start": 28942552, "end": 28943225}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_red_c.png", "start": 28943225, "end": 28943899}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_tan_a.png", "start": 28943899, "end": 28944581}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_tan_b.png", "start": 28944581, "end": 28945263}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_tan_c.png", "start": 28945263, "end": 28945945}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_yellow_a.png", "start": 28945945, "end": 28946627}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_yellow_b.png", "start": 28946627, "end": 28947309}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_yellow_c.png", "start": 28947309, "end": 28947990}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_0_fbr.png", "start": 28947990, "end": 28948167}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_1_fbr.png", "start": 28948167, "end": 28948327}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_2_fbr.png", "start": 28948327, "end": 28948499}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_3_fbr.png", "start": 28948499, "end": 28948678}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_4_fbr.png", "start": 28948678, "end": 28948841}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_5_fbr.png", "start": 28948841, "end": 28949014}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_6_fbr.png", "start": 28949014, "end": 28949186}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_7_fbr.png", "start": 28949186, "end": 28949340}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_8_fbr.png", "start": 28949340, "end": 28949508}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_9_fbr.png", "start": 28949508, "end": 28949675}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_a_fbr.png", "start": 28949675, "end": 28949839}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_b_fbr.png", "start": 28949839, "end": 28950009}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_c_fbr.png", "start": 28950009, "end": 28950159}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_d_fbr.png", "start": 28950159, "end": 28950325}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_e_fbr.png", "start": 28950325, "end": 28950493}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_f_fbr.png", "start": 28950493, "end": 28950653}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_g_fbr.png", "start": 28950653, "end": 28950825}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_h_fbr.png", "start": 28950825, "end": 28950984}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_i_fbr.png", "start": 28950984, "end": 28951143}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_j_fbr.png", "start": 28951143, "end": 28951309}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_k_fbr.png", "start": 28951309, "end": 28951503}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_l_fbr.png", "start": 28951503, "end": 28951644}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_m_fbr.png", "start": 28951644, "end": 28951793}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_n_fbr.png", "start": 28951793, "end": 28951976}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_o_fbr.png", "start": 28951976, "end": 28952126}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_p_fbr.png", "start": 28952126, "end": 28952284}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_q_fbr.png", "start": 28952284, "end": 28952450}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_r_fbr.png", "start": 28952450, "end": 28952622}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_s_fbr.png", "start": 28952622, "end": 28952798}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_t_fbr.png", "start": 28952798, "end": 28952946}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_trans_fbr.png", "start": 28952946, "end": 28953069}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_u_fbr.png", "start": 28953069, "end": 28953215}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_v_fbr.png", "start": 28953215, "end": 28953383}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_w_fbr.png", "start": 28953383, "end": 28953532}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_x_fbr.png", "start": 28953532, "end": 28953714}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_y_fbr.png", "start": 28953714, "end": 28953883}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_z_fbr.png", "start": 28953883, "end": 28954058}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_a_fbr.png", "start": 28954058, "end": 28954213}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_b_fbr.png", "start": 28954213, "end": 28954372}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_c_fbr.png", "start": 28954372, "end": 28954526}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_d_fbr.png", "start": 28954526, "end": 28954687}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_e_fbr.png", "start": 28954687, "end": 28954841}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_f_fbr.png", "start": 28954841, "end": 28955001}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_g_fbr.png", "start": 28955001, "end": 28955159}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_h_fbr.png", "start": 28955159, "end": 28955313}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_i_fbr.png", "start": 28955313, "end": 28955459}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_j_fbr.png", "start": 28955459, "end": 28955619}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_k_fbr.png", "start": 28955619, "end": 28955798}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_l_fbr.png", "start": 28955798, "end": 28955936}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_m_fbr.png", "start": 28955936, "end": 28956089}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_n_fbr.png", "start": 28956089, "end": 28956239}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_o_fbr.png", "start": 28956239, "end": 28956393}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_p_fbr.png", "start": 28956393, "end": 28956548}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_q_fbr.png", "start": 28956548, "end": 28956698}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_r_fbr.png", "start": 28956698, "end": 28956851}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_s_fbr.png", "start": 28956851, "end": 28957003}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_t_fbr.png", "start": 28957003, "end": 28957169}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_u_fbr.png", "start": 28957169, "end": 28957320}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_v_fbr.png", "start": 28957320, "end": 28957494}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_w_fbr.png", "start": 28957494, "end": 28957648}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_x_fbr.png", "start": 28957648, "end": 28957835}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_y_fbr.png", "start": 28957835, "end": 28958009}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_z_fbr.png", "start": 28958009, "end": 28958180}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_add_fbr.png", "start": 28958180, "end": 28958343}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_and_fbr.png", "start": 28958343, "end": 28958535}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_ardown_fbr.png", "start": 28958535, "end": 28958720}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_arleft_fbr.png", "start": 28958720, "end": 28958904}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_arright_fbr.png", "start": 28958904, "end": 28959089}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_arup_fbr.png", "start": 28959089, "end": 28959267}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_at_fbr.png", "start": 28959267, "end": 28959424}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_brackc1_fbr.png", "start": 28959424, "end": 28959603}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_brackc2_fbr.png", "start": 28959603, "end": 28959787}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_brackr1_fbr.png", "start": 28959787, "end": 28959954}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_brackr2_fbr.png", "start": 28959954, "end": 28960124}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_bracks1_fbr.png", "start": 28960124, "end": 28960278}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_bracks2_fbr.png", "start": 28960278, "end": 28960431}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_caret_fbr.png", "start": 28960431, "end": 28960611}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_colon_fbr.png", "start": 28960611, "end": 28960763}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_colonsemi_fbr.png", "start": 28960763, "end": 28960930}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_comma_fbr.png", "start": 28960930, "end": 28961086}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_div_fbr.png", "start": 28961086, "end": 28961255}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_dollar_fbr.png", "start": 28961255, "end": 28961431}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_equ_fbr.png", "start": 28961431, "end": 28961590}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_exclam_fbr.png", "start": 28961590, "end": 28961734}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_grave_fbr.png", "start": 28961734, "end": 28961884}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_hash_fbr.png", "start": 28961884, "end": 28962069}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_heart_fbr.png", "start": 28962069, "end": 28962254}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_multi_fbr.png", "start": 28962254, "end": 28962423}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_percent_fbr.png", "start": 28962423, "end": 28962629}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_perio_fbr.png", "start": 28962629, "end": 28962766}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_pipe_fbr.png", "start": 28962766, "end": 28962913}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_quest_fbr.png", "start": 28962913, "end": 28963086}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_slaback_fbr.png", "start": 28963086, "end": 28963273}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_slafoward_fbr.png", "start": 28963273, "end": 28963452}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_smile_fbr.png", "start": 28963452, "end": 28963612}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_sub_fbr.png", "start": 28963612, "end": 28963759}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_sun_fbr.png", "start": 28963759, "end": 28963958}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_thngreater_fbr.png", "start": 28963958, "end": 28964151}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_thnless_fbr.png", "start": 28964151, "end": 28964338}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_tilde_fbr.png", "start": 28964338, "end": 28964507}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_unders_fbr.png", "start": 28964507, "end": 28964644}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/bone1_1.png", "start": 28964644, "end": 28972051}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/bone1_2.png", "start": 28972051, "end": 28981243}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/bone2_1.png", "start": 28981243, "end": 28991675}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/dopefish_fbr.png", "start": 28991675, "end": 28999905}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_gut1.png", "start": 28999905, "end": 29010220}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_gut2.png", "start": 29010220, "end": 29022020}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_1.png", "start": 29022020, "end": 29035770}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_2.png", "start": 29035770, "end": 29048750}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_3.png", "start": 29048750, "end": 29060783}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_4a.png", "start": 29060783, "end": 29073996}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_4b.png", "start": 29073996, "end": 29087136}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_5a.png", "start": 29087136, "end": 29100780}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_5b.png", "start": 29100780, "end": 29114353}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot2_1.png", "start": 29114353, "end": 29127579}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot3_1.png", "start": 29127579, "end": 29142525}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot3_2.png", "start": 29142525, "end": 29157564}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot4_1.png", "start": 29157564, "end": 29172201}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot5_1.png", "start": 29172201, "end": 29184469}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot6_1.png", "start": 29184469, "end": 29200876}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod1_1.png", "start": 29200876, "end": 29212394}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod1_2.png", "start": 29212394, "end": 29225456}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod1_3.png", "start": 29225456, "end": 29236459}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod1_4.png", "start": 29236459, "end": 29247871}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod1_5.png", "start": 29247871, "end": 29258470}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod2_1.png", "start": 29258470, "end": 29262126}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod2_2.png", "start": 29262126, "end": 29266223}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod2_3.png", "start": 29266223, "end": 29269599}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod2_4.png", "start": 29269599, "end": 29273197}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod2_5.png", "start": 29273197, "end": 29276587}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/fleshtile.png", "start": 29276587, "end": 29287556}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/marbred128.png", "start": 29287556, "end": 29299785}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_eat_eye2_fbr.png", "start": 29299785, "end": 29303658}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_eat_eye3_fbr.png", "start": 29303658, "end": 29307535}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_eat_eye_fbr.png", "start": 29307535, "end": 29311410}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_b.png", "start": 29311410, "end": 29324073}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_c.png", "start": 29324073, "end": 29336797}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_hol1.png", "start": 29336797, "end": 29341469}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_lit1_fbr.png", "start": 29341469, "end": 29345539}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_shut1.png", "start": 29345539, "end": 29360505}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_sp.png", "start": 29360505, "end": 29380647}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_sp2.png", "start": 29380647, "end": 29406270}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_te.png", "start": 29406270, "end": 29421376}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_tet.png", "start": 29421376, "end": 29434596}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh2_b.png", "start": 29434596, "end": 29448119}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh3_1a.png", "start": 29448119, "end": 29461845}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh3_1b.png", "start": 29461845, "end": 29498407}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh3_b.png", "start": 29498407, "end": 29512293}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh4_det.png", "start": 29512293, "end": 29522604}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh4a_det.png", "start": 29522604, "end": 29537465}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh5.png", "start": 29537465, "end": 29547801}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh5_1a.png", "start": 29547801, "end": 29602536}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh5_1b.png", "start": 29602536, "end": 29655254}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh5_1c.png", "start": 29655254, "end": 29698896}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh5_1lit_fbr.png", "start": 29698896, "end": 29715830}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh_dr1a.png", "start": 29715830, "end": 29772723}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_skin_eye.png", "start": 29772723, "end": 29783940}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/meat-teeth0.png", "start": 29783940, "end": 29827311}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/meat-teeth1.png", "start": 29827311, "end": 29872503}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/meat_det1.png", "start": 29872503, "end": 29891234}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/meat_det2.png", "start": 29891234, "end": 29913805}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/meat_pipe1.png", "start": 29913805, "end": 29938229}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_0eye.png", "start": 29938229, "end": 29942374}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_0flesh2_gl.png", "start": 29942374, "end": 29957554}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_0flesh_but1_fbr.png", "start": 29957554, "end": 29961849}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_0flesh_but2_fbr.png", "start": 29961849, "end": 29966038}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_0flsh_vent.png", "start": 29966038, "end": 29980289}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_1eye.png", "start": 29980289, "end": 29984433}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_1flesh2_gl.png", "start": 29984433, "end": 29999592}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_1flesh_but2_fbr.png", "start": 29999592, "end": 30003787}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_2eye.png", "start": 30003787, "end": 30007827}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_2flesh2_gl.png", "start": 30007827, "end": 30023003}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_2flesh_but2_fbr.png", "start": 30023003, "end": 30027186}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_3eye.png", "start": 30027186, "end": 30031234}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_3flesh2_gl.png", "start": 30031234, "end": 30046408}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_3flesh_but2_fbr.png", "start": 30046408, "end": 30050579}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_4eye.png", "start": 30050579, "end": 30054698}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_4flesh2_gl.png", "start": 30054698, "end": 30069878}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_5eye.png", "start": 30069878, "end": 30073931}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_6eye.png", "start": 30073931, "end": 30077979}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_7eye.png", "start": 30077979, "end": 30082050}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_8eye.png", "start": 30082050, "end": 30086158}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_9eye.png", "start": 30086158, "end": 30090306}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_aeye.png", "start": 30090306, "end": 30094298}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_aflesh_but1.png", "start": 30094298, "end": 30098864}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_aflesh_but2.png", "start": 30098864, "end": 30103393}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_aflsh_vent.png", "start": 30103393, "end": 30117793}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/black.png", "start": 30117793, "end": 30117938}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch1.png", "start": 30117938, "end": 30122955}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch1_2.png", "start": 30122955, "end": 30128581}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch1_a.png", "start": 30128581, "end": 30141356}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch1_b.png", "start": 30141356, "end": 30154816}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch1_c.png", "start": 30154816, "end": 30167989}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch1_d.png", "start": 30167989, "end": 30181506}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch2.png", "start": 30181506, "end": 30184788}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch2_2.png", "start": 30184788, "end": 30188110}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch2_a.png", "start": 30188110, "end": 30198513}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch2_b.png", "start": 30198513, "end": 30209030}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch2_c.png", "start": 30209030, "end": 30219159}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch_end.png", "start": 30219159, "end": 30219915}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch_tcap.png", "start": 30219915, "end": 30220629}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch_trim.png", "start": 30220629, "end": 30222101}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_arch1a.png", "start": 30222101, "end": 30229736}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_arch1b.png", "start": 30229736, "end": 30238271}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_brk1a.png", "start": 30238271, "end": 30240743}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_brk1b.png", "start": 30240743, "end": 30242881}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_brk2a.png", "start": 30242881, "end": 30245302}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_brk2b.png", "start": 30245302, "end": 30247762}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_flt1.png", "start": 30247762, "end": 30249986}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_fsh1.png", "start": 30249986, "end": 30252912}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_fsh2.png", "start": 30252912, "end": 30261004}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_pil1.png", "start": 30261004, "end": 30263791}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_trim1.png", "start": 30263791, "end": 30266203}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll1.png", "start": 30266203, "end": 30268923}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll2.png", "start": 30268923, "end": 30271649}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll3a.png", "start": 30271649, "end": 30274007}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll3b.png", "start": 30274007, "end": 30276475}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll4b.png", "start": 30276475, "end": 30279041}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll5a.png", "start": 30279041, "end": 30281258}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll5b.png", "start": 30281258, "end": 30283557}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll5c.png", "start": 30283557, "end": 30285880}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll5d.png", "start": 30285880, "end": 30288287}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk15.png", "start": 30288287, "end": 30297793}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk15_b.png", "start": 30297793, "end": 30307041}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk15_c.png", "start": 30307041, "end": 30323909}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk15_f.png", "start": 30323909, "end": 30332202}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk15_g.png", "start": 30332202, "end": 30339851}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk16.png", "start": 30339851, "end": 30349127}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk16_a.png", "start": 30349127, "end": 30358406}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk16_f.png", "start": 30358406, "end": 30368111}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk17.png", "start": 30368111, "end": 30403255}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk17_f.png", "start": 30403255, "end": 30415857}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_but1.png", "start": 30415857, "end": 30416806}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_det1.png", "start": 30416806, "end": 30419879}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_door1.png", "start": 30419879, "end": 30434986}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_door1_f.png", "start": 30434986, "end": 30445021}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_door2.png", "start": 30445021, "end": 30455881}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_door3.png", "start": 30455881, "end": 30468538}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_ebrick10.png", "start": 30468538, "end": 30479165}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_ebrick10_bl.png", "start": 30479165, "end": 30493699}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_ebrick22.png", "start": 30493699, "end": 30507565}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_ebrick23.png", "start": 30507565, "end": 30516931}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_ebrick24.png", "start": 30516931, "end": 30525124}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr1.png", "start": 30525124, "end": 30525455}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr2.png", "start": 30525455, "end": 30526309}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr3.png", "start": 30526309, "end": 30528905}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_1.png", "start": 30528905, "end": 30531035}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_2.png", "start": 30531035, "end": 30533297}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_3.png", "start": 30533297, "end": 30535660}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_4.png", "start": 30535660, "end": 30537955}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_5.png", "start": 30537955, "end": 30541587}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_6.png", "start": 30541587, "end": 30544593}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_8.png", "start": 30544593, "end": 30547928}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr5_1.png", "start": 30547928, "end": 30550515}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr5_2.png", "start": 30550515, "end": 30553009}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr5_3.png", "start": 30553009, "end": 30555751}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr5_4.png", "start": 30555751, "end": 30558054}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_key_gl2.png", "start": 30558054, "end": 30559078}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_key_sl2.png", "start": 30559078, "end": 30560079}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_lion1.png", "start": 30560079, "end": 30570553}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_lion2.png", "start": 30570553, "end": 30580220}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_lion3.png", "start": 30580220, "end": 30590763}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_lion4.png", "start": 30590763, "end": 30601915}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_met1.png", "start": 30601915, "end": 30612296}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_met1_trim.png", "start": 30612296, "end": 30623571}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_met1a_trim.png", "start": 30623571, "end": 30634539}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_met1b_trim.png", "start": 30634539, "end": 30648652}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_met2_trim.png", "start": 30648652, "end": 30661921}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_met_plt.png", "start": 30661921, "end": 30677312}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_mural1.png", "start": 30677312, "end": 30697170}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_mural2.png", "start": 30697170, "end": 30719887}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_mural3.png", "start": 30719887, "end": 30809167}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_pl1_a.png", "start": 30809167, "end": 30810983}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_pl1_b.png", "start": 30810983, "end": 30813066}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_pl2_a.png", "start": 30813066, "end": 30815298}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_pl2_b.png", "start": 30815298, "end": 30817510}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_plat1_side.png", "start": 30817510, "end": 30818727}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_plat1_top.png", "start": 30818727, "end": 30822624}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_tile2_1.png", "start": 30822624, "end": 30825058}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_tile2_2.png", "start": 30825058, "end": 30827411}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1.png", "start": 30827411, "end": 30834168}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_3.png", "start": 30834168, "end": 30840782}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_3_s.png", "start": 30840782, "end": 30842822}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_4_s.png", "start": 30842822, "end": 30844898}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_5.png", "start": 30844898, "end": 30851834}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_5_s.png", "start": 30851834, "end": 30854054}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_6_s.png", "start": 30854054, "end": 30856331}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_7_s.png", "start": 30856331, "end": 30859673}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim2.png", "start": 30859673, "end": 30862034}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_wall1.png", "start": 30862034, "end": 30865066}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_wall2.png", "start": 30865066, "end": 30867737}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_wall3.png", "start": 30867737, "end": 30873523}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_wall3b.png", "start": 30873523, "end": 30900557}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_win1_a.png", "start": 30900557, "end": 30909696}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_win1_b.png", "start": 30909696, "end": 30919039}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_0grk_but1_fbr.png", "start": 30919039, "end": 30920230}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_0grk_hbut_fbr.png", "start": 30920230, "end": 30921253}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_1grk_but1_fbr.png", "start": 30921253, "end": 30922428}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_1grk_hbut_fbr.png", "start": 30922428, "end": 30923509}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_2grk_but1_fbr.png", "start": 30923509, "end": 30924660}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_2grk_hbut_fbr.png", "start": 30924660, "end": 30925700}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_3grk_but1_fbr.png", "start": 30925700, "end": 30926853}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_3grk_hbut_fbr.png", "start": 30926853, "end": 30927895}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_agrk_but1.png", "start": 30927895, "end": 30928886}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_agrk_hbut.png", "start": 30928886, "end": 30929864}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_bottom.png", "start": 30929864, "end": 30930794}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_c_b1_fbr.png", "start": 30930794, "end": 30931402}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_c_b2_fbr.png", "start": 30931402, "end": 30932014}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_c_b3_fbr.png", "start": 30932014, "end": 30932821}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_c_s1_fbr.png", "start": 30932821, "end": 30933414}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_c_s2_fbr.png", "start": 30933414, "end": 30934404}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_c_s3_fbr.png", "start": 30934404, "end": 30935037}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_fl.png", "start": 30935037, "end": 30935596}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_fl2.png", "start": 30935596, "end": 30936151}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_n_b1_fbr.png", "start": 30936151, "end": 30936905}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_n_b2_fbr.png", "start": 30936905, "end": 30937590}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_n_b3.png", "start": 30937590, "end": 30938161}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_n_s1.png", "start": 30938161, "end": 30938732}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_n_s2_fbr.png", "start": 30938732, "end": 30939387}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_n_s3_fbr.png", "start": 30939387, "end": 30939924}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_r_b1_fbr.png", "start": 30939924, "end": 30940639}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_r_b2_fbr.png", "start": 30940639, "end": 30941265}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_r_b3_fbr.png", "start": 30941265, "end": 30941723}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_r_s1_fbr.png", "start": 30941723, "end": 30942337}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_r_s2_fbr.png", "start": 30942337, "end": 30942686}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_s_b1_fbr.png", "start": 30942686, "end": 30943345}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_s_b2_fbr.png", "start": 30943345, "end": 30943973}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_s_b3.png", "start": 30943973, "end": 30944750}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_s_s1_fbr.png", "start": 30944750, "end": 30945256}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_s_s2_fbr.png", "start": 30945256, "end": 30945834}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_s_s3_fbr.png", "start": 30945834, "end": 30946385}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammobotsmall.png", "start": 30946385, "end": 30947053}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammotop.png", "start": 30947053, "end": 30947798}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammotopsmall.png", "start": 30947798, "end": 30948349}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/boom.png", "start": 30948349, "end": 30949367}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/boomammo_bottom.png", "start": 30949367, "end": 30949733}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/boomammotop.png", "start": 30949733, "end": 30950075}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/boomsmall.png", "start": 30950075, "end": 30950750}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/epboxlarge_fbr.png", "start": 30950750, "end": 30951934}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/epboxsmall_fbr.png", "start": 30951934, "end": 30952831}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/explob_s2.png", "start": 30952831, "end": 30953327}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/hp15_side.png", "start": 30953327, "end": 30953912}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/hp25_top2.png", "start": 30953912, "end": 30954341}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/hp_bottom.png", "start": 30954341, "end": 30955248}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/hp_details.png", "start": 30955248, "end": 30955752}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/nails.png", "start": 30955752, "end": 30956799}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/nailssmall.png", "start": 30956799, "end": 30957540}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp100-winq_fbr.png", "start": 30957540, "end": 30960140}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp100_side_fbr.png", "start": 30960140, "end": 30961032}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp100_top_fbr.png", "start": 30961032, "end": 30961763}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp15_top_fbr.png", "start": 30961763, "end": 30962493}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp15winq_fbr.png", "start": 30962493, "end": 30965280}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp25-winq_fbr.png", "start": 30965280, "end": 30967834}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp25_side_fbr.png", "start": 30967834, "end": 30968656}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp25_top_fbr.png", "start": 30968656, "end": 30969390}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0explob2_s1_fbr.png", "start": 30969390, "end": 30970493}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0explob_s1_fbr.png", "start": 30970493, "end": 30971126}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp100-winq_fbr.png", "start": 30971126, "end": 30973731}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp100_side_fbr.png", "start": 30973731, "end": 30974625}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp100_top_fbr.png", "start": 30974625, "end": 30975359}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp15_top_fbr.png", "start": 30975359, "end": 30976093}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp15winq_fbr.png", "start": 30976093, "end": 30978879}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp25-winq_fbr.png", "start": 30978879, "end": 30981443}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp25_side_fbr.png", "start": 30981443, "end": 30982269}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp25_top_fbr.png", "start": 30982269, "end": 30983008}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1explob2_s1_fbr.png", "start": 30983008, "end": 30984115}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1explob_s1_fbr.png", "start": 30984115, "end": 30984748}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2_hp100-winq_fbr.png", "start": 30984748, "end": 30987352}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2_hp100_side_fbr.png", "start": 30987352, "end": 30988246}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2_hp25-winq_fbr.png", "start": 30988246, "end": 30990804}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2_hp25_side_fbr.png", "start": 30990804, "end": 30991629}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2_hp25_top_fbr.png", "start": 30991629, "end": 30992367}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2explob2_s1_fbr.png", "start": 30992367, "end": 30993463}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2explob_s1_fbr.png", "start": 30993463, "end": 30994105}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3_hp100-winq_fbr.png", "start": 30994105, "end": 30996715}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3_hp100_side_fbr.png", "start": 30996715, "end": 30997612}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3_hp25-winq_fbr.png", "start": 30997612, "end": 31000181}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3_hp25_side_fbr.png", "start": 31000181, "end": 31001012}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3_hp25_top_fbr.png", "start": 31001012, "end": 31001754}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3explob2_s1_fbr.png", "start": 31001754, "end": 31002850}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3explob_s1_fbr.png", "start": 31002850, "end": 31003492}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/shells.png", "start": 31003492, "end": 31004438}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/shellssmall.png", "start": 31004438, "end": 31005164}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/zap.png", "start": 31005164, "end": 31006157}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/zapsmall.png", "start": 31006157, "end": 31006921}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/brick7.png", "start": 31006921, "end": 31009854}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/brick8.png", "start": 31009854, "end": 31012941}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/button_0.png", "start": 31012941, "end": 31016413}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/button_0_grey.png", "start": 31016413, "end": 31019623}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/button_0_grn.png", "start": 31019623, "end": 31022958}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/button_1.png", "start": 31022958, "end": 31026413}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/button_1_grey.png", "start": 31026413, "end": 31029608}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/button_1_grn.png", "start": 31029608, "end": 31032909}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/floor_temp.png", "start": 31032909, "end": 31034967}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/flr.png", "start": 31034967, "end": 31037724}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/gardgrass_1.png", "start": 31037724, "end": 31057894}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/go-savgx.png", "start": 31057894, "end": 31059643}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grass.png", "start": 31059643, "end": 31063044}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_brk15_c_old.png", "start": 31063044, "end": 31073571}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_brk17_f_old.png", "start": 31073571, "end": 31082280}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_door1_old.png", "start": 31082280, "end": 31093297}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_door2_old.png", "start": 31093297, "end": 31102470}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_door3_old.png", "start": 31102470, "end": 31111276}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_ebrick22_old.png", "start": 31111276, "end": 31120593}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_trim1_7_s_old.png", "start": 31120593, "end": 31122798}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/ground_1.png", "start": 31122798, "end": 31124979}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/leaves.png", "start": 31124979, "end": 31128041}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/marble1_4.png", "start": 31128041, "end": 31130944}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/marble1_5.png", "start": 31130944, "end": 31134034}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_cflat1_3.png", "start": 31134034, "end": 31136515}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_csl_trm3.png", "start": 31136515, "end": 31137840}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_csl_trm3b.png", "start": 31137840, "end": 31138830}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_csl_trm3c.png", "start": 31138830, "end": 31140358}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_flat10.png", "start": 31140358, "end": 31144251}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_flat11.png", "start": 31144251, "end": 31147456}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_flat13.png", "start": 31147456, "end": 31158233}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_flat14.png", "start": 31158233, "end": 31168634}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_flat9a.png", "start": 31168634, "end": 31212214}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_flat9b.png", "start": 31212214, "end": 31254113}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_plaster1.png", "start": 31254113, "end": 31262155}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_ret_rock1.png", "start": 31262155, "end": 31265974}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_ret_wood1_old.png", "start": 31265974, "end": 31273055}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_rmet_key.png", "start": 31273055, "end": 31277016}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_rock6.png", "start": 31277016, "end": 31322619}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_rock7.png", "start": 31322619, "end": 31325503}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_rock8.png", "start": 31325503, "end": 31327388}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_rough_block.png", "start": 31327388, "end": 31376231}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_rough_block_f.png", "start": 31376231, "end": 31430776}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_tile.png", "start": 31430776, "end": 31432912}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_wall1.png", "start": 31432912, "end": 31442777}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/metground_1.png", "start": 31442777, "end": 31445708}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/note-savgx.png", "start": 31445708, "end": 31464728}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/plus_0button1.png", "start": 31464728, "end": 31468173}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/plus_0button2_fbr.png", "start": 31468173, "end": 31469660}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/plus_1button2_fbr.png", "start": 31469660, "end": 31471148}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/plus_1button3.png", "start": 31471148, "end": 31473028}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/plus_abutton1_fbr.png", "start": 31473028, "end": 31476630}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/plus_abutton2_fbr.png", "start": 31476630, "end": 31478128}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/readme.txt", "start": 31478128, "end": 31478204}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiflat.png", "start": 31478204, "end": 31492323}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiflat_blu.png", "start": 31492323, "end": 31505258}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiflat_grn.png", "start": 31505258, "end": 31516912}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoilava.png", "start": 31516912, "end": 31521416}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoislime.png", "start": 31521416, "end": 31525121}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoitrim.png", "start": 31525121, "end": 31526608}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoitrim__purp.png", "start": 31526608, "end": 31528324}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoitrim_blu.png", "start": 31528324, "end": 31530054}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiwall.png", "start": 31530054, "end": 31569851}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiwall__purp.png", "start": 31569851, "end": 31605924}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiwall_blu.png", "start": 31605924, "end": 31641985}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiwater.png", "start": 31641985, "end": 31644876}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/rune1_fbr.png", "start": 31644876, "end": 31647889}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/rune2_fbr.png", "start": 31647889, "end": 31650955}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/rune3_fbr.png", "start": 31650955, "end": 31654351}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/rune4_fbr.png", "start": 31654351, "end": 31656903}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_easy.png", "start": 31656903, "end": 31661855}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_empty.png", "start": 31661855, "end": 31666612}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_hard.png", "start": 31666612, "end": 31671485}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_medium.png", "start": 31671485, "end": 31676495}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_metal_1.png", "start": 31676495, "end": 31677707}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_metal_2.png", "start": 31677707, "end": 31679302}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_nmare.png", "start": 31679302, "end": 31684323}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky2.png", "start": 31684323, "end": 31699813}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky4.png", "start": 31699813, "end": 31709325}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky5_fbr.png", "start": 31709325, "end": 31726478}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky5a.png", "start": 31726478, "end": 31735489}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky7.png", "start": 31735489, "end": 31749404}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky8.png", "start": 31749404, "end": 31766077}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky8a_fbr.png", "start": 31766077, "end": 31774873}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/tile.png", "start": 31774873, "end": 31783689}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/tile_blu.png", "start": 31783689, "end": 31792695}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/tile_grn.png", "start": 31792695, "end": 31800958}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/trim2_blu.png", "start": 31800958, "end": 31802071}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/trim2_grn.png", "start": 31802071, "end": 31803135}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/ultrasteel1.png", "start": 31803135, "end": 31816025}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/ultrasteel2.png", "start": 31816025, "end": 31827819}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/vines1_old.png", "start": 31827819, "end": 31832673}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/wiz1_4.png", "start": 31832673, "end": 31836494}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/+0water_f3.png", "start": 31836494, "end": 31839435}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/+1water_f3.png", "start": 31839435, "end": 31842359}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/+2water_f3.png", "start": 31842359, "end": 31845277}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/+3water_f3.png", "start": 31845277, "end": 31848129}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_0blood_f1.png", "start": 31848129, "end": 31849918}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_0fslime.png", "start": 31849918, "end": 31863874}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_0lava_fall3_fbr.png", "start": 31863874, "end": 31872867}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_0water_f1.png", "start": 31872867, "end": 31874716}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_0water_f2.png", "start": 31874716, "end": 31876534}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_0wfall0.png", "start": 31876534, "end": 31886143}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_1blood_f1.png", "start": 31886143, "end": 31887948}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_1fslime.png", "start": 31887948, "end": 31901691}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_1lava_fall3_fbr.png", "start": 31901691, "end": 31911258}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_1water_f1.png", "start": 31911258, "end": 31912906}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_1water_f2.png", "start": 31912906, "end": 31914735}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_1wfall0.png", "start": 31914735, "end": 31924386}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_2blood_f1.png", "start": 31924386, "end": 31926244}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_2fslime.png", "start": 31926244, "end": 31940138}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_2lava_fall3_fbr.png", "start": 31940138, "end": 31949687}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_2water_f1.png", "start": 31949687, "end": 31951458}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_2water_f2.png", "start": 31951458, "end": 31953344}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_2wfall0.png", "start": 31953344, "end": 31963006}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_3blood_f1.png", "start": 31963006, "end": 31964806}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_3fslime.png", "start": 31964806, "end": 31978489}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_3lava_fall3_fbr.png", "start": 31978489, "end": 31987826}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_3water_f1.png", "start": 31987826, "end": 31989477}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_3water_f2.png", "start": 31989477, "end": 31991304}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_3wfall0.png", "start": 31991304, "end": 32001007}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_4fslime.png", "start": 32001007, "end": 32014671}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_4lava_fall3_fbr.png", "start": 32014671, "end": 32024044}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_4wfall0.png", "start": 32024044, "end": 32033623}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_5fslime.png", "start": 32033623, "end": 32047391}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_5lava_fall3_fbr.png", "start": 32047391, "end": 32056754}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_5wfall0.png", "start": 32056754, "end": 32066353}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_6fslime.png", "start": 32066353, "end": 32080205}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_6lava_fall3_fbr.png", "start": 32080205, "end": 32089590}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_6wfall0.png", "start": 32089590, "end": 32099175}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_7fslime.png", "start": 32099175, "end": 32113026}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_7lava_fall3_fbr.png", "start": 32113026, "end": 32122541}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_7wfall0.png", "start": 32122541, "end": 32132210}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_8wfall0.png", "start": 32132210, "end": 32141872}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_9wfall0.png", "start": 32141872, "end": 32151494}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky-test.png", "start": 32151494, "end": 32169090}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky-test.xcf", "start": 32169090, "end": 32302355}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky5_blu.png", "start": 32302355, "end": 32315668}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky5_dismal.png", "start": 32315668, "end": 32328851}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_galx_fbr.png", "start": 32328851, "end": 32351606}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_galx_spark_fbr.png", "start": 32351606, "end": 32371164}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_orng.png", "start": 32371164, "end": 32385803}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_pando.png", "start": 32385803, "end": 32401911}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_pando2.png", "start": 32401911, "end": 32418446}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_star.png", "start": 32418446, "end": 32420106}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_void.png", "start": 32420106, "end": 32420801}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_wfog_fbr.png", "start": 32420801, "end": 32421707}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_acid.png", "start": 32421707, "end": 32423886}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_blood1.png", "start": 32423886, "end": 32425959}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_lava1_fbr.png", "start": 32425959, "end": 32429164}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_lava2_fbr.png", "start": 32429164, "end": 32433246}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_lava3_fbr.png", "start": 32433246, "end": 32437215}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_lava_void_fbr.png", "start": 32437215, "end": 32440905}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_lavaskip.png", "start": 32440905, "end": 32442018}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_meatgoo2_fbr.png", "start": 32442018, "end": 32445206}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_meatgoo_fbr.png", "start": 32445206, "end": 32448003}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_slime1.png", "start": 32448003, "end": 32454294}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_slime2.png", "start": 32454294, "end": 32457483}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_slime3.png", "start": 32457483, "end": 32459515}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_slime_soul.png", "start": 32459515, "end": 32462305}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_slimeskip.png", "start": 32462305, "end": 32463378}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_soul_drain.png", "start": 32463378, "end": 32466346}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_tele1_fbr.png", "start": 32466346, "end": 32468226}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_tele2_fbr.png", "start": 32468226, "end": 32471027}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_tele3_fbr.png", "start": 32471027, "end": 32474063}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_tele4_fbr.png", "start": 32474063, "end": 32476755}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_water0.png", "start": 32476755, "end": 32479603}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_water1.png", "start": 32479603, "end": 32482712}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_water2.png", "start": 32482712, "end": 32484972}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_water3.png", "start": 32484972, "end": 32487353}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_water4.png", "start": 32487353, "end": 32492140}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_waterskip.png", "start": 32492140, "end": 32493880}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_wstill0.png", "start": 32493880, "end": 32496719}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/btn1.png", "start": 32496719, "end": 32499637}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_arrow.png", "start": 32499637, "end": 32500518}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok1_1.png", "start": 32500518, "end": 32503039}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok1_2.png", "start": 32503039, "end": 32505230}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok1_2_m.png", "start": 32505230, "end": 32508326}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok1_m.png", "start": 32508326, "end": 32517600}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok2_1.png", "start": 32517600, "end": 32520164}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok2_1_m.png", "start": 32520164, "end": 32523385}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok2_2.png", "start": 32523385, "end": 32533648}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok2_2_m.png", "start": 32533648, "end": 32546076}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blud1_1.png", "start": 32546076, "end": 32548668}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blud1_1m.png", "start": 32548668, "end": 32551888}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_1.png", "start": 32551888, "end": 32554323}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_1m.png", "start": 32554323, "end": 32557393}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_2.png", "start": 32557393, "end": 32559820}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_2m.png", "start": 32559820, "end": 32562848}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_3.png", "start": 32562848, "end": 32565107}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_3m.png", "start": 32565107, "end": 32568516}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_5.png", "start": 32568516, "end": 32571369}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_5m.png", "start": 32571369, "end": 32575018}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd_skull.png", "start": 32575018, "end": 32577650}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_0.png", "start": 32577650, "end": 32580754}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_0m.png", "start": 32580754, "end": 32583811}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_1.png", "start": 32583811, "end": 32586493}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_1m.png", "start": 32586493, "end": 32589692}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_2.png", "start": 32589692, "end": 32592643}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_2m.png", "start": 32592643, "end": 32596048}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_3.png", "start": 32596048, "end": 32598985}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_3m.png", "start": 32598985, "end": 32602161}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk2_0.png", "start": 32602161, "end": 32612016}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk2_0_m.png", "start": 32612016, "end": 32624349}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk_old.png", "start": 32624349, "end": 32626948}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk_oldm.png", "start": 32626948, "end": 32630287}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_deco1_1.png", "start": 32630287, "end": 32641516}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_deco1_1m.png", "start": 32641516, "end": 32655551}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_deco1_2.png", "start": 32655551, "end": 32658071}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_deco1_2my.png", "start": 32658071, "end": 32661459}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_deco1_3.png", "start": 32661459, "end": 32680032}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_deco1_3m.png", "start": 32680032, "end": 32700383}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_door1_1.png", "start": 32700383, "end": 32709850}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_door1_1m.png", "start": 32709850, "end": 32720941}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_door2_1.png", "start": 32720941, "end": 32732909}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_door2_2.png", "start": 32732909, "end": 32744687}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_drt2_1.png", "start": 32744687, "end": 32747524}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_flr1_1.png", "start": 32747524, "end": 32750429}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_flr1_2.png", "start": 32750429, "end": 32753652}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_flt1_1.png", "start": 32753652, "end": 32755941}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_flt1_1m.png", "start": 32755941, "end": 32759389}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_key1_1.png", "start": 32759389, "end": 32760976}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_key1_2.png", "start": 32760976, "end": 32762973}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_lite1_1_fbr.png", "start": 32762973, "end": 32763903}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_lite1_2.png", "start": 32763903, "end": 32764266}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_lite2_1.png", "start": 32764266, "end": 32767203}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_lite2_2.png", "start": 32767203, "end": 32767689}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_lite3_1_fbr.png", "start": 32767689, "end": 32768646}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_lite_f1.png", "start": 32768646, "end": 32769132}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_oldmtomb1_1_fbr.png", "start": 32769132, "end": 32782984}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_oldtomb1_2_fbr.png", "start": 32782984, "end": 32796876}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_plat_stem_m.png", "start": 32796876, "end": 32797742}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_plats.png", "start": 32797742, "end": 32800404}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_platst.png", "start": 32800404, "end": 32802831}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_platt.png", "start": 32802831, "end": 32805454}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_plr1_1.png", "start": 32805454, "end": 32808648}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_tomb1_1_fbr.png", "start": 32808648, "end": 32822500}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_tomb1_2_fbr.png", "start": 32822500, "end": 32836392}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_trm1_1.png", "start": 32836392, "end": 32845320}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_trm1_2.png", "start": 32845320, "end": 32848025}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_trm1_a.png", "start": 32848025, "end": 32850367}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_tskull.png", "start": 32850367, "end": 32861373}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_wall1_1.png", "start": 32861373, "end": 32864355}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_wall1_2.png", "start": 32864355, "end": 32867321}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_wall1_3.png", "start": 32867321, "end": 32870515}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_wall1_3a.png", "start": 32870515, "end": 32873096}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_wall1_4.png", "start": 32873096, "end": 32876084}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_wall1_4a.png", "start": 32876084, "end": 32878483}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/maya_end_dr1.png", "start": 32878483, "end": 32882869}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/maya_end_dr2.png", "start": 32882869, "end": 32887254}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/maya_end_trim1.png", "start": 32887254, "end": 32891644}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_0_may_btn1.png", "start": 32891644, "end": 32894561}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_0_may_mpiloilon_fbr.png", "start": 32894561, "end": 32896119}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_0_may_mpilon_fbr.png", "start": 32896119, "end": 32897694}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_0_may_mshoohoot_fbr.png", "start": 32897694, "end": 32898639}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_0_may_mshoot_fbr.png", "start": 32898639, "end": 32899582}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_1_may_btn1.png", "start": 32899582, "end": 32902490}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_1_may_mpiloilon_fbr.png", "start": 32902490, "end": 32904063}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_1_may_mpilon_fbr.png", "start": 32904063, "end": 32905656}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_1_may_mshoohoot_fbr.png", "start": 32905656, "end": 32906641}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_1_may_mshoot_fbr.png", "start": 32906641, "end": 32907625}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_2_may_btn1.png", "start": 32907625, "end": 32910516}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_2_may_mpilon.png", "start": 32910516, "end": 32912093}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_2_may_mshoohoot_fbr.png", "start": 32912093, "end": 32913118}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_2_may_mshoot_fbr.png", "start": 32913118, "end": 32914145}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_3_may_btn1.png", "start": 32914145, "end": 32917207}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_3_may_mpiloilon_fbr.png", "start": 32917207, "end": 32918780}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_3_may_mpilon_fbr.png", "start": 32918780, "end": 32920373}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_3_may_mshoohoot_fbr.png", "start": 32920373, "end": 32921358}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_3_may_mshoot_fbr.png", "start": 32921358, "end": 32922342}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_4_may_btn1.png", "start": 32922342, "end": 32925405}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_5_may_btn1.png", "start": 32925405, "end": 32928146}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_a_may_btn1.png", "start": 32928146, "end": 32930869}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_a_may_mpiloilon_fbr.png", "start": 32930869, "end": 32932446}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_a_may_mpilon_fbr.png", "start": 32932446, "end": 32934030}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_a_may_mshoot.png", "start": 32934030, "end": 32934979}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+0med_but1.png", "start": 32934979, "end": 32939234}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+0med_but2.png", "start": 32939234, "end": 32940946}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+0med_but3.png", "start": 32940946, "end": 32942649}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+0med_but_s1.png", "start": 32942649, "end": 32946895}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+0med_sht_but1.png", "start": 32946895, "end": 32948188}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+1med_but3.png", "start": 32948188, "end": 32949890}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+1med_but_s1.png", "start": 32949890, "end": 32954109}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+1med_sht_but1.png", "start": 32954109, "end": 32955422}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+2med_but_s1.png", "start": 32955422, "end": 32959696}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+3med_but_s1.png", "start": 32959696, "end": 32963915}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+amed_but1.png", "start": 32963915, "end": 32967998}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+amed_but2.png", "start": 32967998, "end": 32969716}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+amed_but3.png", "start": 32969716, "end": 32971326}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+amed_but_s1.png", "start": 32971326, "end": 32975539}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+amed_sht_but1.png", "start": 32975539, "end": 32976695}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/Art1.png", "start": 32976695, "end": 33109688}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/afloor1_4.png", "start": 33109688, "end": 33112689}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/afloor1_8.png", "start": 33112689, "end": 33115529}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/afloor3_1.png", "start": 33115529, "end": 33118339}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/altar1_1.png", "start": 33118339, "end": 33122227}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/altar1_3.png", "start": 33122227, "end": 33126708}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/altar1_4.png", "start": 33126708, "end": 33130761}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/brick0.png", "start": 33130761, "end": 33138983}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/brick1.png", "start": 33138983, "end": 33149737}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/brick4_s.png", "start": 33149737, "end": 33151670}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/brown1.png", "start": 33151670, "end": 33160846}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1a.png", "start": 33160846, "end": 33207201}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1b.png", "start": 33207201, "end": 33290565}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1c.png", "start": 33290565, "end": 33388736}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1d.png", "start": 33388736, "end": 33497405}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1e.png", "start": 33497405, "end": 33605898}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1f.png", "start": 33605898, "end": 33673757}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1s.png", "start": 33673757, "end": 33791208}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2a.png", "start": 33791208, "end": 33868592}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2b.png", "start": 33868592, "end": 33946031}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2c.png", "start": 33946031, "end": 34034906}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2d.png", "start": 34034906, "end": 34133100}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2e.png", "start": 34133100, "end": 34231337}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2f.png", "start": 34231337, "end": 34295881}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2s.png", "start": 34295881, "end": 34401633}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_ceil1a.png", "start": 34401633, "end": 34413441}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_ceil1b.png", "start": 34413441, "end": 34425976}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_wal2a.png", "start": 34425976, "end": 34438873}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_wal2b.png", "start": 34438873, "end": 34452129}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_wal2c.png", "start": 34452129, "end": 34464571}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_win1.png", "start": 34464571, "end": 34507569}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_win1b.png", "start": 34507569, "end": 34552895}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_but_side.png", "start": 34552895, "end": 34553893}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet1.png", "start": 34553893, "end": 34557017}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet2a.png", "start": 34557017, "end": 34560168}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet2b.png", "start": 34560168, "end": 34563230}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet2c.png", "start": 34563230, "end": 34566367}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet3a.png", "start": 34566367, "end": 34569639}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet3b.png", "start": 34569639, "end": 34572821}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet4.png", "start": 34572821, "end": 34575662}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet5a.png", "start": 34575662, "end": 34578800}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet5c.png", "start": 34578800, "end": 34581943}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk10.png", "start": 34581943, "end": 34596669}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk10_f.png", "start": 34596669, "end": 34608957}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk10b.png", "start": 34608957, "end": 34623520}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk11.png", "start": 34623520, "end": 34634768}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk12.png", "start": 34634768, "end": 34651717}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk12_f.png", "start": 34651717, "end": 34669050}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk13.png", "start": 34669050, "end": 34725167}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk14.png", "start": 34725167, "end": 34741021}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk14_f.png", "start": 34741021, "end": 34753990}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk14b.png", "start": 34753990, "end": 34767480}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk15.png", "start": 34767480, "end": 34781306}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk15b.png", "start": 34781306, "end": 34795236}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk15f.png", "start": 34795236, "end": 34808434}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk16.png", "start": 34808434, "end": 34824655}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk16b.png", "start": 34824655, "end": 34842247}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk16f.png", "start": 34842247, "end": 34859518}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk17.png", "start": 34859518, "end": 34869625}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk17_f.png", "start": 34869625, "end": 34878650}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk17b.png", "start": 34878650, "end": 34888787}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18_f.png", "start": 34888787, "end": 34897420}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18_gb.png", "start": 34897420, "end": 34900168}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18_gt.png", "start": 34900168, "end": 34902905}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18_t.png", "start": 34902905, "end": 34911091}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18_tb.png", "start": 34911091, "end": 34921717}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18_tc.png", "start": 34921717, "end": 34926246}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18b.png", "start": 34926246, "end": 34934419}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk19_f.png", "start": 34934419, "end": 34946120}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk19_t.png", "start": 34946120, "end": 34957511}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk19b.png", "start": 34957511, "end": 34968831}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk1_1.png", "start": 34968831, "end": 34971316}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk1_2.png", "start": 34971316, "end": 34974487}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk1_3.png", "start": 34974487, "end": 34976730}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk2_1.png", "start": 34976730, "end": 34979279}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk2_2.png", "start": 34979279, "end": 34982652}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk5.png", "start": 34982652, "end": 34985819}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk6_1.png", "start": 34985819, "end": 34988462}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk6_2.png", "start": 34988462, "end": 34993524}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk7_1.png", "start": 34993524, "end": 34996164}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk7_1b.png", "start": 34996164, "end": 34998708}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk7_2.png", "start": 34998708, "end": 35001874}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk8_1c.png", "start": 35001874, "end": 35014479}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk8_1d.png", "start": 35014479, "end": 35027082}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk9_1.png", "start": 35027082, "end": 35040292}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk9_1b.png", "start": 35040292, "end": 35053949}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk9_f.png", "start": 35053949, "end": 35066736}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr1_1.png", "start": 35066736, "end": 35076683}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr2_1.png", "start": 35076683, "end": 35078969}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr2_2.png", "start": 35078969, "end": 35081142}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr4_1.png", "start": 35081142, "end": 35084882}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr4_3.png", "start": 35084882, "end": 35088316}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr4_4.png", "start": 35088316, "end": 35101372}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr4_5.png", "start": 35101372, "end": 35115890}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr5_1.png", "start": 35115890, "end": 35130820}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr5_2.png", "start": 35130820, "end": 35146348}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_stp1.png", "start": 35146348, "end": 35148945}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_stp2.png", "start": 35148945, "end": 35151488}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_trm1.png", "start": 35151488, "end": 35152813}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick1.png", "start": 35152813, "end": 35174286}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick1_t.png", "start": 35174286, "end": 35185673}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick1_t2.png", "start": 35185673, "end": 35198835}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick1_t2b_fbr.png", "start": 35198835, "end": 35212785}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick1_t3.png", "start": 35212785, "end": 35224525}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick1_t4.png", "start": 35224525, "end": 35237301}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick2.png", "start": 35237301, "end": 35242835}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick3.png", "start": 35242835, "end": 35248209}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick4.png", "start": 35248209, "end": 35253517}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick5.png", "start": 35253517, "end": 35259339}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick6.png", "start": 35259339, "end": 35268500}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick6b.png", "start": 35268500, "end": 35277508}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick6f.png", "start": 35277508, "end": 35286798}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_door1.png", "start": 35286798, "end": 35291481}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_door2.png", "start": 35291481, "end": 35295980}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_door3.png", "start": 35295980, "end": 35300629}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_door3b.png", "start": 35300629, "end": 35303170}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_door4.png", "start": 35303170, "end": 35307734}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_door4b.png", "start": 35307734, "end": 35310330}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr1a.png", "start": 35310330, "end": 35329114}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr1a_blu.png", "start": 35329114, "end": 35347478}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr1b.png", "start": 35347478, "end": 35371627}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr1b_blu.png", "start": 35371627, "end": 35391246}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr2a.png", "start": 35391246, "end": 35415623}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr2a_blu.png", "start": 35415623, "end": 35435517}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr3a.png", "start": 35435517, "end": 35449000}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr3a_blu.png", "start": 35449000, "end": 35462290}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr3b.png", "start": 35462290, "end": 35476598}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr3b_blu.png", "start": 35476598, "end": 35490804}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr3c.png", "start": 35490804, "end": 35503647}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr3c_blu.png", "start": 35503647, "end": 35516771}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dwall1.png", "start": 35516771, "end": 35519247}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick1.png", "start": 35519247, "end": 35531829}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick10.png", "start": 35531829, "end": 35542714}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick10b.png", "start": 35542714, "end": 35553200}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick11.png", "start": 35553200, "end": 35568459}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick12.png", "start": 35568459, "end": 35585876}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick12b.png", "start": 35585876, "end": 35599115}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick13.png", "start": 35599115, "end": 35609013}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick14.png", "start": 35609013, "end": 35618028}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick15.png", "start": 35618028, "end": 35632829}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick16.png", "start": 35632829, "end": 35644193}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick16b.png", "start": 35644193, "end": 35652511}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick17.png", "start": 35652511, "end": 35665645}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick17b.png", "start": 35665645, "end": 35677911}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick17c.png", "start": 35677911, "end": 35691170}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick18.png", "start": 35691170, "end": 35707170}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick18b.png", "start": 35707170, "end": 35721558}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick2.png", "start": 35721558, "end": 35734050}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick20.png", "start": 35734050, "end": 35747450}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick21.png", "start": 35747450, "end": 35759592}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick22.png", "start": 35759592, "end": 35771753}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick3.png", "start": 35771753, "end": 35784213}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick4.png", "start": 35784213, "end": 35799763}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick5.png", "start": 35799763, "end": 35813579}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick6.png", "start": 35813579, "end": 35828290}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick7.png", "start": 35828290, "end": 35841935}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick8.png", "start": 35841935, "end": 35854674}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick9.png", "start": 35854674, "end": 35869229}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_etrim1.png", "start": 35869229, "end": 35872558}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_glass1.png", "start": 35872558, "end": 35884937}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_glass2.png", "start": 35884937, "end": 35896594}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_glass3.png", "start": 35896594, "end": 35907609}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_glass4.png", "start": 35907609, "end": 35939428}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_glass5.png", "start": 35939428, "end": 35949210}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_dec1.png", "start": 35949210, "end": 35953350}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_key1a.png", "start": 35953350, "end": 35955110}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_key1b.png", "start": 35955110, "end": 35956444}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_key2a.png", "start": 35956444, "end": 35958204}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_key2b.png", "start": 35958204, "end": 35959910}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_trim1.png", "start": 35959910, "end": 35960937}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_trim2.png", "start": 35960937, "end": 35961884}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_trim3.png", "start": 35961884, "end": 35962758}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_metw1a.png", "start": 35962758, "end": 35975066}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_metw1b.png", "start": 35975066, "end": 35983773}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_metw2a.png", "start": 35983773, "end": 35993680}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_metw2b.png", "start": 35993680, "end": 36006058}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_rmet.png", "start": 36006058, "end": 36019159}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_rmet_slat.png", "start": 36019159, "end": 36033470}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_rmet_tile.png", "start": 36033470, "end": 36047402}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_rmet_trim32.png", "start": 36047402, "end": 36060611}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_roof1.png", "start": 36060611, "end": 36072526}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_roof2.png", "start": 36072526, "end": 36087599}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_roof3.png", "start": 36087599, "end": 36102528}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_roof4.png", "start": 36102528, "end": 36120342}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_roof5.png", "start": 36120342, "end": 36130848}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall1.png", "start": 36130848, "end": 36172526}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall2.png", "start": 36172526, "end": 36232303}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall3.png", "start": 36232303, "end": 36282738}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall4.png", "start": 36282738, "end": 36331879}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall4_f.png", "start": 36331879, "end": 36375832}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall6.png", "start": 36375832, "end": 36420377}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall7.png", "start": 36420377, "end": 36465970}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall8.png", "start": 36465970, "end": 36503568}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall9.png", "start": 36503568, "end": 36552648}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall9_f.png", "start": 36552648, "end": 36596627}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_telepad.png", "start": 36596627, "end": 36601254}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tile1.png", "start": 36601254, "end": 36673865}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_lit1_fbr.png", "start": 36673865, "end": 36675468}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_lit3_fbr.png", "start": 36675468, "end": 36676773}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_tele.png", "start": 36676773, "end": 36679955}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_trim1.png", "start": 36679955, "end": 36683121}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_trim1b.png", "start": 36683121, "end": 36685176}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_trim2.png", "start": 36685176, "end": 36688716}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_trim3.png", "start": 36688716, "end": 36692193}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_trim4.png", "start": 36692193, "end": 36695306}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim1_1.png", "start": 36695306, "end": 36704010}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim2_1.png", "start": 36704010, "end": 36705124}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim3_1.png", "start": 36705124, "end": 36708719}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim3_2.png", "start": 36708719, "end": 36712278}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim3_3.png", "start": 36712278, "end": 36715975}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim3_4.png", "start": 36715975, "end": 36719346}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim4_1.png", "start": 36719346, "end": 36722770}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim4_2.png", "start": 36722770, "end": 36726294}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim4_3.png", "start": 36726294, "end": 36729654}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim4_4.png", "start": 36729654, "end": 36732877}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/plus_0_csl_brk14.png", "start": 36732877, "end": 36749190}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/plus_1_csl_brk14.png", "start": 36749190, "end": 36765483}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/plus_2_csl_brk14.png", "start": 36765483, "end": 36781782}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/plus_3_csl_brk14.png", "start": 36781782, "end": 36798024}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/plus_4_csl_brk14.png", "start": 36798024, "end": 36814365}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/sidewalk.png", "start": 36814365, "end": 36823121}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/sq_trim1_2.png", "start": 36823121, "end": 36831403}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/sq_trim1_2_s.png", "start": 36831403, "end": 36833978}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/tile.png", "start": 36833978, "end": 36837103}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/tile1.png", "start": 36837103, "end": 36909716}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/wall14_5.png", "start": 36909716, "end": 36913796}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/wbrick1_5.png", "start": 36913796, "end": 36917562}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/wswamp2_1.png", "start": 36917562, "end": 36920688}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/wswamp2_2.png", "start": 36920688, "end": 36924465}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig1_bone.png", "start": 36924465, "end": 36934146}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig1_bone_l.png", "start": 36934146, "end": 36964807}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig1_skull.png", "start": 36964807, "end": 36967471}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig1_skull_l.png", "start": 36967471, "end": 36976256}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig1_spine.png", "start": 36976256, "end": 37024957}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig2_bone.png", "start": 37024957, "end": 37033380}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig2_bone_l.png", "start": 37033380, "end": 37059128}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig2_bone_s.png", "start": 37059128, "end": 37061741}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig2_mouth_s.png", "start": 37061741, "end": 37064156}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/med_flat8.png", "start": 37064156, "end": 37067014}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/med_flat9.png", "start": 37067014, "end": 37070002}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_block.png", "start": 37070002, "end": 37073341}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_diam.png", "start": 37073341, "end": 37076231}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_trim28.png", "start": 37076231, "end": 37081274}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_trim32.png", "start": 37081274, "end": 37084420}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_trim32r.png", "start": 37084420, "end": 37087675}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_trim32s.png", "start": 37087675, "end": 37090775}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_trim64.png", "start": 37090775, "end": 37093918}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_block.png", "start": 37093918, "end": 37098938}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_det1.png", "start": 37098938, "end": 37100094}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_diam.png", "start": 37100094, "end": 37102991}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_diam2.png", "start": 37102991, "end": 37105786}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_diamc.png", "start": 37105786, "end": 37109317}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_door1.png", "start": 37109317, "end": 37114000}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_door2.png", "start": 37114000, "end": 37118499}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_door3.png", "start": 37118499, "end": 37123148}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_door4.png", "start": 37123148, "end": 37127712}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_door5.png", "start": 37127712, "end": 37130655}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_door6.png", "start": 37130655, "end": 37133251}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_fac1.png", "start": 37133251, "end": 37135653}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_flat.png", "start": 37135653, "end": 37139087}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_flatst.png", "start": 37139087, "end": 37141316}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_gig1.png", "start": 37141316, "end": 37145435}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_gig2.png", "start": 37145435, "end": 37149048}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_gig2b.png", "start": 37149048, "end": 37155242}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_grate.png", "start": 37155242, "end": 37158297}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_grate2.png", "start": 37158297, "end": 37161128}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_grate3.png", "start": 37161128, "end": 37162788}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_lit1_fbr.png", "start": 37162788, "end": 37164859}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_lit2_fbr.png", "start": 37164859, "end": 37166048}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_lit3.png", "start": 37166048, "end": 37167234}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_lit4.png", "start": 37167234, "end": 37168495}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_lit5.png", "start": 37168495, "end": 37170588}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_pan1.png", "start": 37170588, "end": 37174070}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_pan2.png", "start": 37174070, "end": 37176356}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_pan3.png", "start": 37176356, "end": 37179783}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_rect.png", "start": 37179783, "end": 37183562}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_rivg.png", "start": 37183562, "end": 37187483}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_rivs.png", "start": 37187483, "end": 37190841}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_slat.png", "start": 37190841, "end": 37194627}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_sqr.png", "start": 37194627, "end": 37198398}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_sqrd.png", "start": 37198398, "end": 37202236}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_sqrs.png", "start": 37202236, "end": 37205990}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_stile.png", "start": 37205990, "end": 37209217}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_tile.png", "start": 37209217, "end": 37212666}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim16.png", "start": 37212666, "end": 37216370}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim16g.png", "start": 37216370, "end": 37220120}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim16h.png", "start": 37220120, "end": 37223737}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim16s.png", "start": 37223737, "end": 37226905}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim28.png", "start": 37226905, "end": 37232202}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim32.png", "start": 37232202, "end": 37235652}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim32r.png", "start": 37235652, "end": 37239308}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim32s.png", "start": 37239308, "end": 37242632}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim64.png", "start": 37242632, "end": 37246109}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_vtrim.png", "start": 37246109, "end": 37249679}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn2_pat.png", "start": 37249679, "end": 37253528}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_block.png", "start": 37253528, "end": 37257239}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_blockl.png", "start": 37257239, "end": 37260768}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_det1.png", "start": 37260768, "end": 37261931}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_flat.png", "start": 37261931, "end": 37265452}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_grate.png", "start": 37265452, "end": 37268840}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_grate2.png", "start": 37268840, "end": 37272346}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_grate3.png", "start": 37272346, "end": 37274364}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_lit1_fbr.png", "start": 37274364, "end": 37276477}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_lit2_fbr.png", "start": 37276477, "end": 37277665}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_lit3.png", "start": 37277665, "end": 37278836}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_lit4.png", "start": 37278836, "end": 37280046}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_lit5.png", "start": 37280046, "end": 37282099}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_pan1.png", "start": 37282099, "end": 37285562}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_pan2.png", "start": 37285562, "end": 37287838}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_pan3.png", "start": 37287838, "end": 37291315}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_pan4.png", "start": 37291315, "end": 37295178}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_rect.png", "start": 37295178, "end": 37298856}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_rivg.png", "start": 37298856, "end": 37302884}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_rivs.png", "start": 37302884, "end": 37306307}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_signs.png", "start": 37306307, "end": 37311141}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_slat.png", "start": 37311141, "end": 37314784}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_sqr.png", "start": 37314784, "end": 37318475}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_sqrd.png", "start": 37318475, "end": 37322283}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_sqrs.png", "start": 37322283, "end": 37325987}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_stile.png", "start": 37325987, "end": 37330072}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_tile.png", "start": 37330072, "end": 37333453}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_tile2.png", "start": 37333453, "end": 37336786}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim16.png", "start": 37336786, "end": 37340892}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim16g.png", "start": 37340892, "end": 37345096}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim16h.png", "start": 37345096, "end": 37349293}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim16s.png", "start": 37349293, "end": 37352916}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim32.png", "start": 37352916, "end": 37356403}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim32s.png", "start": 37356403, "end": 37359789}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim64.png", "start": 37359789, "end": 37363429}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_vtrim.png", "start": 37363429, "end": 37366882}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_cop_flat.png", "start": 37366882, "end": 37369959}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_cop_riv.png", "start": 37369959, "end": 37373560}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_dbrn_flat.png", "start": 37373560, "end": 37376940}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_dbrn_rect.png", "start": 37376940, "end": 37380825}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_dbrn_slat.png", "start": 37380825, "end": 37384640}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grate.png", "start": 37384640, "end": 37387990}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_block.png", "start": 37387990, "end": 37391915}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_blockl.png", "start": 37391915, "end": 37395644}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_det1.png", "start": 37395644, "end": 37396895}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_fac1.png", "start": 37396895, "end": 37399805}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_flat.png", "start": 37399805, "end": 37403690}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_grate.png", "start": 37403690, "end": 37406752}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_grate2.png", "start": 37406752, "end": 37409735}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_grate3.png", "start": 37409735, "end": 37411479}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_lit1_fbr.png", "start": 37411479, "end": 37413613}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_lit2_fbr.png", "start": 37413613, "end": 37414814}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_lit3.png", "start": 37414814, "end": 37416026}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_lit4.png", "start": 37416026, "end": 37417312}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_lit5.png", "start": 37417312, "end": 37419504}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_pan1.png", "start": 37419504, "end": 37423220}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_pan2.png", "start": 37423220, "end": 37425585}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_pan3.png", "start": 37425585, "end": 37429185}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_rect.png", "start": 37429185, "end": 37433222}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_rivg.png", "start": 37433222, "end": 37437634}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_rivs.png", "start": 37437634, "end": 37441401}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_slat.png", "start": 37441401, "end": 37445442}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_sqr.png", "start": 37445442, "end": 37449440}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_sqrd.png", "start": 37449440, "end": 37453550}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_sqrs.png", "start": 37453550, "end": 37457580}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_stile.png", "start": 37457580, "end": 37460864}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_tile.png", "start": 37460864, "end": 37464125}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim16.png", "start": 37464125, "end": 37467737}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim16g.png", "start": 37467737, "end": 37471891}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim16h.png", "start": 37471891, "end": 37475440}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim16s.png", "start": 37475440, "end": 37478951}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim28.png", "start": 37478951, "end": 37483886}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim28r.png", "start": 37483886, "end": 37488912}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim32.png", "start": 37488912, "end": 37492737}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim32r.png", "start": 37492737, "end": 37496607}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim32s.png", "start": 37496607, "end": 37500253}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim64.png", "start": 37500253, "end": 37503910}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_vtrim.png", "start": 37503910, "end": 37507160}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_beam.png", "start": 37507160, "end": 37510174}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_block.png", "start": 37510174, "end": 37513197}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_flat.png", "start": 37513197, "end": 37515931}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_lit1_fbr.png", "start": 37515931, "end": 37517742}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_lit2_fbr.png", "start": 37517742, "end": 37518865}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_lit2b.png", "start": 37518865, "end": 37522315}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_pan1.png", "start": 37522315, "end": 37525411}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_pan2.png", "start": 37525411, "end": 37527564}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_pan3.png", "start": 37527564, "end": 37530667}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_rect.png", "start": 37530667, "end": 37533864}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_signs.png", "start": 37533864, "end": 37537694}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_slat.png", "start": 37537694, "end": 37541110}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_sqr.png", "start": 37541110, "end": 37543944}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_sqrd.png", "start": 37543944, "end": 37547192}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_sqrs.png", "start": 37547192, "end": 37550153}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_trim64.png", "start": 37550153, "end": 37553854}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_lbrn_flat.png", "start": 37553854, "end": 37557280}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_lbrn_rect.png", "start": 37557280, "end": 37561168}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_lbrn_slat.png", "start": 37561168, "end": 37565122}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_lift.png", "start": 37565122, "end": 37568167}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_met7_1.png", "start": 37568167, "end": 37570640}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mix_beam.png", "start": 37570640, "end": 37574292}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mix_diam.png", "start": 37574292, "end": 37577481}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mix_diam2.png", "start": 37577481, "end": 37580680}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mix_diamc.png", "start": 37580680, "end": 37584780}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt1_flat.png", "start": 37584780, "end": 37599070}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt1_rect.png", "start": 37599070, "end": 37614780}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt1_slat.png", "start": 37614780, "end": 37630536}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt1_sqr.png", "start": 37630536, "end": 37637076}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt2_flat.png", "start": 37637076, "end": 37653290}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt2_rect.png", "start": 37653290, "end": 37670016}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt2_slat.png", "start": 37670016, "end": 37686654}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt2_sqr.png", "start": 37686654, "end": 37695052}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt3_flat.png", "start": 37695052, "end": 37711652}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt3_rect.png", "start": 37711652, "end": 37728560}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt3_slat.png", "start": 37728560, "end": 37745471}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt3_sqr.png", "start": 37745471, "end": 37754006}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_ora_trim64.png", "start": 37754006, "end": 37756785}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_rail_flat.png", "start": 37756785, "end": 37758932}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_rune1_fbr.png", "start": 37758932, "end": 37761736}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_rune_trim32.png", "start": 37761736, "end": 37766077}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_set1.png", "start": 37766077, "end": 37835922}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_shm_flat.png", "start": 37835922, "end": 37839583}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_shm_rect.png", "start": 37839583, "end": 37843455}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_shm_slat.png", "start": 37843455, "end": 37847506}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_shm_sqr.png", "start": 37847506, "end": 37850968}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_teal_block.png", "start": 37850968, "end": 37854346}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_teal_flat.png", "start": 37854346, "end": 37857692}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_teal_trim32.png", "start": 37857692, "end": 37860968}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_teal_trim32r.png", "start": 37860968, "end": 37863943}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_teal_trim64.png", "start": 37863943, "end": 37867427}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_wall3_1.png", "start": 37867427, "end": 37878693}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_wall3_1_s.png", "start": 37878693, "end": 37881977}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/metal4_4.png", "start": 37881977, "end": 37886318}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0_sqbut1.png", "start": 37886318, "end": 37887400}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0_sqbut2_fbr.png", "start": 37887400, "end": 37890636}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0_sqshoot1_fbr.png", "start": 37890636, "end": 37891743}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0gig2a_fbr.png", "start": 37891743, "end": 37892216}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0gig_shot_fbr.png", "start": 37892216, "end": 37892692}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0gig_sshot_fbr.png", "start": 37892692, "end": 37893136}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0gig_ye_fbr.png", "start": 37893136, "end": 37893576}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0met_blu_keyg_fbr.png", "start": 37893576, "end": 37894476}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0met_blu_keys_fbr.png", "start": 37894476, "end": 37895359}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_1_sqbut1.png", "start": 37895359, "end": 37896525}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_1_sqbut2_fbr.png", "start": 37896525, "end": 37899713}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_1_sqshoot1.png", "start": 37899713, "end": 37900827}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_1met_blu_keyg_fbr.png", "start": 37900827, "end": 37901728}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_1met_blu_keys_fbr.png", "start": 37901728, "end": 37902605}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_2met_blu_keyg_fbr.png", "start": 37902605, "end": 37903514}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_2met_blu_keys_fbr.png", "start": 37903514, "end": 37904383}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_3met_blu_keyg_fbr.png", "start": 37904383, "end": 37905291}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_3met_blu_keys_fbr.png", "start": 37905291, "end": 37906147}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_4met_blu_keyg_fbr.png", "start": 37906147, "end": 37907055}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_4met_blu_keys_fbr.png", "start": 37907055, "end": 37907911}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_5met_blu_keyg_fbr.png", "start": 37907911, "end": 37908820}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_5met_blu_keys_fbr.png", "start": 37908820, "end": 37909689}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_6met_blu_keyg_fbr.png", "start": 37909689, "end": 37910590}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_6met_blu_keys_fbr.png", "start": 37910590, "end": 37911467}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_a_sqbut1.png", "start": 37911467, "end": 37912549}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_a_sqbut2_fbr.png", "start": 37912549, "end": 37915785}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_a_sqshoot1_fbr.png", "start": 37915785, "end": 37916892}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_agig2a.png", "start": 37916892, "end": 37917262}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_agig_shot_fbr.png", "start": 37917262, "end": 37917637}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_agig_sshot_fbr.png", "start": 37917637, "end": 37918000}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_agig_ye.png", "start": 37918000, "end": 37918412}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_amet_blu_keyg.png", "start": 37918412, "end": 37919282}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_amet_blu_keys.png", "start": 37919282, "end": 37920152}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/ret_metal1_tile.png", "start": 37920152, "end": 37934084}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/sq_lit1_fbr.png", "start": 37934084, "end": 37934457}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/sq_lit2_fbr.png", "start": 37934457, "end": 37934718}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_a.png", "start": 37934718, "end": 37935266}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_b.png", "start": 37935266, "end": 37935792}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_c.png", "start": 37935792, "end": 37936318}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_d.png", "start": 37936318, "end": 37936844}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_e.png", "start": 37936844, "end": 37937370}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_f.png", "start": 37937370, "end": 37937896}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_g.png", "start": 37937896, "end": 37938422}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_h.png", "start": 37938422, "end": 37938948}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_i.png", "start": 37938948, "end": 37939475}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_j.png", "start": 37939475, "end": 37940002}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_k.png", "start": 37940002, "end": 37940529}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_l.png", "start": 37940529, "end": 37941056}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_m.png", "start": 37941056, "end": 37941583}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_n.png", "start": 37941583, "end": 37942108}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_o.png", "start": 37942108, "end": 37942633}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_p.png", "start": 37942633, "end": 37943158}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_a.png", "start": 37943158, "end": 37943684}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_b.png", "start": 37943684, "end": 37944210}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_c.png", "start": 37944210, "end": 37944736}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_d.png", "start": 37944736, "end": 37945262}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_e.png", "start": 37945262, "end": 37945788}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_f.png", "start": 37945788, "end": 37946314}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_g.png", "start": 37946314, "end": 37946840}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_h.png", "start": 37946840, "end": 37947366}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_i.png", "start": 37947366, "end": 37947892}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_j.png", "start": 37947892, "end": 37948418}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_k.png", "start": 37948418, "end": 37948944}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_l.png", "start": 37948944, "end": 37949470}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_m.png", "start": 37949470, "end": 37949996}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_n.png", "start": 37949996, "end": 37950522}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_o.png", "start": 37950522, "end": 37951048}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_p.png", "start": 37951048, "end": 37951574}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_a.png", "start": 37951574, "end": 37952100}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_b.png", "start": 37952100, "end": 37952626}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_c.png", "start": 37952626, "end": 37953152}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_d.png", "start": 37953152, "end": 37953678}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_e.png", "start": 37953678, "end": 37954204}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_f.png", "start": 37954204, "end": 37954730}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_g.png", "start": 37954730, "end": 37955256}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_h.png", "start": 37955256, "end": 37955782}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_i.png", "start": 37955782, "end": 37956308}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_j.png", "start": 37956308, "end": 37956834}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_k.png", "start": 37956834, "end": 37957360}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_l.png", "start": 37957360, "end": 37957886}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_m.png", "start": 37957886, "end": 37958413}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_n.png", "start": 37958413, "end": 37958940}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_o.png", "start": 37958940, "end": 37959467}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_p.png", "start": 37959467, "end": 37959994}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_a.png", "start": 37959994, "end": 37960542}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_b.png", "start": 37960542, "end": 37961068}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_c.png", "start": 37961068, "end": 37961594}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_d.png", "start": 37961594, "end": 37962120}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_e.png", "start": 37962120, "end": 37962646}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_f.png", "start": 37962646, "end": 37963172}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_g.png", "start": 37963172, "end": 37963698}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_h.png", "start": 37963698, "end": 37964224}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_i.png", "start": 37964224, "end": 37964750}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_j.png", "start": 37964750, "end": 37965276}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_k.png", "start": 37965276, "end": 37965802}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_l.png", "start": 37965802, "end": 37966328}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_m.png", "start": 37966328, "end": 37966854}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_n.png", "start": 37966854, "end": 37967380}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_o.png", "start": 37967380, "end": 37967906}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_p.png", "start": 37967906, "end": 37968432}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_a.png", "start": 37968432, "end": 37968958}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_b.png", "start": 37968958, "end": 37969484}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_c.png", "start": 37969484, "end": 37970010}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_d.png", "start": 37970010, "end": 37970536}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_e.png", "start": 37970536, "end": 37971062}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_f.png", "start": 37971062, "end": 37971588}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_g.png", "start": 37971588, "end": 37972114}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_h.png", "start": 37972114, "end": 37972640}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_i.png", "start": 37972640, "end": 37973166}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_j.png", "start": 37973166, "end": 37973692}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_k.png", "start": 37973692, "end": 37974218}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_l.png", "start": 37974218, "end": 37974744}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_m.png", "start": 37974744, "end": 37975270}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_n.png", "start": 37975270, "end": 37975796}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_o.png", "start": 37975796, "end": 37976322}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_p.png", "start": 37976322, "end": 37976848}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_a.png", "start": 37976848, "end": 37977374}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_b.png", "start": 37977374, "end": 37977900}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_c.png", "start": 37977900, "end": 37978426}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_d.png", "start": 37978426, "end": 37978952}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_e.png", "start": 37978952, "end": 37979478}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_f.png", "start": 37979478, "end": 37980004}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_g.png", "start": 37980004, "end": 37980530}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_h.png", "start": 37980530, "end": 37981056}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_i.png", "start": 37981056, "end": 37981582}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_j.png", "start": 37981582, "end": 37982108}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_k.png", "start": 37982108, "end": 37982634}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_l.png", "start": 37982634, "end": 37983160}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_m.png", "start": 37983160, "end": 37983686}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_n.png", "start": 37983686, "end": 37984212}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_o.png", "start": 37984212, "end": 37984738}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_p.png", "start": 37984738, "end": 37985264}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_a.png", "start": 37985264, "end": 37985790}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_b.png", "start": 37985790, "end": 37986316}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_c.png", "start": 37986316, "end": 37986842}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_d.png", "start": 37986842, "end": 37987368}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_e.png", "start": 37987368, "end": 37987894}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_f.png", "start": 37987894, "end": 37988420}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_g.png", "start": 37988420, "end": 37988946}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_h.png", "start": 37988946, "end": 37989472}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_i.png", "start": 37989472, "end": 37989998}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_j.png", "start": 37989998, "end": 37990524}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_k.png", "start": 37990524, "end": 37991050}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_l.png", "start": 37991050, "end": 37991576}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_m.png", "start": 37991576, "end": 37992102}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_n.png", "start": 37992102, "end": 37992628}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_o.png", "start": 37992628, "end": 37993154}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_p.png", "start": 37993154, "end": 37993680}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_a.png", "start": 37993680, "end": 37994206}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_b.png", "start": 37994206, "end": 37994732}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_c.png", "start": 37994732, "end": 37995258}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_d.png", "start": 37995258, "end": 37995784}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_e.png", "start": 37995784, "end": 37996310}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_f.png", "start": 37996310, "end": 37996836}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_g.png", "start": 37996836, "end": 37997362}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_h.png", "start": 37997362, "end": 37997888}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_i.png", "start": 37997888, "end": 37998414}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_j.png", "start": 37998414, "end": 37998940}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_k.png", "start": 37998940, "end": 37999466}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_l.png", "start": 37999466, "end": 37999992}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_m.png", "start": 37999992, "end": 38000518}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_n.png", "start": 38000518, "end": 38001045}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_o.png", "start": 38001045, "end": 38001572}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_p.png", "start": 38001572, "end": 38002099}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_a.png", "start": 38002099, "end": 38002626}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_b.png", "start": 38002626, "end": 38003153}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_c.png", "start": 38003153, "end": 38003680}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_d.png", "start": 38003680, "end": 38004206}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_e.png", "start": 38004206, "end": 38004732}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_f.png", "start": 38004732, "end": 38005258}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_g.png", "start": 38005258, "end": 38005784}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_h.png", "start": 38005784, "end": 38006310}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_i.png", "start": 38006310, "end": 38006836}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_j.png", "start": 38006836, "end": 38007362}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_k.png", "start": 38007362, "end": 38007888}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_l.png", "start": 38007888, "end": 38008414}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_m.png", "start": 38008414, "end": 38008940}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_n.png", "start": 38008940, "end": 38009466}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_o.png", "start": 38009466, "end": 38009992}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_p.png", "start": 38009992, "end": 38010518}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_a.png", "start": 38010518, "end": 38011045}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_b.png", "start": 38011045, "end": 38011571}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_c.png", "start": 38011571, "end": 38012097}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_d.png", "start": 38012097, "end": 38012623}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_e.png", "start": 38012623, "end": 38013149}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_f.png", "start": 38013149, "end": 38013675}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_g.png", "start": 38013675, "end": 38014201}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_h.png", "start": 38014201, "end": 38014727}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_i.png", "start": 38014727, "end": 38015253}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_j.png", "start": 38015253, "end": 38015779}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_k.png", "start": 38015779, "end": 38016305}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_l.png", "start": 38016305, "end": 38016831}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_m.png", "start": 38016831, "end": 38017357}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_n.png", "start": 38017357, "end": 38017883}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_o.png", "start": 38017883, "end": 38018409}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_p.png", "start": 38018409, "end": 38018935}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_a.png", "start": 38018935, "end": 38019462}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_b.png", "start": 38019462, "end": 38019989}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_c.png", "start": 38019989, "end": 38020516}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_d.png", "start": 38020516, "end": 38021043}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_e.png", "start": 38021043, "end": 38021570}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_f.png", "start": 38021570, "end": 38022096}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_g.png", "start": 38022096, "end": 38022622}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_h.png", "start": 38022622, "end": 38023148}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_i.png", "start": 38023148, "end": 38023674}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_j.png", "start": 38023674, "end": 38024200}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_k.png", "start": 38024200, "end": 38024726}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_l.png", "start": 38024726, "end": 38025252}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_m.png", "start": 38025252, "end": 38025778}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_n.png", "start": 38025778, "end": 38026304}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_o.png", "start": 38026304, "end": 38026830}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_p.png", "start": 38026830, "end": 38027356}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_a.png", "start": 38027356, "end": 38027882}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_b.png", "start": 38027882, "end": 38028408}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_c.png", "start": 38028408, "end": 38028934}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_d.png", "start": 38028934, "end": 38029460}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_e.png", "start": 38029460, "end": 38029986}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_f.png", "start": 38029986, "end": 38030512}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_g.png", "start": 38030512, "end": 38031038}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_h.png", "start": 38031038, "end": 38031564}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_i.png", "start": 38031564, "end": 38032090}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_j.png", "start": 38032090, "end": 38032616}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_k.png", "start": 38032616, "end": 38033142}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_l.png", "start": 38033142, "end": 38033668}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_m.png", "start": 38033668, "end": 38034194}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_n.png", "start": 38034194, "end": 38034720}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_o.png", "start": 38034720, "end": 38035246}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_p.png", "start": 38035246, "end": 38035772}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_a.png", "start": 38035772, "end": 38036298}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_b.png", "start": 38036298, "end": 38036824}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_c.png", "start": 38036824, "end": 38037350}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_d.png", "start": 38037350, "end": 38037876}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_e.png", "start": 38037876, "end": 38038402}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_f.png", "start": 38038402, "end": 38038928}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_g.png", "start": 38038928, "end": 38039454}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_h.png", "start": 38039454, "end": 38039980}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_i.png", "start": 38039980, "end": 38040506}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_j.png", "start": 38040506, "end": 38041032}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_k.png", "start": 38041032, "end": 38041558}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_l.png", "start": 38041558, "end": 38042084}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_m.png", "start": 38042084, "end": 38042610}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_n.png", "start": 38042610, "end": 38043136}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_o.png", "start": 38043136, "end": 38043662}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_p.png", "start": 38043662, "end": 38044188}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_a.png", "start": 38044188, "end": 38044736}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_b.png", "start": 38044736, "end": 38045262}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_c.png", "start": 38045262, "end": 38045788}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_d.png", "start": 38045788, "end": 38046314}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_e.png", "start": 38046314, "end": 38046840}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_f.png", "start": 38046840, "end": 38047366}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_g.png", "start": 38047366, "end": 38047892}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_h.png", "start": 38047892, "end": 38048418}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_i.png", "start": 38048418, "end": 38048944}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_j.png", "start": 38048944, "end": 38049470}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_k.png", "start": 38049470, "end": 38049996}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_l.png", "start": 38049996, "end": 38050522}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_m.png", "start": 38050522, "end": 38051048}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_n.png", "start": 38051048, "end": 38051574}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_o.png", "start": 38051574, "end": 38052100}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_p.png", "start": 38052100, "end": 38052626}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_a_fbr.png", "start": 38052626, "end": 38053152}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_b_fbr.png", "start": 38053152, "end": 38053678}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_c_fbr.png", "start": 38053678, "end": 38054204}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_d_fbr.png", "start": 38054204, "end": 38054730}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_e_fbr.png", "start": 38054730, "end": 38055256}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_f_fbr.png", "start": 38055256, "end": 38055782}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_g_fbr.png", "start": 38055782, "end": 38056308}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_h_fbr.png", "start": 38056308, "end": 38056834}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_i_fbr.png", "start": 38056834, "end": 38057360}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_j_fbr.png", "start": 38057360, "end": 38057886}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_k_fbr.png", "start": 38057886, "end": 38058412}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_l_fbr.png", "start": 38058412, "end": 38058938}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_m_fbr.png", "start": 38058938, "end": 38059464}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_n_fbr.png", "start": 38059464, "end": 38059990}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_o_fbr.png", "start": 38059990, "end": 38060517}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_p_fbr.png", "start": 38060517, "end": 38061044}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_a_fbr.png", "start": 38061044, "end": 38061570}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_b_fbr.png", "start": 38061570, "end": 38062096}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_c_fbr.png", "start": 38062096, "end": 38062622}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_d_fbr.png", "start": 38062622, "end": 38063148}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_e_fbr.png", "start": 38063148, "end": 38063675}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_f_fbr.png", "start": 38063675, "end": 38064202}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_g_fbr.png", "start": 38064202, "end": 38064727}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_h_fbr.png", "start": 38064727, "end": 38065253}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_i_fbr.png", "start": 38065253, "end": 38065779}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_j_fbr.png", "start": 38065779, "end": 38066305}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_k_fbr.png", "start": 38066305, "end": 38066831}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_l_fbr.png", "start": 38066831, "end": 38067357}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_m_fbr.png", "start": 38067357, "end": 38067884}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_n_fbr.png", "start": 38067884, "end": 38068411}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_o_fbr.png", "start": 38068411, "end": 38068936}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_p_fbr.png", "start": 38068936, "end": 38069462}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/JarBod1.png", "start": 38069462, "end": 38069809}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/JarBod2.png", "start": 38069809, "end": 38070127}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/JarTop1.png", "start": 38070127, "end": 38070352}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/JarTop2.png", "start": 38070352, "end": 38070492}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/crate-door-brn.png", "start": 38070492, "end": 38079130}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/crate-door-grn.png", "start": 38079130, "end": 38089186}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/crate-door-orn.png", "start": 38089186, "end": 38098285}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/crate-side-brn.png", "start": 38098285, "end": 38118411}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/crate-side-grn.png", "start": 38118411, "end": 38141324}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/crate-side-orn.png", "start": 38141324, "end": 38162653}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/go-ep0_fbr.png", "start": 38162653, "end": 38164428}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_book_blue.png", "start": 38164428, "end": 38165008}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_book_green.png", "start": 38165008, "end": 38165631}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_book_pink.png", "start": 38165631, "end": 38166286}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_book_red.png", "start": 38166286, "end": 38167003}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_book_teal.png", "start": 38167003, "end": 38167590}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_books_wood.png", "start": 38167590, "end": 38180520}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_dbrick4_p1.png", "start": 38180520, "end": 38212357}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_dbrick4_p2.png", "start": 38212357, "end": 38245778}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_ebrick9_p1.png", "start": 38245778, "end": 38277669}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_ebrick9_p2.png", "start": 38277669, "end": 38309035}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/note-e0_fbr.png", "start": 38309035, "end": 38328464}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_0blink_fbr.png", "start": 38328464, "end": 38328693}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_0tvnoise.png", "start": 38328693, "end": 38329679}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_1blink_fbr.png", "start": 38329679, "end": 38329908}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_1tvnoise.png", "start": 38329908, "end": 38330886}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_2blink_fbr.png", "start": 38330886, "end": 38331117}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_2tvnoise.png", "start": 38331117, "end": 38332090}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_3blink_fbr.png", "start": 38332090, "end": 38332321}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_3tvnoise.png", "start": 38332321, "end": 38333303}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_4blink_fbr.png", "start": 38333303, "end": 38333533}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_4tvnoise.png", "start": 38333533, "end": 38334517}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_5tvnoise.png", "start": 38334517, "end": 38335481}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_6tvnoise.png", "start": 38335481, "end": 38336471}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_7tvnoise.png", "start": 38336471, "end": 38337445}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_8tvnoise.png", "start": 38337445, "end": 38338446}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_9tvnoise.png", "start": 38338446, "end": 38339423}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_ablink_fbr.png", "start": 38339423, "end": 38339653}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_atvnoise.png", "start": 38339653, "end": 38340213}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_atvnoise64.png", "start": 38340213, "end": 38341483}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/qr.png", "start": 38341483, "end": 38342903}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/radio16.png", "start": 38342903, "end": 38343671}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/radio32.png", "start": 38343671, "end": 38344661}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/radio64.png", "start": 38344661, "end": 38346419}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/radiowood.png", "start": 38346419, "end": 38348562}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/secret_gem_1.png", "start": 38348562, "end": 38351598}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/secret_gem_2.png", "start": 38351598, "end": 38354302}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/secret_gem_3.png", "start": 38354302, "end": 38357362}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/secret_gem_4.png", "start": 38357362, "end": 38361120}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/secret_gem_h.png", "start": 38361120, "end": 38361671}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/_t_fence01_fbr.png", "start": 38361671, "end": 38367388}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/_t_flare01_fbr.png", "start": 38367388, "end": 38367774}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqconc03.png", "start": 38367774, "end": 38378334}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqconc04.png", "start": 38378334, "end": 38389390}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqconc05.png", "start": 38389390, "end": 38393981}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqf006b.png", "start": 38393981, "end": 38396608}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqf032.png", "start": 38396608, "end": 38397879}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqf049.png", "start": 38397879, "end": 38400156}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqf074.png", "start": 38400156, "end": 38403578}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqf075.png", "start": 38403578, "end": 38406737}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqmetl01.png", "start": 38406737, "end": 38409465}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqmetl07.png", "start": 38409465, "end": 38414236}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqmetl14.png", "start": 38414236, "end": 38421968}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqmetl28.png", "start": 38421968, "end": 38423620}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqmetl30.png", "start": 38423620, "end": 38425272}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqmetl33.png", "start": 38425272, "end": 38427708}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpanl09.png", "start": 38427708, "end": 38430500}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpanl10.png", "start": 38430500, "end": 38435394}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe01.png", "start": 38435394, "end": 38439582}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe04.png", "start": 38439582, "end": 38442046}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe05.png", "start": 38442046, "end": 38445212}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe08.png", "start": 38445212, "end": 38450844}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe09.png", "start": 38450844, "end": 38458948}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe12.png", "start": 38458948, "end": 38467068}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe13.png", "start": 38467068, "end": 38473379}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe14.png", "start": 38473379, "end": 38480732}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust01.png", "start": 38480732, "end": 38483363}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust02.png", "start": 38483363, "end": 38488664}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust03.png", "start": 38488664, "end": 38493967}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust03b.png", "start": 38493967, "end": 38496697}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust04.png", "start": 38496697, "end": 38498090}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust09.png", "start": 38498090, "end": 38501978}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust10.png", "start": 38501978, "end": 38505144}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsect14.png", "start": 38505144, "end": 38507520}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsect15.png", "start": 38507520, "end": 38510467}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsect16.png", "start": 38510467, "end": 38513713}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsect16b.png", "start": 38513713, "end": 38518670}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp01.png", "start": 38518670, "end": 38521316}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp02.png", "start": 38521316, "end": 38526181}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp03.png", "start": 38526181, "end": 38530083}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp04.png", "start": 38530083, "end": 38531626}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp06.png", "start": 38531626, "end": 38533005}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp07.png", "start": 38533005, "end": 38534360}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp08.png", "start": 38534360, "end": 38536562}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp09.png", "start": 38536562, "end": 38539415}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqtrim01.png", "start": 38539415, "end": 38540642}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqtrim02.png", "start": 38540642, "end": 38541566}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqtrim03.png", "start": 38541566, "end": 38542137}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqtrim08.png", "start": 38542137, "end": 38542869}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/butmet.png", "start": 38542869, "end": 38544780}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_1.png", "start": 38544780, "end": 38548136}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_2.png", "start": 38548136, "end": 38550815}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_3.png", "start": 38550815, "end": 38553304}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_3b.png", "start": 38553304, "end": 38555932}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_4.png", "start": 38555932, "end": 38557537}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_5.png", "start": 38557537, "end": 38560636}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_6.png", "start": 38560636, "end": 38564078}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_7.png", "start": 38564078, "end": 38566423}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_8.png", "start": 38566423, "end": 38568752}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/compbase.png", "start": 38568752, "end": 38571171}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate.png", "start": 38571171, "end": 38574288}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_bottom.png", "start": 38574288, "end": 38576404}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_s_bottom.png", "start": 38576404, "end": 38577492}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_s_sside.png", "start": 38577492, "end": 38578437}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_s_top.png", "start": 38578437, "end": 38579876}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_s_tside.png", "start": 38579876, "end": 38581539}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_side.png", "start": 38581539, "end": 38585007}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_top.png", "start": 38585007, "end": 38587753}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_xs_bot.png", "start": 38587753, "end": 38588420}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_xs_sside.png", "start": 38588420, "end": 38589365}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_xs_top.png", "start": 38589365, "end": 38590235}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_xs_tside.png", "start": 38590235, "end": 38591175}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_bottom.png", "start": 38591175, "end": 38593139}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_s_bottom.png", "start": 38593139, "end": 38594169}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_s_sside.png", "start": 38594169, "end": 38595047}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_s_top.png", "start": 38595047, "end": 38596180}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_s_tside.png", "start": 38596180, "end": 38597645}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_side.png", "start": 38597645, "end": 38600762}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_top.png", "start": 38600762, "end": 38602883}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_xs_bot.png", "start": 38602883, "end": 38603537}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_xs_sside.png", "start": 38603537, "end": 38604415}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_xs_top.png", "start": 38604415, "end": 38605122}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_xs_tside.png", "start": 38605122, "end": 38605960}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/dem4_1.png", "start": 38605960, "end": 38615359}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/dem4_4.png", "start": 38615359, "end": 38623405}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/dem5_3_fbr.png", "start": 38623405, "end": 38632222}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/door02_1.png", "start": 38632222, "end": 38636354}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/doorr02_1.png", "start": 38636354, "end": 38637920}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/doortrak1.png", "start": 38637920, "end": 38639531}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/doortrak2-corn.png", "start": 38639531, "end": 38640828}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/doortrak2.png", "start": 38640828, "end": 38642493}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/ecop1_1.png", "start": 38642493, "end": 38645570}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/ecop1_4.png", "start": 38645570, "end": 38649171}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/edoor01_1.png", "start": 38649171, "end": 38661317}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/edoor02.png", "start": 38661317, "end": 38673368}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/edoor02.png.png", "start": 38673368, "end": 38685419}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/fddoor01.png", "start": 38685419, "end": 38694173}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/fddoor01b.png", "start": 38694173, "end": 38704856}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/fdoor02.png", "start": 38704856, "end": 38713410}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/flat4.png", "start": 38713410, "end": 38714583}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/floor5_2.png", "start": 38714583, "end": 38717309}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/floor5_3.png", "start": 38717309, "end": 38719489}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/laserfield1_fbr.png", "start": 38719489, "end": 38730498}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/light2.png", "start": 38730498, "end": 38731096}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/lit8nb.png", "start": 38731096, "end": 38731410}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/lit8sfb_fbr.png", "start": 38731410, "end": 38731707}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/met2.png", "start": 38731707, "end": 38744031}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/metalstrip_1.png", "start": 38744031, "end": 38746314}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_side1.png", "start": 38746314, "end": 38747079}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_stem.png", "start": 38747079, "end": 38747725}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_top1.png", "start": 38747725, "end": 38751125}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_top2.png", "start": 38751125, "end": 38754840}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_top3.png", "start": 38754840, "end": 38758248}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_top4.png", "start": 38758248, "end": 38760944}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_top5.png", "start": 38760944, "end": 38763039}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0_gkey.png", "start": 38763039, "end": 38764103}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0_skey.png", "start": 38764103, "end": 38765119}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0_tscrn0.png", "start": 38765119, "end": 38767232}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0_tscrn1.png", "start": 38767232, "end": 38769357}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0arrow2_d_fbr.png", "start": 38769357, "end": 38770611}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0arrow2_h_fbr.png", "start": 38770611, "end": 38771880}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0arrow2_u_fbr.png", "start": 38771880, "end": 38773138}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0arrow_d_fbr.png", "start": 38773138, "end": 38774020}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0arrow_h_fbr.png", "start": 38774020, "end": 38774899}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0arrow_u_fbr.png", "start": 38774899, "end": 38775774}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0basebtn1b_fbr.png", "start": 38775774, "end": 38776663}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0basebtn2_fbr.png", "start": 38776663, "end": 38777009}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0basebtn2b_fbr.png", "start": 38777009, "end": 38777349}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0basebtn_fbr.png", "start": 38777349, "end": 38778630}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0button3_fbr.png", "start": 38778630, "end": 38780506}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0lit8s.png", "start": 38780506, "end": 38780820}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0planet_a_fbr.png", "start": 38780820, "end": 38782411}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0planet_b_fbr.png", "start": 38782411, "end": 38783521}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0planet_c_fbr.png", "start": 38783521, "end": 38784621}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0slipbot.png", "start": 38784621, "end": 38787904}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0sliptop.png", "start": 38787904, "end": 38791547}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0tek_jump1_fbr.png", "start": 38791547, "end": 38794030}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0term128.png", "start": 38794030, "end": 38796834}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0term64.png", "start": 38796834, "end": 38798026}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0tlight1.png", "start": 38798026, "end": 38798624}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0tlight2.png", "start": 38798624, "end": 38799237}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0tlight3.png", "start": 38799237, "end": 38799808}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1_gkey.png", "start": 38799808, "end": 38800858}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1_skey.png", "start": 38800858, "end": 38801863}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1arrow2_d_fbr.png", "start": 38801863, "end": 38803119}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1arrow2_h_fbr.png", "start": 38803119, "end": 38804386}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1arrow2_u_fbr.png", "start": 38804386, "end": 38805644}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1arrow_d_fbr.png", "start": 38805644, "end": 38806530}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1arrow_h_fbr.png", "start": 38806530, "end": 38807412}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1arrow_u_fbr.png", "start": 38807412, "end": 38808290}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1basebtn.png", "start": 38808290, "end": 38809569}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1basebtn1b.png", "start": 38809569, "end": 38810447}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1basebtn2.png", "start": 38810447, "end": 38810797}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1basebtn2b.png", "start": 38810797, "end": 38811155}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1planet_a_fbr.png", "start": 38811155, "end": 38812776}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1planet_b_fbr.png", "start": 38812776, "end": 38813861}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1planet_c_fbr.png", "start": 38813861, "end": 38814933}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1tek_jump1_fbr.png", "start": 38814933, "end": 38817416}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1term128.png", "start": 38817416, "end": 38820219}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1term64.png", "start": 38820219, "end": 38821411}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2_gkey.png", "start": 38821411, "end": 38822473}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2_skey.png", "start": 38822473, "end": 38823479}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2arrow2_d_fbr.png", "start": 38823479, "end": 38824722}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2arrow2_h_fbr.png", "start": 38824722, "end": 38825983}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2arrow2_u_fbr.png", "start": 38825983, "end": 38827229}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2arrow_d_fbr.png", "start": 38827229, "end": 38828127}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2arrow_h_fbr.png", "start": 38828127, "end": 38829017}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2arrow_u_fbr.png", "start": 38829017, "end": 38829904}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2planet_a_fbr.png", "start": 38829904, "end": 38831507}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2planet_b_fbr.png", "start": 38831507, "end": 38832609}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2planet_c_fbr.png", "start": 38832609, "end": 38833727}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_3planet_a_fbr.png", "start": 38833727, "end": 38835317}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_3planet_b_fbr.png", "start": 38835317, "end": 38836411}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_3planet_c_fbr.png", "start": 38836411, "end": 38837534}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_4planet_a_fbr.png", "start": 38837534, "end": 38839143}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_4planet_b_fbr.png", "start": 38839143, "end": 38840258}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_4planet_c_fbr.png", "start": 38840258, "end": 38841405}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_5planet_a_fbr.png", "start": 38841405, "end": 38843037}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_5planet_b_fbr.png", "start": 38843037, "end": 38844164}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_5planet_c_fbr.png", "start": 38844164, "end": 38845271}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_6planet_a_fbr.png", "start": 38845271, "end": 38846892}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_6planet_b_fbr.png", "start": 38846892, "end": 38848016}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_6planet_c_fbr.png", "start": 38848016, "end": 38849110}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_7planet_a_fbr.png", "start": 38849110, "end": 38850698}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_7planet_b_fbr.png", "start": 38850698, "end": 38851811}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_7planet_c_fbr.png", "start": 38851811, "end": 38852903}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_8planet_a_fbr.png", "start": 38852903, "end": 38854525}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_8planet_b_fbr.png", "start": 38854525, "end": 38855652}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_8planet_c_fbr.png", "start": 38855652, "end": 38856741}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_9planet_a_fbr.png", "start": 38856741, "end": 38858325}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_9planet_b_fbr.png", "start": 38858325, "end": 38859413}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_9planet_c_fbr.png", "start": 38859413, "end": 38860489}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_A_tscrn0.png", "start": 38860489, "end": 38862112}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_A_tscrn1.png", "start": 38862112, "end": 38864621}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_A_tscrn2.png", "start": 38864621, "end": 38866221}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_abasebtn.png", "start": 38866221, "end": 38867510}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_abasebtn1b.png", "start": 38867510, "end": 38868799}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_abasebtn2.png", "start": 38868799, "end": 38869170}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_abasebtn2b.png", "start": 38869170, "end": 38869541}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_abasebtnb.png", "start": 38869541, "end": 38869912}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_abutton3_fbr.png", "start": 38869912, "end": 38871800}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_alit8s_fbr.png", "start": 38871800, "end": 38872097}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_atek_jump1_fbr.png", "start": 38872097, "end": 38874590}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_atlight1_fbr.png", "start": 38874590, "end": 38875180}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_atlight2_fbr.png", "start": 38875180, "end": 38875753}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_atlight3_fbr.png", "start": 38875753, "end": 38876355}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_1.png", "start": 38876355, "end": 38881920}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_2.png", "start": 38881920, "end": 38887774}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_3.png", "start": 38887774, "end": 38893520}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_4.png", "start": 38893520, "end": 38896086}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_4b_l.png", "start": 38896086, "end": 38899046}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_5.png", "start": 38899046, "end": 38901918}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_flat.png", "start": 38901918, "end": 38907869}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_lit.png", "start": 38907869, "end": 38908693}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_1.png", "start": 38908693, "end": 38914156}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_2.png", "start": 38914156, "end": 38919998}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_3.png", "start": 38919998, "end": 38925985}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_4.png", "start": 38925985, "end": 38928747}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_5.png", "start": 38928747, "end": 38931796}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_flat.png", "start": 38931796, "end": 38937994}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_lit.png", "start": 38937994, "end": 38938946}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_1.png", "start": 38938946, "end": 38944558}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_2.png", "start": 38944558, "end": 38950688}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_3.png", "start": 38950688, "end": 38956583}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_4.png", "start": 38956583, "end": 38963594}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_trim1.png", "start": 38963594, "end": 38966102}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_trim2.png", "start": 38966102, "end": 38967885}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_trim3.png", "start": 38967885, "end": 38970376}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw39_1_fbr.png", "start": 38970376, "end": 38976321}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/spotlight_fbr.png", "start": 38976321, "end": 38978933}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/star_lasergrid.png", "start": 38978933, "end": 38979313}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_band1a.png", "start": 38979313, "end": 38981817}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_band1b.png", "start": 38981817, "end": 38984425}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok01.png", "start": 38984425, "end": 38987103}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok01a.png", "start": 38987103, "end": 38989875}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok02.png", "start": 38989875, "end": 38995002}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok02a.png", "start": 38995002, "end": 38999861}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok03.png", "start": 38999861, "end": 39002303}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok03a.png", "start": 39002303, "end": 39004158}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok04.png", "start": 39004158, "end": 39007224}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok04h.png", "start": 39007224, "end": 39009948}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok05.png", "start": 39009948, "end": 39014236}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok06.png", "start": 39014236, "end": 39017086}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok06h.png", "start": 39017086, "end": 39019063}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok07.png", "start": 39019063, "end": 39021849}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok07a.png", "start": 39021849, "end": 39024635}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok08.png", "start": 39024635, "end": 39029142}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok09.png", "start": 39029142, "end": 39032462}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok10.png", "start": 39032462, "end": 39038473}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok10b.png", "start": 39038473, "end": 39044181}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok10c.png", "start": 39044181, "end": 39047399}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok11.png", "start": 39047399, "end": 39053340}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok11b.png", "start": 39053340, "end": 39058945}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok12c.png", "start": 39058945, "end": 39062152}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flat01.png", "start": 39062152, "end": 39064586}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flat02.png", "start": 39064586, "end": 39066951}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flat05.png", "start": 39066951, "end": 39075147}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flor1a.png", "start": 39075147, "end": 39078759}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flor1b.png", "start": 39078759, "end": 39082377}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flor2a.png", "start": 39082377, "end": 39084971}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flor2b.png", "start": 39084971, "end": 39086907}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flor2c.png", "start": 39086907, "end": 39088387}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flor2d.png", "start": 39088387, "end": 39088891}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit01_fbr.png", "start": 39088891, "end": 39089071}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit02_fbr.png", "start": 39089071, "end": 39089213}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit03_fbr.png", "start": 39089213, "end": 39089348}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit04_fbr.png", "start": 39089348, "end": 39089480}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit05_fbr.png", "start": 39089480, "end": 39089646}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit06_fbr.png", "start": 39089646, "end": 39089802}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit07_fbr.png", "start": 39089802, "end": 39092105}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit08_fbr.png", "start": 39092105, "end": 39092322}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_metalsheeta.png", "start": 39092322, "end": 39098921}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_metalsheetb.png", "start": 39098921, "end": 39110434}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_0_fbr.png", "start": 39110434, "end": 39111208}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_1_fbr.png", "start": 39111208, "end": 39111970}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_2_fbr.png", "start": 39111970, "end": 39112765}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_3_fbr.png", "start": 39112765, "end": 39113523}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_4_fbr.png", "start": 39113523, "end": 39114328}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_5_fbr.png", "start": 39114328, "end": 39115126}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_6_fbr.png", "start": 39115126, "end": 39115923}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_7_fbr.png", "start": 39115923, "end": 39116697}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_8_fbr.png", "start": 39116697, "end": 39117477}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_9_fbr.png", "start": 39117477, "end": 39118270}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_x.png", "start": 39118270, "end": 39119078}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_rivs01.png", "start": 39119078, "end": 39121506}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_rivs01a.png", "start": 39121506, "end": 39123960}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_sign1.png", "start": 39123960, "end": 39126910}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tech01.png", "start": 39126910, "end": 39130961}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tech02.png", "start": 39130961, "end": 39134064}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tech03.png", "start": 39134064, "end": 39150055}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tech04.png", "start": 39150055, "end": 39152910}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tech05.png", "start": 39152910, "end": 39155689}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tech06.png", "start": 39155689, "end": 39158501}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1a.png", "start": 39158501, "end": 39160989}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1aa.png", "start": 39160989, "end": 39163624}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1b.png", "start": 39163624, "end": 39166355}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1ba.png", "start": 39166355, "end": 39168979}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1c.png", "start": 39168979, "end": 39171764}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1ca.png", "start": 39171764, "end": 39174590}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1d.png", "start": 39174590, "end": 39176890}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1e.png", "start": 39176890, "end": 39179035}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2a.png", "start": 39179035, "end": 39182108}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2aa.png", "start": 39182108, "end": 39184760}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2b.png", "start": 39184760, "end": 39187388}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2ba.png", "start": 39187388, "end": 39190076}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2c.png", "start": 39190076, "end": 39192867}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2ca.png", "start": 39192867, "end": 39195746}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2d.png", "start": 39195746, "end": 39198208}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2e.png", "start": 39198208, "end": 39200715}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tris02.png", "start": 39200715, "end": 39203629}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall05.png", "start": 39203629, "end": 39206955}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall1a.png", "start": 39206955, "end": 39217482}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall1aa.png", "start": 39217482, "end": 39227803}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall1b.png", "start": 39227803, "end": 39238903}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall1ba.png", "start": 39238903, "end": 39249742}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall2a.png", "start": 39249742, "end": 39260526}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall2aa.png", "start": 39260526, "end": 39271546}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall2ab.png", "start": 39271546, "end": 39283303}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall2b.png", "start": 39283303, "end": 39297541}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall2ba.png", "start": 39297541, "end": 39310992}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall3a.png", "start": 39310992, "end": 39318959}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall3aa.png", "start": 39318959, "end": 39327226}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall3b.png", "start": 39327226, "end": 39335548}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall3ba.png", "start": 39335548, "end": 39344511}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall6a.png", "start": 39344511, "end": 39347745}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall6b.png", "start": 39347745, "end": 39350759}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall6c.png", "start": 39350759, "end": 39354435}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall6d.png", "start": 39354435, "end": 39358138}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall6e.png", "start": 39358138, "end": 39361934}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall7a.png", "start": 39361934, "end": 39372826}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall7b.png", "start": 39372826, "end": 39380267}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wire01.png", "start": 39380267, "end": 39383203}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wire02.png", "start": 39383203, "end": 39386738}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wire03.png", "start": 39386738, "end": 39390338}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tech04_1.png", "start": 39390338, "end": 39391161}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tech04_3.png", "start": 39391161, "end": 39392630}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tech08_1.png", "start": 39392630, "end": 39403683}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tech08_2.png", "start": 39403683, "end": 39414736}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tech10_3.png", "start": 39414736, "end": 39418592}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tech14-1.png", "start": 39418592, "end": 39428863}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/techbasetextures.txt", "start": 39428863, "end": 39429373}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/techeye1_fbr.png", "start": 39429373, "end": 39432717}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/techeye2_fbr.png", "start": 39432717, "end": 39436119}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_door1.png", "start": 39436119, "end": 39448265}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_door2.png", "start": 39448265, "end": 39460316}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_flr3.png", "start": 39460316, "end": 39463697}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_grate.png", "start": 39463697, "end": 39466575}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_lit1_fbr.png", "start": 39466575, "end": 39468210}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_lit2_fbr.png", "start": 39468210, "end": 39469196}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_lit3_fbr.png", "start": 39469196, "end": 39471038}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_lit4_fbr.png", "start": 39471038, "end": 39472165}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_pip1_fbr.png", "start": 39472165, "end": 39475247}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_pipe1.png", "start": 39475247, "end": 39478047}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_pipe2.png", "start": 39478047, "end": 39479706}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_trm1.png", "start": 39479706, "end": 39482164}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_trm3.png", "start": 39482164, "end": 39484774}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_wall4_1.png", "start": 39484774, "end": 39499219}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tele_frame1.png", "start": 39499219, "end": 39505374}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tele_frame2.png", "start": 39505374, "end": 39507325}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tele_frame3.png", "start": 39507325, "end": 39511312}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/telepad1_fbr.png", "start": 39511312, "end": 39513288}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlight11_fbr.png", "start": 39513288, "end": 39514914}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlight12_fbr.png", "start": 39514914, "end": 39516609}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlight13_fbr.png", "start": 39516609, "end": 39518668}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlightblfb_fbr.png", "start": 39518668, "end": 39519241}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlightfb_fbr.png", "start": 39519241, "end": 39519831}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlightnb.png", "start": 39519831, "end": 39520429}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlightrdfb_fbr.png", "start": 39520429, "end": 39521031}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/treadplatemetal.png", "start": 39521031, "end": 39535785}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/twall2_3.png", "start": 39535785, "end": 39539293}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/w17_1.png", "start": 39539293, "end": 39556317}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/w94_1.png", "start": 39556317, "end": 39569265}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/z_exit_fbr.png", "start": 39569265, "end": 39570826}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/afloor1_3.png", "start": 39570826, "end": 39573824}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/asphalt.png", "start": 39573824, "end": 39591284}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/azfloor1_1.png", "start": 39591284, "end": 39594303}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/badlawn.png", "start": 39594303, "end": 39636112}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/cracks1-1.png", "start": 39636112, "end": 39639131}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/darkrock.png", "start": 39639131, "end": 39661700}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/grass1.png", "start": 39661700, "end": 39672271}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/gravel1.png", "start": 39672271, "end": 39685625}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/gravel2.png", "start": 39685625, "end": 39700522}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/grk_leaf1_1.png", "start": 39700522, "end": 39704515}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/grk_leaf1_2.png", "start": 39704515, "end": 39708252}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/marbbrn128.png", "start": 39708252, "end": 39718494}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_drt1_1.png", "start": 39718494, "end": 39721390}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_drt1_2.png", "start": 39721390, "end": 39724244}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_drt2_2.png", "start": 39724244, "end": 39726988}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_rck1_1.png", "start": 39726988, "end": 39741088}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_rck1_2.png", "start": 39741088, "end": 39751766}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_rck1_3.png", "start": 39751766, "end": 39764609}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_slat1_1.png", "start": 39764609, "end": 39767457}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_bigdirt.png", "start": 39767457, "end": 39922040}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_bigdirt2.png", "start": 39922040, "end": 40076191}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_bigdirt3.png", "start": 40076191, "end": 40230142}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn1_1.png", "start": 40230142, "end": 40244523}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn1_1a.png", "start": 40244523, "end": 40261497}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn1_2.png", "start": 40261497, "end": 40275936}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn1_2a.png", "start": 40275936, "end": 40292751}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn2_1.png", "start": 40292751, "end": 40306720}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn2_1a.png", "start": 40306720, "end": 40324031}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn2_2.png", "start": 40324031, "end": 40339105}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn2_2a.png", "start": 40339105, "end": 40356209}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cracks1.png", "start": 40356209, "end": 40369671}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat1.png", "start": 40369671, "end": 40382580}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat12.png", "start": 40382580, "end": 40393968}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat15.png", "start": 40393968, "end": 40407069}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat16.png", "start": 40407069, "end": 40419911}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat2.png", "start": 40419911, "end": 40431188}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat3.png", "start": 40431188, "end": 40444553}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat4.png", "start": 40444553, "end": 40454502}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat5.png", "start": 40454502, "end": 40465587}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat5a.png", "start": 40465587, "end": 40476331}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat6.png", "start": 40476331, "end": 40485316}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat7.png", "start": 40485316, "end": 40496774}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_plaster2.png", "start": 40496774, "end": 40506426}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock1.png", "start": 40506426, "end": 40557683}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock10.png", "start": 40557683, "end": 40566978}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock10a.png", "start": 40566978, "end": 40576292}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock10b.png", "start": 40576292, "end": 40589657}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock10c.png", "start": 40589657, "end": 40601992}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock2.png", "start": 40601992, "end": 40643869}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock3.png", "start": 40643869, "end": 40689813}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock3_bump.png", "start": 40689813, "end": 40805907}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock4.png", "start": 40805907, "end": 40817263}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock5.png", "start": 40817263, "end": 40829795}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock9.png", "start": 40829795, "end": 40840819}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/ret_plaster1.png", "start": 40840819, "end": 40852096}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/rock1_1.png", "start": 40852096, "end": 40902467}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/rock1_1b.png", "start": 40902467, "end": 40913482}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/rock1_2.png", "start": 40913482, "end": 40955525}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/rocks07.png", "start": 40955525, "end": 40966549}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/rocks11d.png", "start": 40966549, "end": 40975863}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/rocks11e.png", "start": 40975863, "end": 40985158}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/sand.png", "start": 40985158, "end": 41011245}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/snow1.png", "start": 41011245, "end": 41013108}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/uwall1_2.png", "start": 41013108, "end": 41040759}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/vines1.png", "start": 41040759, "end": 41045814}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/black.png", "start": 41045814, "end": 41046362}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/clip.png", "start": 41046362, "end": 41046868}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/hint.png", "start": 41046868, "end": 41047734}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/hintskip.png", "start": 41047734, "end": 41048627}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/light_fbr.png", "start": 41048627, "end": 41049543}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/origin.png", "start": 41049543, "end": 41050026}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/skip.png", "start": 41050026, "end": 41050514}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/star_lavaskip.png", "start": 41050514, "end": 41051627}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/star_slimeskip.png", "start": 41051627, "end": 41052700}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/star_waterskip.png", "start": 41052700, "end": 41054440}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/trigger.png", "start": 41054440, "end": 41054937}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/crate4.png", "start": 41054937, "end": 41058079}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/crwdh6.png", "start": 41058079, "end": 41063087}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/crwdl12.png", "start": 41063087, "end": 41065520}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/crwds6.png", "start": 41065520, "end": 41066546}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/may_crate3-small.png", "start": 41066546, "end": 41067537}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/may_crate3.png", "start": 41067537, "end": 41070600}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/may_wood1_1.png", "start": 41070600, "end": 41073440}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/may_wood1_2.png", "start": 41073440, "end": 41076289}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_ret_wood1.png", "start": 41076289, "end": 41083258}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood1.png", "start": 41083258, "end": 41110626}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood2.png", "start": 41110626, "end": 41118139}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood2_plk1.png", "start": 41118139, "end": 41129837}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood2_plk2.png", "start": 41129837, "end": 41142329}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood3.png", "start": 41142329, "end": 41148329}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood3_plk1.png", "start": 41148329, "end": 41161544}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood4.png", "start": 41161544, "end": 41168668}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood5.png", "start": 41168668, "end": 41176466}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood6.png", "start": 41176466, "end": 41183246}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood7.png", "start": 41183246, "end": 41192285}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood8.png", "start": 41192285, "end": 41200249}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood_riv1.png", "start": 41200249, "end": 41206874}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood_riv1b.png", "start": 41206874, "end": 41212953}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood_riv1c.png", "start": 41212953, "end": 41218077}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood_riv2.png", "start": 41218077, "end": 41224487}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood_riv2b.png", "start": 41224487, "end": 41230907}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood_riv2c.png", "start": 41230907, "end": 41236279}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank1.png", "start": 41236279, "end": 41242693}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank1s.png", "start": 41242693, "end": 41244574}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank2.png", "start": 41244574, "end": 41251154}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank2s.png", "start": 41251154, "end": 41253043}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank3.png", "start": 41253043, "end": 41259903}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank3s.png", "start": 41259903, "end": 41261884}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank4.png", "start": 41261884, "end": 41268330}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank4s.png", "start": 41268330, "end": 41270220}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank5.png", "start": 41270220, "end": 41275869}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/sq_wood_1.png", "start": 41275869, "end": 41291910}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/sq_wood_2.png", "start": 41291910, "end": 41294798}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/sq_wood_2a.png", "start": 41294798, "end": 41298167}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/wood_1.png", "start": 41298167, "end": 41302345}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/wood_2.png", "start": 41302345, "end": 41306245}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodbark128.png", "start": 41306245, "end": 41316452}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodbark1m28.png", "start": 41316452, "end": 41325901}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodbark64.png", "start": 41325901, "end": 41328838}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodbarkA128.png", "start": 41328838, "end": 41337536}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodbarkm64.png", "start": 41337536, "end": 41340213}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodend.png", "start": 41340213, "end": 41342716}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodring128.png", "start": 41342716, "end": 41351140}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodring64.png", "start": 41351140, "end": 41353601}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodringm128.png", "start": 41353601, "end": 41363900}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodringm64.png", "start": 41363900, "end": 41367159}, {"filename": "/GameData/Textures/brushes/light.png", "start": 41367159, "end": 41367287}, {"filename": "/GameData/Textures/brushes/light_em.png", "start": 41367287, "end": 41367415}, {"filename": "/GameData/Textures/brushes/mask_test_m.png", "start": 41367415, "end": 41371477}, {"filename": "/GameData/Textures/brushes/mirror.png", "start": 41371477, "end": 41371597}, {"filename": "/GameData/Textures/brushes/mirror_orm.png", "start": 41371597, "end": 41371717}, {"filename": "/GameData/Textures/brushes/null_m.png", "start": 41371717, "end": 41373514}, {"filename": "/GameData/Textures/brushes/tormentPack/+0str_bloodfall.png", "start": 41373514, "end": 41375591}, {"filename": "/GameData/Textures/brushes/tormentPack/+1str_bloodfall.png", "start": 41375591, "end": 41377625}, {"filename": "/GameData/Textures/brushes/tormentPack/+2str_bloodfall.png", "start": 41377625, "end": 41379723}, {"filename": "/GameData/Textures/brushes/tormentPack/+3str_bloodfall.png", "start": 41379723, "end": 41381794}, {"filename": "/GameData/Textures/brushes/tormentPack/+4str_bloodfall.png", "start": 41381794, "end": 41383854}, {"filename": "/GameData/Textures/brushes/tormentPack/+5str_bloodfall.png", "start": 41383854, "end": 41385875}, {"filename": "/GameData/Textures/brushes/tormentPack/+6str_bloodfall.png", "start": 41385875, "end": 41387921}, {"filename": "/GameData/Textures/brushes/tormentPack/+7str_bloodfall.png", "start": 41387921, "end": 41389990}, {"filename": "/GameData/Textures/brushes/tormentPack/str_blood.png", "start": 41389990, "end": 41391998}, {"filename": "/GameData/Textures/brushes/tormentPack/str_blood_large.png", "start": 41391998, "end": 41414548}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein1.png", "start": 41414548, "end": 41441223}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein2.png", "start": 41441223, "end": 41473256}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein3.png", "start": 41473256, "end": 41506927}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein4.png", "start": 41506927, "end": 41538569}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein5.png", "start": 41538569, "end": 41572060}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein6.png", "start": 41572060, "end": 41606534}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein7.png", "start": 41606534, "end": 41641425}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein8.png", "start": 41641425, "end": 41669574}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein9.png", "start": 41669574, "end": 41705027}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr1.png", "start": 41705027, "end": 41741721}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr2.png", "start": 41741721, "end": 41778296}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr3.png", "start": 41778296, "end": 41809289}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr4.png", "start": 41809289, "end": 41839459}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr5.png", "start": 41839459, "end": 41878909}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr6.png", "start": 41878909, "end": 41918069}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr7.png", "start": 41918069, "end": 41959761}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr8.png", "start": 41959761, "end": 42000179}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgen1.png", "start": 42000179, "end": 42029449}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgen2.png", "start": 42029449, "end": 42058965}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgen3.png", "start": 42058965, "end": 42088441}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgen4.png", "start": 42088441, "end": 42113264}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgen5.png", "start": 42113264, "end": 42138134}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgen6.png", "start": 42138134, "end": 42162224}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl1.png", "start": 42162224, "end": 42191609}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl2.png", "start": 42191609, "end": 42224339}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl3.png", "start": 42224339, "end": 42250957}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl4.png", "start": 42250957, "end": 42280221}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl5.png", "start": 42280221, "end": 42314596}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl6.png", "start": 42314596, "end": 42353091}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl7.png", "start": 42353091, "end": 42389241}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl8.png", "start": 42389241, "end": 42427234}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan1.png", "start": 42427234, "end": 42462094}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan2.png", "start": 42462094, "end": 42497635}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan3.png", "start": 42497635, "end": 42528071}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan4.png", "start": 42528071, "end": 42559031}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan5.png", "start": 42559031, "end": 42595403}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan6.png", "start": 42595403, "end": 42633445}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan7.png", "start": 42633445, "end": 42672674}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan8.png", "start": 42672674, "end": 42711768}, {"filename": "/GameData/Textures/brushes/tormentPack/str_rotwoodgen1.png", "start": 42711768, "end": 42730347}, {"filename": "/GameData/Textures/brushes/tormentPack/str_rotwoodgen2.png", "start": 42730347, "end": 42749643}, {"filename": "/GameData/Textures/brushes/tormentPack/str_rotwoodgen3.png", "start": 42749643, "end": 42769697}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk1.png", "start": 42769697, "end": 42795089}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk2.png", "start": 42795089, "end": 42822091}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk3.png", "start": 42822091, "end": 42844721}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk4.png", "start": 42844721, "end": 42868774}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk5.png", "start": 42868774, "end": 42899858}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk6.png", "start": 42899858, "end": 42932530}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk7.png", "start": 42932530, "end": 42961054}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk8.png", "start": 42961054, "end": 42991306}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stoneflr1.png", "start": 42991306, "end": 43015462}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stoneflr2.png", "start": 43015462, "end": 43043368}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stoneflr3.png", "start": 43043368, "end": 43064373}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stoneflr4.png", "start": 43064373, "end": 43085517}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stoneflr5.png", "start": 43085517, "end": 43117049}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stoneflr6.png", "start": 43117049, "end": 43151332}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonegen1.png", "start": 43151332, "end": 43170355}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonegen2.png", "start": 43170355, "end": 43189726}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonegen3.png", "start": 43189726, "end": 43211498}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonegen4.png", "start": 43211498, "end": 43236543}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonegen5.png", "start": 43236543, "end": 43261931}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonegen6.png", "start": 43261931, "end": 43289368}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonerubble.png", "start": 43289368, "end": 43317762}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall1.png", "start": 43317762, "end": 43341019}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall2.png", "start": 43341019, "end": 43365824}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall3.png", "start": 43365824, "end": 43390487}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall4.png", "start": 43390487, "end": 43416957}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall5.png", "start": 43416957, "end": 43444424}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall6.png", "start": 43444424, "end": 43472592}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall7.png", "start": 43472592, "end": 43501981}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall8.png", "start": 43501981, "end": 43532877}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_bloodgunk.png", "start": 43532877, "end": 43559326}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_bloodweb1.png", "start": 43559326, "end": 43576566}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_bloodweb2.png", "start": 43576566, "end": 43585517}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_bloodweb3.png", "start": 43585517, "end": 43591528}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_grating1.png", "start": 43591528, "end": 43600109}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_grating2.png", "start": 43600109, "end": 43615312}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_grating3.png", "start": 43615312, "end": 43634660}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_grating4.png", "start": 43634660, "end": 43646817}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_grating5.png", "start": 43646817, "end": 43668322}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_grating6.png", "start": 43668322, "end": 43696568}, {"filename": "/GameData/Textures/brushes/trigger.png", "start": 43696568, "end": 43708444}, {"filename": "/GameData/Textures/brushes/trigger_t.png", "start": 43708444, "end": 43720320}, {"filename": "/GameData/Textures/brushes/white.png", "start": 43720320, "end": 43720440}, {"filename": "/GameData/Textures/gloves.png", "start": 43720440, "end": 43830115}, {"filename": "/GameData/Textures/jacket.png", "start": 43830115, "end": 43993927}, {"filename": "/GameData/Textures/muzzle_t.png", "start": 43993927, "end": 44003335}, {"filename": "/GameData/Textures/particles/blood.png", "start": 44003335, "end": 44007048}, {"filename": "/GameData/Textures/particles/smoke.png", "start": 44007048, "end": 44010918}, {"filename": "/GameData/Textures/particles/trail.png", "start": 44010918, "end": 44033279}, {"filename": "/GameData/Textures/particles/wood.png", "start": 44033279, "end": 44048762}, {"filename": "/GameData/Textures/shirt.png", "start": 44048762, "end": 44240513}, {"filename": "/GameData/arms.glb", "start": 44240513, "end": 44420277}, {"filename": "/GameData/bass_beat.ogg", "start": 44420277, "end": 44787153, "audio": 1}, {"filename": "/GameData/bass_beat.wav", "start": 44787153, "end": 50483487, "audio": 1}, {"filename": "/GameData/cat.png", "start": 50483487, "end": 50732925}, {"filename": "/GameData/cube.mtl", "start": 50732925, "end": 50732976}, {"filename": "/GameData/cube.obj", "start": 50732976, "end": 50733895}, {"filename": "/GameData/dog.bin", "start": 50733895, "end": 50799939}, {"filename": "/GameData/dog.dae", "start": 50799939, "end": 51199011}, {"filename": "/GameData/dog.fbx", "start": 51199011, "end": 52113343}, {"filename": "/GameData/dog.glb", "start": 52113343, "end": 52260831}, {"filename": "/GameData/dog.glb.skmm", "start": 52260831, "end": 52262887}, {"filename": "/GameData/dog.gltf", "start": 52262887, "end": 52354014}, {"filename": "/GameData/happy_hog.png", "start": 52354014, "end": 53271139}, {"filename": "/GameData/hog_sheet.png", "start": 53271139, "end": 55126215}, {"filename": "/GameData/level_1_bg_crop.png", "start": 55126215, "end": 55733394}, {"filename": "/GameData/mini_hog.ico", "start": 55733394, "end": 55737680}, {"filename": "/GameData/monkey.fbx", "start": 55737680, "end": 55769100}, {"filename": "/GameData/sound_test.ogg", "start": 55769100, "end": 55798208, "audio": 1}, {"filename": "/GameData/sprite_sheet_sticher.py", "start": 55798208, "end": 55798734}, {"filename": "/GameData/testViewmodel.glb", "start": 55798734, "end": 56613706}, {"filename": "/GameData/title_bg_crop.png", "start": 56613706, "end": 57528814}, {"filename": "/GameData/yummy.ogg", "start": 57528814, "end": 57564405, "audio": 1}], "remote_package_size": 57564405});

  })();

// end include: C:\Users\Admin\AppData\Local\Temp\tmpa3dy03o8.js
// include: C:\Users\Admin\AppData\Local\Temp\tmpxhom1hl1.js

    // All the pre-js content up to here must remain later on, we need to run
    // it.
    if (Module['$ww'] || (typeof ENVIRONMENT_IS_PTHREAD != 'undefined' && ENVIRONMENT_IS_PTHREAD)) Module['preRun'] = [];
    var necessaryPreJSTasks = Module['preRun'].slice();
  // end include: C:\Users\Admin\AppData\Local\Temp\tmpxhom1hl1.js
// include: C:\Users\Admin\AppData\Local\Temp\tmpe343aa3a.js

    if (!Module['preRun']) throw 'Module.preRun should exist because file support used it; did a pre-js delete it?';
    necessaryPreJSTasks.forEach((task) => {
      if (Module['preRun'].indexOf(task) < 0) throw 'All preRun tasks that exist before user pre-js code should remain after; did you replace Module or modify Module.preRun?';
    });
  // end include: C:\Users\Admin\AppData\Local\Temp\tmpe343aa3a.js


// Sometimes an existing Module object exists with properties
// meant to overwrite the default module functionality. Here
// we collect those properties and reapply _after_ we configure
// the current environment's defaults to avoid having to be so
// defensive during initialization.
var moduleOverrides = {...Module};

var arguments_ = [];
var thisProgram = './this.program';
var quit_ = (status, toThrow) => {
  throw toThrow;
};

// `/` should be present at the end if `scriptDirectory` is not empty
var scriptDirectory = '';
function locateFile(path) {
  if (Module['locateFile']) {
    return Module['locateFile'](path, scriptDirectory);
  }
  return scriptDirectory + path;
}

// Hooks that are implemented differently in different runtime environments.
var readAsync, readBinary;

if (ENVIRONMENT_IS_NODE) {
  if (typeof process == 'undefined' || !process.release || process.release.name !== 'node') throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');

  var nodeVersion = process.versions.node;
  var numericVersion = nodeVersion.split('.').slice(0, 3);
  numericVersion = (numericVersion[0] * 10000) + (numericVersion[1] * 100) + (numericVersion[2].split('-')[0] * 1);
  var minVersion = 160000;
  if (numericVersion < 160000) {
    throw new Error('This emscripten-generated code requires node v16.0.0 (detected v' + nodeVersion + ')');
  }

  // These modules will usually be used on Node.js. Load them eagerly to avoid
  // the complexity of lazy-loading.
  var fs = require('fs');
  var nodePath = require('path');

  scriptDirectory = __dirname + '/';

// include: node_shell_read.js
readBinary = (filename) => {
  // We need to re-wrap `file://` strings to URLs.
  filename = isFileURI(filename) ? new URL(filename) : filename;
  var ret = fs.readFileSync(filename);
  assert(Buffer.isBuffer(ret));
  return ret;
};

readAsync = async (filename, binary = true) => {
  // See the comment in the `readBinary` function.
  filename = isFileURI(filename) ? new URL(filename) : filename;
  var ret = fs.readFileSync(filename, binary ? undefined : 'utf8');
  assert(binary ? Buffer.isBuffer(ret) : typeof ret == 'string');
  return ret;
};
// end include: node_shell_read.js
  if (!Module['thisProgram'] && process.argv.length > 1) {
    thisProgram = process.argv[1].replace(/\\/g, '/');
  }

  arguments_ = process.argv.slice(2);

  if (typeof module != 'undefined') {
    module['exports'] = Module;
  }

  quit_ = (status, toThrow) => {
    process.exitCode = status;
    throw toThrow;
  };

} else
if (ENVIRONMENT_IS_SHELL) {

  if ((typeof process == 'object' && typeof require === 'function') || typeof window == 'object' || typeof WorkerGlobalScope != 'undefined') throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');

} else

// Note that this includes Node.js workers when relevant (pthreads is enabled).
// Node.js workers are detected as a combination of ENVIRONMENT_IS_WORKER and
// ENVIRONMENT_IS_NODE.
if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
  if (ENVIRONMENT_IS_WORKER) { // Check worker, not web, since window could be polyfilled
    scriptDirectory = self.location.href;
  } else if (typeof document != 'undefined' && document.currentScript) { // web
    scriptDirectory = document.currentScript.src;
  }
  // blob urls look like blob:http://site.com/etc/etc and we cannot infer anything from them.
  // otherwise, slice off the final part of the url to find the script directory.
  // if scriptDirectory does not contain a slash, lastIndexOf will return -1,
  // and scriptDirectory will correctly be replaced with an empty string.
  // If scriptDirectory contains a query (starting with ?) or a fragment (starting with #),
  // they are removed because they could contain a slash.
  if (scriptDirectory.startsWith('blob:')) {
    scriptDirectory = '';
  } else {
    scriptDirectory = scriptDirectory.slice(0, scriptDirectory.replace(/[?#].*/, '').lastIndexOf('/')+1);
  }

  if (!(typeof window == 'object' || typeof WorkerGlobalScope != 'undefined')) throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');

  {
// include: web_or_worker_shell_read.js
if (ENVIRONMENT_IS_WORKER) {
    readBinary = (url) => {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', url, false);
      xhr.responseType = 'arraybuffer';
      xhr.send(null);
      return new Uint8Array(/** @type{!ArrayBuffer} */(xhr.response));
    };
  }

  readAsync = async (url) => {
    // Fetch has some additional restrictions over XHR, like it can't be used on a file:// url.
    // See https://github.com/github/fetch/pull/92#issuecomment-140665932
    // Cordova or Electron apps are typically loaded from a file:// url.
    // So use XHR on webview if URL is a file URL.
    if (isFileURI(url)) {
      return new Promise((resolve, reject) => {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', url, true);
        xhr.responseType = 'arraybuffer';
        xhr.onload = () => {
          if (xhr.status == 200 || (xhr.status == 0 && xhr.response)) { // file URLs can return 0
            resolve(xhr.response);
            return;
          }
          reject(xhr.status);
        };
        xhr.onerror = reject;
        xhr.send(null);
      });
    }
    var response = await fetch(url, { credentials: 'same-origin' });
    if (response.ok) {
      return response.arrayBuffer();
    }
    throw new Error(response.status + ' : ' + response.url);
  };
// end include: web_or_worker_shell_read.js
  }
} else
{
  throw new Error('environment detection error');
}

var out = Module['print'] || console.log.bind(console);
var err = Module['printErr'] || console.error.bind(console);

// Merge back in the overrides
Object.assign(Module, moduleOverrides);
// Free the object hierarchy contained in the overrides, this lets the GC
// reclaim data used.
moduleOverrides = null;
checkIncomingModuleAPI();

// Emit code to handle expected values on the Module object. This applies Module.x
// to the proper local x. This has two benefits: first, we only emit it if it is
// expected to arrive, and second, by using a local everywhere else that can be
// minified.

if (Module['arguments']) arguments_ = Module['arguments'];legacyModuleProp('arguments', 'arguments_');

if (Module['thisProgram']) thisProgram = Module['thisProgram'];legacyModuleProp('thisProgram', 'thisProgram');

// perform assertions in shell.js after we set up out() and err(), as otherwise if an assertion fails it cannot print the message
// Assertions on removed incoming Module JS APIs.
assert(typeof Module['memoryInitializerPrefixURL'] == 'undefined', 'Module.memoryInitializerPrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['pthreadMainPrefixURL'] == 'undefined', 'Module.pthreadMainPrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['cdInitializerPrefixURL'] == 'undefined', 'Module.cdInitializerPrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['filePackagePrefixURL'] == 'undefined', 'Module.filePackagePrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['read'] == 'undefined', 'Module.read option was removed');
assert(typeof Module['readAsync'] == 'undefined', 'Module.readAsync option was removed (modify readAsync in JS)');
assert(typeof Module['readBinary'] == 'undefined', 'Module.readBinary option was removed (modify readBinary in JS)');
assert(typeof Module['setWindowTitle'] == 'undefined', 'Module.setWindowTitle option was removed (modify emscripten_set_window_title in JS)');
assert(typeof Module['TOTAL_MEMORY'] == 'undefined', 'Module.TOTAL_MEMORY has been renamed Module.INITIAL_MEMORY');
legacyModuleProp('asm', 'wasmExports');
legacyModuleProp('readAsync', 'readAsync');
legacyModuleProp('readBinary', 'readBinary');
legacyModuleProp('setWindowTitle', 'setWindowTitle');
var IDBFS = 'IDBFS is no longer included by default; build with -lidbfs.js';
var PROXYFS = 'PROXYFS is no longer included by default; build with -lproxyfs.js';
var WORKERFS = 'WORKERFS is no longer included by default; build with -lworkerfs.js';
var FETCHFS = 'FETCHFS is no longer included by default; build with -lfetchfs.js';
var ICASEFS = 'ICASEFS is no longer included by default; build with -licasefs.js';
var JSFILEFS = 'JSFILEFS is no longer included by default; build with -ljsfilefs.js';
var OPFS = 'OPFS is no longer included by default; build with -lopfs.js';

var NODEFS = 'NODEFS is no longer included by default; build with -lnodefs.js';

assert(!ENVIRONMENT_IS_SHELL, 'shell environment detected but not enabled at build time.  Add `shell` to `-sENVIRONMENT` to enable.');

// end include: shell.js

// include: preamble.js
// === Preamble library stuff ===

// Documentation for the public APIs defined in this file must be updated in:
//    site/source/docs/api_reference/preamble.js.rst
// A prebuilt local version of the documentation is available at:
//    site/build/text/docs/api_reference/preamble.js.txt
// You can also build docs locally as HTML or other formats in site/
// An online HTML version (which may be of a different version of Emscripten)
//    is up at http://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html

var wasmBinary = Module['wasmBinary'];legacyModuleProp('wasmBinary', 'wasmBinary');

if (typeof WebAssembly != 'object') {
  err('no native wasm support detected');
}

// Wasm globals

var wasmMemory;

//========================================
// Runtime essentials
//========================================

// whether we are quitting the application. no code should run after this.
// set in exit() and abort()
var ABORT = false;

// set by exit() and abort().  Passed to 'onExit' handler.
// NOTE: This is also used as the process return code code in shell environments
// but only when noExitRuntime is false.
var EXITSTATUS;

// In STRICT mode, we only define assert() when ASSERTIONS is set.  i.e. we
// don't define it at all in release modes.  This matches the behaviour of
// MINIMAL_RUNTIME.
// TODO(sbc): Make this the default even without STRICT enabled.
/** @type {function(*, string=)} */
function assert(condition, text) {
  if (!condition) {
    abort('Assertion failed' + (text ? ': ' + text : ''));
  }
}

// We used to include malloc/free by default in the past. Show a helpful error in
// builds with assertions.

// Memory management

var HEAP,
/** @type {!Int8Array} */
  HEAP8,
/** @type {!Uint8Array} */
  HEAPU8,
/** @type {!Int16Array} */
  HEAP16,
/** @type {!Uint16Array} */
  HEAPU16,
/** @type {!Int32Array} */
  HEAP32,
/** @type {!Uint32Array} */
  HEAPU32,
/** @type {!Float32Array} */
  HEAPF32,
/* BigInt64Array type is not correctly defined in closure
/** not-@type {!BigInt64Array} */
  HEAP64,
/* BigUint64Array type is not correctly defined in closure
/** not-t@type {!BigUint64Array} */
  HEAPU64,
/** @type {!Float64Array} */
  HEAPF64;

var runtimeInitialized = false;

/**
 * Indicates whether filename is delivered via file protocol (as opposed to http/https)
 * @noinline
 */
var isFileURI = (filename) => filename.startsWith('file://');

// include: runtime_shared.js
// include: runtime_stack_check.js
// Initializes the stack cookie. Called at the startup of main and at the startup of each thread in pthreads mode.
function writeStackCookie() {
  var max = _emscripten_stack_get_end();
  assert((max & 3) == 0);
  // If the stack ends at address zero we write our cookies 4 bytes into the
  // stack.  This prevents interference with SAFE_HEAP and ASAN which also
  // monitor writes to address zero.
  if (max == 0) {
    max += 4;
  }
  // The stack grow downwards towards _emscripten_stack_get_end.
  // We write cookies to the final two words in the stack and detect if they are
  // ever overwritten.
  HEAPU32[((max)>>2)] = 0x02135467;
  HEAPU32[(((max)+(4))>>2)] = 0x89BACDFE;
  // Also test the global address 0 for integrity.
  HEAPU32[((0)>>2)] = 1668509029;
}

function checkStackCookie() {
  if (ABORT) return;
  var max = _emscripten_stack_get_end();
  // See writeStackCookie().
  if (max == 0) {
    max += 4;
  }
  var cookie1 = HEAPU32[((max)>>2)];
  var cookie2 = HEAPU32[(((max)+(4))>>2)];
  if (cookie1 != 0x02135467 || cookie2 != 0x89BACDFE) {
    abort(`Stack overflow! Stack cookie has been overwritten at ${ptrToString(max)}, expected hex dwords 0x89BACDFE and 0x2135467, but received ${ptrToString(cookie2)} ${ptrToString(cookie1)}`);
  }
  // Also test the global address 0 for integrity.
  if (HEAPU32[((0)>>2)] != 0x63736d65 /* 'emsc' */) {
    abort('Runtime error: The application has corrupted its heap memory area (address zero)!');
  }
}
// end include: runtime_stack_check.js
// include: runtime_exceptions.js
// Base Emscripten EH error class
class EmscriptenEH extends Error {}

class EmscriptenSjLj extends EmscriptenEH {}

class CppException extends EmscriptenEH {
  constructor(excPtr) {
    super(excPtr);
    this.excPtr = excPtr;
    const excInfo = getExceptionMessage(excPtr);
    this.name = excInfo[0];
    this.message = excInfo[1];
  }
}
// end include: runtime_exceptions.js
// include: runtime_debug.js
// Endianness check
(() => {
  var h16 = new Int16Array(1);
  var h8 = new Int8Array(h16.buffer);
  h16[0] = 0x6373;
  if (h8[0] !== 0x73 || h8[1] !== 0x63) throw 'Runtime error: expected the system to be little-endian! (Run with -sSUPPORT_BIG_ENDIAN to bypass)';
})();

if (Module['ENVIRONMENT']) {
  throw new Error('Module.ENVIRONMENT has been deprecated. To force the environment, use the ENVIRONMENT compile-time option (for example, -sENVIRONMENT=web or -sENVIRONMENT=node)');
}

function legacyModuleProp(prop, newName, incoming=true) {
  if (!Object.getOwnPropertyDescriptor(Module, prop)) {
    Object.defineProperty(Module, prop, {
      configurable: true,
      get() {
        let extra = incoming ? ' (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)' : '';
        abort(`\`Module.${prop}\` has been replaced by \`${newName}\`` + extra);

      }
    });
  }
}

function consumedModuleProp(prop) {
  if (!Object.getOwnPropertyDescriptor(Module, prop)) {
    Object.defineProperty(Module, prop, {
      configurable: true,
      set() {
        abort(`Attempt to set \`Module.${prop}\` after it has already been processed.  This can happen, for example, when code is injected via '--post-js' rather than '--pre-js'`);

      }
    });
  }
}

function ignoredModuleProp(prop) {
  if (Object.getOwnPropertyDescriptor(Module, prop)) {
    abort(`\`Module.${prop}\` was supplied but \`${prop}\` not included in INCOMING_MODULE_JS_API`);
  }
}

// forcing the filesystem exports a few things by default
function isExportedByForceFilesystem(name) {
  return name === 'FS_createPath' ||
         name === 'FS_createDataFile' ||
         name === 'FS_createPreloadedFile' ||
         name === 'FS_unlink' ||
         name === 'addRunDependency' ||
         // The old FS has some functionality that WasmFS lacks.
         name === 'FS_createLazyFile' ||
         name === 'FS_createDevice' ||
         name === 'removeRunDependency';
}

/**
 * Intercept access to a global symbol.  This enables us to give informative
 * warnings/errors when folks attempt to use symbols they did not include in
 * their build, or no symbols that no longer exist.
 */
function hookGlobalSymbolAccess(sym, func) {
  if (typeof globalThis != 'undefined' && !Object.getOwnPropertyDescriptor(globalThis, sym)) {
    Object.defineProperty(globalThis, sym, {
      configurable: true,
      get() {
        func();
        return undefined;
      }
    });
  }
}

function missingGlobal(sym, msg) {
  hookGlobalSymbolAccess(sym, () => {
    warnOnce(`\`${sym}\` is not longer defined by emscripten. ${msg}`);
  });
}

missingGlobal('buffer', 'Please use HEAP8.buffer or wasmMemory.buffer');
missingGlobal('asm', 'Please use wasmExports instead');

function missingLibrarySymbol(sym) {
  hookGlobalSymbolAccess(sym, () => {
    // Can't `abort()` here because it would break code that does runtime
    // checks.  e.g. `if (typeof SDL === 'undefined')`.
    var msg = `\`${sym}\` is a library symbol and not included by default; add it to your library.js __deps or to DEFAULT_LIBRARY_FUNCS_TO_INCLUDE on the command line`;
    // DEFAULT_LIBRARY_FUNCS_TO_INCLUDE requires the name as it appears in
    // library.js, which means $name for a JS name with no prefix, or name
    // for a JS name like _name.
    var librarySymbol = sym;
    if (!librarySymbol.startsWith('_')) {
      librarySymbol = '$' + sym;
    }
    msg += ` (e.g. -sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE='${librarySymbol}')`;
    if (isExportedByForceFilesystem(sym)) {
      msg += '. Alternatively, forcing filesystem support (-sFORCE_FILESYSTEM) can export this for you';
    }
    warnOnce(msg);
  });

  // Any symbol that is not included from the JS library is also (by definition)
  // not exported on the Module object.
  unexportedRuntimeSymbol(sym);
}

function unexportedRuntimeSymbol(sym) {
  if (!Object.getOwnPropertyDescriptor(Module, sym)) {
    Object.defineProperty(Module, sym, {
      configurable: true,
      get() {
        var msg = `'${sym}' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the Emscripten FAQ)`;
        if (isExportedByForceFilesystem(sym)) {
          msg += '. Alternatively, forcing filesystem support (-sFORCE_FILESYSTEM) can export this for you';
        }
        abort(msg);
      }
    });
  }
}

var runtimeDebug = true; // Switch to false at runtime to disable logging at the right times

// Used by XXXXX_DEBUG settings to output debug messages.
function dbg(...args) {
  if (!runtimeDebug && typeof runtimeDebug != 'undefined') return;
  // TODO(sbc): Make this configurable somehow.  Its not always convenient for
  // logging to show up as warnings.
  console.warn(...args);
}
// end include: runtime_debug.js
// include: memoryprofiler.js
// end include: memoryprofiler.js


function updateMemoryViews() {
  var b = wasmMemory.buffer;
  Module['HEAP8'] = HEAP8 = new Int8Array(b);
  Module['HEAP16'] = HEAP16 = new Int16Array(b);
  Module['HEAPU8'] = HEAPU8 = new Uint8Array(b);
  Module['HEAPU16'] = HEAPU16 = new Uint16Array(b);
  Module['HEAP32'] = HEAP32 = new Int32Array(b);
  Module['HEAPU32'] = HEAPU32 = new Uint32Array(b);
  Module['HEAPF32'] = HEAPF32 = new Float32Array(b);
  Module['HEAPF64'] = HEAPF64 = new Float64Array(b);
  Module['HEAP64'] = HEAP64 = new BigInt64Array(b);
  Module['HEAPU64'] = HEAPU64 = new BigUint64Array(b);
}

// end include: runtime_shared.js
assert(!Module['STACK_SIZE'], 'STACK_SIZE can no longer be set at runtime.  Use -sSTACK_SIZE at link time')

assert(typeof Int32Array != 'undefined' && typeof Float64Array !== 'undefined' && Int32Array.prototype.subarray != undefined && Int32Array.prototype.set != undefined,
       'JS engine does not provide full typed array support');

// If memory is defined in wasm, the user can't provide it, or set INITIAL_MEMORY
assert(!Module['wasmMemory'], 'Use of `wasmMemory` detected.  Use -sIMPORTED_MEMORY to define wasmMemory externally');
assert(!Module['INITIAL_MEMORY'], 'Detected runtime INITIAL_MEMORY setting.  Use -sIMPORTED_MEMORY to define wasmMemory dynamically');

function preRun() {
  if (Module['preRun']) {
    if (typeof Module['preRun'] == 'function') Module['preRun'] = [Module['preRun']];
    while (Module['preRun'].length) {
      addOnPreRun(Module['preRun'].shift());
    }
  }
  consumedModuleProp('preRun');
  callRuntimeCallbacks(onPreRuns);
}

function initRuntime() {
  assert(!runtimeInitialized);
  runtimeInitialized = true;

  checkStackCookie();

  if (!Module['noFSInit'] && !FS.initialized) FS.init();
TTY.init();

  wasmExports['__wasm_call_ctors']();

  FS.ignorePermissions = false;
}

function preMain() {
  checkStackCookie();
  
}

function postRun() {
  checkStackCookie();

  if (Module['postRun']) {
    if (typeof Module['postRun'] == 'function') Module['postRun'] = [Module['postRun']];
    while (Module['postRun'].length) {
      addOnPostRun(Module['postRun'].shift());
    }
  }
  consumedModuleProp('postRun');

  callRuntimeCallbacks(onPostRuns);
}

// A counter of dependencies for calling run(). If we need to
// do asynchronous work before running, increment this and
// decrement it. Incrementing must happen in a place like
// Module.preRun (used by emcc to add file preloading).
// Note that you can add dependencies in preRun, even though
// it happens right before run - run will be postponed until
// the dependencies are met.
var runDependencies = 0;
var dependenciesFulfilled = null; // overridden to take different actions when all run dependencies are fulfilled
var runDependencyTracking = {};
var runDependencyWatcher = null;

function getUniqueRunDependency(id) {
  var orig = id;
  while (1) {
    if (!runDependencyTracking[id]) return id;
    id = orig + Math.random();
  }
}

function addRunDependency(id) {
  runDependencies++;

  Module['monitorRunDependencies']?.(runDependencies);

  if (id) {
    assert(!runDependencyTracking[id]);
    runDependencyTracking[id] = 1;
    if (runDependencyWatcher === null && typeof setInterval != 'undefined') {
      // Check for missing dependencies every few seconds
      runDependencyWatcher = setInterval(() => {
        if (ABORT) {
          clearInterval(runDependencyWatcher);
          runDependencyWatcher = null;
          return;
        }
        var shown = false;
        for (var dep in runDependencyTracking) {
          if (!shown) {
            shown = true;
            err('still waiting on run dependencies:');
          }
          err(`dependency: ${dep}`);
        }
        if (shown) {
          err('(end of list)');
        }
      }, 10000);
    }
  } else {
    err('warning: run dependency added without ID');
  }
}

function removeRunDependency(id) {
  runDependencies--;

  Module['monitorRunDependencies']?.(runDependencies);

  if (id) {
    assert(runDependencyTracking[id]);
    delete runDependencyTracking[id];
  } else {
    err('warning: run dependency removed without ID');
  }
  if (runDependencies == 0) {
    if (runDependencyWatcher !== null) {
      clearInterval(runDependencyWatcher);
      runDependencyWatcher = null;
    }
    if (dependenciesFulfilled) {
      var callback = dependenciesFulfilled;
      dependenciesFulfilled = null;
      callback(); // can add another dependenciesFulfilled
    }
  }
}

/** @param {string|number=} what */
function abort(what) {
  Module['onAbort']?.(what);

  what = 'Aborted(' + what + ')';
  // TODO(sbc): Should we remove printing and leave it up to whoever
  // catches the exception?
  err(what);

  ABORT = true;

  // Use a wasm runtime error, because a JS error might be seen as a foreign
  // exception, which means we'd run destructors on it. We need the error to
  // simply make the program stop.
  // FIXME This approach does not work in Wasm EH because it currently does not assume
  // all RuntimeErrors are from traps; it decides whether a RuntimeError is from
  // a trap or not based on a hidden field within the object. So at the moment
  // we don't have a way of throwing a wasm trap from JS. TODO Make a JS API that
  // allows this in the wasm spec.

  // Suppress closure compiler warning here. Closure compiler's builtin extern
  // definition for WebAssembly.RuntimeError claims it takes no arguments even
  // though it can.
  // TODO(https://github.com/google/closure-compiler/pull/3913): Remove if/when upstream closure gets fixed.
  /** @suppress {checkTypes} */
  var e = new WebAssembly.RuntimeError(what);

  // Throw the error whether or not MODULARIZE is set because abort is used
  // in code paths apart from instantiation where an exception is expected
  // to be thrown when abort is called.
  throw e;
}

function createExportWrapper(name, nargs) {
  return (...args) => {
    assert(runtimeInitialized, `native function \`${name}\` called before runtime initialization`);
    var f = wasmExports[name];
    assert(f, `exported native function \`${name}\` not found`);
    // Only assert for too many arguments. Too few can be valid since the missing arguments will be zero filled.
    assert(args.length <= nargs, `native function \`${name}\` called with ${args.length} args but expects ${nargs}`);
    return f(...args);
  };
}

var wasmBinaryFile;

function findWasmBinary() {
    return locateFile('main.wasm');
}

function getBinarySync(file) {
  if (file == wasmBinaryFile && wasmBinary) {
    return new Uint8Array(wasmBinary);
  }
  if (readBinary) {
    return readBinary(file);
  }
  throw 'both async and sync fetching of the wasm failed';
}

async function getWasmBinary(binaryFile) {
  // If we don't have the binary yet, load it asynchronously using readAsync.
  if (!wasmBinary) {
    // Fetch the binary using readAsync
    try {
      var response = await readAsync(binaryFile);
      return new Uint8Array(response);
    } catch {
      // Fall back to getBinarySync below;
    }
  }

  // Otherwise, getBinarySync should be able to get it synchronously
  return getBinarySync(binaryFile);
}

async function instantiateArrayBuffer(binaryFile, imports) {
  try {
    var binary = await getWasmBinary(binaryFile);
    var instance = await WebAssembly.instantiate(binary, imports);
    return instance;
  } catch (reason) {
    err(`failed to asynchronously prepare wasm: ${reason}`);

    // Warn on some common problems.
    if (isFileURI(wasmBinaryFile)) {
      err(`warning: Loading from a file URI (${wasmBinaryFile}) is not supported in most browsers. See https://emscripten.org/docs/getting_started/FAQ.html#how-do-i-run-a-local-webserver-for-testing-why-does-my-program-stall-in-downloading-or-preparing`);
    }
    abort(reason);
  }
}

async function instantiateAsync(binary, binaryFile, imports) {
  if (!binary && typeof WebAssembly.instantiateStreaming == 'function'
      // Don't use streaming for file:// delivered objects in a webview, fetch them synchronously.
      && !isFileURI(binaryFile)
      // Avoid instantiateStreaming() on Node.js environment for now, as while
      // Node.js v18.1.0 implements it, it does not have a full fetch()
      // implementation yet.
      //
      // Reference:
      //   https://github.com/emscripten-core/emscripten/pull/16917
      && !ENVIRONMENT_IS_NODE
     ) {
    try {
      var response = fetch(binaryFile, { credentials: 'same-origin' });
      var instantiationResult = await WebAssembly.instantiateStreaming(response, imports);
      return instantiationResult;
    } catch (reason) {
      // We expect the most common failure cause to be a bad MIME type for the binary,
      // in which case falling back to ArrayBuffer instantiation should work.
      err(`wasm streaming compile failed: ${reason}`);
      err('falling back to ArrayBuffer instantiation');
      // fall back of instantiateArrayBuffer below
    };
  }
  return instantiateArrayBuffer(binaryFile, imports);
}

function getWasmImports() {
  // prepare imports
  return {
    'env': wasmImports,
    'wasi_snapshot_preview1': wasmImports,
  }
}

// Create the wasm instance.
// Receives the wasm imports, returns the exports.
async function createWasm() {
  // Load the wasm module and create an instance of using native support in the JS engine.
  // handle a generated wasm instance, receiving its exports and
  // performing other necessary setup
  /** @param {WebAssembly.Module=} module*/
  function receiveInstance(instance, module) {
    wasmExports = instance.exports;

    

    wasmMemory = wasmExports['memory'];
    
    assert(wasmMemory, 'memory not found in wasm exports');
    updateMemoryViews();

    wasmTable = wasmExports['__indirect_function_table'];
    
    assert(wasmTable, 'table not found in wasm exports');

    removeRunDependency('wasm-instantiate');
    return wasmExports;
  }
  // wait for the pthread pool (if any)
  addRunDependency('wasm-instantiate');

  // Prefer streaming instantiation if available.
  // Async compilation can be confusing when an error on the page overwrites Module
  // (for example, if the order of elements is wrong, and the one defining Module is
  // later), so we save Module and check it later.
  var trueModule = Module;
  function receiveInstantiationResult(result) {
    // 'result' is a ResultObject object which has both the module and instance.
    // receiveInstance() will swap in the exports (to Module.asm) so they can be called
    assert(Module === trueModule, 'the Module object should not be replaced during async compilation - perhaps the order of HTML elements is wrong?');
    trueModule = null;
    // TODO: Due to Closure regression https://github.com/google/closure-compiler/issues/3193, the above line no longer optimizes out down to the following line.
    // When the regression is fixed, can restore the above PTHREADS-enabled path.
    return receiveInstance(result['instance']);
  }

  var info = getWasmImports();

  // User shell pages can write their own Module.instantiateWasm = function(imports, successCallback) callback
  // to manually instantiate the Wasm module themselves. This allows pages to
  // run the instantiation parallel to any other async startup actions they are
  // performing.
  // Also pthreads and wasm workers initialize the wasm instance through this
  // path.
  if (Module['instantiateWasm']) {
    return new Promise((resolve, reject) => {
      try {
        Module['instantiateWasm'](info, (mod, inst) => {
          receiveInstance(mod, inst);
          resolve(mod.exports);
        });
      } catch(e) {
        err(`Module.instantiateWasm callback failed with error: ${e}`);
        reject(e);
      }
    });
  }

  wasmBinaryFile ??= findWasmBinary();
    var result = await instantiateAsync(wasmBinary, wasmBinaryFile, info);
    var exports = receiveInstantiationResult(result);
    return exports;
}

// end include: preamble.js

// Begin JS library code


  class ExitStatus {
      name = 'ExitStatus';
      constructor(status) {
        this.message = `Program terminated with exit(${status})`;
        this.status = status;
      }
    }

  var callRuntimeCallbacks = (callbacks) => {
      while (callbacks.length > 0) {
        // Pass the module as the first argument.
        callbacks.shift()(Module);
      }
    };
  var onPostRuns = [];
  var addOnPostRun = (cb) => onPostRuns.unshift(cb);

  var onPreRuns = [];
  var addOnPreRun = (cb) => onPreRuns.unshift(cb);


  
    /**
     * @param {number} ptr
     * @param {string} type
     */
  function getValue(ptr, type = 'i8') {
    if (type.endsWith('*')) type = '*';
    switch (type) {
      case 'i1': return HEAP8[ptr];
      case 'i8': return HEAP8[ptr];
      case 'i16': return HEAP16[((ptr)>>1)];
      case 'i32': return HEAP32[((ptr)>>2)];
      case 'i64': return HEAP64[((ptr)>>3)];
      case 'float': return HEAPF32[((ptr)>>2)];
      case 'double': return HEAPF64[((ptr)>>3)];
      case '*': return HEAPU32[((ptr)>>2)];
      default: abort(`invalid type for getValue: ${type}`);
    }
  }

  var noExitRuntime = Module['noExitRuntime'] || true;

  var ptrToString = (ptr) => {
      assert(typeof ptr === 'number');
      // With CAN_ADDRESS_2GB or MEMORY64, pointers are already unsigned.
      ptr >>>= 0;
      return '0x' + ptr.toString(16).padStart(8, '0');
    };

  
    /**
     * @param {number} ptr
     * @param {number} value
     * @param {string} type
     */
  function setValue(ptr, value, type = 'i8') {
    if (type.endsWith('*')) type = '*';
    switch (type) {
      case 'i1': HEAP8[ptr] = value; break;
      case 'i8': HEAP8[ptr] = value; break;
      case 'i16': HEAP16[((ptr)>>1)] = value; break;
      case 'i32': HEAP32[((ptr)>>2)] = value; break;
      case 'i64': HEAP64[((ptr)>>3)] = BigInt(value); break;
      case 'float': HEAPF32[((ptr)>>2)] = value; break;
      case 'double': HEAPF64[((ptr)>>3)] = value; break;
      case '*': HEAPU32[((ptr)>>2)] = value; break;
      default: abort(`invalid type for setValue: ${type}`);
    }
  }

  var stackRestore = (val) => __emscripten_stack_restore(val);

  var stackSave = () => _emscripten_stack_get_current();

  var warnOnce = (text) => {
      warnOnce.shown ||= {};
      if (!warnOnce.shown[text]) {
        warnOnce.shown[text] = 1;
        if (ENVIRONMENT_IS_NODE) text = 'warning: ' + text;
        err(text);
      }
    };

  var UTF8Decoder = typeof TextDecoder != 'undefined' ? new TextDecoder() : undefined;
  
    /**
     * Given a pointer 'idx' to a null-terminated UTF8-encoded string in the given
     * array that contains uint8 values, returns a copy of that string as a
     * Javascript String object.
     * heapOrArray is either a regular array, or a JavaScript typed array view.
     * @param {number=} idx
     * @param {number=} maxBytesToRead
     * @return {string}
     */
  var UTF8ArrayToString = (heapOrArray, idx = 0, maxBytesToRead = NaN) => {
      var endIdx = idx + maxBytesToRead;
      var endPtr = idx;
      // TextDecoder needs to know the byte length in advance, it doesn't stop on
      // null terminator by itself.  Also, use the length info to avoid running tiny
      // strings through TextDecoder, since .subarray() allocates garbage.
      // (As a tiny code save trick, compare endPtr against endIdx using a negation,
      // so that undefined/NaN means Infinity)
      while (heapOrArray[endPtr] && !(endPtr >= endIdx)) ++endPtr;
  
      if (endPtr - idx > 16 && heapOrArray.buffer && UTF8Decoder) {
        return UTF8Decoder.decode(heapOrArray.subarray(idx, endPtr));
      }
      var str = '';
      // If building with TextDecoder, we have already computed the string length
      // above, so test loop end condition against that
      while (idx < endPtr) {
        // For UTF8 byte structure, see:
        // http://en.wikipedia.org/wiki/UTF-8#Description
        // https://www.ietf.org/rfc/rfc2279.txt
        // https://tools.ietf.org/html/rfc3629
        var u0 = heapOrArray[idx++];
        if (!(u0 & 0x80)) { str += String.fromCharCode(u0); continue; }
        var u1 = heapOrArray[idx++] & 63;
        if ((u0 & 0xE0) == 0xC0) { str += String.fromCharCode(((u0 & 31) << 6) | u1); continue; }
        var u2 = heapOrArray[idx++] & 63;
        if ((u0 & 0xF0) == 0xE0) {
          u0 = ((u0 & 15) << 12) | (u1 << 6) | u2;
        } else {
          if ((u0 & 0xF8) != 0xF0) warnOnce('Invalid UTF-8 leading byte ' + ptrToString(u0) + ' encountered when deserializing a UTF-8 string in wasm memory to a JS string!');
          u0 = ((u0 & 7) << 18) | (u1 << 12) | (u2 << 6) | (heapOrArray[idx++] & 63);
        }
  
        if (u0 < 0x10000) {
          str += String.fromCharCode(u0);
        } else {
          var ch = u0 - 0x10000;
          str += String.fromCharCode(0xD800 | (ch >> 10), 0xDC00 | (ch & 0x3FF));
        }
      }
      return str;
    };
  
    /**
     * Given a pointer 'ptr' to a null-terminated UTF8-encoded string in the
     * emscripten HEAP, returns a copy of that string as a Javascript String object.
     *
     * @param {number} ptr
     * @param {number=} maxBytesToRead - An optional length that specifies the
     *   maximum number of bytes to read. You can omit this parameter to scan the
     *   string until the first 0 byte. If maxBytesToRead is passed, and the string
     *   at [ptr, ptr+maxBytesToReadr[ contains a null byte in the middle, then the
     *   string will cut short at that byte index (i.e. maxBytesToRead will not
     *   produce a string of exact length [ptr, ptr+maxBytesToRead[) N.B. mixing
     *   frequent uses of UTF8ToString() with and without maxBytesToRead may throw
     *   JS JIT optimizations off, so it is worth to consider consistently using one
     * @return {string}
     */
  var UTF8ToString = (ptr, maxBytesToRead) => {
      assert(typeof ptr == 'number', `UTF8ToString expects a number (got ${typeof ptr})`);
      return ptr ? UTF8ArrayToString(HEAPU8, ptr, maxBytesToRead) : '';
    };
  var ___assert_fail = (condition, filename, line, func) =>
      abort(`Assertion failed: ${UTF8ToString(condition)}, at: ` + [filename ? UTF8ToString(filename) : 'unknown filename', line, func ? UTF8ToString(func) : 'unknown function']);

  var exceptionCaught =  [];
  
  
  
  var uncaughtExceptionCount = 0;
  var ___cxa_begin_catch = (ptr) => {
      var info = new ExceptionInfo(ptr);
      if (!info.get_caught()) {
        info.set_caught(true);
        uncaughtExceptionCount--;
      }
      info.set_rethrown(false);
      exceptionCaught.push(info);
      ___cxa_increment_exception_refcount(ptr);
      return ___cxa_get_exception_ptr(ptr);
    };

  
  var exceptionLast = 0;
  
  
  var ___cxa_end_catch = () => {
      // Clear state flag.
      _setThrew(0, 0);
      assert(exceptionCaught.length > 0);
      // Call destructor if one is registered then clear it.
      var info = exceptionCaught.pop();
  
      ___cxa_decrement_exception_refcount(info.excPtr);
      exceptionLast = 0; // XXX in decRef?
    };

  
  class ExceptionInfo {
      // excPtr - Thrown object pointer to wrap. Metadata pointer is calculated from it.
      constructor(excPtr) {
        this.excPtr = excPtr;
        this.ptr = excPtr - 24;
      }
  
      set_type(type) {
        HEAPU32[(((this.ptr)+(4))>>2)] = type;
      }
  
      get_type() {
        return HEAPU32[(((this.ptr)+(4))>>2)];
      }
  
      set_destructor(destructor) {
        HEAPU32[(((this.ptr)+(8))>>2)] = destructor;
      }
  
      get_destructor() {
        return HEAPU32[(((this.ptr)+(8))>>2)];
      }
  
      set_caught(caught) {
        caught = caught ? 1 : 0;
        HEAP8[(this.ptr)+(12)] = caught;
      }
  
      get_caught() {
        return HEAP8[(this.ptr)+(12)] != 0;
      }
  
      set_rethrown(rethrown) {
        rethrown = rethrown ? 1 : 0;
        HEAP8[(this.ptr)+(13)] = rethrown;
      }
  
      get_rethrown() {
        return HEAP8[(this.ptr)+(13)] != 0;
      }
  
      // Initialize native structure fields. Should be called once after allocated.
      init(type, destructor) {
        this.set_adjusted_ptr(0);
        this.set_type(type);
        this.set_destructor(destructor);
      }
  
      set_adjusted_ptr(adjustedPtr) {
        HEAPU32[(((this.ptr)+(16))>>2)] = adjustedPtr;
      }
  
      get_adjusted_ptr() {
        return HEAPU32[(((this.ptr)+(16))>>2)];
      }
    }
  
  
  var setTempRet0 = (val) => __emscripten_tempret_set(val);
  var findMatchingCatch = (args) => {
      var thrown =
        exceptionLast?.excPtr;
      if (!thrown) {
        // just pass through the null ptr
        setTempRet0(0);
        return 0;
      }
      var info = new ExceptionInfo(thrown);
      info.set_adjusted_ptr(thrown);
      var thrownType = info.get_type();
      if (!thrownType) {
        // just pass through the thrown ptr
        setTempRet0(0);
        return thrown;
      }
  
      // can_catch receives a **, add indirection
      // The different catch blocks are denoted by different types.
      // Due to inheritance, those types may not precisely match the
      // type of the thrown object. Find one which matches, and
      // return the type of the catch block which should be called.
      for (var caughtType of args) {
        if (caughtType === 0 || caughtType === thrownType) {
          // Catch all clause matched or exactly the same type is caught
          break;
        }
        var adjusted_ptr_addr = info.ptr + 16;
        if (___cxa_can_catch(caughtType, thrownType, adjusted_ptr_addr)) {
          setTempRet0(caughtType);
          return thrown;
        }
      }
      setTempRet0(thrownType);
      return thrown;
    };
  var ___cxa_find_matching_catch_2 = () => findMatchingCatch([]);

  var ___cxa_find_matching_catch_3 = (arg0) => findMatchingCatch([arg0]);

  
  
  var ___cxa_rethrow = () => {
      var info = exceptionCaught.pop();
      if (!info) {
        abort('no exception to throw');
      }
      var ptr = info.excPtr;
      if (!info.get_rethrown()) {
        // Only pop if the corresponding push was through rethrow_primary_exception
        exceptionCaught.push(info);
        info.set_rethrown(true);
        info.set_caught(false);
        uncaughtExceptionCount++;
      }
      exceptionLast = new CppException(ptr);
      throw exceptionLast;
    };

  
  
  var ___cxa_throw = (ptr, type, destructor) => {
      var info = new ExceptionInfo(ptr);
      // Initialize ExceptionInfo content after it was allocated in __cxa_allocate_exception.
      info.init(type, destructor);
      exceptionLast = new CppException(ptr);
      uncaughtExceptionCount++;
      throw exceptionLast;
    };

  var ___cxa_uncaught_exceptions = () => uncaughtExceptionCount;

  var ___resumeException = (ptr) => {
      if (!exceptionLast) {
        exceptionLast = new CppException(ptr);
      }
      throw exceptionLast;
    };

  var PATH = {
  isAbs:(path) => path.charAt(0) === '/',
  splitPath:(filename) => {
        var splitPathRe = /^(\/?|)([\s\S]*?)((?:\.{1,2}|[^\/]+?|)(\.[^.\/]*|))(?:[\/]*)$/;
        return splitPathRe.exec(filename).slice(1);
      },
  normalizeArray:(parts, allowAboveRoot) => {
        // if the path tries to go above the root, `up` ends up > 0
        var up = 0;
        for (var i = parts.length - 1; i >= 0; i--) {
          var last = parts[i];
          if (last === '.') {
            parts.splice(i, 1);
          } else if (last === '..') {
            parts.splice(i, 1);
            up++;
          } else if (up) {
            parts.splice(i, 1);
            up--;
          }
        }
        // if the path is allowed to go above the root, restore leading ..s
        if (allowAboveRoot) {
          for (; up; up--) {
            parts.unshift('..');
          }
        }
        return parts;
      },
  normalize:(path) => {
        var isAbsolute = PATH.isAbs(path),
            trailingSlash = path.slice(-1) === '/';
        // Normalize the path
        path = PATH.normalizeArray(path.split('/').filter((p) => !!p), !isAbsolute).join('/');
        if (!path && !isAbsolute) {
          path = '.';
        }
        if (path && trailingSlash) {
          path += '/';
        }
        return (isAbsolute ? '/' : '') + path;
      },
  dirname:(path) => {
        var result = PATH.splitPath(path),
            root = result[0],
            dir = result[1];
        if (!root && !dir) {
          // No dirname whatsoever
          return '.';
        }
        if (dir) {
          // It has a dirname, strip trailing slash
          dir = dir.slice(0, -1);
        }
        return root + dir;
      },
  basename:(path) => path && path.match(/([^\/]+|\/)\/*$/)[1],
  join:(...paths) => PATH.normalize(paths.join('/')),
  join2:(l, r) => PATH.normalize(l + '/' + r),
  };
  
  var initRandomFill = () => {
      // This block is not needed on v19+ since crypto.getRandomValues is builtin
      if (ENVIRONMENT_IS_NODE) {
        var nodeCrypto = require('crypto');
        return (view) => nodeCrypto.randomFillSync(view);
      }
  
      return (view) => crypto.getRandomValues(view);
    };
  var randomFill = (view) => {
      // Lazily init on the first invocation.
      (randomFill = initRandomFill())(view);
    };
  
  
  
  var PATH_FS = {
  resolve:(...args) => {
        var resolvedPath = '',
          resolvedAbsolute = false;
        for (var i = args.length - 1; i >= -1 && !resolvedAbsolute; i--) {
          var path = (i >= 0) ? args[i] : FS.cwd();
          // Skip empty and invalid entries
          if (typeof path != 'string') {
            throw new TypeError('Arguments to path.resolve must be strings');
          } else if (!path) {
            return ''; // an invalid portion invalidates the whole thing
          }
          resolvedPath = path + '/' + resolvedPath;
          resolvedAbsolute = PATH.isAbs(path);
        }
        // At this point the path should be resolved to a full absolute path, but
        // handle relative paths to be safe (might happen when process.cwd() fails)
        resolvedPath = PATH.normalizeArray(resolvedPath.split('/').filter((p) => !!p), !resolvedAbsolute).join('/');
        return ((resolvedAbsolute ? '/' : '') + resolvedPath) || '.';
      },
  relative:(from, to) => {
        from = PATH_FS.resolve(from).slice(1);
        to = PATH_FS.resolve(to).slice(1);
        function trim(arr) {
          var start = 0;
          for (; start < arr.length; start++) {
            if (arr[start] !== '') break;
          }
          var end = arr.length - 1;
          for (; end >= 0; end--) {
            if (arr[end] !== '') break;
          }
          if (start > end) return [];
          return arr.slice(start, end - start + 1);
        }
        var fromParts = trim(from.split('/'));
        var toParts = trim(to.split('/'));
        var length = Math.min(fromParts.length, toParts.length);
        var samePartsLength = length;
        for (var i = 0; i < length; i++) {
          if (fromParts[i] !== toParts[i]) {
            samePartsLength = i;
            break;
          }
        }
        var outputParts = [];
        for (var i = samePartsLength; i < fromParts.length; i++) {
          outputParts.push('..');
        }
        outputParts = outputParts.concat(toParts.slice(samePartsLength));
        return outputParts.join('/');
      },
  };
  
  
  
  var FS_stdin_getChar_buffer = [];
  
  var lengthBytesUTF8 = (str) => {
      var len = 0;
      for (var i = 0; i < str.length; ++i) {
        // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code
        // unit, not a Unicode code point of the character! So decode
        // UTF16->UTF32->UTF8.
        // See http://unicode.org/faq/utf_bom.html#utf16-3
        var c = str.charCodeAt(i); // possibly a lead surrogate
        if (c <= 0x7F) {
          len++;
        } else if (c <= 0x7FF) {
          len += 2;
        } else if (c >= 0xD800 && c <= 0xDFFF) {
          len += 4; ++i;
        } else {
          len += 3;
        }
      }
      return len;
    };
  
  var stringToUTF8Array = (str, heap, outIdx, maxBytesToWrite) => {
      assert(typeof str === 'string', `stringToUTF8Array expects a string (got ${typeof str})`);
      // Parameter maxBytesToWrite is not optional. Negative values, 0, null,
      // undefined and false each don't write out any bytes.
      if (!(maxBytesToWrite > 0))
        return 0;
  
      var startIdx = outIdx;
      var endIdx = outIdx + maxBytesToWrite - 1; // -1 for string null terminator.
      for (var i = 0; i < str.length; ++i) {
        // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code
        // unit, not a Unicode code point of the character! So decode
        // UTF16->UTF32->UTF8.
        // See http://unicode.org/faq/utf_bom.html#utf16-3
        // For UTF8 byte structure, see http://en.wikipedia.org/wiki/UTF-8#Description
        // and https://www.ietf.org/rfc/rfc2279.txt
        // and https://tools.ietf.org/html/rfc3629
        var u = str.charCodeAt(i); // possibly a lead surrogate
        if (u >= 0xD800 && u <= 0xDFFF) {
          var u1 = str.charCodeAt(++i);
          u = 0x10000 + ((u & 0x3FF) << 10) | (u1 & 0x3FF);
        }
        if (u <= 0x7F) {
          if (outIdx >= endIdx) break;
          heap[outIdx++] = u;
        } else if (u <= 0x7FF) {
          if (outIdx + 1 >= endIdx) break;
          heap[outIdx++] = 0xC0 | (u >> 6);
          heap[outIdx++] = 0x80 | (u & 63);
        } else if (u <= 0xFFFF) {
          if (outIdx + 2 >= endIdx) break;
          heap[outIdx++] = 0xE0 | (u >> 12);
          heap[outIdx++] = 0x80 | ((u >> 6) & 63);
          heap[outIdx++] = 0x80 | (u & 63);
        } else {
          if (outIdx + 3 >= endIdx) break;
          if (u > 0x10FFFF) warnOnce('Invalid Unicode code point ' + ptrToString(u) + ' encountered when serializing a JS string to a UTF-8 string in wasm memory! (Valid unicode code points should be in range 0-0x10FFFF).');
          heap[outIdx++] = 0xF0 | (u >> 18);
          heap[outIdx++] = 0x80 | ((u >> 12) & 63);
          heap[outIdx++] = 0x80 | ((u >> 6) & 63);
          heap[outIdx++] = 0x80 | (u & 63);
        }
      }
      // Null-terminate the pointer to the buffer.
      heap[outIdx] = 0;
      return outIdx - startIdx;
    };
  /** @type {function(string, boolean=, number=)} */
  var intArrayFromString = (stringy, dontAddNull, length) => {
      var len = length > 0 ? length : lengthBytesUTF8(stringy)+1;
      var u8array = new Array(len);
      var numBytesWritten = stringToUTF8Array(stringy, u8array, 0, u8array.length);
      if (dontAddNull) u8array.length = numBytesWritten;
      return u8array;
    };
  var FS_stdin_getChar = () => {
      if (!FS_stdin_getChar_buffer.length) {
        var result = null;
        if (ENVIRONMENT_IS_NODE) {
          // we will read data by chunks of BUFSIZE
          var BUFSIZE = 256;
          var buf = Buffer.alloc(BUFSIZE);
          var bytesRead = 0;
  
          // For some reason we must suppress a closure warning here, even though
          // fd definitely exists on process.stdin, and is even the proper way to
          // get the fd of stdin,
          // https://github.com/nodejs/help/issues/2136#issuecomment-523649904
          // This started to happen after moving this logic out of library_tty.js,
          // so it is related to the surrounding code in some unclear manner.
          /** @suppress {missingProperties} */
          var fd = process.stdin.fd;
  
          try {
            bytesRead = fs.readSync(fd, buf, 0, BUFSIZE);
          } catch(e) {
            // Cross-platform differences: on Windows, reading EOF throws an
            // exception, but on other OSes, reading EOF returns 0. Uniformize
            // behavior by treating the EOF exception to return 0.
            if (e.toString().includes('EOF')) bytesRead = 0;
            else throw e;
          }
  
          if (bytesRead > 0) {
            result = buf.slice(0, bytesRead).toString('utf-8');
          }
        } else
        if (typeof window != 'undefined' &&
          typeof window.prompt == 'function') {
          // Browser.
          result = window.prompt('Input: ');  // returns null on cancel
          if (result !== null) {
            result += '\n';
          }
        } else
        {}
        if (!result) {
          return null;
        }
        FS_stdin_getChar_buffer = intArrayFromString(result, true);
      }
      return FS_stdin_getChar_buffer.shift();
    };
  var TTY = {
  ttys:[],
  init() {
        // https://github.com/emscripten-core/emscripten/pull/1555
        // if (ENVIRONMENT_IS_NODE) {
        //   // currently, FS.init does not distinguish if process.stdin is a file or TTY
        //   // device, it always assumes it's a TTY device. because of this, we're forcing
        //   // process.stdin to UTF8 encoding to at least make stdin reading compatible
        //   // with text files until FS.init can be refactored.
        //   process.stdin.setEncoding('utf8');
        // }
      },
  shutdown() {
        // https://github.com/emscripten-core/emscripten/pull/1555
        // if (ENVIRONMENT_IS_NODE) {
        //   // inolen: any idea as to why node -e 'process.stdin.read()' wouldn't exit immediately (with process.stdin being a tty)?
        //   // isaacs: because now it's reading from the stream, you've expressed interest in it, so that read() kicks off a _read() which creates a ReadReq operation
        //   // inolen: I thought read() in that case was a synchronous operation that just grabbed some amount of buffered data if it exists?
        //   // isaacs: it is. but it also triggers a _read() call, which calls readStart() on the handle
        //   // isaacs: do process.stdin.pause() and i'd think it'd probably close the pending call
        //   process.stdin.pause();
        // }
      },
  register(dev, ops) {
        TTY.ttys[dev] = { input: [], output: [], ops: ops };
        FS.registerDevice(dev, TTY.stream_ops);
      },
  stream_ops:{
  open(stream) {
          var tty = TTY.ttys[stream.node.rdev];
          if (!tty) {
            throw new FS.ErrnoError(43);
          }
          stream.tty = tty;
          stream.seekable = false;
        },
  close(stream) {
          // flush any pending line data
          stream.tty.ops.fsync(stream.tty);
        },
  fsync(stream) {
          stream.tty.ops.fsync(stream.tty);
        },
  read(stream, buffer, offset, length, pos /* ignored */) {
          if (!stream.tty || !stream.tty.ops.get_char) {
            throw new FS.ErrnoError(60);
          }
          var bytesRead = 0;
          for (var i = 0; i < length; i++) {
            var result;
            try {
              result = stream.tty.ops.get_char(stream.tty);
            } catch (e) {
              throw new FS.ErrnoError(29);
            }
            if (result === undefined && bytesRead === 0) {
              throw new FS.ErrnoError(6);
            }
            if (result === null || result === undefined) break;
            bytesRead++;
            buffer[offset+i] = result;
          }
          if (bytesRead) {
            stream.node.atime = Date.now();
          }
          return bytesRead;
        },
  write(stream, buffer, offset, length, pos) {
          if (!stream.tty || !stream.tty.ops.put_char) {
            throw new FS.ErrnoError(60);
          }
          try {
            for (var i = 0; i < length; i++) {
              stream.tty.ops.put_char(stream.tty, buffer[offset+i]);
            }
          } catch (e) {
            throw new FS.ErrnoError(29);
          }
          if (length) {
            stream.node.mtime = stream.node.ctime = Date.now();
          }
          return i;
        },
  },
  default_tty_ops:{
  get_char(tty) {
          return FS_stdin_getChar();
        },
  put_char(tty, val) {
          if (val === null || val === 10) {
            out(UTF8ArrayToString(tty.output));
            tty.output = [];
          } else {
            if (val != 0) tty.output.push(val); // val == 0 would cut text output off in the middle.
          }
        },
  fsync(tty) {
          if (tty.output?.length > 0) {
            out(UTF8ArrayToString(tty.output));
            tty.output = [];
          }
        },
  ioctl_tcgets(tty) {
          // typical setting
          return {
            c_iflag: 25856,
            c_oflag: 5,
            c_cflag: 191,
            c_lflag: 35387,
            c_cc: [
              0x03, 0x1c, 0x7f, 0x15, 0x04, 0x00, 0x01, 0x00, 0x11, 0x13, 0x1a, 0x00,
              0x12, 0x0f, 0x17, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ]
          };
        },
  ioctl_tcsets(tty, optional_actions, data) {
          // currently just ignore
          return 0;
        },
  ioctl_tiocgwinsz(tty) {
          return [24, 80];
        },
  },
  default_tty1_ops:{
  put_char(tty, val) {
          if (val === null || val === 10) {
            err(UTF8ArrayToString(tty.output));
            tty.output = [];
          } else {
            if (val != 0) tty.output.push(val);
          }
        },
  fsync(tty) {
          if (tty.output?.length > 0) {
            err(UTF8ArrayToString(tty.output));
            tty.output = [];
          }
        },
  },
  };
  
  
  var zeroMemory = (ptr, size) => HEAPU8.fill(0, ptr, ptr + size);
  
  var alignMemory = (size, alignment) => {
      assert(alignment, "alignment argument is required");
      return Math.ceil(size / alignment) * alignment;
    };
  var mmapAlloc = (size) => {
      size = alignMemory(size, 65536);
      var ptr = _emscripten_builtin_memalign(65536, size);
      if (ptr) zeroMemory(ptr, size);
      return ptr;
    };
  var MEMFS = {
  ops_table:null,
  mount(mount) {
        return MEMFS.createNode(null, '/', 16895, 0);
      },
  createNode(parent, name, mode, dev) {
        if (FS.isBlkdev(mode) || FS.isFIFO(mode)) {
          // no supported
          throw new FS.ErrnoError(63);
        }
        MEMFS.ops_table ||= {
          dir: {
            node: {
              getattr: MEMFS.node_ops.getattr,
              setattr: MEMFS.node_ops.setattr,
              lookup: MEMFS.node_ops.lookup,
              mknod: MEMFS.node_ops.mknod,
              rename: MEMFS.node_ops.rename,
              unlink: MEMFS.node_ops.unlink,
              rmdir: MEMFS.node_ops.rmdir,
              readdir: MEMFS.node_ops.readdir,
              symlink: MEMFS.node_ops.symlink
            },
            stream: {
              llseek: MEMFS.stream_ops.llseek
            }
          },
          file: {
            node: {
              getattr: MEMFS.node_ops.getattr,
              setattr: MEMFS.node_ops.setattr
            },
            stream: {
              llseek: MEMFS.stream_ops.llseek,
              read: MEMFS.stream_ops.read,
              write: MEMFS.stream_ops.write,
              mmap: MEMFS.stream_ops.mmap,
              msync: MEMFS.stream_ops.msync
            }
          },
          link: {
            node: {
              getattr: MEMFS.node_ops.getattr,
              setattr: MEMFS.node_ops.setattr,
              readlink: MEMFS.node_ops.readlink
            },
            stream: {}
          },
          chrdev: {
            node: {
              getattr: MEMFS.node_ops.getattr,
              setattr: MEMFS.node_ops.setattr
            },
            stream: FS.chrdev_stream_ops
          }
        };
        var node = FS.createNode(parent, name, mode, dev);
        if (FS.isDir(node.mode)) {
          node.node_ops = MEMFS.ops_table.dir.node;
          node.stream_ops = MEMFS.ops_table.dir.stream;
          node.contents = {};
        } else if (FS.isFile(node.mode)) {
          node.node_ops = MEMFS.ops_table.file.node;
          node.stream_ops = MEMFS.ops_table.file.stream;
          node.usedBytes = 0; // The actual number of bytes used in the typed array, as opposed to contents.length which gives the whole capacity.
          // When the byte data of the file is populated, this will point to either a typed array, or a normal JS array. Typed arrays are preferred
          // for performance, and used by default. However, typed arrays are not resizable like normal JS arrays are, so there is a small disk size
          // penalty involved for appending file writes that continuously grow a file similar to std::vector capacity vs used -scheme.
          node.contents = null; 
        } else if (FS.isLink(node.mode)) {
          node.node_ops = MEMFS.ops_table.link.node;
          node.stream_ops = MEMFS.ops_table.link.stream;
        } else if (FS.isChrdev(node.mode)) {
          node.node_ops = MEMFS.ops_table.chrdev.node;
          node.stream_ops = MEMFS.ops_table.chrdev.stream;
        }
        node.atime = node.mtime = node.ctime = Date.now();
        // add the new node to the parent
        if (parent) {
          parent.contents[name] = node;
          parent.atime = parent.mtime = parent.ctime = node.atime;
        }
        return node;
      },
  getFileDataAsTypedArray(node) {
        if (!node.contents) return new Uint8Array(0);
        if (node.contents.subarray) return node.contents.subarray(0, node.usedBytes); // Make sure to not return excess unused bytes.
        return new Uint8Array(node.contents);
      },
  expandFileStorage(node, newCapacity) {
        var prevCapacity = node.contents ? node.contents.length : 0;
        if (prevCapacity >= newCapacity) return; // No need to expand, the storage was already large enough.
        // Don't expand strictly to the given requested limit if it's only a very small increase, but instead geometrically grow capacity.
        // For small filesizes (<1MB), perform size*2 geometric increase, but for large sizes, do a much more conservative size*1.125 increase to
        // avoid overshooting the allocation cap by a very large margin.
        var CAPACITY_DOUBLING_MAX = 1024 * 1024;
        newCapacity = Math.max(newCapacity, (prevCapacity * (prevCapacity < CAPACITY_DOUBLING_MAX ? 2.0 : 1.125)) >>> 0);
        if (prevCapacity != 0) newCapacity = Math.max(newCapacity, 256); // At minimum allocate 256b for each file when expanding.
        var oldContents = node.contents;
        node.contents = new Uint8Array(newCapacity); // Allocate new storage.
        if (node.usedBytes > 0) node.contents.set(oldContents.subarray(0, node.usedBytes), 0); // Copy old data over to the new storage.
      },
  resizeFileStorage(node, newSize) {
        if (node.usedBytes == newSize) return;
        if (newSize == 0) {
          node.contents = null; // Fully decommit when requesting a resize to zero.
          node.usedBytes = 0;
        } else {
          var oldContents = node.contents;
          node.contents = new Uint8Array(newSize); // Allocate new storage.
          if (oldContents) {
            node.contents.set(oldContents.subarray(0, Math.min(newSize, node.usedBytes))); // Copy old data over to the new storage.
          }
          node.usedBytes = newSize;
        }
      },
  node_ops:{
  getattr(node) {
          var attr = {};
          // device numbers reuse inode numbers.
          attr.dev = FS.isChrdev(node.mode) ? node.id : 1;
          attr.ino = node.id;
          attr.mode = node.mode;
          attr.nlink = 1;
          attr.uid = 0;
          attr.gid = 0;
          attr.rdev = node.rdev;
          if (FS.isDir(node.mode)) {
            attr.size = 4096;
          } else if (FS.isFile(node.mode)) {
            attr.size = node.usedBytes;
          } else if (FS.isLink(node.mode)) {
            attr.size = node.link.length;
          } else {
            attr.size = 0;
          }
          attr.atime = new Date(node.atime);
          attr.mtime = new Date(node.mtime);
          attr.ctime = new Date(node.ctime);
          // NOTE: In our implementation, st_blocks = Math.ceil(st_size/st_blksize),
          //       but this is not required by the standard.
          attr.blksize = 4096;
          attr.blocks = Math.ceil(attr.size / attr.blksize);
          return attr;
        },
  setattr(node, attr) {
          for (const key of ["mode", "atime", "mtime", "ctime"]) {
            if (attr[key] != null) {
              node[key] = attr[key];
            }
          }
          if (attr.size !== undefined) {
            MEMFS.resizeFileStorage(node, attr.size);
          }
        },
  lookup(parent, name) {
          throw new FS.ErrnoError(44);
        },
  mknod(parent, name, mode, dev) {
          return MEMFS.createNode(parent, name, mode, dev);
        },
  rename(old_node, new_dir, new_name) {
          var new_node;
          try {
            new_node = FS.lookupNode(new_dir, new_name);
          } catch (e) {}
          if (new_node) {
            if (FS.isDir(old_node.mode)) {
              // if we're overwriting a directory at new_name, make sure it's empty.
              for (var i in new_node.contents) {
                throw new FS.ErrnoError(55);
              }
            }
            FS.hashRemoveNode(new_node);
          }
          // do the internal rewiring
          delete old_node.parent.contents[old_node.name];
          new_dir.contents[new_name] = old_node;
          old_node.name = new_name;
          new_dir.ctime = new_dir.mtime = old_node.parent.ctime = old_node.parent.mtime = Date.now();
        },
  unlink(parent, name) {
          delete parent.contents[name];
          parent.ctime = parent.mtime = Date.now();
        },
  rmdir(parent, name) {
          var node = FS.lookupNode(parent, name);
          for (var i in node.contents) {
            throw new FS.ErrnoError(55);
          }
          delete parent.contents[name];
          parent.ctime = parent.mtime = Date.now();
        },
  readdir(node) {
          return ['.', '..', ...Object.keys(node.contents)];
        },
  symlink(parent, newname, oldpath) {
          var node = MEMFS.createNode(parent, newname, 0o777 | 40960, 0);
          node.link = oldpath;
          return node;
        },
  readlink(node) {
          if (!FS.isLink(node.mode)) {
            throw new FS.ErrnoError(28);
          }
          return node.link;
        },
  },
  stream_ops:{
  read(stream, buffer, offset, length, position) {
          var contents = stream.node.contents;
          if (position >= stream.node.usedBytes) return 0;
          var size = Math.min(stream.node.usedBytes - position, length);
          assert(size >= 0);
          if (size > 8 && contents.subarray) { // non-trivial, and typed array
            buffer.set(contents.subarray(position, position + size), offset);
          } else {
            for (var i = 0; i < size; i++) buffer[offset + i] = contents[position + i];
          }
          return size;
        },
  write(stream, buffer, offset, length, position, canOwn) {
          // The data buffer should be a typed array view
          assert(!(buffer instanceof ArrayBuffer));
          // If the buffer is located in main memory (HEAP), and if
          // memory can grow, we can't hold on to references of the
          // memory buffer, as they may get invalidated. That means we
          // need to do copy its contents.
          if (buffer.buffer === HEAP8.buffer) {
            canOwn = false;
          }
  
          if (!length) return 0;
          var node = stream.node;
          node.mtime = node.ctime = Date.now();
  
          if (buffer.subarray && (!node.contents || node.contents.subarray)) { // This write is from a typed array to a typed array?
            if (canOwn) {
              assert(position === 0, 'canOwn must imply no weird position inside the file');
              node.contents = buffer.subarray(offset, offset + length);
              node.usedBytes = length;
              return length;
            } else if (node.usedBytes === 0 && position === 0) { // If this is a simple first write to an empty file, do a fast set since we don't need to care about old data.
              node.contents = buffer.slice(offset, offset + length);
              node.usedBytes = length;
              return length;
            } else if (position + length <= node.usedBytes) { // Writing to an already allocated and used subrange of the file?
              node.contents.set(buffer.subarray(offset, offset + length), position);
              return length;
            }
          }
  
          // Appending to an existing file and we need to reallocate, or source data did not come as a typed array.
          MEMFS.expandFileStorage(node, position+length);
          if (node.contents.subarray && buffer.subarray) {
            // Use typed array write which is available.
            node.contents.set(buffer.subarray(offset, offset + length), position);
          } else {
            for (var i = 0; i < length; i++) {
             node.contents[position + i] = buffer[offset + i]; // Or fall back to manual write if not.
            }
          }
          node.usedBytes = Math.max(node.usedBytes, position + length);
          return length;
        },
  llseek(stream, offset, whence) {
          var position = offset;
          if (whence === 1) {
            position += stream.position;
          } else if (whence === 2) {
            if (FS.isFile(stream.node.mode)) {
              position += stream.node.usedBytes;
            }
          }
          if (position < 0) {
            throw new FS.ErrnoError(28);
          }
          return position;
        },
  mmap(stream, length, position, prot, flags) {
          if (!FS.isFile(stream.node.mode)) {
            throw new FS.ErrnoError(43);
          }
          var ptr;
          var allocated;
          var contents = stream.node.contents;
          // Only make a new copy when MAP_PRIVATE is specified.
          if (!(flags & 2) && contents && contents.buffer === HEAP8.buffer) {
            // We can't emulate MAP_SHARED when the file is not backed by the
            // buffer we're mapping to (e.g. the HEAP buffer).
            allocated = false;
            ptr = contents.byteOffset;
          } else {
            allocated = true;
            ptr = mmapAlloc(length);
            if (!ptr) {
              throw new FS.ErrnoError(48);
            }
            if (contents) {
              // Try to avoid unnecessary slices.
              if (position > 0 || position + length < contents.length) {
                if (contents.subarray) {
                  contents = contents.subarray(position, position + length);
                } else {
                  contents = Array.prototype.slice.call(contents, position, position + length);
                }
              }
              HEAP8.set(contents, ptr);
            }
          }
          return { ptr, allocated };
        },
  msync(stream, buffer, offset, length, mmapFlags) {
          MEMFS.stream_ops.write(stream, buffer, 0, length, offset, false);
          // should we check if bytesWritten and length are the same?
          return 0;
        },
  },
  };
  
  var asyncLoad = async (url) => {
      var arrayBuffer = await readAsync(url);
      assert(arrayBuffer, `Loading data file "${url}" failed (no arrayBuffer).`);
      return new Uint8Array(arrayBuffer);
    };
  
  
  var FS_createDataFile = (parent, name, fileData, canRead, canWrite, canOwn) => {
      FS.createDataFile(parent, name, fileData, canRead, canWrite, canOwn);
    };
  
  var preloadPlugins = Module['preloadPlugins'] || [];
  var FS_handledByPreloadPlugin = (byteArray, fullname, finish, onerror) => {
      // Ensure plugins are ready.
      if (typeof Browser != 'undefined') Browser.init();
  
      var handled = false;
      preloadPlugins.forEach((plugin) => {
        if (handled) return;
        if (plugin['canHandle'](fullname)) {
          plugin['handle'](byteArray, fullname, finish, onerror);
          handled = true;
        }
      });
      return handled;
    };
  var FS_createPreloadedFile = (parent, name, url, canRead, canWrite, onload, onerror, dontCreateFile, canOwn, preFinish) => {
      // TODO we should allow people to just pass in a complete filename instead
      // of parent and name being that we just join them anyways
      var fullname = name ? PATH_FS.resolve(PATH.join2(parent, name)) : parent;
      var dep = getUniqueRunDependency(`cp ${fullname}`); // might have several active requests for the same fullname
      function processData(byteArray) {
        function finish(byteArray) {
          preFinish?.();
          if (!dontCreateFile) {
            FS_createDataFile(parent, name, byteArray, canRead, canWrite, canOwn);
          }
          onload?.();
          removeRunDependency(dep);
        }
        if (FS_handledByPreloadPlugin(byteArray, fullname, finish, () => {
          onerror?.();
          removeRunDependency(dep);
        })) {
          return;
        }
        finish(byteArray);
      }
      addRunDependency(dep);
      if (typeof url == 'string') {
        asyncLoad(url).then(processData, onerror);
      } else {
        processData(url);
      }
    };
  
  var FS_modeStringToFlags = (str) => {
      var flagModes = {
        'r': 0,
        'r+': 2,
        'w': 512 | 64 | 1,
        'w+': 512 | 64 | 2,
        'a': 1024 | 64 | 1,
        'a+': 1024 | 64 | 2,
      };
      var flags = flagModes[str];
      if (typeof flags == 'undefined') {
        throw new Error(`Unknown file open mode: ${str}`);
      }
      return flags;
    };
  
  var FS_getMode = (canRead, canWrite) => {
      var mode = 0;
      if (canRead) mode |= 292 | 73;
      if (canWrite) mode |= 146;
      return mode;
    };
  
  
  
  
  
  
  var IDBFS = {
  dbs:{
  },
  indexedDB:() => {
        if (typeof indexedDB != 'undefined') return indexedDB;
        var ret = null;
        if (typeof window == 'object') ret = window.indexedDB || window.mozIndexedDB || window.webkitIndexedDB || window.msIndexedDB;
        assert(ret, 'IDBFS used, but indexedDB not supported');
        return ret;
      },
  DB_VERSION:21,
  DB_STORE_NAME:"FILE_DATA",
  queuePersist:(mount) => {
        function onPersistComplete() {
          if (mount.idbPersistState === 'again') startPersist(); // If a new sync request has appeared in between, kick off a new sync
          else mount.idbPersistState = 0; // Otherwise reset sync state back to idle to wait for a new sync later
        }
        function startPersist() {
          mount.idbPersistState = 'idb'; // Mark that we are currently running a sync operation
          IDBFS.syncfs(mount, /*populate:*/false, onPersistComplete);
        }
  
        if (!mount.idbPersistState) {
          // Programs typically write/copy/move multiple files in the in-memory
          // filesystem within a single app frame, so when a filesystem sync
          // command is triggered, do not start it immediately, but only after
          // the current frame is finished. This way all the modified files
          // inside the main loop tick will be batched up to the same sync.
          mount.idbPersistState = setTimeout(startPersist, 0);
        } else if (mount.idbPersistState === 'idb') {
          // There is an active IndexedDB sync operation in-flight, but we now
          // have accumulated more files to sync. We should therefore queue up
          // a new sync after the current one finishes so that all writes
          // will be properly persisted.
          mount.idbPersistState = 'again';
        }
      },
  mount:(mount) => {
        // reuse core MEMFS functionality
        var mnt = MEMFS.mount(mount);
        // If the automatic IDBFS persistence option has been selected, then automatically persist
        // all modifications to the filesystem as they occur.
        if (mount?.opts?.autoPersist) {
          mnt.idbPersistState = 0; // IndexedDB sync starts in idle state
          var memfs_node_ops = mnt.node_ops;
          mnt.node_ops = {...mnt.node_ops}; // Clone node_ops to inject write tracking
          mnt.node_ops.mknod = (parent, name, mode, dev) => {
            var node = memfs_node_ops.mknod(parent, name, mode, dev);
            // Propagate injected node_ops to the newly created child node
            node.node_ops = mnt.node_ops;
            // Remember for each IDBFS node which IDBFS mount point they came from so we know which mount to persist on modification.
            node.idbfs_mount = mnt.mount;
            // Remember original MEMFS stream_ops for this node
            node.memfs_stream_ops = node.stream_ops;
            // Clone stream_ops to inject write tracking
            node.stream_ops = {...node.stream_ops};
  
            // Track all file writes
            node.stream_ops.write = (stream, buffer, offset, length, position, canOwn) => {
              // This file has been modified, we must persist IndexedDB when this file closes
              stream.node.isModified = true;
              return node.memfs_stream_ops.write(stream, buffer, offset, length, position, canOwn);
            };
  
            // Persist IndexedDB on file close
            node.stream_ops.close = (stream) => {
              var n = stream.node;
              if (n.isModified) {
                IDBFS.queuePersist(n.idbfs_mount);
                n.isModified = false;
              }
              if (n.memfs_stream_ops.close) return n.memfs_stream_ops.close(stream);
            };
  
            return node;
          };
          // Also kick off persisting the filesystem on other operations that modify the filesystem.
          mnt.node_ops.mkdir   = (...args) => (IDBFS.queuePersist(mnt.mount), memfs_node_ops.mkdir(...args));
          mnt.node_ops.rmdir   = (...args) => (IDBFS.queuePersist(mnt.mount), memfs_node_ops.rmdir(...args));
          mnt.node_ops.symlink = (...args) => (IDBFS.queuePersist(mnt.mount), memfs_node_ops.symlink(...args));
          mnt.node_ops.unlink  = (...args) => (IDBFS.queuePersist(mnt.mount), memfs_node_ops.unlink(...args));
          mnt.node_ops.rename  = (...args) => (IDBFS.queuePersist(mnt.mount), memfs_node_ops.rename(...args));
        }
        return mnt;
      },
  syncfs:(mount, populate, callback) => {
        IDBFS.getLocalSet(mount, (err, local) => {
          if (err) return callback(err);
  
          IDBFS.getRemoteSet(mount, (err, remote) => {
            if (err) return callback(err);
  
            var src = populate ? remote : local;
            var dst = populate ? local : remote;
  
            IDBFS.reconcile(src, dst, callback);
          });
        });
      },
  quit:() => {
        Object.values(IDBFS.dbs).forEach((value) => value.close());
        IDBFS.dbs = {};
      },
  getDB:(name, callback) => {
        // check the cache first
        var db = IDBFS.dbs[name];
        if (db) {
          return callback(null, db);
        }
  
        var req;
        try {
          req = IDBFS.indexedDB().open(name, IDBFS.DB_VERSION);
        } catch (e) {
          return callback(e);
        }
        if (!req) {
          return callback("Unable to connect to IndexedDB");
        }
        req.onupgradeneeded = (e) => {
          var db = /** @type {IDBDatabase} */ (e.target.result);
          var transaction = e.target.transaction;
  
          var fileStore;
  
          if (db.objectStoreNames.contains(IDBFS.DB_STORE_NAME)) {
            fileStore = transaction.objectStore(IDBFS.DB_STORE_NAME);
          } else {
            fileStore = db.createObjectStore(IDBFS.DB_STORE_NAME);
          }
  
          if (!fileStore.indexNames.contains('timestamp')) {
            fileStore.createIndex('timestamp', 'timestamp', { unique: false });
          }
        };
        req.onsuccess = () => {
          db = /** @type {IDBDatabase} */ (req.result);
  
          // add to the cache
          IDBFS.dbs[name] = db;
          callback(null, db);
        };
        req.onerror = (e) => {
          callback(e.target.error);
          e.preventDefault();
        };
      },
  getLocalSet:(mount, callback) => {
        var entries = {};
  
        function isRealDir(p) {
          return p !== '.' && p !== '..';
        };
        function toAbsolute(root) {
          return (p) => PATH.join2(root, p);
        };
  
        var check = FS.readdir(mount.mountpoint).filter(isRealDir).map(toAbsolute(mount.mountpoint));
  
        while (check.length) {
          var path = check.pop();
          var stat;
  
          try {
            stat = FS.stat(path);
          } catch (e) {
            return callback(e);
          }
  
          if (FS.isDir(stat.mode)) {
            check.push(...FS.readdir(path).filter(isRealDir).map(toAbsolute(path)));
          }
  
          entries[path] = { 'timestamp': stat.mtime };
        }
  
        return callback(null, { type: 'local', entries: entries });
      },
  getRemoteSet:(mount, callback) => {
        var entries = {};
  
        IDBFS.getDB(mount.mountpoint, (err, db) => {
          if (err) return callback(err);
  
          try {
            var transaction = db.transaction([IDBFS.DB_STORE_NAME], 'readonly');
            transaction.onerror = (e) => {
              callback(e.target.error);
              e.preventDefault();
            };
  
            var store = transaction.objectStore(IDBFS.DB_STORE_NAME);
            var index = store.index('timestamp');
  
            index.openKeyCursor().onsuccess = (event) => {
              var cursor = event.target.result;
  
              if (!cursor) {
                return callback(null, { type: 'remote', db, entries });
              }
  
              entries[cursor.primaryKey] = { 'timestamp': cursor.key };
  
              cursor.continue();
            };
          } catch (e) {
            return callback(e);
          }
        });
      },
  loadLocalEntry:(path, callback) => {
        var stat, node;
  
        try {
          var lookup = FS.lookupPath(path);
          node = lookup.node;
          stat = FS.stat(path);
        } catch (e) {
          return callback(e);
        }
  
        if (FS.isDir(stat.mode)) {
          return callback(null, { 'timestamp': stat.mtime, 'mode': stat.mode });
        } else if (FS.isFile(stat.mode)) {
          // Performance consideration: storing a normal JavaScript array to a IndexedDB is much slower than storing a typed array.
          // Therefore always convert the file contents to a typed array first before writing the data to IndexedDB.
          node.contents = MEMFS.getFileDataAsTypedArray(node);
          return callback(null, { 'timestamp': stat.mtime, 'mode': stat.mode, 'contents': node.contents });
        } else {
          return callback(new Error('node type not supported'));
        }
      },
  storeLocalEntry:(path, entry, callback) => {
        try {
          if (FS.isDir(entry['mode'])) {
            FS.mkdirTree(path, entry['mode']);
          } else if (FS.isFile(entry['mode'])) {
            FS.writeFile(path, entry['contents'], { canOwn: true });
          } else {
            return callback(new Error('node type not supported'));
          }
  
          FS.chmod(path, entry['mode']);
          FS.utime(path, entry['timestamp'], entry['timestamp']);
        } catch (e) {
          return callback(e);
        }
  
        callback(null);
      },
  removeLocalEntry:(path, callback) => {
        try {
          var stat = FS.stat(path);
  
          if (FS.isDir(stat.mode)) {
            FS.rmdir(path);
          } else if (FS.isFile(stat.mode)) {
            FS.unlink(path);
          }
        } catch (e) {
          return callback(e);
        }
  
        callback(null);
      },
  loadRemoteEntry:(store, path, callback) => {
        var req = store.get(path);
        req.onsuccess = (event) => callback(null, event.target.result);
        req.onerror = (e) => {
          callback(e.target.error);
          e.preventDefault();
        };
      },
  storeRemoteEntry:(store, path, entry, callback) => {
        try {
          var req = store.put(entry, path);
        } catch (e) {
          callback(e);
          return;
        }
        req.onsuccess = (event) => callback();
        req.onerror = (e) => {
          callback(e.target.error);
          e.preventDefault();
        };
      },
  removeRemoteEntry:(store, path, callback) => {
        var req = store.delete(path);
        req.onsuccess = (event) => callback();
        req.onerror = (e) => {
          callback(e.target.error);
          e.preventDefault();
        };
      },
  reconcile:(src, dst, callback) => {
        var total = 0;
  
        var create = [];
        Object.keys(src.entries).forEach((key) => {
          var e = src.entries[key];
          var e2 = dst.entries[key];
          if (!e2 || e['timestamp'].getTime() != e2['timestamp'].getTime()) {
            create.push(key);
            total++;
          }
        });
  
        var remove = [];
        Object.keys(dst.entries).forEach((key) => {
          if (!src.entries[key]) {
            remove.push(key);
            total++;
          }
        });
  
        if (!total) {
          return callback(null);
        }
  
        var errored = false;
        var db = src.type === 'remote' ? src.db : dst.db;
        var transaction = db.transaction([IDBFS.DB_STORE_NAME], 'readwrite');
        var store = transaction.objectStore(IDBFS.DB_STORE_NAME);
  
        function done(err) {
          if (err && !errored) {
            errored = true;
            return callback(err);
          }
        };
  
        // transaction may abort if (for example) there is a QuotaExceededError
        transaction.onerror = transaction.onabort = (e) => {
          done(e.target.error);
          e.preventDefault();
        };
  
        transaction.oncomplete = (e) => {
          if (!errored) {
            callback(null);
          }
        };
  
        // sort paths in ascending order so directory entries are created
        // before the files inside them
        create.sort().forEach((path) => {
          if (dst.type === 'local') {
            IDBFS.loadRemoteEntry(store, path, (err, entry) => {
              if (err) return done(err);
              IDBFS.storeLocalEntry(path, entry, done);
            });
          } else {
            IDBFS.loadLocalEntry(path, (err, entry) => {
              if (err) return done(err);
              IDBFS.storeRemoteEntry(store, path, entry, done);
            });
          }
        });
  
        // sort paths in descending order so files are deleted before their
        // parent directories
        remove.sort().reverse().forEach((path) => {
          if (dst.type === 'local') {
            IDBFS.removeLocalEntry(path, done);
          } else {
            IDBFS.removeRemoteEntry(store, path, done);
          }
        });
      },
  };
  
  
  
  var strError = (errno) => UTF8ToString(_strerror(errno));
  
  var ERRNO_CODES = {
      'EPERM': 63,
      'ENOENT': 44,
      'ESRCH': 71,
      'EINTR': 27,
      'EIO': 29,
      'ENXIO': 60,
      'E2BIG': 1,
      'ENOEXEC': 45,
      'EBADF': 8,
      'ECHILD': 12,
      'EAGAIN': 6,
      'EWOULDBLOCK': 6,
      'ENOMEM': 48,
      'EACCES': 2,
      'EFAULT': 21,
      'ENOTBLK': 105,
      'EBUSY': 10,
      'EEXIST': 20,
      'EXDEV': 75,
      'ENODEV': 43,
      'ENOTDIR': 54,
      'EISDIR': 31,
      'EINVAL': 28,
      'ENFILE': 41,
      'EMFILE': 33,
      'ENOTTY': 59,
      'ETXTBSY': 74,
      'EFBIG': 22,
      'ENOSPC': 51,
      'ESPIPE': 70,
      'EROFS': 69,
      'EMLINK': 34,
      'EPIPE': 64,
      'EDOM': 18,
      'ERANGE': 68,
      'ENOMSG': 49,
      'EIDRM': 24,
      'ECHRNG': 106,
      'EL2NSYNC': 156,
      'EL3HLT': 107,
      'EL3RST': 108,
      'ELNRNG': 109,
      'EUNATCH': 110,
      'ENOCSI': 111,
      'EL2HLT': 112,
      'EDEADLK': 16,
      'ENOLCK': 46,
      'EBADE': 113,
      'EBADR': 114,
      'EXFULL': 115,
      'ENOANO': 104,
      'EBADRQC': 103,
      'EBADSLT': 102,
      'EDEADLOCK': 16,
      'EBFONT': 101,
      'ENOSTR': 100,
      'ENODATA': 116,
      'ETIME': 117,
      'ENOSR': 118,
      'ENONET': 119,
      'ENOPKG': 120,
      'EREMOTE': 121,
      'ENOLINK': 47,
      'EADV': 122,
      'ESRMNT': 123,
      'ECOMM': 124,
      'EPROTO': 65,
      'EMULTIHOP': 36,
      'EDOTDOT': 125,
      'EBADMSG': 9,
      'ENOTUNIQ': 126,
      'EBADFD': 127,
      'EREMCHG': 128,
      'ELIBACC': 129,
      'ELIBBAD': 130,
      'ELIBSCN': 131,
      'ELIBMAX': 132,
      'ELIBEXEC': 133,
      'ENOSYS': 52,
      'ENOTEMPTY': 55,
      'ENAMETOOLONG': 37,
      'ELOOP': 32,
      'EOPNOTSUPP': 138,
      'EPFNOSUPPORT': 139,
      'ECONNRESET': 15,
      'ENOBUFS': 42,
      'EAFNOSUPPORT': 5,
      'EPROTOTYPE': 67,
      'ENOTSOCK': 57,
      'ENOPROTOOPT': 50,
      'ESHUTDOWN': 140,
      'ECONNREFUSED': 14,
      'EADDRINUSE': 3,
      'ECONNABORTED': 13,
      'ENETUNREACH': 40,
      'ENETDOWN': 38,
      'ETIMEDOUT': 73,
      'EHOSTDOWN': 142,
      'EHOSTUNREACH': 23,
      'EINPROGRESS': 26,
      'EALREADY': 7,
      'EDESTADDRREQ': 17,
      'EMSGSIZE': 35,
      'EPROTONOSUPPORT': 66,
      'ESOCKTNOSUPPORT': 137,
      'EADDRNOTAVAIL': 4,
      'ENETRESET': 39,
      'EISCONN': 30,
      'ENOTCONN': 53,
      'ETOOMANYREFS': 141,
      'EUSERS': 136,
      'EDQUOT': 19,
      'ESTALE': 72,
      'ENOTSUP': 138,
      'ENOMEDIUM': 148,
      'EILSEQ': 25,
      'EOVERFLOW': 61,
      'ECANCELED': 11,
      'ENOTRECOVERABLE': 56,
      'EOWNERDEAD': 62,
      'ESTRPIPE': 135,
    };
  var FS = {
  root:null,
  mounts:[],
  devices:{
  },
  streams:[],
  nextInode:1,
  nameTable:null,
  currentPath:"/",
  initialized:false,
  ignorePermissions:true,
  filesystems:null,
  syncFSRequests:0,
  readFiles:{
  },
  ErrnoError:class extends Error {
        name = 'ErrnoError';
        // We set the `name` property to be able to identify `FS.ErrnoError`
        // - the `name` is a standard ECMA-262 property of error objects. Kind of good to have it anyway.
        // - when using PROXYFS, an error can come from an underlying FS
        // as different FS objects have their own FS.ErrnoError each,
        // the test `err instanceof FS.ErrnoError` won't detect an error coming from another filesystem, causing bugs.
        // we'll use the reliable test `err.name == "ErrnoError"` instead
        constructor(errno) {
          super(runtimeInitialized ? strError(errno) : '');
          this.errno = errno;
          for (var key in ERRNO_CODES) {
            if (ERRNO_CODES[key] === errno) {
              this.code = key;
              break;
            }
          }
        }
      },
  FSStream:class {
        shared = {};
        get object() {
          return this.node;
        }
        set object(val) {
          this.node = val;
        }
        get isRead() {
          return (this.flags & 2097155) !== 1;
        }
        get isWrite() {
          return (this.flags & 2097155) !== 0;
        }
        get isAppend() {
          return (this.flags & 1024);
        }
        get flags() {
          return this.shared.flags;
        }
        set flags(val) {
          this.shared.flags = val;
        }
        get position() {
          return this.shared.position;
        }
        set position(val) {
          this.shared.position = val;
        }
      },
  FSNode:class {
        node_ops = {};
        stream_ops = {};
        readMode = 292 | 73;
        writeMode = 146;
        mounted = null;
        constructor(parent, name, mode, rdev) {
          if (!parent) {
            parent = this;  // root node sets parent to itself
          }
          this.parent = parent;
          this.mount = parent.mount;
          this.id = FS.nextInode++;
          this.name = name;
          this.mode = mode;
          this.rdev = rdev;
          this.atime = this.mtime = this.ctime = Date.now();
        }
        get read() {
          return (this.mode & this.readMode) === this.readMode;
        }
        set read(val) {
          val ? this.mode |= this.readMode : this.mode &= ~this.readMode;
        }
        get write() {
          return (this.mode & this.writeMode) === this.writeMode;
        }
        set write(val) {
          val ? this.mode |= this.writeMode : this.mode &= ~this.writeMode;
        }
        get isFolder() {
          return FS.isDir(this.mode);
        }
        get isDevice() {
          return FS.isChrdev(this.mode);
        }
      },
  lookupPath(path, opts = {}) {
        if (!path) {
          throw new FS.ErrnoError(44);
        }
        opts.follow_mount ??= true
  
        if (!PATH.isAbs(path)) {
          path = FS.cwd() + '/' + path;
        }
  
        // limit max consecutive symlinks to 40 (SYMLOOP_MAX).
        linkloop: for (var nlinks = 0; nlinks < 40; nlinks++) {
          // split the absolute path
          var parts = path.split('/').filter((p) => !!p);
  
          // start at the root
          var current = FS.root;
          var current_path = '/';
  
          for (var i = 0; i < parts.length; i++) {
            var islast = (i === parts.length-1);
            if (islast && opts.parent) {
              // stop resolving
              break;
            }
  
            if (parts[i] === '.') {
              continue;
            }
  
            if (parts[i] === '..') {
              current_path = PATH.dirname(current_path);
              current = current.parent;
              continue;
            }
  
            current_path = PATH.join2(current_path, parts[i]);
            try {
              current = FS.lookupNode(current, parts[i]);
            } catch (e) {
              // if noent_okay is true, suppress a ENOENT in the last component
              // and return an object with an undefined node. This is needed for
              // resolving symlinks in the path when creating a file.
              if ((e?.errno === 44) && islast && opts.noent_okay) {
                return { path: current_path };
              }
              throw e;
            }
  
            // jump to the mount's root node if this is a mountpoint
            if (FS.isMountpoint(current) && (!islast || opts.follow_mount)) {
              current = current.mounted.root;
            }
  
            // by default, lookupPath will not follow a symlink if it is the final path component.
            // setting opts.follow = true will override this behavior.
            if (FS.isLink(current.mode) && (!islast || opts.follow)) {
              if (!current.node_ops.readlink) {
                throw new FS.ErrnoError(52);
              }
              var link = current.node_ops.readlink(current);
              if (!PATH.isAbs(link)) {
                link = PATH.dirname(current_path) + '/' + link;
              }
              path = link + '/' + parts.slice(i + 1).join('/');
              continue linkloop;
            }
          }
          return { path: current_path, node: current };
        }
        throw new FS.ErrnoError(32);
      },
  getPath(node) {
        var path;
        while (true) {
          if (FS.isRoot(node)) {
            var mount = node.mount.mountpoint;
            if (!path) return mount;
            return mount[mount.length-1] !== '/' ? `${mount}/${path}` : mount + path;
          }
          path = path ? `${node.name}/${path}` : node.name;
          node = node.parent;
        }
      },
  hashName(parentid, name) {
        var hash = 0;
  
        for (var i = 0; i < name.length; i++) {
          hash = ((hash << 5) - hash + name.charCodeAt(i)) | 0;
        }
        return ((parentid + hash) >>> 0) % FS.nameTable.length;
      },
  hashAddNode(node) {
        var hash = FS.hashName(node.parent.id, node.name);
        node.name_next = FS.nameTable[hash];
        FS.nameTable[hash] = node;
      },
  hashRemoveNode(node) {
        var hash = FS.hashName(node.parent.id, node.name);
        if (FS.nameTable[hash] === node) {
          FS.nameTable[hash] = node.name_next;
        } else {
          var current = FS.nameTable[hash];
          while (current) {
            if (current.name_next === node) {
              current.name_next = node.name_next;
              break;
            }
            current = current.name_next;
          }
        }
      },
  lookupNode(parent, name) {
        var errCode = FS.mayLookup(parent);
        if (errCode) {
          throw new FS.ErrnoError(errCode);
        }
        var hash = FS.hashName(parent.id, name);
        for (var node = FS.nameTable[hash]; node; node = node.name_next) {
          var nodeName = node.name;
          if (node.parent.id === parent.id && nodeName === name) {
            return node;
          }
        }
        // if we failed to find it in the cache, call into the VFS
        return FS.lookup(parent, name);
      },
  createNode(parent, name, mode, rdev) {
        assert(typeof parent == 'object')
        var node = new FS.FSNode(parent, name, mode, rdev);
  
        FS.hashAddNode(node);
  
        return node;
      },
  destroyNode(node) {
        FS.hashRemoveNode(node);
      },
  isRoot(node) {
        return node === node.parent;
      },
  isMountpoint(node) {
        return !!node.mounted;
      },
  isFile(mode) {
        return (mode & 61440) === 32768;
      },
  isDir(mode) {
        return (mode & 61440) === 16384;
      },
  isLink(mode) {
        return (mode & 61440) === 40960;
      },
  isChrdev(mode) {
        return (mode & 61440) === 8192;
      },
  isBlkdev(mode) {
        return (mode & 61440) === 24576;
      },
  isFIFO(mode) {
        return (mode & 61440) === 4096;
      },
  isSocket(mode) {
        return (mode & 49152) === 49152;
      },
  flagsToPermissionString(flag) {
        var perms = ['r', 'w', 'rw'][flag & 3];
        if ((flag & 512)) {
          perms += 'w';
        }
        return perms;
      },
  nodePermissions(node, perms) {
        if (FS.ignorePermissions) {
          return 0;
        }
        // return 0 if any user, group or owner bits are set.
        if (perms.includes('r') && !(node.mode & 292)) {
          return 2;
        } else if (perms.includes('w') && !(node.mode & 146)) {
          return 2;
        } else if (perms.includes('x') && !(node.mode & 73)) {
          return 2;
        }
        return 0;
      },
  mayLookup(dir) {
        if (!FS.isDir(dir.mode)) return 54;
        var errCode = FS.nodePermissions(dir, 'x');
        if (errCode) return errCode;
        if (!dir.node_ops.lookup) return 2;
        return 0;
      },
  mayCreate(dir, name) {
        if (!FS.isDir(dir.mode)) {
          return 54;
        }
        try {
          var node = FS.lookupNode(dir, name);
          return 20;
        } catch (e) {
        }
        return FS.nodePermissions(dir, 'wx');
      },
  mayDelete(dir, name, isdir) {
        var node;
        try {
          node = FS.lookupNode(dir, name);
        } catch (e) {
          return e.errno;
        }
        var errCode = FS.nodePermissions(dir, 'wx');
        if (errCode) {
          return errCode;
        }
        if (isdir) {
          if (!FS.isDir(node.mode)) {
            return 54;
          }
          if (FS.isRoot(node) || FS.getPath(node) === FS.cwd()) {
            return 10;
          }
        } else {
          if (FS.isDir(node.mode)) {
            return 31;
          }
        }
        return 0;
      },
  mayOpen(node, flags) {
        if (!node) {
          return 44;
        }
        if (FS.isLink(node.mode)) {
          return 32;
        } else if (FS.isDir(node.mode)) {
          if (FS.flagsToPermissionString(flags) !== 'r' // opening for write
              || (flags & (512 | 64))) { // TODO: check for O_SEARCH? (== search for dir only)
            return 31;
          }
        }
        return FS.nodePermissions(node, FS.flagsToPermissionString(flags));
      },
  checkOpExists(op, err) {
        if (!op) {
          throw new FS.ErrnoError(err);
        }
        return op;
      },
  MAX_OPEN_FDS:4096,
  nextfd() {
        for (var fd = 0; fd <= FS.MAX_OPEN_FDS; fd++) {
          if (!FS.streams[fd]) {
            return fd;
          }
        }
        throw new FS.ErrnoError(33);
      },
  getStreamChecked(fd) {
        var stream = FS.getStream(fd);
        if (!stream) {
          throw new FS.ErrnoError(8);
        }
        return stream;
      },
  getStream:(fd) => FS.streams[fd],
  createStream(stream, fd = -1) {
        assert(fd >= -1);
  
        // clone it, so we can return an instance of FSStream
        stream = Object.assign(new FS.FSStream(), stream);
        if (fd == -1) {
          fd = FS.nextfd();
        }
        stream.fd = fd;
        FS.streams[fd] = stream;
        return stream;
      },
  closeStream(fd) {
        FS.streams[fd] = null;
      },
  dupStream(origStream, fd = -1) {
        var stream = FS.createStream(origStream, fd);
        stream.stream_ops?.dup?.(stream);
        return stream;
      },
  doSetAttr(stream, node, attr) {
        var setattr = stream?.stream_ops.setattr;
        var arg = setattr ? stream : node;
        setattr ??= node.node_ops.setattr;
        FS.checkOpExists(setattr, 63)
        setattr(arg, attr);
      },
  chrdev_stream_ops:{
  open(stream) {
          var device = FS.getDevice(stream.node.rdev);
          // override node's stream ops with the device's
          stream.stream_ops = device.stream_ops;
          // forward the open call
          stream.stream_ops.open?.(stream);
        },
  llseek() {
          throw new FS.ErrnoError(70);
        },
  },
  major:(dev) => ((dev) >> 8),
  minor:(dev) => ((dev) & 0xff),
  makedev:(ma, mi) => ((ma) << 8 | (mi)),
  registerDevice(dev, ops) {
        FS.devices[dev] = { stream_ops: ops };
      },
  getDevice:(dev) => FS.devices[dev],
  getMounts(mount) {
        var mounts = [];
        var check = [mount];
  
        while (check.length) {
          var m = check.pop();
  
          mounts.push(m);
  
          check.push(...m.mounts);
        }
  
        return mounts;
      },
  syncfs(populate, callback) {
        if (typeof populate == 'function') {
          callback = populate;
          populate = false;
        }
  
        FS.syncFSRequests++;
  
        if (FS.syncFSRequests > 1) {
          err(`warning: ${FS.syncFSRequests} FS.syncfs operations in flight at once, probably just doing extra work`);
        }
  
        var mounts = FS.getMounts(FS.root.mount);
        var completed = 0;
  
        function doCallback(errCode) {
          assert(FS.syncFSRequests > 0);
          FS.syncFSRequests--;
          return callback(errCode);
        }
  
        function done(errCode) {
          if (errCode) {
            if (!done.errored) {
              done.errored = true;
              return doCallback(errCode);
            }
            return;
          }
          if (++completed >= mounts.length) {
            doCallback(null);
          }
        };
  
        // sync all mounts
        mounts.forEach((mount) => {
          if (!mount.type.syncfs) {
            return done(null);
          }
          mount.type.syncfs(mount, populate, done);
        });
      },
  mount(type, opts, mountpoint) {
        if (typeof type == 'string') {
          // The filesystem was not included, and instead we have an error
          // message stored in the variable.
          throw type;
        }
        var root = mountpoint === '/';
        var pseudo = !mountpoint;
        var node;
  
        if (root && FS.root) {
          throw new FS.ErrnoError(10);
        } else if (!root && !pseudo) {
          var lookup = FS.lookupPath(mountpoint, { follow_mount: false });
  
          mountpoint = lookup.path;  // use the absolute path
          node = lookup.node;
  
          if (FS.isMountpoint(node)) {
            throw new FS.ErrnoError(10);
          }
  
          if (!FS.isDir(node.mode)) {
            throw new FS.ErrnoError(54);
          }
        }
  
        var mount = {
          type,
          opts,
          mountpoint,
          mounts: []
        };
  
        // create a root node for the fs
        var mountRoot = type.mount(mount);
        mountRoot.mount = mount;
        mount.root = mountRoot;
  
        if (root) {
          FS.root = mountRoot;
        } else if (node) {
          // set as a mountpoint
          node.mounted = mount;
  
          // add the new mount to the current mount's children
          if (node.mount) {
            node.mount.mounts.push(mount);
          }
        }
  
        return mountRoot;
      },
  unmount(mountpoint) {
        var lookup = FS.lookupPath(mountpoint, { follow_mount: false });
  
        if (!FS.isMountpoint(lookup.node)) {
          throw new FS.ErrnoError(28);
        }
  
        // destroy the nodes for this mount, and all its child mounts
        var node = lookup.node;
        var mount = node.mounted;
        var mounts = FS.getMounts(mount);
  
        Object.keys(FS.nameTable).forEach((hash) => {
          var current = FS.nameTable[hash];
  
          while (current) {
            var next = current.name_next;
  
            if (mounts.includes(current.mount)) {
              FS.destroyNode(current);
            }
  
            current = next;
          }
        });
  
        // no longer a mountpoint
        node.mounted = null;
  
        // remove this mount from the child mounts
        var idx = node.mount.mounts.indexOf(mount);
        assert(idx !== -1);
        node.mount.mounts.splice(idx, 1);
      },
  lookup(parent, name) {
        return parent.node_ops.lookup(parent, name);
      },
  mknod(path, mode, dev) {
        var lookup = FS.lookupPath(path, { parent: true });
        var parent = lookup.node;
        var name = PATH.basename(path);
        if (!name) {
          throw new FS.ErrnoError(28);
        }
        if (name === '.' || name === '..') {
          throw new FS.ErrnoError(20);
        }
        var errCode = FS.mayCreate(parent, name);
        if (errCode) {
          throw new FS.ErrnoError(errCode);
        }
        if (!parent.node_ops.mknod) {
          throw new FS.ErrnoError(63);
        }
        return parent.node_ops.mknod(parent, name, mode, dev);
      },
  statfs(path) {
        return FS.statfsNode(FS.lookupPath(path, {follow: true}).node);
      },
  statfsStream(stream) {
        // We keep a separate statfsStream function because noderawfs overrides
        // it. In noderawfs, stream.node is sometimes null. Instead, we need to
        // look at stream.path.
        return FS.statfsNode(stream.node);
      },
  statfsNode(node) {
        // NOTE: None of the defaults here are true. We're just returning safe and
        //       sane values. Currently nodefs and rawfs replace these defaults,
        //       other file systems leave them alone.
        var rtn = {
          bsize: 4096,
          frsize: 4096,
          blocks: 1e6,
          bfree: 5e5,
          bavail: 5e5,
          files: FS.nextInode,
          ffree: FS.nextInode - 1,
          fsid: 42,
          flags: 2,
          namelen: 255,
        };
  
        if (node.node_ops.statfs) {
          Object.assign(rtn, node.node_ops.statfs(node.mount.opts.root));
        }
        return rtn;
      },
  create(path, mode = 0o666) {
        mode &= 4095;
        mode |= 32768;
        return FS.mknod(path, mode, 0);
      },
  mkdir(path, mode = 0o777) {
        mode &= 511 | 512;
        mode |= 16384;
        return FS.mknod(path, mode, 0);
      },
  mkdirTree(path, mode) {
        var dirs = path.split('/');
        var d = '';
        for (var dir of dirs) {
          if (!dir) continue;
          if (d || PATH.isAbs(path)) d += '/';
          d += dir;
          try {
            FS.mkdir(d, mode);
          } catch(e) {
            if (e.errno != 20) throw e;
          }
        }
      },
  mkdev(path, mode, dev) {
        if (typeof dev == 'undefined') {
          dev = mode;
          mode = 0o666;
        }
        mode |= 8192;
        return FS.mknod(path, mode, dev);
      },
  symlink(oldpath, newpath) {
        if (!PATH_FS.resolve(oldpath)) {
          throw new FS.ErrnoError(44);
        }
        var lookup = FS.lookupPath(newpath, { parent: true });
        var parent = lookup.node;
        if (!parent) {
          throw new FS.ErrnoError(44);
        }
        var newname = PATH.basename(newpath);
        var errCode = FS.mayCreate(parent, newname);
        if (errCode) {
          throw new FS.ErrnoError(errCode);
        }
        if (!parent.node_ops.symlink) {
          throw new FS.ErrnoError(63);
        }
        return parent.node_ops.symlink(parent, newname, oldpath);
      },
  rename(old_path, new_path) {
        var old_dirname = PATH.dirname(old_path);
        var new_dirname = PATH.dirname(new_path);
        var old_name = PATH.basename(old_path);
        var new_name = PATH.basename(new_path);
        // parents must exist
        var lookup, old_dir, new_dir;
  
        // let the errors from non existent directories percolate up
        lookup = FS.lookupPath(old_path, { parent: true });
        old_dir = lookup.node;
        lookup = FS.lookupPath(new_path, { parent: true });
        new_dir = lookup.node;
  
        if (!old_dir || !new_dir) throw new FS.ErrnoError(44);
        // need to be part of the same mount
        if (old_dir.mount !== new_dir.mount) {
          throw new FS.ErrnoError(75);
        }
        // source must exist
        var old_node = FS.lookupNode(old_dir, old_name);
        // old path should not be an ancestor of the new path
        var relative = PATH_FS.relative(old_path, new_dirname);
        if (relative.charAt(0) !== '.') {
          throw new FS.ErrnoError(28);
        }
        // new path should not be an ancestor of the old path
        relative = PATH_FS.relative(new_path, old_dirname);
        if (relative.charAt(0) !== '.') {
          throw new FS.ErrnoError(55);
        }
        // see if the new path already exists
        var new_node;
        try {
          new_node = FS.lookupNode(new_dir, new_name);
        } catch (e) {
          // not fatal
        }
        // early out if nothing needs to change
        if (old_node === new_node) {
          return;
        }
        // we'll need to delete the old entry
        var isdir = FS.isDir(old_node.mode);
        var errCode = FS.mayDelete(old_dir, old_name, isdir);
        if (errCode) {
          throw new FS.ErrnoError(errCode);
        }
        // need delete permissions if we'll be overwriting.
        // need create permissions if new doesn't already exist.
        errCode = new_node ?
          FS.mayDelete(new_dir, new_name, isdir) :
          FS.mayCreate(new_dir, new_name);
        if (errCode) {
          throw new FS.ErrnoError(errCode);
        }
        if (!old_dir.node_ops.rename) {
          throw new FS.ErrnoError(63);
        }
        if (FS.isMountpoint(old_node) || (new_node && FS.isMountpoint(new_node))) {
          throw new FS.ErrnoError(10);
        }
        // if we are going to change the parent, check write permissions
        if (new_dir !== old_dir) {
          errCode = FS.nodePermissions(old_dir, 'w');
          if (errCode) {
            throw new FS.ErrnoError(errCode);
          }
        }
        // remove the node from the lookup hash
        FS.hashRemoveNode(old_node);
        // do the underlying fs rename
        try {
          old_dir.node_ops.rename(old_node, new_dir, new_name);
          // update old node (we do this here to avoid each backend
          // needing to)
          old_node.parent = new_dir;
        } catch (e) {
          throw e;
        } finally {
          // add the node back to the hash (in case node_ops.rename
          // changed its name)
          FS.hashAddNode(old_node);
        }
      },
  rmdir(path) {
        var lookup = FS.lookupPath(path, { parent: true });
        var parent = lookup.node;
        var name = PATH.basename(path);
        var node = FS.lookupNode(parent, name);
        var errCode = FS.mayDelete(parent, name, true);
        if (errCode) {
          throw new FS.ErrnoError(errCode);
        }
        if (!parent.node_ops.rmdir) {
          throw new FS.ErrnoError(63);
        }
        if (FS.isMountpoint(node)) {
          throw new FS.ErrnoError(10);
        }
        parent.node_ops.rmdir(parent, name);
        FS.destroyNode(node);
      },
  readdir(path) {
        var lookup = FS.lookupPath(path, { follow: true });
        var node = lookup.node;
        var readdir = FS.checkOpExists(node.node_ops.readdir, 54);
        return readdir(node);
      },
  unlink(path) {
        var lookup = FS.lookupPath(path, { parent: true });
        var parent = lookup.node;
        if (!parent) {
          throw new FS.ErrnoError(44);
        }
        var name = PATH.basename(path);
        var node = FS.lookupNode(parent, name);
        var errCode = FS.mayDelete(parent, name, false);
        if (errCode) {
          // According to POSIX, we should map EISDIR to EPERM, but
          // we instead do what Linux does (and we must, as we use
          // the musl linux libc).
          throw new FS.ErrnoError(errCode);
        }
        if (!parent.node_ops.unlink) {
          throw new FS.ErrnoError(63);
        }
        if (FS.isMountpoint(node)) {
          throw new FS.ErrnoError(10);
        }
        parent.node_ops.unlink(parent, name);
        FS.destroyNode(node);
      },
  readlink(path) {
        var lookup = FS.lookupPath(path);
        var link = lookup.node;
        if (!link) {
          throw new FS.ErrnoError(44);
        }
        if (!link.node_ops.readlink) {
          throw new FS.ErrnoError(28);
        }
        return link.node_ops.readlink(link);
      },
  stat(path, dontFollow) {
        var lookup = FS.lookupPath(path, { follow: !dontFollow });
        var node = lookup.node;
        var getattr = FS.checkOpExists(node.node_ops.getattr, 63);
        return getattr(node);
      },
  fstat(fd) {
        var stream = FS.getStreamChecked(fd);
        var node = stream.node;
        var getattr = stream.stream_ops.getattr;
        var arg = getattr ? stream : node;
        getattr ??= node.node_ops.getattr;
        FS.checkOpExists(getattr, 63)
        return getattr(arg);
      },
  lstat(path) {
        return FS.stat(path, true);
      },
  doChmod(stream, node, mode, dontFollow) {
        FS.doSetAttr(stream, node, {
          mode: (mode & 4095) | (node.mode & ~4095),
          ctime: Date.now(),
          dontFollow
        });
      },
  chmod(path, mode, dontFollow) {
        var node;
        if (typeof path == 'string') {
          var lookup = FS.lookupPath(path, { follow: !dontFollow });
          node = lookup.node;
        } else {
          node = path;
        }
        FS.doChmod(null, node, mode, dontFollow);
      },
  lchmod(path, mode) {
        FS.chmod(path, mode, true);
      },
  fchmod(fd, mode) {
        var stream = FS.getStreamChecked(fd);
        FS.doChmod(stream, stream.node, mode, false);
      },
  doChown(stream, node, dontFollow) {
        FS.doSetAttr(stream, node, {
          timestamp: Date.now(),
          dontFollow
          // we ignore the uid / gid for now
        });
      },
  chown(path, uid, gid, dontFollow) {
        var node;
        if (typeof path == 'string') {
          var lookup = FS.lookupPath(path, { follow: !dontFollow });
          node = lookup.node;
        } else {
          node = path;
        }
        FS.doChown(null, node, dontFollow);
      },
  lchown(path, uid, gid) {
        FS.chown(path, uid, gid, true);
      },
  fchown(fd, uid, gid) {
        var stream = FS.getStreamChecked(fd);
        FS.doChown(stream, stream.node, false);
      },
  doTruncate(stream, node, len) {
        if (FS.isDir(node.mode)) {
          throw new FS.ErrnoError(31);
        }
        if (!FS.isFile(node.mode)) {
          throw new FS.ErrnoError(28);
        }
        var errCode = FS.nodePermissions(node, 'w');
        if (errCode) {
          throw new FS.ErrnoError(errCode);
        }
        FS.doSetAttr(stream, node, {
          size: len,
          timestamp: Date.now()
        });
      },
  truncate(path, len) {
        if (len < 0) {
          throw new FS.ErrnoError(28);
        }
        var node;
        if (typeof path == 'string') {
          var lookup = FS.lookupPath(path, { follow: true });
          node = lookup.node;
        } else {
          node = path;
        }
        FS.doTruncate(null, node, len);
      },
  ftruncate(fd, len) {
        var stream = FS.getStreamChecked(fd);
        if (len < 0 || (stream.flags & 2097155) === 0) {
          throw new FS.ErrnoError(28);
        }
        FS.doTruncate(stream, stream.node, len);
      },
  utime(path, atime, mtime) {
        var lookup = FS.lookupPath(path, { follow: true });
        var node = lookup.node;
        var setattr = FS.checkOpExists(node.node_ops.setattr, 63);
        setattr(node, {
          atime: atime,
          mtime: mtime
        });
      },
  open(path, flags, mode = 0o666) {
        if (path === "") {
          throw new FS.ErrnoError(44);
        }
        flags = typeof flags == 'string' ? FS_modeStringToFlags(flags) : flags;
        if ((flags & 64)) {
          mode = (mode & 4095) | 32768;
        } else {
          mode = 0;
        }
        var node;
        var isDirPath;
        if (typeof path == 'object') {
          node = path;
        } else {
          isDirPath = path.endsWith("/");
          // noent_okay makes it so that if the final component of the path
          // doesn't exist, lookupPath returns `node: undefined`. `path` will be
          // updated to point to the target of all symlinks.
          var lookup = FS.lookupPath(path, {
            follow: !(flags & 131072),
            noent_okay: true
          });
          node = lookup.node;
          path = lookup.path;
        }
        // perhaps we need to create the node
        var created = false;
        if ((flags & 64)) {
          if (node) {
            // if O_CREAT and O_EXCL are set, error out if the node already exists
            if ((flags & 128)) {
              throw new FS.ErrnoError(20);
            }
          } else if (isDirPath) {
            throw new FS.ErrnoError(31);
          } else {
            // node doesn't exist, try to create it
            // Ignore the permission bits here to ensure we can `open` this new
            // file below. We use chmod below the apply the permissions once the
            // file is open.
            node = FS.mknod(path, mode | 0o777, 0);
            created = true;
          }
        }
        if (!node) {
          throw new FS.ErrnoError(44);
        }
        // can't truncate a device
        if (FS.isChrdev(node.mode)) {
          flags &= ~512;
        }
        // if asked only for a directory, then this must be one
        if ((flags & 65536) && !FS.isDir(node.mode)) {
          throw new FS.ErrnoError(54);
        }
        // check permissions, if this is not a file we just created now (it is ok to
        // create and write to a file with read-only permissions; it is read-only
        // for later use)
        if (!created) {
          var errCode = FS.mayOpen(node, flags);
          if (errCode) {
            throw new FS.ErrnoError(errCode);
          }
        }
        // do truncation if necessary
        if ((flags & 512) && !created) {
          FS.truncate(node, 0);
        }
        // we've already handled these, don't pass down to the underlying vfs
        flags &= ~(128 | 512 | 131072);
  
        // register the stream with the filesystem
        var stream = FS.createStream({
          node,
          path: FS.getPath(node),  // we want the absolute path to the node
          flags,
          seekable: true,
          position: 0,
          stream_ops: node.stream_ops,
          // used by the file family libc calls (fopen, fwrite, ferror, etc.)
          ungotten: [],
          error: false
        });
        // call the new stream's open function
        if (stream.stream_ops.open) {
          stream.stream_ops.open(stream);
        }
        if (created) {
          FS.chmod(node, mode & 0o777);
        }
        if (Module['logReadFiles'] && !(flags & 1)) {
          if (!(path in FS.readFiles)) {
            FS.readFiles[path] = 1;
          }
        }
        return stream;
      },
  close(stream) {
        if (FS.isClosed(stream)) {
          throw new FS.ErrnoError(8);
        }
        if (stream.getdents) stream.getdents = null; // free readdir state
        try {
          if (stream.stream_ops.close) {
            stream.stream_ops.close(stream);
          }
        } catch (e) {
          throw e;
        } finally {
          FS.closeStream(stream.fd);
        }
        stream.fd = null;
      },
  isClosed(stream) {
        return stream.fd === null;
      },
  llseek(stream, offset, whence) {
        if (FS.isClosed(stream)) {
          throw new FS.ErrnoError(8);
        }
        if (!stream.seekable || !stream.stream_ops.llseek) {
          throw new FS.ErrnoError(70);
        }
        if (whence != 0 && whence != 1 && whence != 2) {
          throw new FS.ErrnoError(28);
        }
        stream.position = stream.stream_ops.llseek(stream, offset, whence);
        stream.ungotten = [];
        return stream.position;
      },
  read(stream, buffer, offset, length, position) {
        assert(offset >= 0);
        if (length < 0 || position < 0) {
          throw new FS.ErrnoError(28);
        }
        if (FS.isClosed(stream)) {
          throw new FS.ErrnoError(8);
        }
        if ((stream.flags & 2097155) === 1) {
          throw new FS.ErrnoError(8);
        }
        if (FS.isDir(stream.node.mode)) {
          throw new FS.ErrnoError(31);
        }
        if (!stream.stream_ops.read) {
          throw new FS.ErrnoError(28);
        }
        var seeking = typeof position != 'undefined';
        if (!seeking) {
          position = stream.position;
        } else if (!stream.seekable) {
          throw new FS.ErrnoError(70);
        }
        var bytesRead = stream.stream_ops.read(stream, buffer, offset, length, position);
        if (!seeking) stream.position += bytesRead;
        return bytesRead;
      },
  write(stream, buffer, offset, length, position, canOwn) {
        assert(offset >= 0);
        if (length < 0 || position < 0) {
          throw new FS.ErrnoError(28);
        }
        if (FS.isClosed(stream)) {
          throw new FS.ErrnoError(8);
        }
        if ((stream.flags & 2097155) === 0) {
          throw new FS.ErrnoError(8);
        }
        if (FS.isDir(stream.node.mode)) {
          throw new FS.ErrnoError(31);
        }
        if (!stream.stream_ops.write) {
          throw new FS.ErrnoError(28);
        }
        if (stream.seekable && stream.flags & 1024) {
          // seek to the end before writing in append mode
          FS.llseek(stream, 0, 2);
        }
        var seeking = typeof position != 'undefined';
        if (!seeking) {
          position = stream.position;
        } else if (!stream.seekable) {
          throw new FS.ErrnoError(70);
        }
        var bytesWritten = stream.stream_ops.write(stream, buffer, offset, length, position, canOwn);
        if (!seeking) stream.position += bytesWritten;
        return bytesWritten;
      },
  mmap(stream, length, position, prot, flags) {
        // User requests writing to file (prot & PROT_WRITE != 0).
        // Checking if we have permissions to write to the file unless
        // MAP_PRIVATE flag is set. According to POSIX spec it is possible
        // to write to file opened in read-only mode with MAP_PRIVATE flag,
        // as all modifications will be visible only in the memory of
        // the current process.
        if ((prot & 2) !== 0
            && (flags & 2) === 0
            && (stream.flags & 2097155) !== 2) {
          throw new FS.ErrnoError(2);
        }
        if ((stream.flags & 2097155) === 1) {
          throw new FS.ErrnoError(2);
        }
        if (!stream.stream_ops.mmap) {
          throw new FS.ErrnoError(43);
        }
        if (!length) {
          throw new FS.ErrnoError(28);
        }
        return stream.stream_ops.mmap(stream, length, position, prot, flags);
      },
  msync(stream, buffer, offset, length, mmapFlags) {
        assert(offset >= 0);
        if (!stream.stream_ops.msync) {
          return 0;
        }
        return stream.stream_ops.msync(stream, buffer, offset, length, mmapFlags);
      },
  ioctl(stream, cmd, arg) {
        if (!stream.stream_ops.ioctl) {
          throw new FS.ErrnoError(59);
        }
        return stream.stream_ops.ioctl(stream, cmd, arg);
      },
  readFile(path, opts = {}) {
        opts.flags = opts.flags || 0;
        opts.encoding = opts.encoding || 'binary';
        if (opts.encoding !== 'utf8' && opts.encoding !== 'binary') {
          throw new Error(`Invalid encoding type "${opts.encoding}"`);
        }
        var ret;
        var stream = FS.open(path, opts.flags);
        var stat = FS.stat(path);
        var length = stat.size;
        var buf = new Uint8Array(length);
        FS.read(stream, buf, 0, length, 0);
        if (opts.encoding === 'utf8') {
          ret = UTF8ArrayToString(buf);
        } else if (opts.encoding === 'binary') {
          ret = buf;
        }
        FS.close(stream);
        return ret;
      },
  writeFile(path, data, opts = {}) {
        opts.flags = opts.flags || 577;
        var stream = FS.open(path, opts.flags, opts.mode);
        if (typeof data == 'string') {
          var buf = new Uint8Array(lengthBytesUTF8(data)+1);
          var actualNumBytes = stringToUTF8Array(data, buf, 0, buf.length);
          FS.write(stream, buf, 0, actualNumBytes, undefined, opts.canOwn);
        } else if (ArrayBuffer.isView(data)) {
          FS.write(stream, data, 0, data.byteLength, undefined, opts.canOwn);
        } else {
          throw new Error('Unsupported data type');
        }
        FS.close(stream);
      },
  cwd:() => FS.currentPath,
  chdir(path) {
        var lookup = FS.lookupPath(path, { follow: true });
        if (lookup.node === null) {
          throw new FS.ErrnoError(44);
        }
        if (!FS.isDir(lookup.node.mode)) {
          throw new FS.ErrnoError(54);
        }
        var errCode = FS.nodePermissions(lookup.node, 'x');
        if (errCode) {
          throw new FS.ErrnoError(errCode);
        }
        FS.currentPath = lookup.path;
      },
  createDefaultDirectories() {
        FS.mkdir('/tmp');
        FS.mkdir('/home');
        FS.mkdir('/home/web_user');
      },
  createDefaultDevices() {
        // create /dev
        FS.mkdir('/dev');
        // setup /dev/null
        FS.registerDevice(FS.makedev(1, 3), {
          read: () => 0,
          write: (stream, buffer, offset, length, pos) => length,
          llseek: () => 0,
        });
        FS.mkdev('/dev/null', FS.makedev(1, 3));
        // setup /dev/tty and /dev/tty1
        // stderr needs to print output using err() rather than out()
        // so we register a second tty just for it.
        TTY.register(FS.makedev(5, 0), TTY.default_tty_ops);
        TTY.register(FS.makedev(6, 0), TTY.default_tty1_ops);
        FS.mkdev('/dev/tty', FS.makedev(5, 0));
        FS.mkdev('/dev/tty1', FS.makedev(6, 0));
        // setup /dev/[u]random
        // use a buffer to avoid overhead of individual crypto calls per byte
        var randomBuffer = new Uint8Array(1024), randomLeft = 0;
        var randomByte = () => {
          if (randomLeft === 0) {
            randomFill(randomBuffer);
            randomLeft = randomBuffer.byteLength;
          }
          return randomBuffer[--randomLeft];
        };
        FS.createDevice('/dev', 'random', randomByte);
        FS.createDevice('/dev', 'urandom', randomByte);
        // we're not going to emulate the actual shm device,
        // just create the tmp dirs that reside in it commonly
        FS.mkdir('/dev/shm');
        FS.mkdir('/dev/shm/tmp');
      },
  createSpecialDirectories() {
        // create /proc/self/fd which allows /proc/self/fd/6 => readlink gives the
        // name of the stream for fd 6 (see test_unistd_ttyname)
        FS.mkdir('/proc');
        var proc_self = FS.mkdir('/proc/self');
        FS.mkdir('/proc/self/fd');
        FS.mount({
          mount() {
            var node = FS.createNode(proc_self, 'fd', 16895, 73);
            node.stream_ops = {
              llseek: MEMFS.stream_ops.llseek,
            };
            node.node_ops = {
              lookup(parent, name) {
                var fd = +name;
                var stream = FS.getStreamChecked(fd);
                var ret = {
                  parent: null,
                  mount: { mountpoint: 'fake' },
                  node_ops: { readlink: () => stream.path },
                  id: fd + 1,
                };
                ret.parent = ret; // make it look like a simple root node
                return ret;
              },
              readdir() {
                return Array.from(FS.streams.entries())
                  .filter(([k, v]) => v)
                  .map(([k, v]) => k.toString());
              }
            };
            return node;
          }
        }, {}, '/proc/self/fd');
      },
  createStandardStreams(input, output, error) {
        // TODO deprecate the old functionality of a single
        // input / output callback and that utilizes FS.createDevice
        // and instead require a unique set of stream ops
  
        // by default, we symlink the standard streams to the
        // default tty devices. however, if the standard streams
        // have been overwritten we create a unique device for
        // them instead.
        if (input) {
          FS.createDevice('/dev', 'stdin', input);
        } else {
          FS.symlink('/dev/tty', '/dev/stdin');
        }
        if (output) {
          FS.createDevice('/dev', 'stdout', null, output);
        } else {
          FS.symlink('/dev/tty', '/dev/stdout');
        }
        if (error) {
          FS.createDevice('/dev', 'stderr', null, error);
        } else {
          FS.symlink('/dev/tty1', '/dev/stderr');
        }
  
        // open default streams for the stdin, stdout and stderr devices
        var stdin = FS.open('/dev/stdin', 0);
        var stdout = FS.open('/dev/stdout', 1);
        var stderr = FS.open('/dev/stderr', 1);
        assert(stdin.fd === 0, `invalid handle for stdin (${stdin.fd})`);
        assert(stdout.fd === 1, `invalid handle for stdout (${stdout.fd})`);
        assert(stderr.fd === 2, `invalid handle for stderr (${stderr.fd})`);
      },
  staticInit() {
        FS.nameTable = new Array(4096);
  
        FS.mount(MEMFS, {}, '/');
  
        FS.createDefaultDirectories();
        FS.createDefaultDevices();
        FS.createSpecialDirectories();
  
        FS.filesystems = {
          'MEMFS': MEMFS,
          'IDBFS': IDBFS,
        };
      },
  init(input, output, error) {
        assert(!FS.initialized, 'FS.init was previously called. If you want to initialize later with custom parameters, remove any earlier calls (note that one is automatically added to the generated code)');
        FS.initialized = true;
  
        // Allow Module.stdin etc. to provide defaults, if none explicitly passed to us here
        input ??= Module['stdin'];
        output ??= Module['stdout'];
        error ??= Module['stderr'];
  
        FS.createStandardStreams(input, output, error);
      },
  quit() {
        FS.initialized = false;
        // force-flush all streams, so we get musl std streams printed out
        _fflush(0);
        // close all of our streams
        for (var stream of FS.streams) {
          if (stream) {
            FS.close(stream);
          }
        }
      },
  findObject(path, dontResolveLastLink) {
        var ret = FS.analyzePath(path, dontResolveLastLink);
        if (!ret.exists) {
          return null;
        }
        return ret.object;
      },
  analyzePath(path, dontResolveLastLink) {
        // operate from within the context of the symlink's target
        try {
          var lookup = FS.lookupPath(path, { follow: !dontResolveLastLink });
          path = lookup.path;
        } catch (e) {
        }
        var ret = {
          isRoot: false, exists: false, error: 0, name: null, path: null, object: null,
          parentExists: false, parentPath: null, parentObject: null
        };
        try {
          var lookup = FS.lookupPath(path, { parent: true });
          ret.parentExists = true;
          ret.parentPath = lookup.path;
          ret.parentObject = lookup.node;
          ret.name = PATH.basename(path);
          lookup = FS.lookupPath(path, { follow: !dontResolveLastLink });
          ret.exists = true;
          ret.path = lookup.path;
          ret.object = lookup.node;
          ret.name = lookup.node.name;
          ret.isRoot = lookup.path === '/';
        } catch (e) {
          ret.error = e.errno;
        };
        return ret;
      },
  createPath(parent, path, canRead, canWrite) {
        parent = typeof parent == 'string' ? parent : FS.getPath(parent);
        var parts = path.split('/').reverse();
        while (parts.length) {
          var part = parts.pop();
          if (!part) continue;
          var current = PATH.join2(parent, part);
          try {
            FS.mkdir(current);
          } catch (e) {
            if (e.errno != 20) throw e;
          }
          parent = current;
        }
        return current;
      },
  createFile(parent, name, properties, canRead, canWrite) {
        var path = PATH.join2(typeof parent == 'string' ? parent : FS.getPath(parent), name);
        var mode = FS_getMode(canRead, canWrite);
        return FS.create(path, mode);
      },
  createDataFile(parent, name, data, canRead, canWrite, canOwn) {
        var path = name;
        if (parent) {
          parent = typeof parent == 'string' ? parent : FS.getPath(parent);
          path = name ? PATH.join2(parent, name) : parent;
        }
        var mode = FS_getMode(canRead, canWrite);
        var node = FS.create(path, mode);
        if (data) {
          if (typeof data == 'string') {
            var arr = new Array(data.length);
            for (var i = 0, len = data.length; i < len; ++i) arr[i] = data.charCodeAt(i);
            data = arr;
          }
          // make sure we can write to the file
          FS.chmod(node, mode | 146);
          var stream = FS.open(node, 577);
          FS.write(stream, data, 0, data.length, 0, canOwn);
          FS.close(stream);
          FS.chmod(node, mode);
        }
      },
  createDevice(parent, name, input, output) {
        var path = PATH.join2(typeof parent == 'string' ? parent : FS.getPath(parent), name);
        var mode = FS_getMode(!!input, !!output);
        FS.createDevice.major ??= 64;
        var dev = FS.makedev(FS.createDevice.major++, 0);
        // Create a fake device that a set of stream ops to emulate
        // the old behavior.
        FS.registerDevice(dev, {
          open(stream) {
            stream.seekable = false;
          },
          close(stream) {
            // flush any pending line data
            if (output?.buffer?.length) {
              output(10);
            }
          },
          read(stream, buffer, offset, length, pos /* ignored */) {
            var bytesRead = 0;
            for (var i = 0; i < length; i++) {
              var result;
              try {
                result = input();
              } catch (e) {
                throw new FS.ErrnoError(29);
              }
              if (result === undefined && bytesRead === 0) {
                throw new FS.ErrnoError(6);
              }
              if (result === null || result === undefined) break;
              bytesRead++;
              buffer[offset+i] = result;
            }
            if (bytesRead) {
              stream.node.atime = Date.now();
            }
            return bytesRead;
          },
          write(stream, buffer, offset, length, pos) {
            for (var i = 0; i < length; i++) {
              try {
                output(buffer[offset+i]);
              } catch (e) {
                throw new FS.ErrnoError(29);
              }
            }
            if (length) {
              stream.node.mtime = stream.node.ctime = Date.now();
            }
            return i;
          }
        });
        return FS.mkdev(path, mode, dev);
      },
  forceLoadFile(obj) {
        if (obj.isDevice || obj.isFolder || obj.link || obj.contents) return true;
        if (typeof XMLHttpRequest != 'undefined') {
          throw new Error("Lazy loading should have been performed (contents set) in createLazyFile, but it was not. Lazy loading only works in web workers. Use --embed-file or --preload-file in emcc on the main thread.");
        } else { // Command-line.
          try {
            obj.contents = readBinary(obj.url);
            obj.usedBytes = obj.contents.length;
          } catch (e) {
            throw new FS.ErrnoError(29);
          }
        }
      },
  createLazyFile(parent, name, url, canRead, canWrite) {
        // Lazy chunked Uint8Array (implements get and length from Uint8Array).
        // Actual getting is abstracted away for eventual reuse.
        class LazyUint8Array {
          lengthKnown = false;
          chunks = []; // Loaded chunks. Index is the chunk number
          get(idx) {
            if (idx > this.length-1 || idx < 0) {
              return undefined;
            }
            var chunkOffset = idx % this.chunkSize;
            var chunkNum = (idx / this.chunkSize)|0;
            return this.getter(chunkNum)[chunkOffset];
          }
          setDataGetter(getter) {
            this.getter = getter;
          }
          cacheLength() {
            // Find length
            var xhr = new XMLHttpRequest();
            xhr.open('HEAD', url, false);
            xhr.send(null);
            if (!(xhr.status >= 200 && xhr.status < 300 || xhr.status === 304)) throw new Error("Couldn't load " + url + ". Status: " + xhr.status);
            var datalength = Number(xhr.getResponseHeader("Content-length"));
            var header;
            var hasByteServing = (header = xhr.getResponseHeader("Accept-Ranges")) && header === "bytes";
            var usesGzip = (header = xhr.getResponseHeader("Content-Encoding")) && header === "gzip";
  
            var chunkSize = 1024*1024; // Chunk size in bytes
  
            if (!hasByteServing) chunkSize = datalength;
  
            // Function to get a range from the remote URL.
            var doXHR = (from, to) => {
              if (from > to) throw new Error("invalid range (" + from + ", " + to + ") or no bytes requested!");
              if (to > datalength-1) throw new Error("only " + datalength + " bytes available! programmer error!");
  
              // TODO: Use mozResponseArrayBuffer, responseStream, etc. if available.
              var xhr = new XMLHttpRequest();
              xhr.open('GET', url, false);
              if (datalength !== chunkSize) xhr.setRequestHeader("Range", "bytes=" + from + "-" + to);
  
              // Some hints to the browser that we want binary data.
              xhr.responseType = 'arraybuffer';
              if (xhr.overrideMimeType) {
                xhr.overrideMimeType('text/plain; charset=x-user-defined');
              }
  
              xhr.send(null);
              if (!(xhr.status >= 200 && xhr.status < 300 || xhr.status === 304)) throw new Error("Couldn't load " + url + ". Status: " + xhr.status);
              if (xhr.response !== undefined) {
                return new Uint8Array(/** @type{Array<number>} */(xhr.response || []));
              }
              return intArrayFromString(xhr.responseText || '', true);
            };
            var lazyArray = this;
            lazyArray.setDataGetter((chunkNum) => {
              var start = chunkNum * chunkSize;
              var end = (chunkNum+1) * chunkSize - 1; // including this byte
              end = Math.min(end, datalength-1); // if datalength-1 is selected, this is the last block
              if (typeof lazyArray.chunks[chunkNum] == 'undefined') {
                lazyArray.chunks[chunkNum] = doXHR(start, end);
              }
              if (typeof lazyArray.chunks[chunkNum] == 'undefined') throw new Error('doXHR failed!');
              return lazyArray.chunks[chunkNum];
            });
  
            if (usesGzip || !datalength) {
              // if the server uses gzip or doesn't supply the length, we have to download the whole file to get the (uncompressed) length
              chunkSize = datalength = 1; // this will force getter(0)/doXHR do download the whole file
              datalength = this.getter(0).length;
              chunkSize = datalength;
              out("LazyFiles on gzip forces download of the whole file when length is accessed");
            }
  
            this._length = datalength;
            this._chunkSize = chunkSize;
            this.lengthKnown = true;
          }
          get length() {
            if (!this.lengthKnown) {
              this.cacheLength();
            }
            return this._length;
          }
          get chunkSize() {
            if (!this.lengthKnown) {
              this.cacheLength();
            }
            return this._chunkSize;
          }
        }
  
        if (typeof XMLHttpRequest != 'undefined') {
          if (!ENVIRONMENT_IS_WORKER) throw 'Cannot do synchronous binary XHRs outside webworkers in modern browsers. Use --embed-file or --preload-file in emcc';
          var lazyArray = new LazyUint8Array();
          var properties = { isDevice: false, contents: lazyArray };
        } else {
          var properties = { isDevice: false, url: url };
        }
  
        var node = FS.createFile(parent, name, properties, canRead, canWrite);
        // This is a total hack, but I want to get this lazy file code out of the
        // core of MEMFS. If we want to keep this lazy file concept I feel it should
        // be its own thin LAZYFS proxying calls to MEMFS.
        if (properties.contents) {
          node.contents = properties.contents;
        } else if (properties.url) {
          node.contents = null;
          node.url = properties.url;
        }
        // Add a function that defers querying the file size until it is asked the first time.
        Object.defineProperties(node, {
          usedBytes: {
            get: function() { return this.contents.length; }
          }
        });
        // override each stream op with one that tries to force load the lazy file first
        var stream_ops = {};
        var keys = Object.keys(node.stream_ops);
        keys.forEach((key) => {
          var fn = node.stream_ops[key];
          stream_ops[key] = (...args) => {
            FS.forceLoadFile(node);
            return fn(...args);
          };
        });
        function writeChunks(stream, buffer, offset, length, position) {
          var contents = stream.node.contents;
          if (position >= contents.length)
            return 0;
          var size = Math.min(contents.length - position, length);
          assert(size >= 0);
          if (contents.slice) { // normal array
            for (var i = 0; i < size; i++) {
              buffer[offset + i] = contents[position + i];
            }
          } else {
            for (var i = 0; i < size; i++) { // LazyUint8Array from sync binary XHR
              buffer[offset + i] = contents.get(position + i);
            }
          }
          return size;
        }
        // use a custom read function
        stream_ops.read = (stream, buffer, offset, length, position) => {
          FS.forceLoadFile(node);
          return writeChunks(stream, buffer, offset, length, position)
        };
        // use a custom mmap function
        stream_ops.mmap = (stream, length, position, prot, flags) => {
          FS.forceLoadFile(node);
          var ptr = mmapAlloc(length);
          if (!ptr) {
            throw new FS.ErrnoError(48);
          }
          writeChunks(stream, HEAP8, ptr, length, position);
          return { ptr, allocated: true };
        };
        node.stream_ops = stream_ops;
        return node;
      },
  absolutePath() {
        abort('FS.absolutePath has been removed; use PATH_FS.resolve instead');
      },
  createFolder() {
        abort('FS.createFolder has been removed; use FS.mkdir instead');
      },
  createLink() {
        abort('FS.createLink has been removed; use FS.symlink instead');
      },
  joinPath() {
        abort('FS.joinPath has been removed; use PATH.join instead');
      },
  mmapAlloc() {
        abort('FS.mmapAlloc has been replaced by the top level function mmapAlloc');
      },
  standardizePath() {
        abort('FS.standardizePath has been removed; use PATH.normalize instead');
      },
  };
  
  var SYSCALLS = {
  DEFAULT_POLLMASK:5,
  calculateAt(dirfd, path, allowEmpty) {
        if (PATH.isAbs(path)) {
          return path;
        }
        // relative path
        var dir;
        if (dirfd === -100) {
          dir = FS.cwd();
        } else {
          var dirstream = SYSCALLS.getStreamFromFD(dirfd);
          dir = dirstream.path;
        }
        if (path.length == 0) {
          if (!allowEmpty) {
            throw new FS.ErrnoError(44);;
          }
          return dir;
        }
        return dir + '/' + path;
      },
  writeStat(buf, stat) {
        HEAP32[((buf)>>2)] = stat.dev;
        HEAP32[(((buf)+(4))>>2)] = stat.mode;
        HEAPU32[(((buf)+(8))>>2)] = stat.nlink;
        HEAP32[(((buf)+(12))>>2)] = stat.uid;
        HEAP32[(((buf)+(16))>>2)] = stat.gid;
        HEAP32[(((buf)+(20))>>2)] = stat.rdev;
        HEAP64[(((buf)+(24))>>3)] = BigInt(stat.size);
        HEAP32[(((buf)+(32))>>2)] = 4096;
        HEAP32[(((buf)+(36))>>2)] = stat.blocks;
        var atime = stat.atime.getTime();
        var mtime = stat.mtime.getTime();
        var ctime = stat.ctime.getTime();
        HEAP64[(((buf)+(40))>>3)] = BigInt(Math.floor(atime / 1000));
        HEAPU32[(((buf)+(48))>>2)] = (atime % 1000) * 1000 * 1000;
        HEAP64[(((buf)+(56))>>3)] = BigInt(Math.floor(mtime / 1000));
        HEAPU32[(((buf)+(64))>>2)] = (mtime % 1000) * 1000 * 1000;
        HEAP64[(((buf)+(72))>>3)] = BigInt(Math.floor(ctime / 1000));
        HEAPU32[(((buf)+(80))>>2)] = (ctime % 1000) * 1000 * 1000;
        HEAP64[(((buf)+(88))>>3)] = BigInt(stat.ino);
        return 0;
      },
  writeStatFs(buf, stats) {
        HEAP32[(((buf)+(4))>>2)] = stats.bsize;
        HEAP32[(((buf)+(40))>>2)] = stats.bsize;
        HEAP32[(((buf)+(8))>>2)] = stats.blocks;
        HEAP32[(((buf)+(12))>>2)] = stats.bfree;
        HEAP32[(((buf)+(16))>>2)] = stats.bavail;
        HEAP32[(((buf)+(20))>>2)] = stats.files;
        HEAP32[(((buf)+(24))>>2)] = stats.ffree;
        HEAP32[(((buf)+(28))>>2)] = stats.fsid;
        HEAP32[(((buf)+(44))>>2)] = stats.flags;  // ST_NOSUID
        HEAP32[(((buf)+(36))>>2)] = stats.namelen;
      },
  doMsync(addr, stream, len, flags, offset) {
        if (!FS.isFile(stream.node.mode)) {
          throw new FS.ErrnoError(43);
        }
        if (flags & 2) {
          // MAP_PRIVATE calls need not to be synced back to underlying fs
          return 0;
        }
        var buffer = HEAPU8.slice(addr, addr + len);
        FS.msync(stream, buffer, offset, len, flags);
      },
  getStreamFromFD(fd) {
        var stream = FS.getStreamChecked(fd);
        return stream;
      },
  varargs:undefined,
  getStr(ptr) {
        var ret = UTF8ToString(ptr);
        return ret;
      },
  };
  function ___syscall_chdir(path) {
  try {
  
      path = SYSCALLS.getStr(path);
      FS.chdir(path);
      return 0;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  /** @suppress {duplicate } */
  var syscallGetVarargI = () => {
      assert(SYSCALLS.varargs != undefined);
      // the `+` prepended here is necessary to convince the JSCompiler that varargs is indeed a number.
      var ret = HEAP32[((+SYSCALLS.varargs)>>2)];
      SYSCALLS.varargs += 4;
      return ret;
    };
  var syscallGetVarargP = syscallGetVarargI;
  
  
  function ___syscall_fcntl64(fd, cmd, varargs) {
  SYSCALLS.varargs = varargs;
  try {
  
      var stream = SYSCALLS.getStreamFromFD(fd);
      switch (cmd) {
        case 0: {
          var arg = syscallGetVarargI();
          if (arg < 0) {
            return -28;
          }
          while (FS.streams[arg]) {
            arg++;
          }
          var newStream;
          newStream = FS.dupStream(stream, arg);
          return newStream.fd;
        }
        case 1:
        case 2:
          return 0;  // FD_CLOEXEC makes no sense for a single process.
        case 3:
          return stream.flags;
        case 4: {
          var arg = syscallGetVarargI();
          stream.flags |= arg;
          return 0;
        }
        case 12: {
          var arg = syscallGetVarargP();
          var offset = 0;
          // We're always unlocked.
          HEAP16[(((arg)+(offset))>>1)] = 2;
          return 0;
        }
        case 13:
        case 14:
          // Pretend that the locking is successful. These are process-level locks,
          // and Emscripten programs are a single process. If we supported linking a
          // filesystem between programs, we'd need to do more here.
          // See https://github.com/emscripten-core/emscripten/issues/23697
          return 0;
      }
      return -28;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  function ___syscall_fstat64(fd, buf) {
  try {
  
      return SYSCALLS.writeStat(buf, FS.fstat(fd));
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  
  var stringToUTF8 = (str, outPtr, maxBytesToWrite) => {
      assert(typeof maxBytesToWrite == 'number', 'stringToUTF8(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!');
      return stringToUTF8Array(str, HEAPU8, outPtr, maxBytesToWrite);
    };
  function ___syscall_getcwd(buf, size) {
  try {
  
      if (size === 0) return -28;
      var cwd = FS.cwd();
      var cwdLengthInBytes = lengthBytesUTF8(cwd) + 1;
      if (size < cwdLengthInBytes) return -68;
      stringToUTF8(cwd, buf, size);
      return cwdLengthInBytes;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  
  function ___syscall_ioctl(fd, op, varargs) {
  SYSCALLS.varargs = varargs;
  try {
  
      var stream = SYSCALLS.getStreamFromFD(fd);
      switch (op) {
        case 21509: {
          if (!stream.tty) return -59;
          return 0;
        }
        case 21505: {
          if (!stream.tty) return -59;
          if (stream.tty.ops.ioctl_tcgets) {
            var termios = stream.tty.ops.ioctl_tcgets(stream);
            var argp = syscallGetVarargP();
            HEAP32[((argp)>>2)] = termios.c_iflag || 0;
            HEAP32[(((argp)+(4))>>2)] = termios.c_oflag || 0;
            HEAP32[(((argp)+(8))>>2)] = termios.c_cflag || 0;
            HEAP32[(((argp)+(12))>>2)] = termios.c_lflag || 0;
            for (var i = 0; i < 32; i++) {
              HEAP8[(argp + i)+(17)] = termios.c_cc[i] || 0;
            }
            return 0;
          }
          return 0;
        }
        case 21510:
        case 21511:
        case 21512: {
          if (!stream.tty) return -59;
          return 0; // no-op, not actually adjusting terminal settings
        }
        case 21506:
        case 21507:
        case 21508: {
          if (!stream.tty) return -59;
          if (stream.tty.ops.ioctl_tcsets) {
            var argp = syscallGetVarargP();
            var c_iflag = HEAP32[((argp)>>2)];
            var c_oflag = HEAP32[(((argp)+(4))>>2)];
            var c_cflag = HEAP32[(((argp)+(8))>>2)];
            var c_lflag = HEAP32[(((argp)+(12))>>2)];
            var c_cc = []
            for (var i = 0; i < 32; i++) {
              c_cc.push(HEAP8[(argp + i)+(17)]);
            }
            return stream.tty.ops.ioctl_tcsets(stream.tty, op, { c_iflag, c_oflag, c_cflag, c_lflag, c_cc });
          }
          return 0; // no-op, not actually adjusting terminal settings
        }
        case 21519: {
          if (!stream.tty) return -59;
          var argp = syscallGetVarargP();
          HEAP32[((argp)>>2)] = 0;
          return 0;
        }
        case 21520: {
          if (!stream.tty) return -59;
          return -28; // not supported
        }
        case 21531: {
          var argp = syscallGetVarargP();
          return FS.ioctl(stream, op, argp);
        }
        case 21523: {
          // TODO: in theory we should write to the winsize struct that gets
          // passed in, but for now musl doesn't read anything on it
          if (!stream.tty) return -59;
          if (stream.tty.ops.ioctl_tiocgwinsz) {
            var winsize = stream.tty.ops.ioctl_tiocgwinsz(stream.tty);
            var argp = syscallGetVarargP();
            HEAP16[((argp)>>1)] = winsize[0];
            HEAP16[(((argp)+(2))>>1)] = winsize[1];
          }
          return 0;
        }
        case 21524: {
          // TODO: technically, this ioctl call should change the window size.
          // but, since emscripten doesn't have any concept of a terminal window
          // yet, we'll just silently throw it away as we do TIOCGWINSZ
          if (!stream.tty) return -59;
          return 0;
        }
        case 21515: {
          if (!stream.tty) return -59;
          return 0;
        }
        default: return -28; // not supported
      }
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  function ___syscall_lstat64(path, buf) {
  try {
  
      path = SYSCALLS.getStr(path);
      return SYSCALLS.writeStat(buf, FS.lstat(path));
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  function ___syscall_mkdirat(dirfd, path, mode) {
  try {
  
      path = SYSCALLS.getStr(path);
      path = SYSCALLS.calculateAt(dirfd, path);
      FS.mkdir(path, mode, 0);
      return 0;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  function ___syscall_newfstatat(dirfd, path, buf, flags) {
  try {
  
      path = SYSCALLS.getStr(path);
      var nofollow = flags & 256;
      var allowEmpty = flags & 4096;
      flags = flags & (~6400);
      assert(!flags, `unknown flags in __syscall_newfstatat: ${flags}`);
      path = SYSCALLS.calculateAt(dirfd, path, allowEmpty);
      return SYSCALLS.writeStat(buf, nofollow ? FS.lstat(path) : FS.stat(path));
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  
  function ___syscall_openat(dirfd, path, flags, varargs) {
  SYSCALLS.varargs = varargs;
  try {
  
      path = SYSCALLS.getStr(path);
      path = SYSCALLS.calculateAt(dirfd, path);
      var mode = varargs ? syscallGetVarargI() : 0;
      return FS.open(path, flags, mode).fd;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  
  
  function ___syscall_readlinkat(dirfd, path, buf, bufsize) {
  try {
  
      path = SYSCALLS.getStr(path);
      path = SYSCALLS.calculateAt(dirfd, path);
      if (bufsize <= 0) return -28;
      var ret = FS.readlink(path);
  
      var len = Math.min(bufsize, lengthBytesUTF8(ret));
      var endChar = HEAP8[buf+len];
      stringToUTF8(ret, buf, bufsize+1);
      // readlink is one of the rare functions that write out a C string, but does never append a null to the output buffer(!)
      // stringToUTF8() always appends a null byte, so restore the character under the null byte after the write.
      HEAP8[buf+len] = endChar;
      return len;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  function ___syscall_rmdir(path) {
  try {
  
      path = SYSCALLS.getStr(path);
      FS.rmdir(path);
      return 0;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  function ___syscall_stat64(path, buf) {
  try {
  
      path = SYSCALLS.getStr(path);
      return SYSCALLS.writeStat(buf, FS.stat(path));
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  function ___syscall_unlinkat(dirfd, path, flags) {
  try {
  
      path = SYSCALLS.getStr(path);
      path = SYSCALLS.calculateAt(dirfd, path);
      if (flags === 0) {
        FS.unlink(path);
      } else if (flags === 512) {
        FS.rmdir(path);
      } else {
        abort('Invalid flags passed to unlinkat');
      }
      return 0;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  }

  var __abort_js = () =>
      abort('native code called abort()');

  var __emscripten_throw_longjmp = () => {
      throw new EmscriptenSjLj;
    };

  
  
  
  
  
  var INT53_MAX = 9007199254740992;
  
  var INT53_MIN = -9007199254740992;
  var bigintToI53Checked = (num) => (num < INT53_MIN || num > INT53_MAX) ? NaN : Number(num);
  function __mmap_js(len, prot, flags, fd, offset, allocated, addr) {
    offset = bigintToI53Checked(offset);
  
  
  try {
  
      if (isNaN(offset)) return 61;
      var stream = SYSCALLS.getStreamFromFD(fd);
      var res = FS.mmap(stream, len, offset, prot, flags);
      var ptr = res.ptr;
      HEAP32[((allocated)>>2)] = res.allocated;
      HEAPU32[((addr)>>2)] = ptr;
      return 0;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  ;
  }

  
  function __munmap_js(addr, len, prot, flags, fd, offset) {
    offset = bigintToI53Checked(offset);
  
  
  try {
  
      var stream = SYSCALLS.getStreamFromFD(fd);
      if (prot & 2) {
        SYSCALLS.doMsync(addr, stream, len, flags, offset);
      }
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return -e.errno;
  }
  ;
  }

  
  var __tzset_js = (timezone, daylight, std_name, dst_name) => {
      // TODO: Use (malleable) environment variables instead of system settings.
      var currentYear = new Date().getFullYear();
      var winter = new Date(currentYear, 0, 1);
      var summer = new Date(currentYear, 6, 1);
      var winterOffset = winter.getTimezoneOffset();
      var summerOffset = summer.getTimezoneOffset();
  
      // Local standard timezone offset. Local standard time is not adjusted for
      // daylight savings.  This code uses the fact that getTimezoneOffset returns
      // a greater value during Standard Time versus Daylight Saving Time (DST).
      // Thus it determines the expected output during Standard Time, and it
      // compares whether the output of the given date the same (Standard) or less
      // (DST).
      var stdTimezoneOffset = Math.max(winterOffset, summerOffset);
  
      // timezone is specified as seconds west of UTC ("The external variable
      // `timezone` shall be set to the difference, in seconds, between
      // Coordinated Universal Time (UTC) and local standard time."), the same
      // as returned by stdTimezoneOffset.
      // See http://pubs.opengroup.org/onlinepubs/009695399/functions/tzset.html
      HEAPU32[((timezone)>>2)] = stdTimezoneOffset * 60;
  
      HEAP32[((daylight)>>2)] = Number(winterOffset != summerOffset);
  
      var extractZone = (timezoneOffset) => {
        // Why inverse sign?
        // Read here https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Date/getTimezoneOffset
        var sign = timezoneOffset >= 0 ? "-" : "+";
  
        var absOffset = Math.abs(timezoneOffset)
        var hours = String(Math.floor(absOffset / 60)).padStart(2, "0");
        var minutes = String(absOffset % 60).padStart(2, "0");
  
        return `UTC${sign}${hours}${minutes}`;
      }
  
      var winterName = extractZone(winterOffset);
      var summerName = extractZone(summerOffset);
      assert(winterName);
      assert(summerName);
      assert(lengthBytesUTF8(winterName) <= 16, `timezone name truncated to fit in TZNAME_MAX (${winterName})`);
      assert(lengthBytesUTF8(summerName) <= 16, `timezone name truncated to fit in TZNAME_MAX (${summerName})`);
      if (summerOffset < winterOffset) {
        // Northern hemisphere
        stringToUTF8(winterName, std_name, 17);
        stringToUTF8(summerName, dst_name, 17);
      } else {
        stringToUTF8(winterName, dst_name, 17);
        stringToUTF8(summerName, std_name, 17);
      }
    };

  
  var _emscripten_set_main_loop_timing = (mode, value) => {
      MainLoop.timingMode = mode;
      MainLoop.timingValue = value;
  
      if (!MainLoop.func) {
        err('emscripten_set_main_loop_timing: Cannot set timing mode for main loop since a main loop does not exist! Call emscripten_set_main_loop first to set one up.');
        return 1; // Return non-zero on failure, can't set timing mode when there is no main loop.
      }
  
      if (!MainLoop.running) {
        
        MainLoop.running = true;
      }
      if (mode == 0) {
        MainLoop.scheduler = function MainLoop_scheduler_setTimeout() {
          var timeUntilNextTick = Math.max(0, MainLoop.tickStartTime + value - _emscripten_get_now())|0;
          setTimeout(MainLoop.runner, timeUntilNextTick); // doing this each time means that on exception, we stop
        };
        MainLoop.method = 'timeout';
      } else if (mode == 1) {
        MainLoop.scheduler = function MainLoop_scheduler_rAF() {
          MainLoop.requestAnimationFrame(MainLoop.runner);
        };
        MainLoop.method = 'rAF';
      } else if (mode == 2) {
        if (typeof MainLoop.setImmediate == 'undefined') {
          if (typeof setImmediate == 'undefined') {
            // Emulate setImmediate. (note: not a complete polyfill, we don't emulate clearImmediate() to keep code size to minimum, since not needed)
            var setImmediates = [];
            var emscriptenMainLoopMessageId = 'setimmediate';
            /** @param {Event} event */
            var MainLoop_setImmediate_messageHandler = (event) => {
              // When called in current thread or Worker, the main loop ID is structured slightly different to accommodate for --proxy-to-worker runtime listening to Worker events,
              // so check for both cases.
              if (event.data === emscriptenMainLoopMessageId || event.data.target === emscriptenMainLoopMessageId) {
                event.stopPropagation();
                setImmediates.shift()();
              }
            };
            addEventListener("message", MainLoop_setImmediate_messageHandler, true);
            MainLoop.setImmediate = /** @type{function(function(): ?, ...?): number} */((func) => {
              setImmediates.push(func);
              if (ENVIRONMENT_IS_WORKER) {
                Module['setImmediates'] ??= [];
                Module['setImmediates'].push(func);
                postMessage({target: emscriptenMainLoopMessageId}); // In --proxy-to-worker, route the message via proxyClient.js
              } else postMessage(emscriptenMainLoopMessageId, "*"); // On the main thread, can just send the message to itself.
            });
          } else {
            MainLoop.setImmediate = setImmediate;
          }
        }
        MainLoop.scheduler = function MainLoop_scheduler_setImmediate() {
          MainLoop.setImmediate(MainLoop.runner);
        };
        MainLoop.method = 'immediate';
      }
      return 0;
    };
  
  var _emscripten_get_now = () => performance.now();
  
  
  var runtimeKeepaliveCounter = 0;
  var keepRuntimeAlive = () => noExitRuntime || runtimeKeepaliveCounter > 0;
  var _proc_exit = (code) => {
      EXITSTATUS = code;
      if (!keepRuntimeAlive()) {
        Module['onExit']?.(code);
        ABORT = true;
      }
      quit_(code, new ExitStatus(code));
    };
  
  
  /** @suppress {duplicate } */
  /** @param {boolean|number=} implicit */
  var exitJS = (status, implicit) => {
      EXITSTATUS = status;
  
      checkUnflushedContent();
  
      // if exit() was called explicitly, warn the user if the runtime isn't actually being shut down
      if (keepRuntimeAlive() && !implicit) {
        var msg = `program exited (with status: ${status}), but keepRuntimeAlive() is set (counter=${runtimeKeepaliveCounter}) due to an async operation, so halting execution but not exiting the runtime or preventing further async execution (you can use emscripten_force_exit, if you want to force a true shutdown)`;
        err(msg);
      }
  
      _proc_exit(status);
    };
  var _exit = exitJS;
  
  var handleException = (e) => {
      // Certain exception types we do not treat as errors since they are used for
      // internal control flow.
      // 1. ExitStatus, which is thrown by exit()
      // 2. "unwind", which is thrown by emscripten_unwind_to_js_event_loop() and others
      //    that wish to return to JS event loop.
      if (e instanceof ExitStatus || e == 'unwind') {
        return EXITSTATUS;
      }
      checkStackCookie();
      if (e instanceof WebAssembly.RuntimeError) {
        if (_emscripten_stack_get_current() <= 0) {
          err('Stack overflow detected.  You can try increasing -sSTACK_SIZE (currently set to 500000)');
        }
      }
      quit_(1, e);
    };
  
  var maybeExit = () => {
      if (!keepRuntimeAlive()) {
        try {
          _exit(EXITSTATUS);
        } catch (e) {
          handleException(e);
        }
      }
    };
  
    /**
     * @param {number=} arg
     * @param {boolean=} noSetTiming
     */
  var setMainLoop = (iterFunc, fps, simulateInfiniteLoop, arg, noSetTiming) => {
      assert(!MainLoop.func, 'emscripten_set_main_loop: there can only be one main loop function at once: call emscripten_cancel_main_loop to cancel the previous one before setting a new one with different parameters.');
      MainLoop.func = iterFunc;
      MainLoop.arg = arg;
  
      var thisMainLoopId = MainLoop.currentlyRunningMainloop;
      function checkIsRunning() {
        if (thisMainLoopId < MainLoop.currentlyRunningMainloop) {
          
          maybeExit();
          return false;
        }
        return true;
      }
  
      // We create the loop runner here but it is not actually running until
      // _emscripten_set_main_loop_timing is called (which might happen a
      // later time).  This member signifies that the current runner has not
      // yet been started so that we can call runtimeKeepalivePush when it
      // gets it timing set for the first time.
      MainLoop.running = false;
      MainLoop.runner = function MainLoop_runner() {
        if (ABORT) return;
        if (MainLoop.queue.length > 0) {
          var start = Date.now();
          var blocker = MainLoop.queue.shift();
          blocker.func(blocker.arg);
          if (MainLoop.remainingBlockers) {
            var remaining = MainLoop.remainingBlockers;
            var next = remaining%1 == 0 ? remaining-1 : Math.floor(remaining);
            if (blocker.counted) {
              MainLoop.remainingBlockers = next;
            } else {
              // not counted, but move the progress along a tiny bit
              next = next + 0.5; // do not steal all the next one's progress
              MainLoop.remainingBlockers = (8*remaining + next)/9;
            }
          }
          MainLoop.updateStatus();
  
          // catches pause/resume main loop from blocker execution
          if (!checkIsRunning()) return;
  
          setTimeout(MainLoop.runner, 0);
          return;
        }
  
        // catch pauses from non-main loop sources
        if (!checkIsRunning()) return;
  
        // Implement very basic swap interval control
        MainLoop.currentFrameNumber = MainLoop.currentFrameNumber + 1 | 0;
        if (MainLoop.timingMode == 1 && MainLoop.timingValue > 1 && MainLoop.currentFrameNumber % MainLoop.timingValue != 0) {
          // Not the scheduled time to render this frame - skip.
          MainLoop.scheduler();
          return;
        } else if (MainLoop.timingMode == 0) {
          MainLoop.tickStartTime = _emscripten_get_now();
        }
  
        if (MainLoop.method === 'timeout' && Module['ctx']) {
          warnOnce('Looks like you are rendering without using requestAnimationFrame for the main loop. You should use 0 for the frame rate in emscripten_set_main_loop in order to use requestAnimationFrame, as that can greatly improve your frame rates!');
          MainLoop.method = ''; // just warn once per call to set main loop
        }
  
        MainLoop.runIter(iterFunc);
  
        // catch pauses from the main loop itself
        if (!checkIsRunning()) return;
  
        MainLoop.scheduler();
      }
  
      if (!noSetTiming) {
        if (fps > 0) {
          _emscripten_set_main_loop_timing(0, 1000.0 / fps);
        } else {
          // Do rAF by rendering each frame (no decimating)
          _emscripten_set_main_loop_timing(1, 1);
        }
  
        MainLoop.scheduler();
      }
  
      if (simulateInfiniteLoop) {
        throw 'unwind';
      }
    };
  
  
  var callUserCallback = (func) => {
      if (ABORT) {
        err('user callback triggered after runtime exited or application aborted.  Ignoring.');
        return;
      }
      try {
        func();
        maybeExit();
      } catch (e) {
        handleException(e);
      }
    };
  
  var MainLoop = {
  running:false,
  scheduler:null,
  method:"",
  currentlyRunningMainloop:0,
  func:null,
  arg:0,
  timingMode:0,
  timingValue:0,
  currentFrameNumber:0,
  queue:[],
  preMainLoop:[],
  postMainLoop:[],
  pause() {
        MainLoop.scheduler = null;
        // Incrementing this signals the previous main loop that it's now become old, and it must return.
        MainLoop.currentlyRunningMainloop++;
      },
  resume() {
        MainLoop.currentlyRunningMainloop++;
        var timingMode = MainLoop.timingMode;
        var timingValue = MainLoop.timingValue;
        var func = MainLoop.func;
        MainLoop.func = null;
        // do not set timing and call scheduler, we will do it on the next lines
        setMainLoop(func, 0, false, MainLoop.arg, true);
        _emscripten_set_main_loop_timing(timingMode, timingValue);
        MainLoop.scheduler();
      },
  updateStatus() {
        if (Module['setStatus']) {
          var message = Module['statusMessage'] || 'Please wait...';
          var remaining = MainLoop.remainingBlockers ?? 0;
          var expected = MainLoop.expectedBlockers ?? 0;
          if (remaining) {
            if (remaining < expected) {
              Module['setStatus'](`{message} ({expected - remaining}/{expected})`);
            } else {
              Module['setStatus'](message);
            }
          } else {
            Module['setStatus']('');
          }
        }
      },
  init() {
        Module['preMainLoop'] && MainLoop.preMainLoop.push(Module['preMainLoop']);
        Module['postMainLoop'] && MainLoop.postMainLoop.push(Module['postMainLoop']);
      },
  runIter(func) {
        if (ABORT) return;
        for (var pre of MainLoop.preMainLoop) {
          if (pre() === false) {
            return; // |return false| skips a frame
          }
        }
        callUserCallback(func);
        for (var post of MainLoop.postMainLoop) {
          post();
        }
        checkStackCookie();
      },
  nextRAF:0,
  fakeRequestAnimationFrame(func) {
        // try to keep 60fps between calls to here
        var now = Date.now();
        if (MainLoop.nextRAF === 0) {
          MainLoop.nextRAF = now + 1000/60;
        } else {
          while (now + 2 >= MainLoop.nextRAF) { // fudge a little, to avoid timer jitter causing us to do lots of delay:0
            MainLoop.nextRAF += 1000/60;
          }
        }
        var delay = Math.max(MainLoop.nextRAF - now, 0);
        setTimeout(func, delay);
      },
  requestAnimationFrame(func) {
        if (typeof requestAnimationFrame == 'function') {
          requestAnimationFrame(func);
          return;
        }
        var RAF = MainLoop.fakeRequestAnimationFrame;
        RAF(func);
      },
  };
  
  var AL = {
  QUEUE_INTERVAL:25,
  QUEUE_LOOKAHEAD:0.1,
  DEVICE_NAME:"Emscripten OpenAL",
  CAPTURE_DEVICE_NAME:"Emscripten OpenAL capture",
  ALC_EXTENSIONS:{
  ALC_SOFT_pause_device:true,
  ALC_SOFT_HRTF:true,
  },
  AL_EXTENSIONS:{
  AL_EXT_float32:true,
  AL_SOFT_loop_points:true,
  AL_SOFT_source_length:true,
  AL_EXT_source_distance_model:true,
  AL_SOFT_source_spatialize:true,
  },
  _alcErr:0,
  alcErr:0,
  deviceRefCounts:{
  },
  alcStringCache:{
  },
  paused:false,
  stringCache:{
  },
  contexts:{
  },
  currentCtx:null,
  buffers:{
  0:{
  id:0,
  refCount:0,
  audioBuf:null,
  frequency:0,
  bytesPerSample:2,
  channels:1,
  length:0,
  },
  },
  paramArray:[],
  _nextId:1,
  newId:() => AL.freeIds.length > 0 ? AL.freeIds.pop() : AL._nextId++,
  freeIds:[],
  scheduleContextAudio:(ctx) => {
        // If we are animating using the requestAnimationFrame method, then the main loop does not run when in the background.
        // To give a perfect glitch-free audio stop when switching from foreground to background, we need to avoid updating
        // audio altogether when in the background, so detect that case and kill audio buffer streaming if so.
        if (MainLoop.timingMode === 1 && document['visibilityState'] != 'visible') {
          return;
        }
  
        for (var i in ctx.sources) {
          AL.scheduleSourceAudio(ctx.sources[i]);
        }
      },
  scheduleSourceAudio:(src, lookahead) => {
        // See comment on scheduleContextAudio above.
        if (MainLoop.timingMode === 1 && document['visibilityState'] != 'visible') {
          return;
        }
        if (src.state !== 4114) {
          return;
        }
  
        var currentTime = AL.updateSourceTime(src);
  
        var startTime = src.bufStartTime;
        var startOffset = src.bufOffset;
        var bufCursor = src.bufsProcessed;
  
        // Advance past any audio that is already scheduled
        for (var i = 0; i < src.audioQueue.length; i++) {
          var audioSrc = src.audioQueue[i];
          startTime = audioSrc._startTime + audioSrc._duration;
          startOffset = 0.0;
          bufCursor += audioSrc._skipCount + 1;
        }
  
        if (!lookahead) {
          lookahead = AL.QUEUE_LOOKAHEAD;
        }
        var lookaheadTime = currentTime + lookahead;
        var skipCount = 0;
        while (startTime < lookaheadTime) {
          if (bufCursor >= src.bufQueue.length) {
            if (src.looping) {
              bufCursor %= src.bufQueue.length;
            } else {
              break;
            }
          }
  
          var buf = src.bufQueue[bufCursor % src.bufQueue.length];
          // If the buffer contains no data, skip it
          if (buf.length === 0) {
            skipCount++;
            // If we've gone through the whole queue and everything is 0 length, just give up
            if (skipCount === src.bufQueue.length) {
              break;
            }
          } else {
            var audioSrc = src.context.audioCtx.createBufferSource();
            audioSrc.buffer = buf.audioBuf;
            audioSrc.playbackRate.value = src.playbackRate;
            if (buf.audioBuf._loopStart || buf.audioBuf._loopEnd) {
              audioSrc.loopStart = buf.audioBuf._loopStart;
              audioSrc.loopEnd = buf.audioBuf._loopEnd;
            }
  
            var duration = 0.0;
            // If the source is a looping static buffer, use native looping for gapless playback
            if (src.type === 4136 && src.looping) {
              duration = Number.POSITIVE_INFINITY;
              audioSrc.loop = true;
              if (buf.audioBuf._loopStart) {
                audioSrc.loopStart = buf.audioBuf._loopStart;
              }
              if (buf.audioBuf._loopEnd) {
                audioSrc.loopEnd = buf.audioBuf._loopEnd;
              }
            } else {
              duration = (buf.audioBuf.duration - startOffset) / src.playbackRate;
            }
  
            audioSrc._startOffset = startOffset;
            audioSrc._duration = duration;
            audioSrc._skipCount = skipCount;
            skipCount = 0;
  
            audioSrc.connect(src.gain);
  
            if (typeof audioSrc.start != 'undefined') {
              // Sample the current time as late as possible to mitigate drift
              startTime = Math.max(startTime, src.context.audioCtx.currentTime);
              audioSrc.start(startTime, startOffset);
            } else if (typeof audioSrc.noteOn != 'undefined') {
              startTime = Math.max(startTime, src.context.audioCtx.currentTime);
              audioSrc.noteOn(startTime);
            }
            audioSrc._startTime = startTime;
            src.audioQueue.push(audioSrc);
  
            startTime += duration;
          }
  
          startOffset = 0.0;
          bufCursor++;
        }
      },
  updateSourceTime:(src) => {
        var currentTime = src.context.audioCtx.currentTime;
        if (src.state !== 4114) {
          return currentTime;
        }
  
        // if the start time is unset, determine it based on the current offset.
        // This will be the case when a source is resumed after being paused, and
        // allows us to pretend that the source actually started playing some time
        // in the past such that it would just now have reached the stored offset.
        if (!isFinite(src.bufStartTime)) {
          src.bufStartTime = currentTime - src.bufOffset / src.playbackRate;
          src.bufOffset = 0.0;
        }
  
        var nextStartTime = 0.0;
        while (src.audioQueue.length) {
          var audioSrc = src.audioQueue[0];
          src.bufsProcessed += audioSrc._skipCount;
          nextStartTime = audioSrc._startTime + audioSrc._duration; // n.b. audioSrc._duration already factors in playbackRate, so no divide by src.playbackRate on it.
  
          if (currentTime < nextStartTime) {
            break;
          }
  
          src.audioQueue.shift();
          src.bufStartTime = nextStartTime;
          src.bufOffset = 0.0;
          src.bufsProcessed++;
        }
  
        if (src.bufsProcessed >= src.bufQueue.length && !src.looping) {
          // The source has played its entire queue and is non-looping, so just mark it as stopped.
          AL.setSourceState(src, 4116);
        } else if (src.type === 4136 && src.looping) {
          // If the source is a looping static buffer, determine the buffer offset based on the loop points
          var buf = src.bufQueue[0];
          if (buf.length === 0) {
            src.bufOffset = 0.0;
          } else {
            var delta = (currentTime - src.bufStartTime) * src.playbackRate;
            var loopStart = buf.audioBuf._loopStart || 0.0;
            var loopEnd = buf.audioBuf._loopEnd || buf.audioBuf.duration;
            if (loopEnd <= loopStart) {
              loopEnd = buf.audioBuf.duration;
            }
  
            if (delta < loopEnd) {
              src.bufOffset = delta;
            } else {
              src.bufOffset = loopStart + (delta - loopStart) % (loopEnd - loopStart);
            }
          }
        } else if (src.audioQueue[0]) {
          // The source is still actively playing, so we just need to calculate where we are in the current buffer
          // so it can be remembered if the source gets paused.
          src.bufOffset = (currentTime - src.audioQueue[0]._startTime) * src.playbackRate;
        } else {
          // The source hasn't finished yet, but there is no scheduled audio left for it. This can be because
          // the source has just been started/resumed, or due to an underrun caused by a long blocking operation.
          // We need to determine what state we would be in by this point in time so that when we next schedule
          // audio playback, it will be just as if no underrun occurred.
  
          if (src.type !== 4136 && src.looping) {
            // if the source is a looping buffer queue, let's first calculate the queue duration, so we can
            // quickly fast forward past any full loops of the queue and only worry about the remainder.
            var srcDuration = AL.sourceDuration(src) / src.playbackRate;
            if (srcDuration > 0.0) {
              src.bufStartTime += Math.floor((currentTime - src.bufStartTime) / srcDuration) * srcDuration;
            }
          }
  
          // Since we've already skipped any full-queue loops if there were any, we just need to find
          // out where in the queue the remaining time puts us, which won't require stepping through the
          // entire queue more than once.
          for (var i = 0; i < src.bufQueue.length; i++) {
            if (src.bufsProcessed >= src.bufQueue.length) {
              if (src.looping) {
                src.bufsProcessed %= src.bufQueue.length;
              } else {
                AL.setSourceState(src, 4116);
                break;
              }
            }
  
            var buf = src.bufQueue[src.bufsProcessed];
            if (buf.length > 0) {
              nextStartTime = src.bufStartTime + buf.audioBuf.duration / src.playbackRate;
  
              if (currentTime < nextStartTime) {
                src.bufOffset = (currentTime - src.bufStartTime) * src.playbackRate;
                break;
              }
  
              src.bufStartTime = nextStartTime;
            }
  
            src.bufOffset = 0.0;
            src.bufsProcessed++;
          }
        }
  
        return currentTime;
      },
  cancelPendingSourceAudio:(src) => {
        AL.updateSourceTime(src);
  
        for (var i = 1; i < src.audioQueue.length; i++) {
          var audioSrc = src.audioQueue[i];
          audioSrc.stop();
        }
  
        if (src.audioQueue.length > 1) {
          src.audioQueue.length = 1;
        }
      },
  stopSourceAudio:(src) => {
        for (var i = 0; i < src.audioQueue.length; i++) {
          src.audioQueue[i].stop();
        }
        src.audioQueue.length = 0;
      },
  setSourceState:(src, state) => {
        if (state === 4114) {
          if (src.state === 4114 || src.state == 4116) {
            src.bufsProcessed = 0;
            src.bufOffset = 0.0;
          } else {
          }
  
          AL.stopSourceAudio(src);
  
          src.state = 4114;
          src.bufStartTime = Number.NEGATIVE_INFINITY;
          AL.scheduleSourceAudio(src);
        } else if (state === 4115) {
          if (src.state === 4114) {
            // Store off the current offset to restore with on resume.
            AL.updateSourceTime(src);
            AL.stopSourceAudio(src);
  
            src.state = 4115;
          }
        } else if (state === 4116) {
          if (src.state !== 4113) {
            src.state = 4116;
            src.bufsProcessed = src.bufQueue.length;
            src.bufStartTime = Number.NEGATIVE_INFINITY;
            src.bufOffset = 0.0;
            AL.stopSourceAudio(src);
          }
        } else if (state === 4113) {
          if (src.state !== 4113) {
            src.state = 4113;
            src.bufsProcessed = 0;
            src.bufStartTime = Number.NEGATIVE_INFINITY;
            src.bufOffset = 0.0;
            AL.stopSourceAudio(src);
          }
        }
      },
  initSourcePanner:(src) => {
        if (src.type === 0x1030 /* AL_UNDETERMINED */) {
          return;
        }
  
        // Find the first non-zero buffer in the queue to determine the proper format
        var templateBuf = AL.buffers[0];
        for (var i = 0; i < src.bufQueue.length; i++) {
          if (src.bufQueue[i].id !== 0) {
            templateBuf = src.bufQueue[i];
            break;
          }
        }
        // Create a panner if AL_SOURCE_SPATIALIZE_SOFT is set to true, or alternatively if it's set to auto and the source is mono
        if (src.spatialize === 1 || (src.spatialize === 2 /* AL_AUTO_SOFT */ && templateBuf.channels === 1)) {
          if (src.panner) {
            return;
          }
          src.panner = src.context.audioCtx.createPanner();
  
          AL.updateSourceGlobal(src);
          AL.updateSourceSpace(src);
  
          src.panner.connect(src.context.gain);
          src.gain.disconnect();
          src.gain.connect(src.panner);
        } else {
          if (!src.panner) {
            return;
          }
  
          src.panner.disconnect();
          src.gain.disconnect();
          src.gain.connect(src.context.gain);
          src.panner = null;
        }
      },
  updateContextGlobal:(ctx) => {
        for (var i in ctx.sources) {
          AL.updateSourceGlobal(ctx.sources[i]);
        }
      },
  updateSourceGlobal:(src) => {
        var panner = src.panner;
        if (!panner) {
          return;
        }
  
        panner.refDistance = src.refDistance;
        panner.maxDistance = src.maxDistance;
        panner.rolloffFactor = src.rolloffFactor;
  
        panner.panningModel = src.context.hrtf ? 'HRTF' : 'equalpower';
  
        // Use the source's distance model if AL_SOURCE_DISTANCE_MODEL is enabled
        var distanceModel = src.context.sourceDistanceModel ? src.distanceModel : src.context.distanceModel;
        switch (distanceModel) {
        case 0:
          panner.distanceModel = 'inverse';
          panner.refDistance = 3.40282e38 /* FLT_MAX */;
          break;
        case 0xd001 /* AL_INVERSE_DISTANCE */:
        case 0xd002 /* AL_INVERSE_DISTANCE_CLAMPED */:
          panner.distanceModel = 'inverse';
          break;
        case 0xd003 /* AL_LINEAR_DISTANCE */:
        case 0xd004 /* AL_LINEAR_DISTANCE_CLAMPED */:
          panner.distanceModel = 'linear';
          break;
        case 0xd005 /* AL_EXPONENT_DISTANCE */:
        case 0xd006 /* AL_EXPONENT_DISTANCE_CLAMPED */:
          panner.distanceModel = 'exponential';
          break;
        }
      },
  updateListenerSpace:(ctx) => {
        var listener = ctx.audioCtx.listener;
        if (listener.positionX) {
          listener.positionX.value = ctx.listener.position[0];
          listener.positionY.value = ctx.listener.position[1];
          listener.positionZ.value = ctx.listener.position[2];
        } else {
          listener.setPosition(ctx.listener.position[0], ctx.listener.position[1], ctx.listener.position[2]);
        }
        if (listener.forwardX) {
          listener.forwardX.value = ctx.listener.direction[0];
          listener.forwardY.value = ctx.listener.direction[1];
          listener.forwardZ.value = ctx.listener.direction[2];
          listener.upX.value = ctx.listener.up[0];
          listener.upY.value = ctx.listener.up[1];
          listener.upZ.value = ctx.listener.up[2];
        } else {
          listener.setOrientation(
            ctx.listener.direction[0], ctx.listener.direction[1], ctx.listener.direction[2],
            ctx.listener.up[0], ctx.listener.up[1], ctx.listener.up[2]);
        }
  
        // Update sources that are relative to the listener
        for (var i in ctx.sources) {
          AL.updateSourceSpace(ctx.sources[i]);
        }
      },
  updateSourceSpace:(src) => {
        if (!src.panner) {
          return;
        }
        var panner = src.panner;
  
        var posX = src.position[0];
        var posY = src.position[1];
        var posZ = src.position[2];
        var dirX = src.direction[0];
        var dirY = src.direction[1];
        var dirZ = src.direction[2];
  
        var listener = src.context.listener;
        var lPosX = listener.position[0];
        var lPosY = listener.position[1];
        var lPosZ = listener.position[2];
  
        // WebAudio does spatialization in world-space coordinates, meaning both the buffer sources and
        // the listener position are in the same absolute coordinate system relative to a fixed origin.
        // By default, OpenAL works this way as well, but it also provides a "listener relative" mode, where
        // a buffer source's coordinate are interpreted not in absolute world space, but as being relative
        // to the listener object itself, so as the listener moves the source appears to move with it
        // with no update required. Since web audio does not support this mode, we must transform the source
        // coordinates from listener-relative space to absolute world space.
        //
        // We do this via affine transformation matrices applied to the source position and source direction.
        // A change-of-basis converts from listener-space displacements to world-space displacements,
        // which must be done for both the source position and direction. Lastly, the source position must be
        // added to the listener position to get the final source position, since the source position represents
        // a displacement from the listener.
        if (src.relative) {
          // Negate the listener direction since forward is -Z.
          var lBackX = -listener.direction[0];
          var lBackY = -listener.direction[1];
          var lBackZ = -listener.direction[2];
          var lUpX = listener.up[0];
          var lUpY = listener.up[1];
          var lUpZ = listener.up[2];
  
          var inverseMagnitude = (x, y, z) => {
            var length = Math.sqrt(x * x + y * y + z * z);
  
            if (length < Number.EPSILON) {
              return 0.0;
            }
  
            return 1.0 / length;
          };
  
          // Normalize the Back vector
          var invMag = inverseMagnitude(lBackX, lBackY, lBackZ);
          lBackX *= invMag;
          lBackY *= invMag;
          lBackZ *= invMag;
  
          // ...and the Up vector
          invMag = inverseMagnitude(lUpX, lUpY, lUpZ);
          lUpX *= invMag;
          lUpY *= invMag;
          lUpZ *= invMag;
  
          // Calculate the Right vector as the cross product of the Up and Back vectors
          var lRightX = (lUpY * lBackZ - lUpZ * lBackY);
          var lRightY = (lUpZ * lBackX - lUpX * lBackZ);
          var lRightZ = (lUpX * lBackY - lUpY * lBackX);
  
          // Back and Up might not be exactly perpendicular, so the cross product also needs normalization
          invMag = inverseMagnitude(lRightX, lRightY, lRightZ);
          lRightX *= invMag;
          lRightY *= invMag;
          lRightZ *= invMag;
  
          // Recompute Up from the now orthonormal Right and Back vectors so we have a fully orthonormal basis
          lUpX = (lBackY * lRightZ - lBackZ * lRightY);
          lUpY = (lBackZ * lRightX - lBackX * lRightZ);
          lUpZ = (lBackX * lRightY - lBackY * lRightX);
  
          var oldX = dirX;
          var oldY = dirY;
          var oldZ = dirZ;
  
          // Use our 3 vectors to apply a change-of-basis matrix to the source direction
          dirX = oldX * lRightX + oldY * lUpX + oldZ * lBackX;
          dirY = oldX * lRightY + oldY * lUpY + oldZ * lBackY;
          dirZ = oldX * lRightZ + oldY * lUpZ + oldZ * lBackZ;
  
          oldX = posX;
          oldY = posY;
          oldZ = posZ;
  
          // ...and to the source position
          posX = oldX * lRightX + oldY * lUpX + oldZ * lBackX;
          posY = oldX * lRightY + oldY * lUpY + oldZ * lBackY;
          posZ = oldX * lRightZ + oldY * lUpZ + oldZ * lBackZ;
  
          // The change-of-basis corrects the orientation, but the origin is still the listener.
          // Translate the source position by the listener position to finish.
          posX += lPosX;
          posY += lPosY;
          posZ += lPosZ;
        }
  
        if (panner.positionX) {
          // Assigning to panner.positionX/Y/Z unnecessarily seems to cause performance issues
          // See https://github.com/emscripten-core/emscripten/issues/15847
  
          if (posX != panner.positionX.value) panner.positionX.value = posX;
          if (posY != panner.positionY.value) panner.positionY.value = posY;
          if (posZ != panner.positionZ.value) panner.positionZ.value = posZ;
        } else {
          panner.setPosition(posX, posY, posZ);
        }
        if (panner.orientationX) {
          // Assigning to panner.orientation/Y/Z unnecessarily seems to cause performance issues
          // See https://github.com/emscripten-core/emscripten/issues/15847
  
          if (dirX != panner.orientationX.value) panner.orientationX.value = dirX;
          if (dirY != panner.orientationY.value) panner.orientationY.value = dirY;
          if (dirZ != panner.orientationZ.value) panner.orientationZ.value = dirZ;
        } else {
          panner.setOrientation(dirX, dirY, dirZ);
        }
  
        var oldShift = src.dopplerShift;
        var velX = src.velocity[0];
        var velY = src.velocity[1];
        var velZ = src.velocity[2];
        var lVelX = listener.velocity[0];
        var lVelY = listener.velocity[1];
        var lVelZ = listener.velocity[2];
        if (posX === lPosX && posY === lPosY && posZ === lPosZ
          || velX === lVelX && velY === lVelY && velZ === lVelZ)
        {
          src.dopplerShift = 1.0;
        } else {
          // Doppler algorithm from 1.1 spec
          var speedOfSound = src.context.speedOfSound;
          var dopplerFactor = src.context.dopplerFactor;
  
          var slX = lPosX - posX;
          var slY = lPosY - posY;
          var slZ = lPosZ - posZ;
  
          var magSl = Math.sqrt(slX * slX + slY * slY + slZ * slZ);
          var vls = (slX * lVelX + slY * lVelY + slZ * lVelZ) / magSl;
          var vss = (slX * velX + slY * velY + slZ * velZ) / magSl;
  
          vls = Math.min(vls, speedOfSound / dopplerFactor);
          vss = Math.min(vss, speedOfSound / dopplerFactor);
  
          src.dopplerShift = (speedOfSound - dopplerFactor * vls) / (speedOfSound - dopplerFactor * vss);
        }
        if (src.dopplerShift !== oldShift) {
          AL.updateSourceRate(src);
        }
      },
  updateSourceRate:(src) => {
        if (src.state === 4114) {
          // clear scheduled buffers
          AL.cancelPendingSourceAudio(src);
  
          var audioSrc = src.audioQueue[0];
          if (!audioSrc) {
            return; // It is possible that AL.scheduleContextAudio() has not yet fed the next buffer, if so, skip.
          }
  
          var duration;
          if (src.type === 4136 && src.looping) {
            duration = Number.POSITIVE_INFINITY;
          } else {
            // audioSrc._duration is expressed after factoring in playbackRate, so when changing playback rate, need
            // to recompute/rescale the rate to the new playback speed.
            duration = (audioSrc.buffer.duration - audioSrc._startOffset) / src.playbackRate;
          }
  
          audioSrc._duration = duration;
          audioSrc.playbackRate.value = src.playbackRate;
  
          // reschedule buffers with the new playbackRate
          AL.scheduleSourceAudio(src);
        }
      },
  sourceDuration:(src) => {
        var length = 0.0;
        for (var i = 0; i < src.bufQueue.length; i++) {
          var audioBuf = src.bufQueue[i].audioBuf;
          length += audioBuf ? audioBuf.duration : 0.0;
        }
        return length;
      },
  sourceTell:(src) => {
        AL.updateSourceTime(src);
  
        var offset = 0.0;
        for (var i = 0; i < src.bufsProcessed; i++) {
          if (src.bufQueue[i].audioBuf) {
            offset += src.bufQueue[i].audioBuf.duration;
          }
        }
        offset += src.bufOffset;
  
        return offset;
      },
  sourceSeek:(src, offset) => {
        var playing = src.state == 4114;
        if (playing) {
          AL.setSourceState(src, 4113);
        }
  
        if (src.bufQueue[src.bufsProcessed].audioBuf !== null) {
          src.bufsProcessed = 0;
          while (offset > src.bufQueue[src.bufsProcessed].audioBuf.duration) {
            offset -= src.bufQueue[src.bufsProcessed].audioBuf.duration;
            src.bufsProcessed++;
          }
  
          src.bufOffset = offset;
        }
  
        if (playing) {
          AL.setSourceState(src, 4114);
        }
      },
  getGlobalParam:(funcname, param) => {
        if (!AL.currentCtx) {
          return null;
        }
  
        switch (param) {
        case 49152:
          return AL.currentCtx.dopplerFactor;
        case 49155:
          return AL.currentCtx.speedOfSound;
        case 53248:
          return AL.currentCtx.distanceModel;
        default:
          AL.currentCtx.err = 40962;
          return null;
        }
      },
  setGlobalParam:(funcname, param, value) => {
        if (!AL.currentCtx) {
          return;
        }
  
        switch (param) {
        case 49152:
          if (!Number.isFinite(value) || value < 0.0) { // Strictly negative values are disallowed
            AL.currentCtx.err = 40963;
            return;
          }
  
          AL.currentCtx.dopplerFactor = value;
          AL.updateListenerSpace(AL.currentCtx);
          break;
        case 49155:
          if (!Number.isFinite(value) || value <= 0.0) { // Negative or zero values are disallowed
            AL.currentCtx.err = 40963;
            return;
          }
  
          AL.currentCtx.speedOfSound = value;
          AL.updateListenerSpace(AL.currentCtx);
          break;
        case 53248:
          switch (value) {
          case 0:
          case 0xd001 /* AL_INVERSE_DISTANCE */:
          case 0xd002 /* AL_INVERSE_DISTANCE_CLAMPED */:
          case 0xd003 /* AL_LINEAR_DISTANCE */:
          case 0xd004 /* AL_LINEAR_DISTANCE_CLAMPED */:
          case 0xd005 /* AL_EXPONENT_DISTANCE */:
          case 0xd006 /* AL_EXPONENT_DISTANCE_CLAMPED */:
            AL.currentCtx.distanceModel = value;
            AL.updateContextGlobal(AL.currentCtx);
            break;
          default:
            AL.currentCtx.err = 40963;
            return;
          }
          break;
        default:
          AL.currentCtx.err = 40962;
          return;
        }
      },
  getListenerParam:(funcname, param) => {
        if (!AL.currentCtx) {
          return null;
        }
  
        switch (param) {
        case 4100:
          return AL.currentCtx.listener.position;
        case 4102:
          return AL.currentCtx.listener.velocity;
        case 4111:
          return AL.currentCtx.listener.direction.concat(AL.currentCtx.listener.up);
        case 4106:
          return AL.currentCtx.gain.gain.value;
        default:
          AL.currentCtx.err = 40962;
          return null;
        }
      },
  setListenerParam:(funcname, param, value) => {
        if (!AL.currentCtx) {
          return;
        }
        if (value === null) {
          AL.currentCtx.err = 40962;
          return;
        }
  
        var listener = AL.currentCtx.listener;
        switch (param) {
        case 4100:
          if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          listener.position[0] = value[0];
          listener.position[1] = value[1];
          listener.position[2] = value[2];
          AL.updateListenerSpace(AL.currentCtx);
          break;
        case 4102:
          if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          listener.velocity[0] = value[0];
          listener.velocity[1] = value[1];
          listener.velocity[2] = value[2];
          AL.updateListenerSpace(AL.currentCtx);
          break;
        case 4106:
          if (!Number.isFinite(value) || value < 0.0) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          AL.currentCtx.gain.gain.value = value;
          break;
        case 4111:
          if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])
            || !Number.isFinite(value[3]) || !Number.isFinite(value[4]) || !Number.isFinite(value[5])
          ) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          listener.direction[0] = value[0];
          listener.direction[1] = value[1];
          listener.direction[2] = value[2];
          listener.up[0] = value[3];
          listener.up[1] = value[4];
          listener.up[2] = value[5];
          AL.updateListenerSpace(AL.currentCtx);
          break;
        default:
          AL.currentCtx.err = 40962;
          return;
        }
      },
  getBufferParam:(funcname, bufferId, param) => {
        if (!AL.currentCtx) {
          return;
        }
        var buf = AL.buffers[bufferId];
        if (!buf || bufferId === 0) {
          AL.currentCtx.err = 40961;
          return;
        }
  
        switch (param) {
        case 0x2001 /* AL_FREQUENCY */:
          return buf.frequency;
        case 0x2002 /* AL_BITS */:
          return buf.bytesPerSample * 8;
        case 0x2003 /* AL_CHANNELS */:
          return buf.channels;
        case 0x2004 /* AL_SIZE */:
          return buf.length * buf.bytesPerSample * buf.channels;
        case 0x2015 /* AL_LOOP_POINTS_SOFT */:
          if (buf.length === 0) {
            return [0, 0];
          }
          return [
            (buf.audioBuf._loopStart || 0.0) * buf.frequency,
            (buf.audioBuf._loopEnd || buf.length) * buf.frequency
          ];
        default:
          AL.currentCtx.err = 40962;
          return null;
        }
      },
  setBufferParam:(funcname, bufferId, param, value) => {
        if (!AL.currentCtx) {
          return;
        }
        var buf = AL.buffers[bufferId];
        if (!buf || bufferId === 0) {
          AL.currentCtx.err = 40961;
          return;
        }
        if (value === null) {
          AL.currentCtx.err = 40962;
          return;
        }
  
        switch (param) {
        case 0x2004 /* AL_SIZE */:
          if (value !== 0) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          // Per the spec, setting AL_SIZE to 0 is a legal NOP.
          break;
        case 0x2015 /* AL_LOOP_POINTS_SOFT */:
          if (value[0] < 0 || value[0] > buf.length || value[1] < 0 || value[1] > buf.Length || value[0] >= value[1]) {
            AL.currentCtx.err = 40963;
            return;
          }
          if (buf.refCount > 0) {
            AL.currentCtx.err = 40964;
            return;
          }
  
          if (buf.audioBuf) {
            buf.audioBuf._loopStart = value[0] / buf.frequency;
            buf.audioBuf._loopEnd = value[1] / buf.frequency;
          }
          break;
        default:
          AL.currentCtx.err = 40962;
          return;
        }
      },
  getSourceParam:(funcname, sourceId, param) => {
        if (!AL.currentCtx) {
          return null;
        }
        var src = AL.currentCtx.sources[sourceId];
        if (!src) {
          AL.currentCtx.err = 40961;
          return null;
        }
  
        switch (param) {
        case 0x202 /* AL_SOURCE_RELATIVE */:
          return src.relative;
        case 0x1001 /* AL_CONE_INNER_ANGLE */:
          return src.coneInnerAngle;
        case 0x1002 /* AL_CONE_OUTER_ANGLE */:
          return src.coneOuterAngle;
        case 0x1003 /* AL_PITCH */:
          return src.pitch;
        case 4100:
          return src.position;
        case 4101:
          return src.direction;
        case 4102:
          return src.velocity;
        case 0x1007 /* AL_LOOPING */:
          return src.looping;
        case 0x1009 /* AL_BUFFER */:
          if (src.type === 4136) {
            return src.bufQueue[0].id;
          }
          return 0;
        case 4106:
          return src.gain.gain.value;
         case 0x100D /* AL_MIN_GAIN */:
          return src.minGain;
        case 0x100E /* AL_MAX_GAIN */:
          return src.maxGain;
        case 0x1010 /* AL_SOURCE_STATE */:
          return src.state;
        case 0x1015 /* AL_BUFFERS_QUEUED */:
          if (src.bufQueue.length === 1 && src.bufQueue[0].id === 0) {
            return 0;
          }
          return src.bufQueue.length;
        case 0x1016 /* AL_BUFFERS_PROCESSED */:
          if ((src.bufQueue.length === 1 && src.bufQueue[0].id === 0) || src.looping) {
            return 0;
          }
          return src.bufsProcessed;
        case 0x1020 /* AL_REFERENCE_DISTANCE */:
          return src.refDistance;
        case 0x1021 /* AL_ROLLOFF_FACTOR */:
          return src.rolloffFactor;
        case 0x1022 /* AL_CONE_OUTER_GAIN */:
          return src.coneOuterGain;
        case 0x1023 /* AL_MAX_DISTANCE */:
          return src.maxDistance;
        case 0x1024 /* AL_SEC_OFFSET */:
          return AL.sourceTell(src);
        case 0x1025 /* AL_SAMPLE_OFFSET */:
          var offset = AL.sourceTell(src);
          if (offset > 0.0) {
            offset *= src.bufQueue[0].frequency;
          }
          return offset;
        case 0x1026 /* AL_BYTE_OFFSET */:
          var offset = AL.sourceTell(src);
          if (offset > 0.0) {
            offset *= src.bufQueue[0].frequency * src.bufQueue[0].bytesPerSample;
          }
          return offset;
        case 0x1027 /* AL_SOURCE_TYPE */:
          return src.type;
        case 0x1214 /* AL_SOURCE_SPATIALIZE_SOFT */:
          return src.spatialize;
        case 0x2009 /* AL_BYTE_LENGTH_SOFT */:
          var length = 0;
          var bytesPerFrame = 0;
          for (var i = 0; i < src.bufQueue.length; i++) {
            length += src.bufQueue[i].length;
            if (src.bufQueue[i].id !== 0) {
              bytesPerFrame = src.bufQueue[i].bytesPerSample * src.bufQueue[i].channels;
            }
          }
          return length * bytesPerFrame;
        case 0x200A /* AL_SAMPLE_LENGTH_SOFT */:
          var length = 0;
          for (var i = 0; i < src.bufQueue.length; i++) {
            length += src.bufQueue[i].length;
          }
          return length;
        case 0x200B /* AL_SEC_LENGTH_SOFT */:
          return AL.sourceDuration(src);
        case 53248:
          return src.distanceModel;
        default:
          AL.currentCtx.err = 40962;
          return null;
        }
      },
  setSourceParam:(funcname, sourceId, param, value) => {
        if (!AL.currentCtx) {
          return;
        }
        var src = AL.currentCtx.sources[sourceId];
        if (!src) {
          AL.currentCtx.err = 40961;
          return;
        }
        if (value === null) {
          AL.currentCtx.err = 40962;
          return;
        }
  
        switch (param) {
        case 0x202 /* AL_SOURCE_RELATIVE */:
          if (value === 1) {
            src.relative = true;
            AL.updateSourceSpace(src);
          } else if (value === 0) {
            src.relative = false;
            AL.updateSourceSpace(src);
          } else {
            AL.currentCtx.err = 40963;
            return;
          }
          break;
        case 0x1001 /* AL_CONE_INNER_ANGLE */:
          if (!Number.isFinite(value)) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          src.coneInnerAngle = value;
          if (src.panner) {
            src.panner.coneInnerAngle = value % 360.0;
          }
          break;
        case 0x1002 /* AL_CONE_OUTER_ANGLE */:
          if (!Number.isFinite(value)) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          src.coneOuterAngle = value;
          if (src.panner) {
            src.panner.coneOuterAngle = value % 360.0;
          }
          break;
        case 0x1003 /* AL_PITCH */:
          if (!Number.isFinite(value) || value <= 0.0) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          if (src.pitch === value) {
            break;
          }
  
          src.pitch = value;
          AL.updateSourceRate(src);
          break;
        case 4100:
          if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          src.position[0] = value[0];
          src.position[1] = value[1];
          src.position[2] = value[2];
          AL.updateSourceSpace(src);
          break;
        case 4101:
          if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          src.direction[0] = value[0];
          src.direction[1] = value[1];
          src.direction[2] = value[2];
          AL.updateSourceSpace(src);
          break;
        case 4102:
          if (!Number.isFinite(value[0]) || !Number.isFinite(value[1]) || !Number.isFinite(value[2])) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          src.velocity[0] = value[0];
          src.velocity[1] = value[1];
          src.velocity[2] = value[2];
          AL.updateSourceSpace(src);
          break;
        case 0x1007 /* AL_LOOPING */:
          if (value === 1) {
            src.looping = true;
            AL.updateSourceTime(src);
            if (src.type === 4136 && src.audioQueue.length > 0) {
              var audioSrc  = src.audioQueue[0];
              audioSrc.loop = true;
              audioSrc._duration = Number.POSITIVE_INFINITY;
            }
          } else if (value === 0) {
            src.looping = false;
            var currentTime = AL.updateSourceTime(src);
            if (src.type === 4136 && src.audioQueue.length > 0) {
              var audioSrc  = src.audioQueue[0];
              audioSrc.loop = false;
              audioSrc._duration = src.bufQueue[0].audioBuf.duration / src.playbackRate;
              audioSrc._startTime = currentTime - src.bufOffset / src.playbackRate;
            }
          } else {
            AL.currentCtx.err = 40963;
            return;
          }
          break;
        case 0x1009 /* AL_BUFFER */:
          if (src.state === 4114 || src.state === 4115) {
            AL.currentCtx.err = 40964;
            return;
          }
  
          if (value === 0) {
            for (var i in src.bufQueue) {
              src.bufQueue[i].refCount--;
            }
            src.bufQueue.length = 1;
            src.bufQueue[0] = AL.buffers[0];
  
            src.bufsProcessed = 0;
            src.type = 0x1030 /* AL_UNDETERMINED */;
          } else {
            var buf = AL.buffers[value];
            if (!buf) {
              AL.currentCtx.err = 40963;
              return;
            }
  
            for (var i in src.bufQueue) {
              src.bufQueue[i].refCount--;
            }
            src.bufQueue.length = 0;
  
            buf.refCount++;
            src.bufQueue = [buf];
            src.bufsProcessed = 0;
            src.type = 4136;
          }
  
          AL.initSourcePanner(src);
          AL.scheduleSourceAudio(src);
          break;
        case 4106:
          if (!Number.isFinite(value) || value < 0.0) {
            AL.currentCtx.err = 40963;
            return;
          }
          src.gain.gain.value = value;
          break;
        case 0x100D /* AL_MIN_GAIN */:
          if (!Number.isFinite(value) || value < 0.0 || value > Math.min(src.maxGain, 1.0)) {
            AL.currentCtx.err = 40963;
            return;
          }
          src.minGain = value;
          break;
        case 0x100E /* AL_MAX_GAIN */:
          if (!Number.isFinite(value) || value < Math.max(0.0, src.minGain) || value > 1.0) {
            AL.currentCtx.err = 40963;
            return;
          }
          src.maxGain = value;
          break;
        case 0x1020 /* AL_REFERENCE_DISTANCE */:
          if (!Number.isFinite(value) || value < 0.0) {
            AL.currentCtx.err = 40963;
            return;
          }
          src.refDistance = value;
          if (src.panner) {
            src.panner.refDistance = value;
          }
          break;
        case 0x1021 /* AL_ROLLOFF_FACTOR */:
          if (!Number.isFinite(value) || value < 0.0) {
            AL.currentCtx.err = 40963;
            return;
          }
          src.rolloffFactor = value;
          if (src.panner) {
            src.panner.rolloffFactor = value;
          }
          break;
        case 0x1022 /* AL_CONE_OUTER_GAIN */:
          if (!Number.isFinite(value) || value < 0.0 || value > 1.0) {
            AL.currentCtx.err = 40963;
            return;
          }
          src.coneOuterGain = value;
          if (src.panner) {
            src.panner.coneOuterGain = value;
          }
          break;
        case 0x1023 /* AL_MAX_DISTANCE */:
          if (!Number.isFinite(value) || value < 0.0) {
            AL.currentCtx.err = 40963;
            return;
          }
          src.maxDistance = value;
          if (src.panner) {
            src.panner.maxDistance = value;
          }
          break;
        case 0x1024 /* AL_SEC_OFFSET */:
          if (value < 0.0 || value > AL.sourceDuration(src)) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          AL.sourceSeek(src, value);
          break;
        case 0x1025 /* AL_SAMPLE_OFFSET */:
          var srcLen = AL.sourceDuration(src);
          if (srcLen > 0.0) {
            var frequency;
            for (var bufId in src.bufQueue) {
              if (bufId) {
                frequency = src.bufQueue[bufId].frequency;
                break;
              }
            }
            value /= frequency;
          }
          if (value < 0.0 || value > srcLen) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          AL.sourceSeek(src, value);
          break;
        case 0x1026 /* AL_BYTE_OFFSET */:
          var srcLen = AL.sourceDuration(src);
          if (srcLen > 0.0) {
            var bytesPerSec;
            for (var bufId in src.bufQueue) {
              if (bufId) {
                var buf = src.bufQueue[bufId];
                bytesPerSec = buf.frequency * buf.bytesPerSample * buf.channels;
                break;
              }
            }
            value /= bytesPerSec;
          }
          if (value < 0.0 || value > srcLen) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          AL.sourceSeek(src, value);
          break;
        case 0x1214 /* AL_SOURCE_SPATIALIZE_SOFT */:
          if (value !== 0 && value !== 1 && value !== 2 /* AL_AUTO_SOFT */) {
            AL.currentCtx.err = 40963;
            return;
          }
  
          src.spatialize = value;
          AL.initSourcePanner(src);
          break;
        case 0x2009 /* AL_BYTE_LENGTH_SOFT */:
        case 0x200A /* AL_SAMPLE_LENGTH_SOFT */:
        case 0x200B /* AL_SEC_LENGTH_SOFT */:
          AL.currentCtx.err = 40964;
          break;
        case 53248:
          switch (value) {
          case 0:
          case 0xd001 /* AL_INVERSE_DISTANCE */:
          case 0xd002 /* AL_INVERSE_DISTANCE_CLAMPED */:
          case 0xd003 /* AL_LINEAR_DISTANCE */:
          case 0xd004 /* AL_LINEAR_DISTANCE_CLAMPED */:
          case 0xd005 /* AL_EXPONENT_DISTANCE */:
          case 0xd006 /* AL_EXPONENT_DISTANCE_CLAMPED */:
            src.distanceModel = value;
            if (AL.currentCtx.sourceDistanceModel) {
              AL.updateContextGlobal(AL.currentCtx);
            }
            break;
          default:
            AL.currentCtx.err = 40963;
            return;
          }
          break;
        default:
          AL.currentCtx.err = 40962;
          return;
        }
      },
  captures:{
  },
  sharedCaptureAudioCtx:null,
  requireValidCaptureDevice:(deviceId, funcname) => {
        if (deviceId === 0) {
          AL.alcErr = 40961;
          return null;
        }
        var c = AL.captures[deviceId];
        if (!c) {
          AL.alcErr = 40961;
          return null;
        }
        var err = c.mediaStreamError;
        if (err) {
          AL.alcErr = 40961;
          return null;
        }
        return c;
      },
  };
  var _alBufferData = (bufferId, format, pData, size, freq) => {
      if (!AL.currentCtx) {
        return;
      }
      var buf = AL.buffers[bufferId];
      if (!buf) {
        AL.currentCtx.err = 40963;
        return;
      }
      if (freq <= 0) {
        AL.currentCtx.err = 40963;
        return;
      }
  
      var audioBuf = null;
      try {
        switch (format) {
        case 0x1100 /* AL_FORMAT_MONO8 */:
          if (size > 0) {
            audioBuf = AL.currentCtx.audioCtx.createBuffer(1, size, freq);
            var channel0 = audioBuf.getChannelData(0);
            for (var i = 0; i < size; ++i) {
              channel0[i] = HEAPU8[pData++] * 0.0078125 /* 1/128 */ - 1.0;
            }
          }
          buf.bytesPerSample = 1;
          buf.channels = 1;
          buf.length = size;
          break;
        case 0x1101 /* AL_FORMAT_MONO16 */:
          if (size > 0) {
            audioBuf = AL.currentCtx.audioCtx.createBuffer(1, size >> 1, freq);
            var channel0 = audioBuf.getChannelData(0);
            pData >>= 1;
            for (var i = 0; i < size >> 1; ++i) {
              channel0[i] = HEAP16[pData++] * 0.000030517578125 /* 1/32768 */;
            }
          }
          buf.bytesPerSample = 2;
          buf.channels = 1;
          buf.length = size >> 1;
          break;
        case 0x1102 /* AL_FORMAT_STEREO8 */:
          if (size > 0) {
            audioBuf = AL.currentCtx.audioCtx.createBuffer(2, size >> 1, freq);
            var channel0 = audioBuf.getChannelData(0);
            var channel1 = audioBuf.getChannelData(1);
            for (var i = 0; i < size >> 1; ++i) {
              channel0[i] = HEAPU8[pData++] * 0.0078125 /* 1/128 */ - 1.0;
              channel1[i] = HEAPU8[pData++] * 0.0078125 /* 1/128 */ - 1.0;
            }
          }
          buf.bytesPerSample = 1;
          buf.channels = 2;
          buf.length = size >> 1;
          break;
        case 0x1103 /* AL_FORMAT_STEREO16 */:
          if (size > 0) {
            audioBuf = AL.currentCtx.audioCtx.createBuffer(2, size >> 2, freq);
            var channel0 = audioBuf.getChannelData(0);
            var channel1 = audioBuf.getChannelData(1);
            pData >>= 1;
            for (var i = 0; i < size >> 2; ++i) {
              channel0[i] = HEAP16[pData++] * 0.000030517578125 /* 1/32768 */;
              channel1[i] = HEAP16[pData++] * 0.000030517578125 /* 1/32768 */;
            }
          }
          buf.bytesPerSample = 2;
          buf.channels = 2;
          buf.length = size >> 2;
          break;
        case 0x10010 /* AL_FORMAT_MONO_FLOAT32 */:
          if (size > 0) {
            audioBuf = AL.currentCtx.audioCtx.createBuffer(1, size >> 2, freq);
            var channel0 = audioBuf.getChannelData(0);
            pData >>= 2;
            for (var i = 0; i < size >> 2; ++i) {
              channel0[i] = HEAPF32[pData++];
            }
          }
          buf.bytesPerSample = 4;
          buf.channels = 1;
          buf.length = size >> 2;
          break;
        case 0x10011 /* AL_FORMAT_STEREO_FLOAT32 */:
          if (size > 0) {
            audioBuf = AL.currentCtx.audioCtx.createBuffer(2, size >> 3, freq);
            var channel0 = audioBuf.getChannelData(0);
            var channel1 = audioBuf.getChannelData(1);
            pData >>= 2;
            for (var i = 0; i < size >> 3; ++i) {
              channel0[i] = HEAPF32[pData++];
              channel1[i] = HEAPF32[pData++];
            }
          }
          buf.bytesPerSample = 4;
          buf.channels = 2;
          buf.length = size >> 3;
          break;
        default:
          AL.currentCtx.err = 40963;
          return;
        }
        buf.frequency = freq;
        buf.audioBuf = audioBuf;
      } catch (e) {
        AL.currentCtx.err = 40963;
        return;
      }
    };

  var _alDistanceModel = (model) => {
      AL.setGlobalParam('alDistanceModel', 53248, model);
    };

  var _alGenBuffers = (count, pBufferIds) => {
      if (!AL.currentCtx) {
        return;
      }
  
      for (var i = 0; i < count; ++i) {
        var buf = {
          deviceId: AL.currentCtx.deviceId,
          id: AL.newId(),
          refCount: 0,
          audioBuf: null,
          frequency: 0,
          bytesPerSample: 2,
          channels: 1,
          length: 0,
        };
        AL.deviceRefCounts[buf.deviceId]++;
        AL.buffers[buf.id] = buf;
        HEAP32[(((pBufferIds)+(i*4))>>2)] = buf.id;
      }
    };

  var _alGenSources = (count, pSourceIds) => {
      if (!AL.currentCtx) {
        return;
      }
      for (var i = 0; i < count; ++i) {
        var gain = AL.currentCtx.audioCtx.createGain();
        gain.connect(AL.currentCtx.gain);
        var src = {
          context: AL.currentCtx,
          id: AL.newId(),
          type: 0x1030 /* AL_UNDETERMINED */,
          state: 4113,
          bufQueue: [AL.buffers[0]],
          audioQueue: [],
          looping: false,
          pitch: 1.0,
          dopplerShift: 1.0,
          gain,
          minGain: 0.0,
          maxGain: 1.0,
          panner: null,
          bufsProcessed: 0,
          bufStartTime: Number.NEGATIVE_INFINITY,
          bufOffset: 0.0,
          relative: false,
          refDistance: 1.0,
          maxDistance: 3.40282e38 /* FLT_MAX */,
          rolloffFactor: 1.0,
          position: [0.0, 0.0, 0.0],
          velocity: [0.0, 0.0, 0.0],
          direction: [0.0, 0.0, 0.0],
          coneOuterGain: 0.0,
          coneInnerAngle: 360.0,
          coneOuterAngle: 360.0,
          distanceModel: 0xd002 /* AL_INVERSE_DISTANCE_CLAMPED */,
          spatialize: 2 /* AL_AUTO_SOFT */,
  
          get playbackRate() {
            return this.pitch * this.dopplerShift;
          }
        };
        AL.currentCtx.sources[src.id] = src;
        HEAP32[(((pSourceIds)+(i*4))>>2)] = src.id;
      }
    };

  var _alGetListenerfv = (param, pValues) => {
      var val = AL.getListenerParam('alGetListenerfv', param);
      if (val === null) {
        return;
      }
      if (!pValues) {
        AL.currentCtx.err = 40963;
        return;
      }
  
      switch (param) {
      case 4100:
      case 4102:
        HEAPF32[((pValues)>>2)] = val[0];
        HEAPF32[(((pValues)+(4))>>2)] = val[1];
        HEAPF32[(((pValues)+(8))>>2)] = val[2];
        break;
      case 4111:
        HEAPF32[((pValues)>>2)] = val[0];
        HEAPF32[(((pValues)+(4))>>2)] = val[1];
        HEAPF32[(((pValues)+(8))>>2)] = val[2];
        HEAPF32[(((pValues)+(12))>>2)] = val[3];
        HEAPF32[(((pValues)+(16))>>2)] = val[4];
        HEAPF32[(((pValues)+(20))>>2)] = val[5];
        break;
      default:
        AL.currentCtx.err = 40962;
        return;
      }
    };

  
  
  var stringToNewUTF8 = (str) => {
      var size = lengthBytesUTF8(str) + 1;
      var ret = _malloc(size);
      if (ret) stringToUTF8(str, ret, size);
      return ret;
    };
  
  var _alGetString = (param) => {
      if (AL.stringCache[param]) {
        return AL.stringCache[param];
      }
  
      var ret;
      switch (param) {
      case 0:
        ret = 'No Error';
        break;
      case 40961:
        ret = 'Invalid Name';
        break;
      case 40962:
        ret = 'Invalid Enum';
        break;
      case 40963:
        ret = 'Invalid Value';
        break;
      case 40964:
        ret = 'Invalid Operation';
        break;
      case 0xA005 /* AL_OUT_OF_MEMORY */:
        ret = 'Out of Memory';
        break;
      case 0xB001 /* AL_VENDOR */:
        ret = 'Emscripten';
        break;
      case 0xB002 /* AL_VERSION */:
        ret = '1.1';
        break;
      case 0xB003 /* AL_RENDERER */:
        ret = 'WebAudio';
        break;
      case 0xB004 /* AL_EXTENSIONS */:
        ret = Object.keys(AL.AL_EXTENSIONS).join(' ');
        break;
      default:
        if (AL.currentCtx) {
          AL.currentCtx.err = 40962;
        } else {
        }
        return 0;
      }
  
      ret = stringToNewUTF8(ret);
      AL.stringCache[param] = ret;
      return ret;
    };

  var _alIsSource = (sourceId) => {
      if (!AL.currentCtx) {
        return false;
      }
  
      if (!AL.currentCtx.sources[sourceId]) {
        return false;
      }
      return true;
    };

  var _alListener3f = (param, value0, value1, value2) => {
      switch (param) {
      case 4100:
      case 4102:
        AL.paramArray[0] = value0;
        AL.paramArray[1] = value1;
        AL.paramArray[2] = value2;
        AL.setListenerParam('alListener3f', param, AL.paramArray);
        break;
      default:
        AL.setListenerParam('alListener3f', param, null);
        break;
      }
    };

  var _alListenerfv = (param, pValues) => {
      if (!AL.currentCtx) {
        return;
      }
      if (!pValues) {
        AL.currentCtx.err = 40963;
        return;
      }
  
      switch (param) {
      case 4100:
      case 4102:
        AL.paramArray[0] = HEAPF32[((pValues)>>2)];
        AL.paramArray[1] = HEAPF32[(((pValues)+(4))>>2)];
        AL.paramArray[2] = HEAPF32[(((pValues)+(8))>>2)];
        AL.setListenerParam('alListenerfv', param, AL.paramArray);
        break;
      case 4111:
        AL.paramArray[0] = HEAPF32[((pValues)>>2)];
        AL.paramArray[1] = HEAPF32[(((pValues)+(4))>>2)];
        AL.paramArray[2] = HEAPF32[(((pValues)+(8))>>2)];
        AL.paramArray[3] = HEAPF32[(((pValues)+(12))>>2)];
        AL.paramArray[4] = HEAPF32[(((pValues)+(16))>>2)];
        AL.paramArray[5] = HEAPF32[(((pValues)+(20))>>2)];
        AL.setListenerParam('alListenerfv', param, AL.paramArray);
        break;
      default:
        AL.setListenerParam('alListenerfv', param, null);
        break;
      }
    };

  var _alSource3f = (sourceId, param, value0, value1, value2) => {
      switch (param) {
      case 4100:
      case 4101:
      case 4102:
        AL.paramArray[0] = value0;
        AL.paramArray[1] = value1;
        AL.paramArray[2] = value2;
        AL.setSourceParam('alSource3f', sourceId, param, AL.paramArray);
        break;
      default:
        AL.setSourceParam('alSource3f', sourceId, param, null);
        break;
      }
    };

  var _alSourcePause = (sourceId) => {
      if (!AL.currentCtx) {
        return;
      }
      var src = AL.currentCtx.sources[sourceId];
      if (!src) {
        AL.currentCtx.err = 40961;
        return;
      }
      AL.setSourceState(src, 4115);
    };

  var _alSourcePlay = (sourceId) => {
      if (!AL.currentCtx) {
        return;
      }
      var src = AL.currentCtx.sources[sourceId];
      if (!src) {
        AL.currentCtx.err = 40961;
        return;
      }
      AL.setSourceState(src, 4114);
    };

  var _alSourcef = (sourceId, param, value) => {
      switch (param) {
      case 0x1001 /* AL_CONE_INNER_ANGLE */:
      case 0x1002 /* AL_CONE_OUTER_ANGLE */:
      case 0x1003 /* AL_PITCH */:
      case 4106:
      case 0x100D /* AL_MIN_GAIN */:
      case 0x100E /* AL_MAX_GAIN */:
      case 0x1020 /* AL_REFERENCE_DISTANCE */:
      case 0x1021 /* AL_ROLLOFF_FACTOR */:
      case 0x1022 /* AL_CONE_OUTER_GAIN */:
      case 0x1023 /* AL_MAX_DISTANCE */:
      case 0x1024 /* AL_SEC_OFFSET */:
      case 0x1025 /* AL_SAMPLE_OFFSET */:
      case 0x1026 /* AL_BYTE_OFFSET */:
      case 0x200B /* AL_SEC_LENGTH_SOFT */:
        AL.setSourceParam('alSourcef', sourceId, param, value);
        break;
      default:
        AL.setSourceParam('alSourcef', sourceId, param, null);
        break;
      }
    };

  var _alSourcei = (sourceId, param, value) => {
      switch (param) {
      case 0x202 /* AL_SOURCE_RELATIVE */:
      case 0x1001 /* AL_CONE_INNER_ANGLE */:
      case 0x1002 /* AL_CONE_OUTER_ANGLE */:
      case 0x1007 /* AL_LOOPING */:
      case 0x1009 /* AL_BUFFER */:
      case 0x1020 /* AL_REFERENCE_DISTANCE */:
      case 0x1021 /* AL_ROLLOFF_FACTOR */:
      case 0x1023 /* AL_MAX_DISTANCE */:
      case 0x1024 /* AL_SEC_OFFSET */:
      case 0x1025 /* AL_SAMPLE_OFFSET */:
      case 0x1026 /* AL_BYTE_OFFSET */:
      case 0x1214 /* AL_SOURCE_SPATIALIZE_SOFT */:
      case 0x2009 /* AL_BYTE_LENGTH_SOFT */:
      case 0x200A /* AL_SAMPLE_LENGTH_SOFT */:
      case 53248:
        AL.setSourceParam('alSourcei', sourceId, param, value);
        break;
      default:
        AL.setSourceParam('alSourcei', sourceId, param, null);
        break;
      }
    };

  var _alcCloseDevice = (deviceId) => {
      if (!(deviceId in AL.deviceRefCounts) || AL.deviceRefCounts[deviceId] > 0) {
        return 0;
      }
  
      delete AL.deviceRefCounts[deviceId];
      AL.freeIds.push(deviceId);
      return 1;
    };

  var listenOnce = (object, event, func) =>
      object.addEventListener(event, func, { 'once': true });
  /** @param {Object=} elements */
  var autoResumeAudioContext = (ctx, elements) => {
      if (!elements) {
        elements = [document, document.getElementById('canvas')];
      }
      ['keydown', 'mousedown', 'touchstart'].forEach((event) => {
        elements.forEach((element) => {
          if (element) {
            listenOnce(element, event, () => {
              if (ctx.state === 'suspended') ctx.resume();
            });
          }
        });
      });
    };
  
  var _alcCreateContext = (deviceId, pAttrList) => {
      if (!(deviceId in AL.deviceRefCounts)) {
        AL.alcErr = 0xA001; /* ALC_INVALID_DEVICE */
        return 0;
      }
  
      var options = null;
      var attrs = [];
      var hrtf = null;
      pAttrList >>= 2;
      if (pAttrList) {
        var attr = 0;
        var val = 0;
        while (true) {
          attr = HEAP32[pAttrList++];
          attrs.push(attr);
          if (attr === 0) {
            break;
          }
          val = HEAP32[pAttrList++];
          attrs.push(val);
  
          switch (attr) {
          case 0x1007 /* ALC_FREQUENCY */:
            if (!options) {
              options = {};
            }
  
            options.sampleRate = val;
            break;
          case 0x1010 /* ALC_MONO_SOURCES */: // fallthrough
          case 0x1011 /* ALC_STEREO_SOURCES */:
            // Do nothing; these hints are satisfied by default
            break
          case 0x1992 /* ALC_HRTF_SOFT */:
            switch (val) {
              case 0:
                hrtf = false;
                break;
              case 1:
                hrtf = true;
                break;
              case 2 /* ALC_DONT_CARE_SOFT */:
                break;
              default:
                AL.alcErr = 40964;
                return 0;
            }
            break;
          case 0x1996 /* ALC_HRTF_ID_SOFT */:
            if (val !== 0) {
              AL.alcErr = 40964;
              return 0;
            }
            break;
          default:
            AL.alcErr = 0xA004; /* ALC_INVALID_VALUE */
            return 0;
          }
        }
      }
  
      var AudioContext = window.AudioContext || window.webkitAudioContext;
      var ac = null;
      try {
        // Only try to pass options if there are any, for compat with browsers that don't support this
        if (options) {
          ac = new AudioContext(options);
        } else {
          ac = new AudioContext();
        }
      } catch (e) {
        if (e.name === 'NotSupportedError') {
          AL.alcErr = 0xA004; /* ALC_INVALID_VALUE */
        } else {
          AL.alcErr = 0xA001; /* ALC_INVALID_DEVICE */
        }
  
        return 0;
      }
  
      autoResumeAudioContext(ac);
  
      // Old Web Audio API (e.g. Safari 6.0.5) had an inconsistently named createGainNode function.
      if (typeof ac.createGain == 'undefined') {
        ac.createGain = ac.createGainNode;
      }
  
      var gain = ac.createGain();
      gain.connect(ac.destination);
      var ctx = {
        deviceId,
        id: AL.newId(),
        attrs,
        audioCtx: ac,
        listener: {
          position: [0.0, 0.0, 0.0],
          velocity: [0.0, 0.0, 0.0],
          direction: [0.0, 0.0, 0.0],
          up: [0.0, 0.0, 0.0]
        },
        sources: [],
        interval: setInterval(() => AL.scheduleContextAudio(ctx), AL.QUEUE_INTERVAL),
        gain,
        distanceModel: 0xd002 /* AL_INVERSE_DISTANCE_CLAMPED */,
        speedOfSound: 343.3,
        dopplerFactor: 1.0,
        sourceDistanceModel: false,
        hrtf: hrtf || false,
  
        _err: 0,
        get err() {
          return this._err;
        },
        set err(val) {
          // Errors should not be overwritten by later errors until they are cleared by a query.
          if (this._err === 0 || val === 0) {
            this._err = val;
          }
        }
      };
      AL.deviceRefCounts[deviceId]++;
      AL.contexts[ctx.id] = ctx;
  
      if (hrtf !== null) {
        // Apply hrtf attrib to all contexts for this device
        for (var ctxId in AL.contexts) {
          var c = AL.contexts[ctxId];
          if (c.deviceId === deviceId) {
            c.hrtf = hrtf;
            AL.updateContextGlobal(c);
          }
        }
      }
  
      return ctx.id;
    };

  var _alcDestroyContext = (contextId) => {
      var ctx = AL.contexts[contextId];
      if (AL.currentCtx === ctx) {
        AL.alcErr = 0xA002 /* ALC_INVALID_CONTEXT */;
        return;
      }
  
      // Stop playback, etc
      if (AL.contexts[contextId].interval) {
        clearInterval(AL.contexts[contextId].interval);
      }
      AL.deviceRefCounts[ctx.deviceId]--;
      delete AL.contexts[contextId];
      AL.freeIds.push(contextId);
    };

  
  var _alcIsExtensionPresent = (deviceId, pExtName) => {
      var name = UTF8ToString(pExtName);
  
      return AL.ALC_EXTENSIONS[name] ? 1 : 0;
    };

  var _alcMakeContextCurrent = (contextId) => {
      if (contextId === 0) {
        AL.currentCtx = null;
      } else {
        AL.currentCtx = AL.contexts[contextId];
      }
      return 1;
    };

  
  var _alcOpenDevice = (pDeviceName) => {
      if (pDeviceName) {
        var name = UTF8ToString(pDeviceName);
        if (name !== AL.DEVICE_NAME) {
          return 0;
        }
      }
  
      if (typeof AudioContext != 'undefined' || typeof webkitAudioContext != 'undefined') {
        var deviceId = AL.newId();
        AL.deviceRefCounts[deviceId] = 0;
        return deviceId;
      }
      return 0;
    };

  
  var _emscripten_date_now = () => Date.now();
  
  var nowIsMonotonic = 1;
  
  var checkWasiClock = (clock_id) => clock_id >= 0 && clock_id <= 3;
  
  function _clock_time_get(clk_id, ignored_precision, ptime) {
    ignored_precision = bigintToI53Checked(ignored_precision);
  
  
      if (!checkWasiClock(clk_id)) {
        return 28;
      }
      var now;
      // all wasi clocks but realtime are monotonic
      if (clk_id === 0) {
        now = _emscripten_date_now();
      } else if (nowIsMonotonic) {
        now = _emscripten_get_now();
      } else {
        return 52;
      }
      // "now" is in ms, and wasi times are in ns.
      var nsec = Math.round(now * 1000 * 1000);
      HEAP64[((ptime)>>3)] = BigInt(nsec);
      return 0;
    ;
  }

  
  /** @param {number=} timeout */
  var safeSetTimeout = (func, timeout) => {
      
      return setTimeout(() => {
        
        callUserCallback(func);
      }, timeout);
    };
  
  
  
  var Browser = {
  useWebGL:false,
  isFullscreen:false,
  pointerLock:false,
  moduleContextCreatedCallbacks:[],
  workers:[],
  preloadedImages:{
  },
  preloadedAudios:{
  },
  getCanvas:() => Module['canvas'],
  init() {
        if (Browser.initted) return;
        Browser.initted = true;
  
        // Support for plugins that can process preloaded files. You can add more of these to
        // your app by creating and appending to preloadPlugins.
        //
        // Each plugin is asked if it can handle a file based on the file's name. If it can,
        // it is given the file's raw data. When it is done, it calls a callback with the file's
        // (possibly modified) data. For example, a plugin might decompress a file, or it
        // might create some side data structure for use later (like an Image element, etc.).
  
        var imagePlugin = {};
        imagePlugin['canHandle'] = function imagePlugin_canHandle(name) {
          return !Module['noImageDecoding'] && /\.(jpg|jpeg|png|bmp|webp)$/i.test(name);
        };
        imagePlugin['handle'] = function imagePlugin_handle(byteArray, name, onload, onerror) {
          var b = new Blob([byteArray], { type: Browser.getMimetype(name) });
          if (b.size !== byteArray.length) { // Safari bug #118630
            // Safari's Blob can only take an ArrayBuffer
            b = new Blob([(new Uint8Array(byteArray)).buffer], { type: Browser.getMimetype(name) });
          }
          var url = URL.createObjectURL(b);
          assert(typeof url == 'string', 'createObjectURL must return a url as a string');
          var img = new Image();
          img.onload = () => {
            assert(img.complete, `Image ${name} could not be decoded`);
            var canvas = /** @type {!HTMLCanvasElement} */ (document.createElement('canvas'));
            canvas.width = img.width;
            canvas.height = img.height;
            var ctx = canvas.getContext('2d');
            ctx.drawImage(img, 0, 0);
            Browser.preloadedImages[name] = canvas;
            URL.revokeObjectURL(url);
            onload?.(byteArray);
          };
          img.onerror = (event) => {
            err(`Image ${url} could not be decoded`);
            onerror?.();
          };
          img.src = url;
        };
        preloadPlugins.push(imagePlugin);
  
        var audioPlugin = {};
        audioPlugin['canHandle'] = function audioPlugin_canHandle(name) {
          return !Module['noAudioDecoding'] && name.slice(-4) in { '.ogg': 1, '.wav': 1, '.mp3': 1 };
        };
        audioPlugin['handle'] = function audioPlugin_handle(byteArray, name, onload, onerror) {
          var done = false;
          function finish(audio) {
            if (done) return;
            done = true;
            Browser.preloadedAudios[name] = audio;
            onload?.(byteArray);
          }
          function fail() {
            if (done) return;
            done = true;
            Browser.preloadedAudios[name] = new Audio(); // empty shim
            onerror?.();
          }
          var b = new Blob([byteArray], { type: Browser.getMimetype(name) });
          var url = URL.createObjectURL(b); // XXX we never revoke this!
          assert(typeof url == 'string', 'createObjectURL must return a url as a string');
          var audio = new Audio();
          audio.addEventListener('canplaythrough', () => finish(audio), false); // use addEventListener due to chromium bug 124926
          audio.onerror = function audio_onerror(event) {
            if (done) return;
            err(`warning: browser could not fully decode audio ${name}, trying slower base64 approach`);
            function encode64(data) {
              var BASE = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
              var PAD = '=';
              var ret = '';
              var leftchar = 0;
              var leftbits = 0;
              for (var i = 0; i < data.length; i++) {
                leftchar = (leftchar << 8) | data[i];
                leftbits += 8;
                while (leftbits >= 6) {
                  var curr = (leftchar >> (leftbits-6)) & 0x3f;
                  leftbits -= 6;
                  ret += BASE[curr];
                }
              }
              if (leftbits == 2) {
                ret += BASE[(leftchar&3) << 4];
                ret += PAD + PAD;
              } else if (leftbits == 4) {
                ret += BASE[(leftchar&0xf) << 2];
                ret += PAD;
              }
              return ret;
            }
            audio.src = 'data:audio/x-' + name.slice(-3) + ';base64,' + encode64(byteArray);
            finish(audio); // we don't wait for confirmation this worked - but it's worth trying
          };
          audio.src = url;
          // workaround for chrome bug 124926 - we do not always get oncanplaythrough or onerror
          safeSetTimeout(() => {
            finish(audio); // try to use it even though it is not necessarily ready to play
          }, 10000);
        };
        preloadPlugins.push(audioPlugin);
  
        // Canvas event setup
  
        function pointerLockChange() {
          var canvas = Browser.getCanvas();
          Browser.pointerLock = document['pointerLockElement'] === canvas ||
                                document['mozPointerLockElement'] === canvas ||
                                document['webkitPointerLockElement'] === canvas ||
                                document['msPointerLockElement'] === canvas;
        }
        var canvas = Browser.getCanvas();
        if (canvas) {
          // forced aspect ratio can be enabled by defining 'forcedAspectRatio' on Module
          // Module['forcedAspectRatio'] = 4 / 3;
  
          canvas.requestPointerLock = canvas['requestPointerLock'] ||
                                      canvas['mozRequestPointerLock'] ||
                                      canvas['webkitRequestPointerLock'] ||
                                      canvas['msRequestPointerLock'] ||
                                      (() => {});
          canvas.exitPointerLock = document['exitPointerLock'] ||
                                   document['mozExitPointerLock'] ||
                                   document['webkitExitPointerLock'] ||
                                   document['msExitPointerLock'] ||
                                   (() => {}); // no-op if function does not exist
          canvas.exitPointerLock = canvas.exitPointerLock.bind(document);
  
          document.addEventListener('pointerlockchange', pointerLockChange, false);
          document.addEventListener('mozpointerlockchange', pointerLockChange, false);
          document.addEventListener('webkitpointerlockchange', pointerLockChange, false);
          document.addEventListener('mspointerlockchange', pointerLockChange, false);
  
          if (Module['elementPointerLock']) {
            canvas.addEventListener("click", (ev) => {
              if (!Browser.pointerLock && Browser.getCanvas().requestPointerLock) {
                Browser.getCanvas().requestPointerLock();
                ev.preventDefault();
              }
            }, false);
          }
        }
      },
  createContext(/** @type {HTMLCanvasElement} */ canvas, useWebGL, setInModule, webGLContextAttributes) {
        if (useWebGL && Module['ctx'] && canvas == Browser.getCanvas()) return Module['ctx']; // no need to recreate GL context if it's already been created for this canvas.
  
        var ctx;
        var contextHandle;
        if (useWebGL) {
          // For GLES2/desktop GL compatibility, adjust a few defaults to be different to WebGL defaults, so that they align better with the desktop defaults.
          var contextAttributes = {
            antialias: false,
            alpha: false,
            majorVersion: 2,
          };
  
          if (webGLContextAttributes) {
            for (var attribute in webGLContextAttributes) {
              contextAttributes[attribute] = webGLContextAttributes[attribute];
            }
          }
  
          // This check of existence of GL is here to satisfy Closure compiler, which yells if variable GL is referenced below but GL object is not
          // actually compiled in because application is not doing any GL operations. TODO: Ideally if GL is not being used, this function
          // Browser.createContext() should not even be emitted.
          if (typeof GL != 'undefined') {
            contextHandle = GL.createContext(canvas, contextAttributes);
            if (contextHandle) {
              ctx = GL.getContext(contextHandle).GLctx;
            }
          }
        } else {
          ctx = canvas.getContext('2d');
        }
  
        if (!ctx) return null;
  
        if (setInModule) {
          if (!useWebGL) assert(typeof GLctx == 'undefined', 'cannot set in module if GLctx is used, but we are a non-GL context that would replace it');
          Module['ctx'] = ctx;
          if (useWebGL) GL.makeContextCurrent(contextHandle);
          Browser.useWebGL = useWebGL;
          Browser.moduleContextCreatedCallbacks.forEach((callback) => callback());
          Browser.init();
        }
        return ctx;
      },
  fullscreenHandlersInstalled:false,
  lockPointer:undefined,
  resizeCanvas:undefined,
  requestFullscreen(lockPointer, resizeCanvas) {
        Browser.lockPointer = lockPointer;
        Browser.resizeCanvas = resizeCanvas;
        if (typeof Browser.lockPointer == 'undefined') Browser.lockPointer = true;
        if (typeof Browser.resizeCanvas == 'undefined') Browser.resizeCanvas = false;
  
        var canvas = Browser.getCanvas();
        function fullscreenChange() {
          Browser.isFullscreen = false;
          var canvasContainer = canvas.parentNode;
          if ((document['fullscreenElement'] || document['mozFullScreenElement'] ||
               document['msFullscreenElement'] || document['webkitFullscreenElement'] ||
               document['webkitCurrentFullScreenElement']) === canvasContainer) {
            canvas.exitFullscreen = Browser.exitFullscreen;
            if (Browser.lockPointer) canvas.requestPointerLock();
            Browser.isFullscreen = true;
            if (Browser.resizeCanvas) {
              Browser.setFullscreenCanvasSize();
            } else {
              Browser.updateCanvasDimensions(canvas);
            }
          } else {
            // remove the full screen specific parent of the canvas again to restore the HTML structure from before going full screen
            canvasContainer.parentNode.insertBefore(canvas, canvasContainer);
            canvasContainer.parentNode.removeChild(canvasContainer);
  
            if (Browser.resizeCanvas) {
              Browser.setWindowedCanvasSize();
            } else {
              Browser.updateCanvasDimensions(canvas);
            }
          }
          Module['onFullScreen']?.(Browser.isFullscreen);
          Module['onFullscreen']?.(Browser.isFullscreen);
        }
  
        if (!Browser.fullscreenHandlersInstalled) {
          Browser.fullscreenHandlersInstalled = true;
          document.addEventListener('fullscreenchange', fullscreenChange, false);
          document.addEventListener('mozfullscreenchange', fullscreenChange, false);
          document.addEventListener('webkitfullscreenchange', fullscreenChange, false);
          document.addEventListener('MSFullscreenChange', fullscreenChange, false);
        }
  
        // create a new parent to ensure the canvas has no siblings. this allows browsers to optimize full screen performance when its parent is the full screen root
        var canvasContainer = document.createElement("div");
        canvas.parentNode.insertBefore(canvasContainer, canvas);
        canvasContainer.appendChild(canvas);
  
        // use parent of canvas as full screen root to allow aspect ratio correction (Firefox stretches the root to screen size)
        canvasContainer.requestFullscreen = canvasContainer['requestFullscreen'] ||
                                            canvasContainer['mozRequestFullScreen'] ||
                                            canvasContainer['msRequestFullscreen'] ||
                                           (canvasContainer['webkitRequestFullscreen'] ? () => canvasContainer['webkitRequestFullscreen'](Element['ALLOW_KEYBOARD_INPUT']) : null) ||
                                           (canvasContainer['webkitRequestFullScreen'] ? () => canvasContainer['webkitRequestFullScreen'](Element['ALLOW_KEYBOARD_INPUT']) : null);
  
        canvasContainer.requestFullscreen();
      },
  requestFullScreen() {
        abort('Module.requestFullScreen has been replaced by Module.requestFullscreen (without a capital S)');
      },
  exitFullscreen() {
        // This is workaround for chrome. Trying to exit from fullscreen
        // not in fullscreen state will cause "TypeError: Document not active"
        // in chrome. See https://github.com/emscripten-core/emscripten/pull/8236
        if (!Browser.isFullscreen) {
          return false;
        }
  
        var CFS = document['exitFullscreen'] ||
                  document['cancelFullScreen'] ||
                  document['mozCancelFullScreen'] ||
                  document['msExitFullscreen'] ||
                  document['webkitCancelFullScreen'] ||
            (() => {});
        CFS.apply(document, []);
        return true;
      },
  safeSetTimeout(func, timeout) {
        // Legacy function, this is used by the SDL2 port so we need to keep it
        // around at least until that is updated.
        // See https://github.com/libsdl-org/SDL/pull/6304
        return safeSetTimeout(func, timeout);
      },
  getMimetype(name) {
        return {
          'jpg': 'image/jpeg',
          'jpeg': 'image/jpeg',
          'png': 'image/png',
          'bmp': 'image/bmp',
          'ogg': 'audio/ogg',
          'wav': 'audio/wav',
          'mp3': 'audio/mpeg'
        }[name.slice(name.lastIndexOf('.')+1)];
      },
  getUserMedia(func) {
        window.getUserMedia ||= navigator['getUserMedia'] ||
                                navigator['mozGetUserMedia'];
        window.getUserMedia(func);
      },
  getMovementX(event) {
        return event['movementX'] ||
               event['mozMovementX'] ||
               event['webkitMovementX'] ||
               0;
      },
  getMovementY(event) {
        return event['movementY'] ||
               event['mozMovementY'] ||
               event['webkitMovementY'] ||
               0;
      },
  getMouseWheelDelta(event) {
        var delta = 0;
        switch (event.type) {
          case 'DOMMouseScroll':
            // 3 lines make up a step
            delta = event.detail / 3;
            break;
          case 'mousewheel':
            // 120 units make up a step
            delta = event.wheelDelta / 120;
            break;
          case 'wheel':
            delta = event.deltaY
            switch (event.deltaMode) {
              case 0:
                // DOM_DELTA_PIXEL: 100 pixels make up a step
                delta /= 100;
                break;
              case 1:
                // DOM_DELTA_LINE: 3 lines make up a step
                delta /= 3;
                break;
              case 2:
                // DOM_DELTA_PAGE: A page makes up 80 steps
                delta *= 80;
                break;
              default:
                throw 'unrecognized mouse wheel delta mode: ' + event.deltaMode;
            }
            break;
          default:
            throw 'unrecognized mouse wheel event: ' + event.type;
        }
        return delta;
      },
  mouseX:0,
  mouseY:0,
  mouseMovementX:0,
  mouseMovementY:0,
  touches:{
  },
  lastTouches:{
  },
  calculateMouseCoords(pageX, pageY) {
        // Calculate the movement based on the changes
        // in the coordinates.
        var canvas = Browser.getCanvas();
        var rect = canvas.getBoundingClientRect();
  
        // Neither .scrollX or .pageXOffset are defined in a spec, but
        // we prefer .scrollX because it is currently in a spec draft.
        // (see: http://www.w3.org/TR/2013/WD-cssom-view-20131217/)
        var scrollX = ((typeof window.scrollX != 'undefined') ? window.scrollX : window.pageXOffset);
        var scrollY = ((typeof window.scrollY != 'undefined') ? window.scrollY : window.pageYOffset);
        // If this assert lands, it's likely because the browser doesn't support scrollX or pageXOffset
        // and we have no viable fallback.
        assert((typeof scrollX != 'undefined') && (typeof scrollY != 'undefined'), 'Unable to retrieve scroll position, mouse positions likely broken.');
        var adjustedX = pageX - (scrollX + rect.left);
        var adjustedY = pageY - (scrollY + rect.top);
  
        // the canvas might be CSS-scaled compared to its backbuffer;
        // SDL-using content will want mouse coordinates in terms
        // of backbuffer units.
        adjustedX = adjustedX * (canvas.width / rect.width);
        adjustedY = adjustedY * (canvas.height / rect.height);
  
        return { x: adjustedX, y: adjustedY };
      },
  setMouseCoords(pageX, pageY) {
        const {x, y} = Browser.calculateMouseCoords(pageX, pageY);
        Browser.mouseMovementX = x - Browser.mouseX;
        Browser.mouseMovementY = y - Browser.mouseY;
        Browser.mouseX = x;
        Browser.mouseY = y;
      },
  calculateMouseEvent(event) { // event should be mousemove, mousedown or mouseup
        if (Browser.pointerLock) {
          // When the pointer is locked, calculate the coordinates
          // based on the movement of the mouse.
          // Workaround for Firefox bug 764498
          if (event.type != 'mousemove' &&
              ('mozMovementX' in event)) {
            Browser.mouseMovementX = Browser.mouseMovementY = 0;
          } else {
            Browser.mouseMovementX = Browser.getMovementX(event);
            Browser.mouseMovementY = Browser.getMovementY(event);
          }
  
          // add the mouse delta to the current absolute mouse position
          Browser.mouseX += Browser.mouseMovementX;
          Browser.mouseY += Browser.mouseMovementY;
        } else {
          if (event.type === 'touchstart' || event.type === 'touchend' || event.type === 'touchmove') {
            var touch = event.touch;
            if (touch === undefined) {
              return; // the "touch" property is only defined in SDL
  
            }
            var coords = Browser.calculateMouseCoords(touch.pageX, touch.pageY);
  
            if (event.type === 'touchstart') {
              Browser.lastTouches[touch.identifier] = coords;
              Browser.touches[touch.identifier] = coords;
            } else if (event.type === 'touchend' || event.type === 'touchmove') {
              var last = Browser.touches[touch.identifier];
              last ||= coords;
              Browser.lastTouches[touch.identifier] = last;
              Browser.touches[touch.identifier] = coords;
            }
            return;
          }
  
          Browser.setMouseCoords(event.pageX, event.pageY);
        }
      },
  resizeListeners:[],
  updateResizeListeners() {
        var canvas = Browser.getCanvas();
        Browser.resizeListeners.forEach((listener) => listener(canvas.width, canvas.height));
      },
  setCanvasSize(width, height, noUpdates) {
        var canvas = Browser.getCanvas();
        Browser.updateCanvasDimensions(canvas, width, height);
        if (!noUpdates) Browser.updateResizeListeners();
      },
  windowedWidth:0,
  windowedHeight:0,
  setFullscreenCanvasSize() {
        // check if SDL is available
        if (typeof SDL != "undefined") {
          var flags = HEAPU32[((SDL.screen)>>2)];
          flags = flags | 0x00800000; // set SDL_FULLSCREEN flag
          HEAP32[((SDL.screen)>>2)] = flags;
        }
        Browser.updateCanvasDimensions(Browser.getCanvas());
        Browser.updateResizeListeners();
      },
  setWindowedCanvasSize() {
        // check if SDL is available
        if (typeof SDL != "undefined") {
          var flags = HEAPU32[((SDL.screen)>>2)];
          flags = flags & ~0x00800000; // clear SDL_FULLSCREEN flag
          HEAP32[((SDL.screen)>>2)] = flags;
        }
        Browser.updateCanvasDimensions(Browser.getCanvas());
        Browser.updateResizeListeners();
      },
  updateCanvasDimensions(canvas, wNative, hNative) {
        if (wNative && hNative) {
          canvas.widthNative = wNative;
          canvas.heightNative = hNative;
        } else {
          wNative = canvas.widthNative;
          hNative = canvas.heightNative;
        }
        var w = wNative;
        var h = hNative;
        if (Module['forcedAspectRatio'] > 0) {
          if (w/h < Module['forcedAspectRatio']) {
            w = Math.round(h * Module['forcedAspectRatio']);
          } else {
            h = Math.round(w / Module['forcedAspectRatio']);
          }
        }
        if (((document['fullscreenElement'] || document['mozFullScreenElement'] ||
             document['msFullscreenElement'] || document['webkitFullscreenElement'] ||
             document['webkitCurrentFullScreenElement']) === canvas.parentNode) && (typeof screen != 'undefined')) {
           var factor = Math.min(screen.width / w, screen.height / h);
           w = Math.round(w * factor);
           h = Math.round(h * factor);
        }
        if (Browser.resizeCanvas) {
          if (canvas.width  != w) canvas.width  = w;
          if (canvas.height != h) canvas.height = h;
          if (typeof canvas.style != 'undefined') {
            canvas.style.removeProperty( "width");
            canvas.style.removeProperty("height");
          }
        } else {
          if (canvas.width  != wNative) canvas.width  = wNative;
          if (canvas.height != hNative) canvas.height = hNative;
          if (typeof canvas.style != 'undefined') {
            if (w != wNative || h != hNative) {
              canvas.style.setProperty( "width", w + "px", "important");
              canvas.style.setProperty("height", h + "px", "important");
            } else {
              canvas.style.removeProperty( "width");
              canvas.style.removeProperty("height");
            }
          }
        }
      },
  };
  
  var EGL = {
  errorCode:12288,
  defaultDisplayInitialized:false,
  currentContext:0,
  currentReadSurface:0,
  currentDrawSurface:0,
  contextAttributes:{
  alpha:false,
  depth:false,
  stencil:false,
  antialias:false,
  },
  stringCache:{
  },
  setErrorCode(code) {
        EGL.errorCode = code;
      },
  chooseConfig(display, attribList, config, config_size, numConfigs) {
        if (display != 62000) {
          EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
          return 0;
        }
  
        if (attribList) {
          // read attribList if it is non-null
          for (;;) {
            var param = HEAP32[((attribList)>>2)];
            if (param == 0x3021 /*EGL_ALPHA_SIZE*/) {
              var alphaSize = HEAP32[(((attribList)+(4))>>2)];
              EGL.contextAttributes.alpha = (alphaSize > 0);
            } else if (param == 0x3025 /*EGL_DEPTH_SIZE*/) {
              var depthSize = HEAP32[(((attribList)+(4))>>2)];
              EGL.contextAttributes.depth = (depthSize > 0);
            } else if (param == 0x3026 /*EGL_STENCIL_SIZE*/) {
              var stencilSize = HEAP32[(((attribList)+(4))>>2)];
              EGL.contextAttributes.stencil = (stencilSize > 0);
            } else if (param == 0x3031 /*EGL_SAMPLES*/) {
              var samples = HEAP32[(((attribList)+(4))>>2)];
              EGL.contextAttributes.antialias = (samples > 0);
            } else if (param == 0x3032 /*EGL_SAMPLE_BUFFERS*/) {
              var samples = HEAP32[(((attribList)+(4))>>2)];
              EGL.contextAttributes.antialias = (samples == 1);
            } else if (param == 0x3100 /*EGL_CONTEXT_PRIORITY_LEVEL_IMG*/) {
              var requestedPriority = HEAP32[(((attribList)+(4))>>2)];
              EGL.contextAttributes.lowLatency = (requestedPriority != 0x3103 /*EGL_CONTEXT_PRIORITY_LOW_IMG*/);
            } else if (param == 0x3038 /*EGL_NONE*/) {
                break;
            }
            attribList += 8;
          }
        }
  
        if ((!config || !config_size) && !numConfigs) {
          EGL.setErrorCode(0x300C /* EGL_BAD_PARAMETER */);
          return 0;
        }
        if (numConfigs) {
          HEAP32[((numConfigs)>>2)] = 1; // Total number of supported configs: 1.
        }
        if (config && config_size > 0) {
          HEAPU32[((config)>>2)] = 62002;
        }
  
        EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
        return 1;
      },
  };
  var _eglBindAPI = (api) => {
      if (api == 0x30A0 /* EGL_OPENGL_ES_API */) {
        EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
        return 1;
      }
      // if (api == 0x30A1 /* EGL_OPENVG_API */ || api == 0x30A2 /* EGL_OPENGL_API */) {
      EGL.setErrorCode(0x300C /* EGL_BAD_PARAMETER */);
      return 0;
    };

  var _eglChooseConfig = (display, attrib_list, configs, config_size, numConfigs) =>
      EGL.chooseConfig(display, attrib_list, configs, config_size, numConfigs);

  var GLctx;
  
  var webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance = (ctx) =>
      // Closure is expected to be allowed to minify the '.dibvbi' property, so not accessing it quoted.
      !!(ctx.dibvbi = ctx.getExtension('WEBGL_draw_instanced_base_vertex_base_instance'));
  
  var webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance = (ctx) => {
      // Closure is expected to be allowed to minify the '.mdibvbi' property, so not accessing it quoted.
      return !!(ctx.mdibvbi = ctx.getExtension('WEBGL_multi_draw_instanced_base_vertex_base_instance'));
    };
  
  var webgl_enable_EXT_polygon_offset_clamp = (ctx) =>
      !!(ctx.extPolygonOffsetClamp = ctx.getExtension('EXT_polygon_offset_clamp'));
  
  var webgl_enable_EXT_clip_control = (ctx) =>
      !!(ctx.extClipControl = ctx.getExtension('EXT_clip_control'));
  
  var webgl_enable_WEBGL_polygon_mode = (ctx) =>
      !!(ctx.webglPolygonMode = ctx.getExtension('WEBGL_polygon_mode'));
  
  var webgl_enable_WEBGL_multi_draw = (ctx) =>
      // Closure is expected to be allowed to minify the '.multiDrawWebgl' property, so not accessing it quoted.
      !!(ctx.multiDrawWebgl = ctx.getExtension('WEBGL_multi_draw'));
  
  var getEmscriptenSupportedExtensions = (ctx) => {
      // Restrict the list of advertised extensions to those that we actually
      // support.
      var supportedExtensions = [
        // WebGL 2 extensions
        'EXT_color_buffer_float',
        'EXT_conservative_depth',
        'EXT_disjoint_timer_query_webgl2',
        'EXT_texture_norm16',
        'NV_shader_noperspective_interpolation',
        'WEBGL_clip_cull_distance',
        // WebGL 1 and WebGL 2 extensions
        'EXT_clip_control',
        'EXT_color_buffer_half_float',
        'EXT_depth_clamp',
        'EXT_float_blend',
        'EXT_polygon_offset_clamp',
        'EXT_texture_compression_bptc',
        'EXT_texture_compression_rgtc',
        'EXT_texture_filter_anisotropic',
        'KHR_parallel_shader_compile',
        'OES_texture_float_linear',
        'WEBGL_blend_func_extended',
        'WEBGL_compressed_texture_astc',
        'WEBGL_compressed_texture_etc',
        'WEBGL_compressed_texture_etc1',
        'WEBGL_compressed_texture_s3tc',
        'WEBGL_compressed_texture_s3tc_srgb',
        'WEBGL_debug_renderer_info',
        'WEBGL_debug_shaders',
        'WEBGL_lose_context',
        'WEBGL_multi_draw',
        'WEBGL_polygon_mode'
      ];
      // .getSupportedExtensions() can return null if context is lost, so coerce to empty array.
      return (ctx.getSupportedExtensions() || []).filter(ext => supportedExtensions.includes(ext));
    };
  
  
  var GL = {
  counter:1,
  buffers:[],
  programs:[],
  framebuffers:[],
  renderbuffers:[],
  textures:[],
  shaders:[],
  vaos:[],
  contexts:[],
  offscreenCanvases:{
  },
  queries:[],
  samplers:[],
  transformFeedbacks:[],
  syncs:[],
  stringCache:{
  },
  stringiCache:{
  },
  unpackAlignment:4,
  unpackRowLength:0,
  recordError:(errorCode) => {
        if (!GL.lastError) {
          GL.lastError = errorCode;
        }
      },
  getNewId:(table) => {
        var ret = GL.counter++;
        for (var i = table.length; i < ret; i++) {
          table[i] = null;
        }
        return ret;
      },
  genObject:(n, buffers, createFunction, objectTable
        ) => {
        for (var i = 0; i < n; i++) {
          var buffer = GLctx[createFunction]();
          var id = buffer && GL.getNewId(objectTable);
          if (buffer) {
            buffer.name = id;
            objectTable[id] = buffer;
          } else {
            GL.recordError(0x502 /* GL_INVALID_OPERATION */);
          }
          HEAP32[(((buffers)+(i*4))>>2)] = id;
        }
      },
  getSource:(shader, count, string, length) => {
        var source = '';
        for (var i = 0; i < count; ++i) {
          var len = length ? HEAPU32[(((length)+(i*4))>>2)] : undefined;
          source += UTF8ToString(HEAPU32[(((string)+(i*4))>>2)], len);
        }
        return source;
      },
  createContext:(/** @type {HTMLCanvasElement} */ canvas, webGLContextAttributes) => {
  
        // If WebGL context has already been preinitialized for the page on the JS
        // side, reuse that context instead. This is useful for example when the
        // main page precompiles shaders for the application, in which case the
        // WebGL context is created already before any Emscripten compiled code
        // has been downloaded.
        if (Module['preinitializedWebGLContext']) {
          var ctx = Module['preinitializedWebGLContext'];
          // The ctx object may not be of a known class (e.g. it may be a debug
          // wrapper), so we ask it for its version rather than use instanceof.
          webGLContextAttributes.majorVersion = Number(ctx.getParameter(ctx.VERSION).match(/^WebGL (\d+).\d+/)[1]);
        } else {
  
        // BUG: Workaround Safari WebGL issue: After successfully acquiring WebGL
        // context on a canvas, calling .getContext() will always return that
        // context independent of which 'webgl' or 'webgl2'
        // context version was passed. See:
        //   https://bugs.webkit.org/show_bug.cgi?id=222758
        // and:
        //   https://github.com/emscripten-core/emscripten/issues/13295.
        // TODO: Once the bug is fixed and shipped in Safari, adjust the Safari
        // version field in above check.
        if (!canvas.getContextSafariWebGL2Fixed) {
          canvas.getContextSafariWebGL2Fixed = canvas.getContext;
          /** @type {function(this:HTMLCanvasElement, string, (Object|null)=): (Object|null)} */
          function fixedGetContext(ver, attrs) {
            var gl = canvas.getContextSafariWebGL2Fixed(ver, attrs);
            return ((ver == 'webgl') == (gl instanceof WebGLRenderingContext)) ? gl : null;
          }
          canvas.getContext = fixedGetContext;
        }
  
        var ctx = canvas.getContext("webgl2", webGLContextAttributes);
  
        }
  
        if (!ctx) return 0;
  
        var handle = GL.registerContext(ctx, webGLContextAttributes);
  
        return handle;
      },
  registerContext:(ctx, webGLContextAttributes) => {
        // without pthreads a context is just an integer ID
        var handle = GL.getNewId(GL.contexts);
  
        var context = {
          handle,
          attributes: webGLContextAttributes,
          version: webGLContextAttributes.majorVersion,
          GLctx: ctx
        };
  
        // Store the created context object so that we can access the context
        // given a canvas without having to pass the parameters again.
        if (ctx.canvas) ctx.canvas.GLctxObject = context;
        GL.contexts[handle] = context;
        if (typeof webGLContextAttributes.enableExtensionsByDefault == 'undefined' || webGLContextAttributes.enableExtensionsByDefault) {
          GL.initExtensions(context);
        }
  
        return handle;
      },
  makeContextCurrent:(contextHandle) => {
  
        // Active Emscripten GL layer context object.
        GL.currentContext = GL.contexts[contextHandle];
        // Active WebGL context object.
        Module['ctx'] = GLctx = GL.currentContext?.GLctx;
        return !(contextHandle && !GLctx);
      },
  getContext:(contextHandle) => {
        return GL.contexts[contextHandle];
      },
  deleteContext:(contextHandle) => {
        if (GL.currentContext === GL.contexts[contextHandle]) {
          GL.currentContext = null;
        }
        if (typeof JSEvents == 'object') {
          // Release all JS event handlers on the DOM element that the GL context is
          // associated with since the context is now deleted.
          JSEvents.removeAllHandlersOnTarget(GL.contexts[contextHandle].GLctx.canvas);
        }
        // Make sure the canvas object no longer refers to the context object so
        // there are no GC surprises.
        if (GL.contexts[contextHandle]?.GLctx.canvas) {
          GL.contexts[contextHandle].GLctx.canvas.GLctxObject = undefined;
        }
        GL.contexts[contextHandle] = null;
      },
  initExtensions:(context) => {
        // If this function is called without a specific context object, init the
        // extensions of the currently active context.
        context ||= GL.currentContext;
  
        if (context.initExtensionsDone) return;
        context.initExtensionsDone = true;
  
        var GLctx = context.GLctx;
  
        // Detect the presence of a few extensions manually, ction GL interop
        // layer itself will need to know if they exist.
  
        // Extensions that are available in both WebGL 1 and WebGL 2
        webgl_enable_WEBGL_multi_draw(GLctx);
        webgl_enable_EXT_polygon_offset_clamp(GLctx);
        webgl_enable_EXT_clip_control(GLctx);
        webgl_enable_WEBGL_polygon_mode(GLctx);
        // Extensions that are available from WebGL >= 2 (no-op if called on a WebGL 1 context active)
        webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance(GLctx);
        webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance(GLctx);
  
        // On WebGL 2, EXT_disjoint_timer_query is replaced with an alternative
        // that's based on core APIs, and exposes only the queryCounterEXT()
        // entrypoint.
        if (context.version >= 2) {
          GLctx.disjointTimerQueryExt = GLctx.getExtension("EXT_disjoint_timer_query_webgl2");
        }
  
        // However, Firefox exposes the WebGL 1 version on WebGL 2 as well and
        // thus we look for the WebGL 1 version again if the WebGL 2 version
        // isn't present. https://bugzilla.mozilla.org/show_bug.cgi?id=1328882
        if (context.version < 2 || !GLctx.disjointTimerQueryExt)
        {
          GLctx.disjointTimerQueryExt = GLctx.getExtension("EXT_disjoint_timer_query");
        }
  
        getEmscriptenSupportedExtensions(GLctx).forEach((ext) => {
          // WEBGL_lose_context, WEBGL_debug_renderer_info and WEBGL_debug_shaders
          // are not enabled by default.
          if (!ext.includes('lose_context') && !ext.includes('debug')) {
            // Call .getExtension() to enable that extension permanently.
            GLctx.getExtension(ext);
          }
        });
      },
  };
  
  var _eglCreateContext = (display, config, hmm, contextAttribs) => {
      if (display != 62000) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0;
      }
  
      // EGL 1.4 spec says default EGL_CONTEXT_CLIENT_VERSION is GLES1, but this is not supported by Emscripten.
      // So user must pass EGL_CONTEXT_CLIENT_VERSION == 2 to initialize EGL.
      var glesContextVersion = 1;
      for (;;) {
        var param = HEAP32[((contextAttribs)>>2)];
        if (param == 0x3098 /*EGL_CONTEXT_CLIENT_VERSION*/) {
          glesContextVersion = HEAP32[(((contextAttribs)+(4))>>2)];
        } else if (param == 0x3038 /*EGL_NONE*/) {
          break;
        } else {
          /* EGL1.4 specifies only EGL_CONTEXT_CLIENT_VERSION as supported attribute */
          EGL.setErrorCode(0x3004 /*EGL_BAD_ATTRIBUTE*/);
          return 0;
        }
        contextAttribs += 8;
      }
      if (glesContextVersion < 2 || glesContextVersion > 3) {
        EGL.setErrorCode(0x3005 /* EGL_BAD_CONFIG */);
        return 0; /* EGL_NO_CONTEXT */
      }
  
      EGL.contextAttributes.majorVersion = glesContextVersion - 1; // WebGL 1 is GLES 2, WebGL2 is GLES3
      EGL.contextAttributes.minorVersion = 0;
  
      EGL.context = GL.createContext(Browser.getCanvas(), EGL.contextAttributes);
  
      if (EGL.context != 0) {
        EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
  
        // Run callbacks so that GL emulation works
        GL.makeContextCurrent(EGL.context);
        Browser.useWebGL = true;
        Browser.moduleContextCreatedCallbacks.forEach((callback) => callback());
  
        // Note: This function only creates a context, but it shall not make it active.
        GL.makeContextCurrent(null);
        return 62004;
      } else {
        EGL.setErrorCode(0x3009 /* EGL_BAD_MATCH */); // By the EGL 1.4 spec, an implementation that does not support GLES2 (WebGL in this case), this error code is set.
        return 0; /* EGL_NO_CONTEXT */
      }
    };

  var _eglCreateWindowSurface = (display, config, win, attrib_list) => {
      if (display != 62000) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0;
      }
      if (config != 62002) {
        EGL.setErrorCode(0x3005 /* EGL_BAD_CONFIG */);
        return 0;
      }
      // TODO: Examine attrib_list! Parameters that can be present there are:
      // - EGL_RENDER_BUFFER (must be EGL_BACK_BUFFER)
      // - EGL_VG_COLORSPACE (can't be set)
      // - EGL_VG_ALPHA_FORMAT (can't be set)
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      return 62006; /* Magic ID for Emscripten 'default surface' */
    };

  
  var _eglDestroyContext = (display, context) => {
      if (display != 62000) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0;
      }
      if (context != 62004) {
        EGL.setErrorCode(0x3006 /* EGL_BAD_CONTEXT */);
        return 0;
      }
  
      GL.deleteContext(EGL.context);
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      if (EGL.currentContext == context) {
        EGL.currentContext = 0;
      }
      return 1 /* EGL_TRUE */;
    };

  var _eglDestroySurface = (display, surface) => {
      if (display != 62000) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0;
      }
      if (surface != 62006 /* Magic ID for the only EGLSurface supported by Emscripten */) {
        EGL.setErrorCode(0x300D /* EGL_BAD_SURFACE */);
        return 1;
      }
      if (EGL.currentReadSurface == surface) {
        EGL.currentReadSurface = 0;
      }
      if (EGL.currentDrawSurface == surface) {
        EGL.currentDrawSurface = 0;
      }
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      return 1; /* Magic ID for Emscripten 'default surface' */
    };

  var _eglGetConfigAttrib = (display, config, attribute, value) => {
      if (display != 62000) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0;
      }
      if (config != 62002) {
        EGL.setErrorCode(0x3005 /* EGL_BAD_CONFIG */);
        return 0;
      }
      if (!value) {
        EGL.setErrorCode(0x300C /* EGL_BAD_PARAMETER */);
        return 0;
      }
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      switch (attribute) {
      case 0x3020: // EGL_BUFFER_SIZE
        HEAP32[((value)>>2)] = EGL.contextAttributes.alpha ? 32 : 24;
        return 1;
      case 0x3021: // EGL_ALPHA_SIZE
        HEAP32[((value)>>2)] = EGL.contextAttributes.alpha ? 8 : 0;
        return 1;
      case 0x3022: // EGL_BLUE_SIZE
        HEAP32[((value)>>2)] = 8;
        return 1;
      case 0x3023: // EGL_GREEN_SIZE
        HEAP32[((value)>>2)] = 8;
        return 1;
      case 0x3024: // EGL_RED_SIZE
        HEAP32[((value)>>2)] = 8;
        return 1;
      case 0x3025: // EGL_DEPTH_SIZE
        HEAP32[((value)>>2)] = EGL.contextAttributes.depth ? 24 : 0;
        return 1;
      case 0x3026: // EGL_STENCIL_SIZE
        HEAP32[((value)>>2)] = EGL.contextAttributes.stencil ? 8 : 0;
        return 1;
      case 0x3027: // EGL_CONFIG_CAVEAT
        // We can return here one of EGL_NONE (0x3038), EGL_SLOW_CONFIG (0x3050) or EGL_NON_CONFORMANT_CONFIG (0x3051).
        HEAP32[((value)>>2)] = 0x3038;
        return 1;
      case 0x3028: // EGL_CONFIG_ID
        HEAP32[((value)>>2)] = 62002;
        return 1;
      case 0x3029: // EGL_LEVEL
        HEAP32[((value)>>2)] = 0;
        return 1;
      case 0x302A: // EGL_MAX_PBUFFER_HEIGHT
        HEAP32[((value)>>2)] = 4096;
        return 1;
      case 0x302B: // EGL_MAX_PBUFFER_PIXELS
        HEAP32[((value)>>2)] = 16777216;
        return 1;
      case 0x302C: // EGL_MAX_PBUFFER_WIDTH
        HEAP32[((value)>>2)] = 4096;
        return 1;
      case 0x302D: // EGL_NATIVE_RENDERABLE
        HEAP32[((value)>>2)] = 0;
        return 1;
      case 0x302E: // EGL_NATIVE_VISUAL_ID
        HEAP32[((value)>>2)] = 0;
        return 1;
      case 0x302F: // EGL_NATIVE_VISUAL_TYPE
        HEAP32[((value)>>2)] = 0x3038;
        return 1;
      case 0x3031: // EGL_SAMPLES
        HEAP32[((value)>>2)] = EGL.contextAttributes.antialias ? 4 : 0;
        return 1;
      case 0x3032: // EGL_SAMPLE_BUFFERS
        HEAP32[((value)>>2)] = EGL.contextAttributes.antialias ? 1 : 0;
        return 1;
      case 0x3033: // EGL_SURFACE_TYPE
        HEAP32[((value)>>2)] = 0x4;
        return 1;
      case 0x3034: // EGL_TRANSPARENT_TYPE
        // If this returns EGL_TRANSPARENT_RGB (0x3052), transparency is used through color-keying. No such thing applies to Emscripten canvas.
        HEAP32[((value)>>2)] = 0x3038;
        return 1;
      case 0x3035: // EGL_TRANSPARENT_BLUE_VALUE
      case 0x3036: // EGL_TRANSPARENT_GREEN_VALUE
      case 0x3037: // EGL_TRANSPARENT_RED_VALUE
        // "If EGL_TRANSPARENT_TYPE is EGL_NONE, then the values for EGL_TRANSPARENT_RED_VALUE, EGL_TRANSPARENT_GREEN_VALUE, and EGL_TRANSPARENT_BLUE_VALUE are undefined."
        HEAP32[((value)>>2)] = -1;
        return 1;
      case 0x3039: // EGL_BIND_TO_TEXTURE_RGB
      case 0x303A: // EGL_BIND_TO_TEXTURE_RGBA
        HEAP32[((value)>>2)] = 0;
        return 1;
      case 0x303B: // EGL_MIN_SWAP_INTERVAL
        HEAP32[((value)>>2)] = 0;
        return 1;
      case 0x303C: // EGL_MAX_SWAP_INTERVAL
        HEAP32[((value)>>2)] = 1;
        return 1;
      case 0x303D: // EGL_LUMINANCE_SIZE
      case 0x303E: // EGL_ALPHA_MASK_SIZE
        HEAP32[((value)>>2)] = 0;
        return 1;
      case 0x303F: // EGL_COLOR_BUFFER_TYPE
        // EGL has two types of buffers: EGL_RGB_BUFFER and EGL_LUMINANCE_BUFFER.
        HEAP32[((value)>>2)] = 0x308E;
        return 1;
      case 0x3040: // EGL_RENDERABLE_TYPE
        // A bit combination of EGL_OPENGL_ES_BIT,EGL_OPENVG_BIT,EGL_OPENGL_ES2_BIT and EGL_OPENGL_BIT.
        HEAP32[((value)>>2)] = 0x4;
        return 1;
      case 0x3042: // EGL_CONFORMANT
        // "EGL_CONFORMANT is a mask indicating if a client API context created with respect to the corresponding EGLConfig will pass the required conformance tests for that API."
        HEAP32[((value)>>2)] = 0;
        return 1;
      default:
        EGL.setErrorCode(0x3004 /* EGL_BAD_ATTRIBUTE */);
        return 0;
      }
    };

  var _eglGetDisplay = (nativeDisplayType) => {
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      // Emscripten EGL implementation "emulates" X11, and eglGetDisplay is
      // expected to accept/receive a pointer to an X11 Display object (or
      // EGL_DEFAULT_DISPLAY).
      if (nativeDisplayType != 0 /* EGL_DEFAULT_DISPLAY */ && nativeDisplayType != 1 /* see library_xlib.js */) {
        return 0; // EGL_NO_DISPLAY
      }
      return 62000;
    };

  var _eglGetError = () => EGL.errorCode;

  var _eglInitialize = (display, majorVersion, minorVersion) => {
      if (display != 62000) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0;
      }
      if (majorVersion) {
        HEAP32[((majorVersion)>>2)] = 1; // Advertise EGL Major version: '1'
      }
      if (minorVersion) {
        HEAP32[((minorVersion)>>2)] = 4; // Advertise EGL Minor version: '4'
      }
      EGL.defaultDisplayInitialized = true;
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      return 1;
    };

  
  var _eglMakeCurrent = (display, draw, read, context) => {
      if (display != 62000) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0 /* EGL_FALSE */;
      }
      //\todo An EGL_NOT_INITIALIZED error is generated if EGL is not initialized for dpy.
      if (context != 0 && context != 62004) {
        EGL.setErrorCode(0x3006 /* EGL_BAD_CONTEXT */);
        return 0;
      }
      if ((read != 0 && read != 62006) || (draw != 0 && draw != 62006 /* Magic ID for Emscripten 'default surface' */)) {
        EGL.setErrorCode(0x300D /* EGL_BAD_SURFACE */);
        return 0;
      }
  
      GL.makeContextCurrent(context ? EGL.context : null);
  
      EGL.currentContext = context;
      EGL.currentDrawSurface = draw;
      EGL.currentReadSurface = read;
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      return 1 /* EGL_TRUE */;
    };

  
  var _eglQueryString = (display, name) => {
      if (display != 62000) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0;
      }
      //\todo An EGL_NOT_INITIALIZED error is generated if EGL is not initialized for dpy.
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      if (EGL.stringCache[name]) return EGL.stringCache[name];
      var ret;
      switch (name) {
        case 0x3053 /* EGL_VENDOR */: ret = stringToNewUTF8("Emscripten"); break;
        case 0x3054 /* EGL_VERSION */: ret = stringToNewUTF8("1.4 Emscripten EGL"); break;
        case 0x3055 /* EGL_EXTENSIONS */:  ret = stringToNewUTF8(""); break; // Currently not supporting any EGL extensions.
        case 0x308D /* EGL_CLIENT_APIS */: ret = stringToNewUTF8("OpenGL_ES"); break;
        default:
          EGL.setErrorCode(0x300C /* EGL_BAD_PARAMETER */);
          return 0;
      }
      EGL.stringCache[name] = ret;
      return ret;
    };

  
  var _eglSwapBuffers = (dpy, surface) => {
  
      if (!EGL.defaultDisplayInitialized) {
        EGL.setErrorCode(0x3001 /* EGL_NOT_INITIALIZED */);
      } else if (!GLctx) {
        EGL.setErrorCode(0x3002 /* EGL_BAD_ACCESS */);
      } else if (GLctx.isContextLost()) {
        EGL.setErrorCode(0x300E /* EGL_CONTEXT_LOST */);
      } else {
        // According to documentation this does an implicit flush.
        // Due to discussion at https://github.com/emscripten-core/emscripten/pull/1871
        // the flush was removed since this _may_ result in slowing code down.
        //_glFlush();
        EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
        return 1 /* EGL_TRUE */;
      }
      return 0 /* EGL_FALSE */;
    };

  
  var _eglSwapInterval = (display, interval) => {
      if (display != 62000) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0;
      }
      if (interval == 0) _emscripten_set_main_loop_timing(0, 0);
      else _emscripten_set_main_loop_timing(1, interval);
  
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      return 1;
    };

  var _eglTerminate = (display) => {
      if (display != 62000) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0;
      }
      EGL.currentContext = 0;
      EGL.currentReadSurface = 0;
      EGL.currentDrawSurface = 0;
      EGL.defaultDisplayInitialized = false;
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      return 1;
    };

  
  /** @suppress {duplicate } */
  var _eglWaitClient = () => {
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      return 1;
    };
  var _eglWaitGL = _eglWaitClient;

  var _eglWaitNative = (nativeEngineId) => {
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      return 1;
    };

  var readEmAsmArgsArray = [];
  var readEmAsmArgs = (sigPtr, buf) => {
      // Nobody should have mutated _readEmAsmArgsArray underneath us to be something else than an array.
      assert(Array.isArray(readEmAsmArgsArray));
      // The input buffer is allocated on the stack, so it must be stack-aligned.
      assert(buf % 16 == 0);
      readEmAsmArgsArray.length = 0;
      var ch;
      // Most arguments are i32s, so shift the buffer pointer so it is a plain
      // index into HEAP32.
      while (ch = HEAPU8[sigPtr++]) {
        var chr = String.fromCharCode(ch);
        var validChars = ['d', 'f', 'i', 'p'];
        // In WASM_BIGINT mode we support passing i64 values as bigint.
        validChars.push('j');
        assert(validChars.includes(chr), `Invalid character ${ch}("${chr}") in readEmAsmArgs! Use only [${validChars}], and do not specify "v" for void return argument.`);
        // Floats are always passed as doubles, so all types except for 'i'
        // are 8 bytes and require alignment.
        var wide = (ch != 105);
        wide &= (ch != 112);
        buf += wide && (buf % 8) ? 4 : 0;
        readEmAsmArgsArray.push(
          // Special case for pointers under wasm64 or CAN_ADDRESS_2GB mode.
          ch == 112 ? HEAPU32[((buf)>>2)] :
          ch == 106 ? HEAP64[((buf)>>3)] :
          ch == 105 ?
            HEAP32[((buf)>>2)] :
            HEAPF64[((buf)>>3)]
        );
        buf += wide ? 8 : 4;
      }
      return readEmAsmArgsArray;
    };
  var runEmAsmFunction = (code, sigPtr, argbuf) => {
      var args = readEmAsmArgs(sigPtr, argbuf);
      assert(ASM_CONSTS.hasOwnProperty(code), `No EM_ASM constant found at address ${code}.  The loaded WebAssembly file is likely out of sync with the generated JavaScript.`);
      return ASM_CONSTS[code](...args);
    };
  var _emscripten_asm_const_int = (code, sigPtr, argbuf) => {
      return runEmAsmFunction(code, sigPtr, argbuf);
    };

  var runMainThreadEmAsm = (emAsmAddr, sigPtr, argbuf, sync) => {
      var args = readEmAsmArgs(sigPtr, argbuf);
      assert(ASM_CONSTS.hasOwnProperty(emAsmAddr), `No EM_ASM constant found at address ${emAsmAddr}.  The loaded WebAssembly file is likely out of sync with the generated JavaScript.`);
      return ASM_CONSTS[emAsmAddr](...args);
    };
  var _emscripten_asm_const_int_sync_on_main_thread = (emAsmAddr, sigPtr, argbuf) => runMainThreadEmAsm(emAsmAddr, sigPtr, argbuf, 1);

  var _emscripten_asm_const_ptr_sync_on_main_thread = (emAsmAddr, sigPtr, argbuf) => runMainThreadEmAsm(emAsmAddr, sigPtr, argbuf, 1);


  var onExits = [];
  var addOnExit = (cb) => onExits.unshift(cb);
  var JSEvents = {
  memcpy(target, src, size) {
        HEAP8.set(HEAP8.subarray(src, src + size), target);
      },
  removeAllEventListeners() {
        while (JSEvents.eventHandlers.length) {
          JSEvents._removeHandler(JSEvents.eventHandlers.length - 1);
        }
        JSEvents.deferredCalls = [];
      },
  inEventHandler:0,
  deferredCalls:[],
  deferCall(targetFunction, precedence, argsList) {
        function arraysHaveEqualContent(arrA, arrB) {
          if (arrA.length != arrB.length) return false;
  
          for (var i in arrA) {
            if (arrA[i] != arrB[i]) return false;
          }
          return true;
        }
        // Test if the given call was already queued, and if so, don't add it again.
        for (var call of JSEvents.deferredCalls) {
          if (call.targetFunction == targetFunction && arraysHaveEqualContent(call.argsList, argsList)) {
            return;
          }
        }
        JSEvents.deferredCalls.push({
          targetFunction,
          precedence,
          argsList
        });
  
        JSEvents.deferredCalls.sort((x,y) => x.precedence < y.precedence);
      },
  removeDeferredCalls(targetFunction) {
        JSEvents.deferredCalls = JSEvents.deferredCalls.filter((call) => call.targetFunction != targetFunction);
      },
  canPerformEventHandlerRequests() {
        if (navigator.userActivation) {
          // Verify against transient activation status from UserActivation API
          // whether it is possible to perform a request here without needing to defer. See
          // https://developer.mozilla.org/en-US/docs/Web/Security/User_activation#transient_activation
          // and https://caniuse.com/mdn-api_useractivation
          // At the time of writing, Firefox does not support this API: https://bugzilla.mozilla.org/show_bug.cgi?id=1791079
          return navigator.userActivation.isActive;
        }
  
        return JSEvents.inEventHandler && JSEvents.currentEventHandler.allowsDeferredCalls;
      },
  runDeferredCalls() {
        if (!JSEvents.canPerformEventHandlerRequests()) {
          return;
        }
        var deferredCalls = JSEvents.deferredCalls;
        JSEvents.deferredCalls = [];
        for (var call of deferredCalls) {
          call.targetFunction(...call.argsList);
        }
      },
  eventHandlers:[],
  removeAllHandlersOnTarget:(target, eventTypeString) => {
        for (var i = 0; i < JSEvents.eventHandlers.length; ++i) {
          if (JSEvents.eventHandlers[i].target == target &&
            (!eventTypeString || eventTypeString == JSEvents.eventHandlers[i].eventTypeString)) {
             JSEvents._removeHandler(i--);
           }
        }
      },
  _removeHandler(i) {
        var h = JSEvents.eventHandlers[i];
        h.target.removeEventListener(h.eventTypeString, h.eventListenerFunc, h.useCapture);
        JSEvents.eventHandlers.splice(i, 1);
      },
  registerOrRemoveHandler(eventHandler) {
        if (!eventHandler.target) {
          err('registerOrRemoveHandler: the target element for event handler registration does not exist, when processing the following event handler registration:');
          console.dir(eventHandler);
          return -4;
        }
        if (eventHandler.callbackfunc) {
          eventHandler.eventListenerFunc = function(event) {
            // Increment nesting count for the event handler.
            ++JSEvents.inEventHandler;
            JSEvents.currentEventHandler = eventHandler;
            // Process any old deferred calls the user has placed.
            JSEvents.runDeferredCalls();
            // Process the actual event, calls back to user C code handler.
            eventHandler.handlerFunc(event);
            // Process any new deferred calls that were placed right now from this event handler.
            JSEvents.runDeferredCalls();
            // Out of event handler - restore nesting count.
            --JSEvents.inEventHandler;
          };
  
          eventHandler.target.addEventListener(eventHandler.eventTypeString,
                                               eventHandler.eventListenerFunc,
                                               eventHandler.useCapture);
          JSEvents.eventHandlers.push(eventHandler);
        } else {
          for (var i = 0; i < JSEvents.eventHandlers.length; ++i) {
            if (JSEvents.eventHandlers[i].target == eventHandler.target
             && JSEvents.eventHandlers[i].eventTypeString == eventHandler.eventTypeString) {
               JSEvents._removeHandler(i--);
             }
          }
        }
        return 0;
      },
  getNodeNameForTarget(target) {
        if (!target) return '';
        if (target == window) return '#window';
        if (target == screen) return '#screen';
        return target?.nodeName || '';
      },
  fullscreenEnabled() {
        return document.fullscreenEnabled
        // Safari 13.0.3 on macOS Catalina 10.15.1 still ships with prefixed webkitFullscreenEnabled.
        // TODO: If Safari at some point ships with unprefixed version, update the version check above.
        || document.webkitFullscreenEnabled
         ;
      },
  };
  
  var hideEverythingExceptGivenElement = (onlyVisibleElement) => {
      var child = onlyVisibleElement;
      var parent = child.parentNode;
      var hiddenElements = [];
      while (child != document.body) {
        var children = parent.children;
        for (var currChild of children) {
          if (currChild != child) {
            hiddenElements.push({ node: currChild, displayState: currChild.style.display });
            currChild.style.display = 'none';
          }
        }
        child = parent;
        parent = parent.parentNode;
      }
      return hiddenElements;
    };
  
  var restoreOldWindowedStyle = null;
  
  var restoreHiddenElements = (hiddenElements) => {
      for (var elem of hiddenElements) {
        elem.node.style.display = elem.displayState;
      }
    };
  
  var currentFullscreenStrategy = {
  };
  
  var setLetterbox = (element, topBottom, leftRight) => {
      // Cannot use margin to specify letterboxes in FF or Chrome, since those ignore margins in fullscreen mode.
      element.style.paddingLeft = element.style.paddingRight = leftRight + 'px';
      element.style.paddingTop = element.style.paddingBottom = topBottom + 'px';
    };
  
  
  var maybeCStringToJsString = (cString) => {
      // "cString > 2" checks if the input is a number, and isn't of the special
      // values we accept here, EMSCRIPTEN_EVENT_TARGET_* (which map to 0, 1, 2).
      // In other words, if cString > 2 then it's a pointer to a valid place in
      // memory, and points to a C string.
      return cString > 2 ? UTF8ToString(cString) : cString;
    };
  
  /** @type {Object} */
  var specialHTMLTargets = [0, typeof document != 'undefined' ? document : 0, typeof window != 'undefined' ? window : 0];
  /** @suppress {duplicate } */
  var findEventTarget = (target) => {
      target = maybeCStringToJsString(target);
      var domElement = specialHTMLTargets[target] || (typeof document != 'undefined' ? document.querySelector(target) : null);
      return domElement;
    };
  var findCanvasEventTarget = findEventTarget;
  var _emscripten_get_canvas_element_size = (target, width, height) => {
      var canvas = findCanvasEventTarget(target);
      if (!canvas) return -4;
      HEAP32[((width)>>2)] = canvas.width;
      HEAP32[((height)>>2)] = canvas.height;
    };
  
  
  
  
  
  var stackAlloc = (sz) => __emscripten_stack_alloc(sz);
  var stringToUTF8OnStack = (str) => {
      var size = lengthBytesUTF8(str) + 1;
      var ret = stackAlloc(size);
      stringToUTF8(str, ret, size);
      return ret;
    };
  var getCanvasElementSize = (target) => {
      var sp = stackSave();
      var w = stackAlloc(8);
      var h = w + 4;
  
      var targetInt = stringToUTF8OnStack(target.id);
      var ret = _emscripten_get_canvas_element_size(targetInt, w, h);
      var size = [HEAP32[((w)>>2)], HEAP32[((h)>>2)]];
      stackRestore(sp);
      return size;
    };
  
  var _emscripten_set_canvas_element_size = (target, width, height) => {
      var canvas = findCanvasEventTarget(target);
      if (!canvas) return -4;
      canvas.width = width;
      canvas.height = height;
      return 0;
    };
  
  
  
  var setCanvasElementSize = (target, width, height) => {
      if (!target.controlTransferredOffscreen) {
        target.width = width;
        target.height = height;
      } else {
        // This function is being called from high-level JavaScript code instead of asm.js/Wasm,
        // and it needs to synchronously proxy over to another thread, so marshal the string onto the heap to do the call.
        var sp = stackSave();
        var targetInt = stringToUTF8OnStack(target.id);
        _emscripten_set_canvas_element_size(targetInt, width, height);
        stackRestore(sp);
      }
    };
  
  /** @suppress {checkTypes} */
  var jstoi_q = (str) => parseInt(str);
  
  var wasmTableMirror = [];
  
  /** @type {WebAssembly.Table} */
  var wasmTable;
  var getWasmTableEntry = (funcPtr) => {
      var func = wasmTableMirror[funcPtr];
      if (!func) {
        /** @suppress {checkTypes} */
        wasmTableMirror[funcPtr] = func = wasmTable.get(funcPtr);
      }
      /** @suppress {checkTypes} */
      assert(wasmTable.get(funcPtr) == func, 'JavaScript-side Wasm function table mirror is out of date!');
      return func;
    };
  var softFullscreenResizeWebGLRenderTarget = () => {
      var dpr = devicePixelRatio;
      var inHiDPIFullscreenMode = currentFullscreenStrategy.canvasResolutionScaleMode == 2;
      var inAspectRatioFixedFullscreenMode = currentFullscreenStrategy.scaleMode == 2;
      var inPixelPerfectFullscreenMode = currentFullscreenStrategy.canvasResolutionScaleMode != 0;
      var inCenteredWithoutScalingFullscreenMode = currentFullscreenStrategy.scaleMode == 3;
      var screenWidth = inHiDPIFullscreenMode ? Math.round(innerWidth*dpr) : innerWidth;
      var screenHeight = inHiDPIFullscreenMode ? Math.round(innerHeight*dpr) : innerHeight;
      var w = screenWidth;
      var h = screenHeight;
      var canvas = currentFullscreenStrategy.target;
      var canvasSize = getCanvasElementSize(canvas);
      var x = canvasSize[0];
      var y = canvasSize[1];
      var topMargin;
  
      if (inAspectRatioFixedFullscreenMode) {
        if (w*y < x*h) h = (w * y / x) | 0;
        else if (w*y > x*h) w = (h * x / y) | 0;
        topMargin = ((screenHeight - h) / 2) | 0;
      }
  
      if (inPixelPerfectFullscreenMode) {
        setCanvasElementSize(canvas, w, h);
        if (canvas.GLctxObject) canvas.GLctxObject.GLctx.viewport(0, 0, w, h);
      }
  
      // Back to CSS pixels.
      if (inHiDPIFullscreenMode) {
        topMargin /= dpr;
        w /= dpr;
        h /= dpr;
        // Round to nearest 4 digits of precision.
        w = Math.round(w*1e4)/1e4;
        h = Math.round(h*1e4)/1e4;
        topMargin = Math.round(topMargin*1e4)/1e4;
      }
  
      if (inCenteredWithoutScalingFullscreenMode) {
        var t = (innerHeight - jstoi_q(canvas.style.height)) / 2;
        var b = (innerWidth - jstoi_q(canvas.style.width)) / 2;
        setLetterbox(canvas, t, b);
      } else {
        canvas.style.width = w + 'px';
        canvas.style.height = h + 'px';
        var b = (innerWidth - w) / 2;
        setLetterbox(canvas, topMargin, b);
      }
  
      if (!inCenteredWithoutScalingFullscreenMode && currentFullscreenStrategy.canvasResizedCallback) {
        getWasmTableEntry(currentFullscreenStrategy.canvasResizedCallback)(37, 0, currentFullscreenStrategy.canvasResizedCallbackUserData);
      }
    };
  
  
  
  
  var registerRestoreOldStyle = (canvas) => {
      var canvasSize = getCanvasElementSize(canvas);
      var oldWidth = canvasSize[0];
      var oldHeight = canvasSize[1];
      var oldCssWidth = canvas.style.width;
      var oldCssHeight = canvas.style.height;
      var oldBackgroundColor = canvas.style.backgroundColor; // Chrome reads color from here.
      var oldDocumentBackgroundColor = document.body.style.backgroundColor; // IE11 reads color from here.
      // Firefox always has black background color.
      var oldPaddingLeft = canvas.style.paddingLeft; // Chrome, FF, Safari
      var oldPaddingRight = canvas.style.paddingRight;
      var oldPaddingTop = canvas.style.paddingTop;
      var oldPaddingBottom = canvas.style.paddingBottom;
      var oldMarginLeft = canvas.style.marginLeft; // IE11
      var oldMarginRight = canvas.style.marginRight;
      var oldMarginTop = canvas.style.marginTop;
      var oldMarginBottom = canvas.style.marginBottom;
      var oldDocumentBodyMargin = document.body.style.margin;
      var oldDocumentOverflow = document.documentElement.style.overflow; // Chrome, Firefox
      var oldDocumentScroll = document.body.scroll; // IE
      var oldImageRendering = canvas.style.imageRendering;
  
      function restoreOldStyle() {
        var fullscreenElement = document.fullscreenElement
          || document.webkitFullscreenElement
          ;
        if (!fullscreenElement) {
          document.removeEventListener('fullscreenchange', restoreOldStyle);
  
          // Unprefixed Fullscreen API shipped in Chromium 71 (https://bugs.chromium.org/p/chromium/issues/detail?id=383813)
          // As of Safari 13.0.3 on macOS Catalina 10.15.1 still ships with prefixed webkitfullscreenchange. TODO: revisit this check once Safari ships unprefixed version.
          document.removeEventListener('webkitfullscreenchange', restoreOldStyle);
  
          setCanvasElementSize(canvas, oldWidth, oldHeight);
  
          canvas.style.width = oldCssWidth;
          canvas.style.height = oldCssHeight;
          canvas.style.backgroundColor = oldBackgroundColor; // Chrome
          // IE11 hack: assigning 'undefined' or an empty string to document.body.style.backgroundColor has no effect, so first assign back the default color
          // before setting the undefined value. Setting undefined value is also important, or otherwise we would later treat that as something that the user
          // had explicitly set so subsequent fullscreen transitions would not set background color properly.
          if (!oldDocumentBackgroundColor) document.body.style.backgroundColor = 'white';
          document.body.style.backgroundColor = oldDocumentBackgroundColor; // IE11
          canvas.style.paddingLeft = oldPaddingLeft; // Chrome, FF, Safari
          canvas.style.paddingRight = oldPaddingRight;
          canvas.style.paddingTop = oldPaddingTop;
          canvas.style.paddingBottom = oldPaddingBottom;
          canvas.style.marginLeft = oldMarginLeft; // IE11
          canvas.style.marginRight = oldMarginRight;
          canvas.style.marginTop = oldMarginTop;
          canvas.style.marginBottom = oldMarginBottom;
          document.body.style.margin = oldDocumentBodyMargin;
          document.documentElement.style.overflow = oldDocumentOverflow; // Chrome, Firefox
          document.body.scroll = oldDocumentScroll; // IE
          canvas.style.imageRendering = oldImageRendering;
          if (canvas.GLctxObject) canvas.GLctxObject.GLctx.viewport(0, 0, oldWidth, oldHeight);
  
          if (currentFullscreenStrategy.canvasResizedCallback) {
            getWasmTableEntry(currentFullscreenStrategy.canvasResizedCallback)(37, 0, currentFullscreenStrategy.canvasResizedCallbackUserData);
          }
        }
      }
      document.addEventListener('fullscreenchange', restoreOldStyle);
      // Unprefixed Fullscreen API shipped in Chromium 71 (https://bugs.chromium.org/p/chromium/issues/detail?id=383813)
      // As of Safari 13.0.3 on macOS Catalina 10.15.1 still ships with prefixed webkitfullscreenchange. TODO: revisit this check once Safari ships unprefixed version.
      document.addEventListener('webkitfullscreenchange', restoreOldStyle);
      return restoreOldStyle;
    };
  
  
  
  
  var getBoundingClientRect = (e) => specialHTMLTargets.indexOf(e) < 0 ? e.getBoundingClientRect() : {'left':0,'top':0};
  var JSEvents_resizeCanvasForFullscreen = (target, strategy) => {
      var restoreOldStyle = registerRestoreOldStyle(target);
      var cssWidth = strategy.softFullscreen ? innerWidth : screen.width;
      var cssHeight = strategy.softFullscreen ? innerHeight : screen.height;
      var rect = getBoundingClientRect(target);
      var windowedCssWidth = rect.width;
      var windowedCssHeight = rect.height;
      var canvasSize = getCanvasElementSize(target);
      var windowedRttWidth = canvasSize[0];
      var windowedRttHeight = canvasSize[1];
  
      if (strategy.scaleMode == 3) {
        setLetterbox(target, (cssHeight - windowedCssHeight) / 2, (cssWidth - windowedCssWidth) / 2);
        cssWidth = windowedCssWidth;
        cssHeight = windowedCssHeight;
      } else if (strategy.scaleMode == 2) {
        if (cssWidth*windowedRttHeight < windowedRttWidth*cssHeight) {
          var desiredCssHeight = windowedRttHeight * cssWidth / windowedRttWidth;
          setLetterbox(target, (cssHeight - desiredCssHeight) / 2, 0);
          cssHeight = desiredCssHeight;
        } else {
          var desiredCssWidth = windowedRttWidth * cssHeight / windowedRttHeight;
          setLetterbox(target, 0, (cssWidth - desiredCssWidth) / 2);
          cssWidth = desiredCssWidth;
        }
      }
  
      // If we are adding padding, must choose a background color or otherwise Chrome will give the
      // padding a default white color. Do it only if user has not customized their own background color.
      target.style.backgroundColor ||= 'black';
      // IE11 does the same, but requires the color to be set in the document body.
      document.body.style.backgroundColor ||= 'black'; // IE11
      // Firefox always shows black letterboxes independent of style color.
  
      target.style.width = cssWidth + 'px';
      target.style.height = cssHeight + 'px';
  
      if (strategy.filteringMode == 1) {
        target.style.imageRendering = 'optimizeSpeed';
        target.style.imageRendering = '-moz-crisp-edges';
        target.style.imageRendering = '-o-crisp-edges';
        target.style.imageRendering = '-webkit-optimize-contrast';
        target.style.imageRendering = 'optimize-contrast';
        target.style.imageRendering = 'crisp-edges';
        target.style.imageRendering = 'pixelated';
      }
  
      var dpiScale = (strategy.canvasResolutionScaleMode == 2) ? devicePixelRatio : 1;
      if (strategy.canvasResolutionScaleMode != 0) {
        var newWidth = (cssWidth * dpiScale)|0;
        var newHeight = (cssHeight * dpiScale)|0;
        setCanvasElementSize(target, newWidth, newHeight);
        if (target.GLctxObject) target.GLctxObject.GLctx.viewport(0, 0, newWidth, newHeight);
      }
      return restoreOldStyle;
    };
  
  
  var _emscripten_enter_soft_fullscreen = (target, fullscreenStrategy) => {
      target = findEventTarget(target);
      if (!target) return -4;
  
      var strategy = {
          scaleMode: HEAP32[((fullscreenStrategy)>>2)],
          canvasResolutionScaleMode: HEAP32[(((fullscreenStrategy)+(4))>>2)],
          filteringMode: HEAP32[(((fullscreenStrategy)+(8))>>2)],
          canvasResizedCallback: HEAP32[(((fullscreenStrategy)+(12))>>2)],
          canvasResizedCallbackUserData: HEAP32[(((fullscreenStrategy)+(16))>>2)],
          target,
          softFullscreen: true
      };
  
      var restoreOldStyle = JSEvents_resizeCanvasForFullscreen(target, strategy);
  
      document.documentElement.style.overflow = 'hidden';  // Firefox, Chrome
      document.body.scroll = "no"; // IE11
      document.body.style.margin = '0px'; // Override default document margin area on all browsers.
  
      var hiddenElements = hideEverythingExceptGivenElement(target);
  
      function restoreWindowedState() {
        restoreOldStyle();
        restoreHiddenElements(hiddenElements);
        removeEventListener('resize', softFullscreenResizeWebGLRenderTarget);
        if (strategy.canvasResizedCallback) {
          getWasmTableEntry(strategy.canvasResizedCallback)(37, 0, strategy.canvasResizedCallbackUserData);
        }
        currentFullscreenStrategy = 0;
      }
      restoreOldWindowedStyle = restoreWindowedState;
      currentFullscreenStrategy = strategy;
      addEventListener('resize', softFullscreenResizeWebGLRenderTarget);
  
      // Inform the caller that the canvas size has changed.
      if (strategy.canvasResizedCallback) {
        getWasmTableEntry(strategy.canvasResizedCallback)(37, 0, strategy.canvasResizedCallbackUserData);
      }
  
      return 0;
    };

  var _emscripten_err = (str) => err(UTF8ToString(str));

  
  
  
  
  var JSEvents_requestFullscreen = (target, strategy) => {
      // EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT + EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE is a mode where no extra logic is performed to the DOM elements.
      if (strategy.scaleMode != 0 || strategy.canvasResolutionScaleMode != 0) {
        JSEvents_resizeCanvasForFullscreen(target, strategy);
      }
  
      if (target.requestFullscreen) {
        target.requestFullscreen();
      } else if (target.webkitRequestFullscreen) {
        target.webkitRequestFullscreen(Element.ALLOW_KEYBOARD_INPUT);
      } else {
        return JSEvents.fullscreenEnabled() ? -3 : -1;
      }
  
      currentFullscreenStrategy = strategy;
  
      if (strategy.canvasResizedCallback) {
        getWasmTableEntry(strategy.canvasResizedCallback)(37, 0, strategy.canvasResizedCallbackUserData);
      }
  
      return 0;
    };
  var _emscripten_exit_fullscreen = () => {
      if (!JSEvents.fullscreenEnabled()) return -1;
      // Make sure no queued up calls will fire after this.
      JSEvents.removeDeferredCalls(JSEvents_requestFullscreen);
  
      var d = specialHTMLTargets[1];
      if (d.exitFullscreen) {
        d.fullscreenElement && d.exitFullscreen();
      } else if (d.webkitExitFullscreen) {
        d.webkitFullscreenElement && d.webkitExitFullscreen();
      } else {
        return -1;
      }
  
      return 0;
    };

  
  var requestPointerLock = (target) => {
      if (target.requestPointerLock) {
        target.requestPointerLock();
      } else {
        // document.body is known to accept pointer lock, so use that to differentiate if the user passed a bad element,
        // or if the whole browser just doesn't support the feature.
        if (document.body.requestPointerLock
          ) {
          return -3;
        }
        return -1;
      }
      return 0;
    };
  var _emscripten_exit_pointerlock = () => {
      // Make sure no queued up calls will fire after this.
      JSEvents.removeDeferredCalls(requestPointerLock);
  
      if (document.exitPointerLock) {
        document.exitPointerLock();
      } else {
        return -1;
      }
      return 0;
    };

  var _emscripten_get_device_pixel_ratio = () => {
      return (typeof devicePixelRatio == 'number' && devicePixelRatio) || 1.0;
    };

  
  var _emscripten_get_element_css_size = (target, width, height) => {
      target = findEventTarget(target);
      if (!target) return -4;
  
      var rect = getBoundingClientRect(target);
      HEAPF64[((width)>>3)] = rect.width;
      HEAPF64[((height)>>3)] = rect.height;
  
      return 0;
    };

  
  var fillGamepadEventData = (eventStruct, e) => {
      HEAPF64[((eventStruct)>>3)] = e.timestamp;
      for (var i = 0; i < e.axes.length; ++i) {
        HEAPF64[(((eventStruct+i*8)+(16))>>3)] = e.axes[i];
      }
      for (var i = 0; i < e.buttons.length; ++i) {
        if (typeof e.buttons[i] == 'object') {
          HEAPF64[(((eventStruct+i*8)+(528))>>3)] = e.buttons[i].value;
        } else {
          HEAPF64[(((eventStruct+i*8)+(528))>>3)] = e.buttons[i];
        }
      }
      for (var i = 0; i < e.buttons.length; ++i) {
        if (typeof e.buttons[i] == 'object') {
          HEAP8[(eventStruct+i)+(1040)] = e.buttons[i].pressed;
        } else {
          // Assigning a boolean to HEAP32, that's ok, but Closure would like to warn about it:
          /** @suppress {checkTypes} */
          HEAP8[(eventStruct+i)+(1040)] = e.buttons[i] == 1;
        }
      }
      HEAP8[(eventStruct)+(1104)] = e.connected;
      HEAP32[(((eventStruct)+(1108))>>2)] = e.index;
      HEAP32[(((eventStruct)+(8))>>2)] = e.axes.length;
      HEAP32[(((eventStruct)+(12))>>2)] = e.buttons.length;
      stringToUTF8(e.id, eventStruct + 1112, 64);
      stringToUTF8(e.mapping, eventStruct + 1176, 64);
    };
  var _emscripten_get_gamepad_status = (index, gamepadState) => {
      if (!JSEvents.lastGamepadState) throw 'emscripten_get_gamepad_status() can only be called after having first called emscripten_sample_gamepad_data() and that function has returned EMSCRIPTEN_RESULT_SUCCESS!';
      // INVALID_PARAM is returned on a Gamepad index that never was there.
      if (index < 0 || index >= JSEvents.lastGamepadState.length) return -5;
  
      // NO_DATA is returned on a Gamepad index that was removed.
      // For previously disconnected gamepads there should be an empty slot (null/undefined/false) at the index.
      // This is because gamepads must keep their original position in the array.
      // For example, removing the first of two gamepads produces [null/undefined/false, gamepad].
      if (!JSEvents.lastGamepadState[index]) return -7;
  
      fillGamepadEventData(gamepadState, JSEvents.lastGamepadState[index]);
      return 0;
    };

  var getHeapMax = () =>
      // Stay one Wasm page short of 4GB: while e.g. Chrome is able to allocate
      // full 4GB Wasm memories, the size will wrap back to 0 bytes in Wasm side
      // for any code that deals with heap sizes, which would require special
      // casing all heap size related code to treat 0 specially.
      2147483648;
  var _emscripten_get_heap_max = () => getHeapMax();


  var _emscripten_get_num_gamepads = () => {
      if (!JSEvents.lastGamepadState) throw 'emscripten_get_num_gamepads() can only be called after having first called emscripten_sample_gamepad_data() and that function has returned EMSCRIPTEN_RESULT_SUCCESS!';
      // N.B. Do not call emscripten_get_num_gamepads() unless having first called emscripten_sample_gamepad_data(), and that has returned EMSCRIPTEN_RESULT_SUCCESS.
      // Otherwise the following line will throw an exception.
      return JSEvents.lastGamepadState.length;
    };

  var getPreloadedImageData = (path, w, h) => {
      path = PATH_FS.resolve(path);
  
      var canvas = /** @type {HTMLCanvasElement} */(Browser.preloadedImages[path]);
      if (!canvas) return 0;
  
      var ctx = canvas.getContext("2d");
      var image = ctx.getImageData(0, 0, canvas.width, canvas.height);
      var buf = _malloc(canvas.width * canvas.height * 4);
  
      HEAPU8.set(image.data, buf);
  
      HEAP32[((w)>>2)] = canvas.width;
      HEAP32[((h)>>2)] = canvas.height;
      return buf;
    };
  
  
  
  var _emscripten_get_preloaded_image_data = (path, w, h) => getPreloadedImageData(UTF8ToString(path), w, h);

  
  
  var _emscripten_get_preloaded_image_data_from_FILE = (file, w, h) => {
      var fd = _fileno(file);
      var stream = FS.getStream(fd);
      if (stream) {
        return getPreloadedImageData(stream.path, w, h);
      }
  
      return 0;
    };

  var _emscripten_get_screen_size = (width, height) => {
      HEAP32[((width)>>2)] = screen.width;
      HEAP32[((height)>>2)] = screen.height;
    };

  /** @suppress {duplicate } */
  var _glActiveTexture = (x0) => GLctx.activeTexture(x0);
  var _emscripten_glActiveTexture = _glActiveTexture;

  /** @suppress {duplicate } */
  var _glAttachShader = (program, shader) => {
      GLctx.attachShader(GL.programs[program], GL.shaders[shader]);
    };
  var _emscripten_glAttachShader = _glAttachShader;

  /** @suppress {duplicate } */
  var _glBeginQuery = (target, id) => {
      GLctx.beginQuery(target, GL.queries[id]);
    };
  var _emscripten_glBeginQuery = _glBeginQuery;

  /** @suppress {duplicate } */
  var _glBeginQueryEXT = (target, id) => {
      GLctx.disjointTimerQueryExt['beginQueryEXT'](target, GL.queries[id]);
    };
  var _emscripten_glBeginQueryEXT = _glBeginQueryEXT;

  /** @suppress {duplicate } */
  var _glBeginTransformFeedback = (x0) => GLctx.beginTransformFeedback(x0);
  var _emscripten_glBeginTransformFeedback = _glBeginTransformFeedback;

  
  /** @suppress {duplicate } */
  var _glBindAttribLocation = (program, index, name) => {
      GLctx.bindAttribLocation(GL.programs[program], index, UTF8ToString(name));
    };
  var _emscripten_glBindAttribLocation = _glBindAttribLocation;

  /** @suppress {duplicate } */
  var _glBindBuffer = (target, buffer) => {
  
      if (target == 0x88EB /*GL_PIXEL_PACK_BUFFER*/) {
        // In WebGL 2 glReadPixels entry point, we need to use a different WebGL 2
        // API function call when a buffer is bound to
        // GL_PIXEL_PACK_BUFFER_BINDING point, so must keep track whether that
        // binding point is non-null to know what is the proper API function to
        // call.
        GLctx.currentPixelPackBufferBinding = buffer;
      } else if (target == 0x88EC /*GL_PIXEL_UNPACK_BUFFER*/) {
        // In WebGL 2 gl(Compressed)Tex(Sub)Image[23]D entry points, we need to
        // use a different WebGL 2 API function call when a buffer is bound to
        // GL_PIXEL_UNPACK_BUFFER_BINDING point, so must keep track whether that
        // binding point is non-null to know what is the proper API function to
        // call.
        GLctx.currentPixelUnpackBufferBinding = buffer;
      }
      GLctx.bindBuffer(target, GL.buffers[buffer]);
    };
  var _emscripten_glBindBuffer = _glBindBuffer;

  /** @suppress {duplicate } */
  var _glBindBufferBase = (target, index, buffer) => {
      GLctx.bindBufferBase(target, index, GL.buffers[buffer]);
    };
  var _emscripten_glBindBufferBase = _glBindBufferBase;

  /** @suppress {duplicate } */
  var _glBindBufferRange = (target, index, buffer, offset, ptrsize) => {
      GLctx.bindBufferRange(target, index, GL.buffers[buffer], offset, ptrsize);
    };
  var _emscripten_glBindBufferRange = _glBindBufferRange;

  /** @suppress {duplicate } */
  var _glBindFramebuffer = (target, framebuffer) => {
  
      GLctx.bindFramebuffer(target, GL.framebuffers[framebuffer]);
  
    };
  var _emscripten_glBindFramebuffer = _glBindFramebuffer;

  /** @suppress {duplicate } */
  var _glBindRenderbuffer = (target, renderbuffer) => {
      GLctx.bindRenderbuffer(target, GL.renderbuffers[renderbuffer]);
    };
  var _emscripten_glBindRenderbuffer = _glBindRenderbuffer;

  /** @suppress {duplicate } */
  var _glBindSampler = (unit, sampler) => {
      GLctx.bindSampler(unit, GL.samplers[sampler]);
    };
  var _emscripten_glBindSampler = _glBindSampler;

  /** @suppress {duplicate } */
  var _glBindTexture = (target, texture) => {
      GLctx.bindTexture(target, GL.textures[texture]);
    };
  var _emscripten_glBindTexture = _glBindTexture;

  /** @suppress {duplicate } */
  var _glBindTransformFeedback = (target, id) => {
      GLctx.bindTransformFeedback(target, GL.transformFeedbacks[id]);
    };
  var _emscripten_glBindTransformFeedback = _glBindTransformFeedback;

  /** @suppress {duplicate } */
  var _glBindVertexArray = (vao) => {
      GLctx.bindVertexArray(GL.vaos[vao]);
    };
  var _emscripten_glBindVertexArray = _glBindVertexArray;

  
  /** @suppress {duplicate } */
  var _glBindVertexArrayOES = _glBindVertexArray;
  var _emscripten_glBindVertexArrayOES = _glBindVertexArrayOES;

  /** @suppress {duplicate } */
  var _glBlendColor = (x0, x1, x2, x3) => GLctx.blendColor(x0, x1, x2, x3);
  var _emscripten_glBlendColor = _glBlendColor;

  /** @suppress {duplicate } */
  var _glBlendEquation = (x0) => GLctx.blendEquation(x0);
  var _emscripten_glBlendEquation = _glBlendEquation;

  /** @suppress {duplicate } */
  var _glBlendEquationSeparate = (x0, x1) => GLctx.blendEquationSeparate(x0, x1);
  var _emscripten_glBlendEquationSeparate = _glBlendEquationSeparate;

  /** @suppress {duplicate } */
  var _glBlendFunc = (x0, x1) => GLctx.blendFunc(x0, x1);
  var _emscripten_glBlendFunc = _glBlendFunc;

  /** @suppress {duplicate } */
  var _glBlendFuncSeparate = (x0, x1, x2, x3) => GLctx.blendFuncSeparate(x0, x1, x2, x3);
  var _emscripten_glBlendFuncSeparate = _glBlendFuncSeparate;

  /** @suppress {duplicate } */
  var _glBlitFramebuffer = (x0, x1, x2, x3, x4, x5, x6, x7, x8, x9) => GLctx.blitFramebuffer(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9);
  var _emscripten_glBlitFramebuffer = _glBlitFramebuffer;

  /** @suppress {duplicate } */
  var _glBufferData = (target, size, data, usage) => {
  
      if (true) {
        // If size is zero, WebGL would interpret uploading the whole input
        // arraybuffer (starting from given offset), which would not make sense in
        // WebAssembly, so avoid uploading if size is zero. However we must still
        // call bufferData to establish a backing storage of zero bytes.
        if (data && size) {
          GLctx.bufferData(target, HEAPU8, usage, data, size);
        } else {
          GLctx.bufferData(target, size, usage);
        }
        return;
      }
    };
  var _emscripten_glBufferData = _glBufferData;

  /** @suppress {duplicate } */
  var _glBufferSubData = (target, offset, size, data) => {
      if (true) {
        size && GLctx.bufferSubData(target, offset, HEAPU8, data, size);
        return;
      }
    };
  var _emscripten_glBufferSubData = _glBufferSubData;

  /** @suppress {duplicate } */
  var _glCheckFramebufferStatus = (x0) => GLctx.checkFramebufferStatus(x0);
  var _emscripten_glCheckFramebufferStatus = _glCheckFramebufferStatus;

  /** @suppress {duplicate } */
  var _glClear = (x0) => GLctx.clear(x0);
  var _emscripten_glClear = _glClear;

  /** @suppress {duplicate } */
  var _glClearBufferfi = (x0, x1, x2, x3) => GLctx.clearBufferfi(x0, x1, x2, x3);
  var _emscripten_glClearBufferfi = _glClearBufferfi;

  /** @suppress {duplicate } */
  var _glClearBufferfv = (buffer, drawbuffer, value) => {
  
      GLctx.clearBufferfv(buffer, drawbuffer, HEAPF32, ((value)>>2));
    };
  var _emscripten_glClearBufferfv = _glClearBufferfv;

  /** @suppress {duplicate } */
  var _glClearBufferiv = (buffer, drawbuffer, value) => {
  
      GLctx.clearBufferiv(buffer, drawbuffer, HEAP32, ((value)>>2));
    };
  var _emscripten_glClearBufferiv = _glClearBufferiv;

  /** @suppress {duplicate } */
  var _glClearBufferuiv = (buffer, drawbuffer, value) => {
  
      GLctx.clearBufferuiv(buffer, drawbuffer, HEAPU32, ((value)>>2));
    };
  var _emscripten_glClearBufferuiv = _glClearBufferuiv;

  /** @suppress {duplicate } */
  var _glClearColor = (x0, x1, x2, x3) => GLctx.clearColor(x0, x1, x2, x3);
  var _emscripten_glClearColor = _glClearColor;

  /** @suppress {duplicate } */
  var _glClearDepthf = (x0) => GLctx.clearDepth(x0);
  var _emscripten_glClearDepthf = _glClearDepthf;

  /** @suppress {duplicate } */
  var _glClearStencil = (x0) => GLctx.clearStencil(x0);
  var _emscripten_glClearStencil = _glClearStencil;

  /** @suppress {duplicate } */
  var _glClientWaitSync = (sync, flags, timeout) => {
      // WebGL2 vs GLES3 differences: in GLES3, the timeout parameter is a uint64, where 0xFFFFFFFFFFFFFFFFULL means GL_TIMEOUT_IGNORED.
      // In JS, there's no 64-bit value types, so instead timeout is taken to be signed, and GL_TIMEOUT_IGNORED is given value -1.
      // Inherently the value accepted in the timeout is lossy, and can't take in arbitrary u64 bit pattern (but most likely doesn't matter)
      // See https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.15
      timeout = Number(timeout);
      return GLctx.clientWaitSync(GL.syncs[sync], flags, timeout);
    };
  var _emscripten_glClientWaitSync = _glClientWaitSync;

  /** @suppress {duplicate } */
  var _glClipControlEXT = (origin, depth) => {
      GLctx.extClipControl['clipControlEXT'](origin, depth);
    };
  var _emscripten_glClipControlEXT = _glClipControlEXT;

  /** @suppress {duplicate } */
  var _glColorMask = (red, green, blue, alpha) => {
      GLctx.colorMask(!!red, !!green, !!blue, !!alpha);
    };
  var _emscripten_glColorMask = _glColorMask;

  /** @suppress {duplicate } */
  var _glCompileShader = (shader) => {
      GLctx.compileShader(GL.shaders[shader]);
    };
  var _emscripten_glCompileShader = _glCompileShader;

  /** @suppress {duplicate } */
  var _glCompressedTexImage2D = (target, level, internalFormat, width, height, border, imageSize, data) => {
      // `data` may be null here, which means "allocate uniniitalized space but
      // don't upload" in GLES parlance, but `compressedTexImage2D` requires the
      // final data parameter, so we simply pass a heap view starting at zero
      // effectively uploading whatever happens to be near address zero.  See
      // https://github.com/emscripten-core/emscripten/issues/19300.
      if (true) {
        if (GLctx.currentPixelUnpackBufferBinding || !imageSize) {
          GLctx.compressedTexImage2D(target, level, internalFormat, width, height, border, imageSize, data);
          return;
        }
        GLctx.compressedTexImage2D(target, level, internalFormat, width, height, border, HEAPU8, data, imageSize);
        return;
      }
    };
  var _emscripten_glCompressedTexImage2D = _glCompressedTexImage2D;

  /** @suppress {duplicate } */
  var _glCompressedTexImage3D = (target, level, internalFormat, width, height, depth, border, imageSize, data) => {
      if (GLctx.currentPixelUnpackBufferBinding) {
        GLctx.compressedTexImage3D(target, level, internalFormat, width, height, depth, border, imageSize, data);
      } else {
        GLctx.compressedTexImage3D(target, level, internalFormat, width, height, depth, border, HEAPU8, data, imageSize);
      }
    };
  var _emscripten_glCompressedTexImage3D = _glCompressedTexImage3D;

  /** @suppress {duplicate } */
  var _glCompressedTexSubImage2D = (target, level, xoffset, yoffset, width, height, format, imageSize, data) => {
      if (true) {
        if (GLctx.currentPixelUnpackBufferBinding || !imageSize) {
          GLctx.compressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
          return;
        }
        GLctx.compressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, HEAPU8, data, imageSize);
        return;
      }
    };
  var _emscripten_glCompressedTexSubImage2D = _glCompressedTexSubImage2D;

  /** @suppress {duplicate } */
  var _glCompressedTexSubImage3D = (target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data) => {
      if (GLctx.currentPixelUnpackBufferBinding) {
        GLctx.compressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
      } else {
        GLctx.compressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, HEAPU8, data, imageSize);
      }
    };
  var _emscripten_glCompressedTexSubImage3D = _glCompressedTexSubImage3D;

  /** @suppress {duplicate } */
  var _glCopyBufferSubData = (x0, x1, x2, x3, x4) => GLctx.copyBufferSubData(x0, x1, x2, x3, x4);
  var _emscripten_glCopyBufferSubData = _glCopyBufferSubData;

  /** @suppress {duplicate } */
  var _glCopyTexImage2D = (x0, x1, x2, x3, x4, x5, x6, x7) => GLctx.copyTexImage2D(x0, x1, x2, x3, x4, x5, x6, x7);
  var _emscripten_glCopyTexImage2D = _glCopyTexImage2D;

  /** @suppress {duplicate } */
  var _glCopyTexSubImage2D = (x0, x1, x2, x3, x4, x5, x6, x7) => GLctx.copyTexSubImage2D(x0, x1, x2, x3, x4, x5, x6, x7);
  var _emscripten_glCopyTexSubImage2D = _glCopyTexSubImage2D;

  /** @suppress {duplicate } */
  var _glCopyTexSubImage3D = (x0, x1, x2, x3, x4, x5, x6, x7, x8) => GLctx.copyTexSubImage3D(x0, x1, x2, x3, x4, x5, x6, x7, x8);
  var _emscripten_glCopyTexSubImage3D = _glCopyTexSubImage3D;

  /** @suppress {duplicate } */
  var _glCreateProgram = () => {
      var id = GL.getNewId(GL.programs);
      var program = GLctx.createProgram();
      // Store additional information needed for each shader program:
      program.name = id;
      // Lazy cache results of
      // glGetProgramiv(GL_ACTIVE_UNIFORM_MAX_LENGTH/GL_ACTIVE_ATTRIBUTE_MAX_LENGTH/GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH)
      program.maxUniformLength = program.maxAttributeLength = program.maxUniformBlockNameLength = 0;
      program.uniformIdCounter = 1;
      GL.programs[id] = program;
      return id;
    };
  var _emscripten_glCreateProgram = _glCreateProgram;

  /** @suppress {duplicate } */
  var _glCreateShader = (shaderType) => {
      var id = GL.getNewId(GL.shaders);
      GL.shaders[id] = GLctx.createShader(shaderType);
  
      return id;
    };
  var _emscripten_glCreateShader = _glCreateShader;

  /** @suppress {duplicate } */
  var _glCullFace = (x0) => GLctx.cullFace(x0);
  var _emscripten_glCullFace = _glCullFace;

  /** @suppress {duplicate } */
  var _glDeleteBuffers = (n, buffers) => {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((buffers)+(i*4))>>2)];
        var buffer = GL.buffers[id];
  
        // From spec: "glDeleteBuffers silently ignores 0's and names that do not
        // correspond to existing buffer objects."
        if (!buffer) continue;
  
        GLctx.deleteBuffer(buffer);
        buffer.name = 0;
        GL.buffers[id] = null;
  
        if (id == GLctx.currentPixelPackBufferBinding) GLctx.currentPixelPackBufferBinding = 0;
        if (id == GLctx.currentPixelUnpackBufferBinding) GLctx.currentPixelUnpackBufferBinding = 0;
      }
    };
  var _emscripten_glDeleteBuffers = _glDeleteBuffers;

  /** @suppress {duplicate } */
  var _glDeleteFramebuffers = (n, framebuffers) => {
      for (var i = 0; i < n; ++i) {
        var id = HEAP32[(((framebuffers)+(i*4))>>2)];
        var framebuffer = GL.framebuffers[id];
        if (!framebuffer) continue; // GL spec: "glDeleteFramebuffers silently ignores 0s and names that do not correspond to existing framebuffer objects".
        GLctx.deleteFramebuffer(framebuffer);
        framebuffer.name = 0;
        GL.framebuffers[id] = null;
      }
    };
  var _emscripten_glDeleteFramebuffers = _glDeleteFramebuffers;

  /** @suppress {duplicate } */
  var _glDeleteProgram = (id) => {
      if (!id) return;
      var program = GL.programs[id];
      if (!program) {
        // glDeleteProgram actually signals an error when deleting a nonexisting
        // object, unlike some other GL delete functions.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      GLctx.deleteProgram(program);
      program.name = 0;
      GL.programs[id] = null;
    };
  var _emscripten_glDeleteProgram = _glDeleteProgram;

  /** @suppress {duplicate } */
  var _glDeleteQueries = (n, ids) => {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((ids)+(i*4))>>2)];
        var query = GL.queries[id];
        if (!query) continue; // GL spec: "unused names in ids are ignored, as is the name zero."
        GLctx.deleteQuery(query);
        GL.queries[id] = null;
      }
    };
  var _emscripten_glDeleteQueries = _glDeleteQueries;

  /** @suppress {duplicate } */
  var _glDeleteQueriesEXT = (n, ids) => {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((ids)+(i*4))>>2)];
        var query = GL.queries[id];
        if (!query) continue; // GL spec: "unused names in ids are ignored, as is the name zero."
        GLctx.disjointTimerQueryExt['deleteQueryEXT'](query);
        GL.queries[id] = null;
      }
    };
  var _emscripten_glDeleteQueriesEXT = _glDeleteQueriesEXT;

  /** @suppress {duplicate } */
  var _glDeleteRenderbuffers = (n, renderbuffers) => {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((renderbuffers)+(i*4))>>2)];
        var renderbuffer = GL.renderbuffers[id];
        if (!renderbuffer) continue; // GL spec: "glDeleteRenderbuffers silently ignores 0s and names that do not correspond to existing renderbuffer objects".
        GLctx.deleteRenderbuffer(renderbuffer);
        renderbuffer.name = 0;
        GL.renderbuffers[id] = null;
      }
    };
  var _emscripten_glDeleteRenderbuffers = _glDeleteRenderbuffers;

  /** @suppress {duplicate } */
  var _glDeleteSamplers = (n, samplers) => {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((samplers)+(i*4))>>2)];
        var sampler = GL.samplers[id];
        if (!sampler) continue;
        GLctx.deleteSampler(sampler);
        sampler.name = 0;
        GL.samplers[id] = null;
      }
    };
  var _emscripten_glDeleteSamplers = _glDeleteSamplers;

  /** @suppress {duplicate } */
  var _glDeleteShader = (id) => {
      if (!id) return;
      var shader = GL.shaders[id];
      if (!shader) {
        // glDeleteShader actually signals an error when deleting a nonexisting
        // object, unlike some other GL delete functions.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      GLctx.deleteShader(shader);
      GL.shaders[id] = null;
    };
  var _emscripten_glDeleteShader = _glDeleteShader;

  /** @suppress {duplicate } */
  var _glDeleteSync = (id) => {
      if (!id) return;
      var sync = GL.syncs[id];
      if (!sync) { // glDeleteSync signals an error when deleting a nonexisting object, unlike some other GL delete functions.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      GLctx.deleteSync(sync);
      sync.name = 0;
      GL.syncs[id] = null;
    };
  var _emscripten_glDeleteSync = _glDeleteSync;

  /** @suppress {duplicate } */
  var _glDeleteTextures = (n, textures) => {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((textures)+(i*4))>>2)];
        var texture = GL.textures[id];
        // GL spec: "glDeleteTextures silently ignores 0s and names that do not
        // correspond to existing textures".
        if (!texture) continue;
        GLctx.deleteTexture(texture);
        texture.name = 0;
        GL.textures[id] = null;
      }
    };
  var _emscripten_glDeleteTextures = _glDeleteTextures;

  /** @suppress {duplicate } */
  var _glDeleteTransformFeedbacks = (n, ids) => {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((ids)+(i*4))>>2)];
        var transformFeedback = GL.transformFeedbacks[id];
        if (!transformFeedback) continue; // GL spec: "unused names in ids are ignored, as is the name zero."
        GLctx.deleteTransformFeedback(transformFeedback);
        transformFeedback.name = 0;
        GL.transformFeedbacks[id] = null;
      }
    };
  var _emscripten_glDeleteTransformFeedbacks = _glDeleteTransformFeedbacks;

  /** @suppress {duplicate } */
  var _glDeleteVertexArrays = (n, vaos) => {
      for (var i = 0; i < n; i++) {
        var id = HEAP32[(((vaos)+(i*4))>>2)];
        GLctx.deleteVertexArray(GL.vaos[id]);
        GL.vaos[id] = null;
      }
    };
  var _emscripten_glDeleteVertexArrays = _glDeleteVertexArrays;

  
  /** @suppress {duplicate } */
  var _glDeleteVertexArraysOES = _glDeleteVertexArrays;
  var _emscripten_glDeleteVertexArraysOES = _glDeleteVertexArraysOES;

  /** @suppress {duplicate } */
  var _glDepthFunc = (x0) => GLctx.depthFunc(x0);
  var _emscripten_glDepthFunc = _glDepthFunc;

  /** @suppress {duplicate } */
  var _glDepthMask = (flag) => {
      GLctx.depthMask(!!flag);
    };
  var _emscripten_glDepthMask = _glDepthMask;

  /** @suppress {duplicate } */
  var _glDepthRangef = (x0, x1) => GLctx.depthRange(x0, x1);
  var _emscripten_glDepthRangef = _glDepthRangef;

  /** @suppress {duplicate } */
  var _glDetachShader = (program, shader) => {
      GLctx.detachShader(GL.programs[program], GL.shaders[shader]);
    };
  var _emscripten_glDetachShader = _glDetachShader;

  /** @suppress {duplicate } */
  var _glDisable = (x0) => GLctx.disable(x0);
  var _emscripten_glDisable = _glDisable;

  /** @suppress {duplicate } */
  var _glDisableVertexAttribArray = (index) => {
      GLctx.disableVertexAttribArray(index);
    };
  var _emscripten_glDisableVertexAttribArray = _glDisableVertexAttribArray;

  /** @suppress {duplicate } */
  var _glDrawArrays = (mode, first, count) => {
  
      GLctx.drawArrays(mode, first, count);
  
    };
  var _emscripten_glDrawArrays = _glDrawArrays;

  /** @suppress {duplicate } */
  var _glDrawArraysInstanced = (mode, first, count, primcount) => {
      GLctx.drawArraysInstanced(mode, first, count, primcount);
    };
  var _emscripten_glDrawArraysInstanced = _glDrawArraysInstanced;

  
  /** @suppress {duplicate } */
  var _glDrawArraysInstancedANGLE = _glDrawArraysInstanced;
  var _emscripten_glDrawArraysInstancedANGLE = _glDrawArraysInstancedANGLE;

  
  /** @suppress {duplicate } */
  var _glDrawArraysInstancedARB = _glDrawArraysInstanced;
  var _emscripten_glDrawArraysInstancedARB = _glDrawArraysInstancedARB;

  
  /** @suppress {duplicate } */
  var _glDrawArraysInstancedEXT = _glDrawArraysInstanced;
  var _emscripten_glDrawArraysInstancedEXT = _glDrawArraysInstancedEXT;

  
  /** @suppress {duplicate } */
  var _glDrawArraysInstancedNV = _glDrawArraysInstanced;
  var _emscripten_glDrawArraysInstancedNV = _glDrawArraysInstancedNV;

  var tempFixedLengthArray = [];
  
  /** @suppress {duplicate } */
  var _glDrawBuffers = (n, bufs) => {
  
      var bufArray = tempFixedLengthArray[n];
      for (var i = 0; i < n; i++) {
        bufArray[i] = HEAP32[(((bufs)+(i*4))>>2)];
      }
  
      GLctx.drawBuffers(bufArray);
    };
  var _emscripten_glDrawBuffers = _glDrawBuffers;

  
  /** @suppress {duplicate } */
  var _glDrawBuffersEXT = _glDrawBuffers;
  var _emscripten_glDrawBuffersEXT = _glDrawBuffersEXT;

  
  /** @suppress {duplicate } */
  var _glDrawBuffersWEBGL = _glDrawBuffers;
  var _emscripten_glDrawBuffersWEBGL = _glDrawBuffersWEBGL;

  /** @suppress {duplicate } */
  var _glDrawElements = (mode, count, type, indices) => {
  
      GLctx.drawElements(mode, count, type, indices);
  
    };
  var _emscripten_glDrawElements = _glDrawElements;

  /** @suppress {duplicate } */
  var _glDrawElementsInstanced = (mode, count, type, indices, primcount) => {
      GLctx.drawElementsInstanced(mode, count, type, indices, primcount);
    };
  var _emscripten_glDrawElementsInstanced = _glDrawElementsInstanced;

  
  /** @suppress {duplicate } */
  var _glDrawElementsInstancedANGLE = _glDrawElementsInstanced;
  var _emscripten_glDrawElementsInstancedANGLE = _glDrawElementsInstancedANGLE;

  
  /** @suppress {duplicate } */
  var _glDrawElementsInstancedARB = _glDrawElementsInstanced;
  var _emscripten_glDrawElementsInstancedARB = _glDrawElementsInstancedARB;

  
  /** @suppress {duplicate } */
  var _glDrawElementsInstancedEXT = _glDrawElementsInstanced;
  var _emscripten_glDrawElementsInstancedEXT = _glDrawElementsInstancedEXT;

  
  /** @suppress {duplicate } */
  var _glDrawElementsInstancedNV = _glDrawElementsInstanced;
  var _emscripten_glDrawElementsInstancedNV = _glDrawElementsInstancedNV;

  /** @suppress {duplicate } */
  var _glDrawRangeElements = (mode, start, end, count, type, indices) => {
      // TODO: This should be a trivial pass-though function registered at the bottom of this page as
      // glFuncs[6][1] += ' drawRangeElements';
      // but due to https://bugzilla.mozilla.org/show_bug.cgi?id=1202427,
      // we work around by ignoring the range.
      _glDrawElements(mode, count, type, indices);
    };
  var _emscripten_glDrawRangeElements = _glDrawRangeElements;

  /** @suppress {duplicate } */
  var _glEnable = (x0) => GLctx.enable(x0);
  var _emscripten_glEnable = _glEnable;

  /** @suppress {duplicate } */
  var _glEnableVertexAttribArray = (index) => {
      GLctx.enableVertexAttribArray(index);
    };
  var _emscripten_glEnableVertexAttribArray = _glEnableVertexAttribArray;

  /** @suppress {duplicate } */
  var _glEndQuery = (x0) => GLctx.endQuery(x0);
  var _emscripten_glEndQuery = _glEndQuery;

  /** @suppress {duplicate } */
  var _glEndQueryEXT = (target) => {
      GLctx.disjointTimerQueryExt['endQueryEXT'](target);
    };
  var _emscripten_glEndQueryEXT = _glEndQueryEXT;

  /** @suppress {duplicate } */
  var _glEndTransformFeedback = () => GLctx.endTransformFeedback();
  var _emscripten_glEndTransformFeedback = _glEndTransformFeedback;

  /** @suppress {duplicate } */
  var _glFenceSync = (condition, flags) => {
      var sync = GLctx.fenceSync(condition, flags);
      if (sync) {
        var id = GL.getNewId(GL.syncs);
        sync.name = id;
        GL.syncs[id] = sync;
        return id;
      }
      return 0; // Failed to create a sync object
    };
  var _emscripten_glFenceSync = _glFenceSync;

  /** @suppress {duplicate } */
  var _glFinish = () => GLctx.finish();
  var _emscripten_glFinish = _glFinish;

  /** @suppress {duplicate } */
  var _glFlush = () => GLctx.flush();
  var _emscripten_glFlush = _glFlush;

  /** @suppress {duplicate } */
  var _glFramebufferRenderbuffer = (target, attachment, renderbuffertarget, renderbuffer) => {
      GLctx.framebufferRenderbuffer(target, attachment, renderbuffertarget,
                                         GL.renderbuffers[renderbuffer]);
    };
  var _emscripten_glFramebufferRenderbuffer = _glFramebufferRenderbuffer;

  /** @suppress {duplicate } */
  var _glFramebufferTexture2D = (target, attachment, textarget, texture, level) => {
      GLctx.framebufferTexture2D(target, attachment, textarget,
                                      GL.textures[texture], level);
    };
  var _emscripten_glFramebufferTexture2D = _glFramebufferTexture2D;

  /** @suppress {duplicate } */
  var _glFramebufferTextureLayer = (target, attachment, texture, level, layer) => {
      GLctx.framebufferTextureLayer(target, attachment, GL.textures[texture], level, layer);
    };
  var _emscripten_glFramebufferTextureLayer = _glFramebufferTextureLayer;

  /** @suppress {duplicate } */
  var _glFrontFace = (x0) => GLctx.frontFace(x0);
  var _emscripten_glFrontFace = _glFrontFace;

  /** @suppress {duplicate } */
  var _glGenBuffers = (n, buffers) => {
      GL.genObject(n, buffers, 'createBuffer', GL.buffers
        );
    };
  var _emscripten_glGenBuffers = _glGenBuffers;

  /** @suppress {duplicate } */
  var _glGenFramebuffers = (n, ids) => {
      GL.genObject(n, ids, 'createFramebuffer', GL.framebuffers
        );
    };
  var _emscripten_glGenFramebuffers = _glGenFramebuffers;

  /** @suppress {duplicate } */
  var _glGenQueries = (n, ids) => {
      GL.genObject(n, ids, 'createQuery', GL.queries
        );
    };
  var _emscripten_glGenQueries = _glGenQueries;

  /** @suppress {duplicate } */
  var _glGenQueriesEXT = (n, ids) => {
      for (var i = 0; i < n; i++) {
        var query = GLctx.disjointTimerQueryExt['createQueryEXT']();
        if (!query) {
          GL.recordError(0x502 /* GL_INVALID_OPERATION */);
          while (i < n) HEAP32[(((ids)+(i++*4))>>2)] = 0;
          return;
        }
        var id = GL.getNewId(GL.queries);
        query.name = id;
        GL.queries[id] = query;
        HEAP32[(((ids)+(i*4))>>2)] = id;
      }
    };
  var _emscripten_glGenQueriesEXT = _glGenQueriesEXT;

  /** @suppress {duplicate } */
  var _glGenRenderbuffers = (n, renderbuffers) => {
      GL.genObject(n, renderbuffers, 'createRenderbuffer', GL.renderbuffers
        );
    };
  var _emscripten_glGenRenderbuffers = _glGenRenderbuffers;

  /** @suppress {duplicate } */
  var _glGenSamplers = (n, samplers) => {
      GL.genObject(n, samplers, 'createSampler', GL.samplers
        );
    };
  var _emscripten_glGenSamplers = _glGenSamplers;

  /** @suppress {duplicate } */
  var _glGenTextures = (n, textures) => {
      GL.genObject(n, textures, 'createTexture', GL.textures
        );
    };
  var _emscripten_glGenTextures = _glGenTextures;

  /** @suppress {duplicate } */
  var _glGenTransformFeedbacks = (n, ids) => {
      GL.genObject(n, ids, 'createTransformFeedback', GL.transformFeedbacks
        );
    };
  var _emscripten_glGenTransformFeedbacks = _glGenTransformFeedbacks;

  /** @suppress {duplicate } */
  var _glGenVertexArrays = (n, arrays) => {
      GL.genObject(n, arrays, 'createVertexArray', GL.vaos
        );
    };
  var _emscripten_glGenVertexArrays = _glGenVertexArrays;

  
  /** @suppress {duplicate } */
  var _glGenVertexArraysOES = _glGenVertexArrays;
  var _emscripten_glGenVertexArraysOES = _glGenVertexArraysOES;

  /** @suppress {duplicate } */
  var _glGenerateMipmap = (x0) => GLctx.generateMipmap(x0);
  var _emscripten_glGenerateMipmap = _glGenerateMipmap;

  
  var __glGetActiveAttribOrUniform = (funcName, program, index, bufSize, length, size, type, name) => {
      program = GL.programs[program];
      var info = GLctx[funcName](program, index);
      if (info) {
        // If an error occurs, nothing will be written to length, size and type and name.
        var numBytesWrittenExclNull = name && stringToUTF8(info.name, name, bufSize);
        if (length) HEAP32[((length)>>2)] = numBytesWrittenExclNull;
        if (size) HEAP32[((size)>>2)] = info.size;
        if (type) HEAP32[((type)>>2)] = info.type;
      }
    };
  
  /** @suppress {duplicate } */
  var _glGetActiveAttrib = (program, index, bufSize, length, size, type, name) =>
      __glGetActiveAttribOrUniform('getActiveAttrib', program, index, bufSize, length, size, type, name);
  var _emscripten_glGetActiveAttrib = _glGetActiveAttrib;

  
  /** @suppress {duplicate } */
  var _glGetActiveUniform = (program, index, bufSize, length, size, type, name) =>
      __glGetActiveAttribOrUniform('getActiveUniform', program, index, bufSize, length, size, type, name);
  var _emscripten_glGetActiveUniform = _glGetActiveUniform;

  /** @suppress {duplicate } */
  var _glGetActiveUniformBlockName = (program, uniformBlockIndex, bufSize, length, uniformBlockName) => {
      program = GL.programs[program];
  
      var result = GLctx.getActiveUniformBlockName(program, uniformBlockIndex);
      if (!result) return; // If an error occurs, nothing will be written to uniformBlockName or length.
      if (uniformBlockName && bufSize > 0) {
        var numBytesWrittenExclNull = stringToUTF8(result, uniformBlockName, bufSize);
        if (length) HEAP32[((length)>>2)] = numBytesWrittenExclNull;
      } else {
        if (length) HEAP32[((length)>>2)] = 0;
      }
    };
  var _emscripten_glGetActiveUniformBlockName = _glGetActiveUniformBlockName;

  /** @suppress {duplicate } */
  var _glGetActiveUniformBlockiv = (program, uniformBlockIndex, pname, params) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if params == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      program = GL.programs[program];
  
      if (pname == 0x8A41 /* GL_UNIFORM_BLOCK_NAME_LENGTH */) {
        var name = GLctx.getActiveUniformBlockName(program, uniformBlockIndex);
        HEAP32[((params)>>2)] = name.length+1;
        return;
      }
  
      var result = GLctx.getActiveUniformBlockParameter(program, uniformBlockIndex, pname);
      if (result === null) return; // If an error occurs, nothing should be written to params.
      if (pname == 0x8A43 /*GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES*/) {
        for (var i = 0; i < result.length; i++) {
          HEAP32[(((params)+(i*4))>>2)] = result[i];
        }
      } else {
        HEAP32[((params)>>2)] = result;
      }
    };
  var _emscripten_glGetActiveUniformBlockiv = _glGetActiveUniformBlockiv;

  /** @suppress {duplicate } */
  var _glGetActiveUniformsiv = (program, uniformCount, uniformIndices, pname, params) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if params == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      if (uniformCount > 0 && uniformIndices == 0) {
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      program = GL.programs[program];
      var ids = [];
      for (var i = 0; i < uniformCount; i++) {
        ids.push(HEAP32[(((uniformIndices)+(i*4))>>2)]);
      }
  
      var result = GLctx.getActiveUniforms(program, ids, pname);
      if (!result) return; // GL spec: If an error is generated, nothing is written out to params.
  
      var len = result.length;
      for (var i = 0; i < len; i++) {
        HEAP32[(((params)+(i*4))>>2)] = result[i];
      }
    };
  var _emscripten_glGetActiveUniformsiv = _glGetActiveUniformsiv;

  /** @suppress {duplicate } */
  var _glGetAttachedShaders = (program, maxCount, count, shaders) => {
      var result = GLctx.getAttachedShaders(GL.programs[program]);
      var len = result.length;
      if (len > maxCount) {
        len = maxCount;
      }
      HEAP32[((count)>>2)] = len;
      for (var i = 0; i < len; ++i) {
        var id = GL.shaders.indexOf(result[i]);
        HEAP32[(((shaders)+(i*4))>>2)] = id;
      }
    };
  var _emscripten_glGetAttachedShaders = _glGetAttachedShaders;

  
  /** @suppress {duplicate } */
  var _glGetAttribLocation = (program, name) =>
      GLctx.getAttribLocation(GL.programs[program], UTF8ToString(name));
  var _emscripten_glGetAttribLocation = _glGetAttribLocation;

  var readI53FromI64 = (ptr) => {
      return HEAPU32[((ptr)>>2)] + HEAP32[(((ptr)+(4))>>2)] * 4294967296;
    };
  
  var readI53FromU64 = (ptr) => {
      return HEAPU32[((ptr)>>2)] + HEAPU32[(((ptr)+(4))>>2)] * 4294967296;
    };
  var writeI53ToI64 = (ptr, num) => {
      HEAPU32[((ptr)>>2)] = num;
      var lower = HEAPU32[((ptr)>>2)];
      HEAPU32[(((ptr)+(4))>>2)] = (num - lower)/4294967296;
      var deserialized = (num >= 0) ? readI53FromU64(ptr) : readI53FromI64(ptr);
      var offset = ((ptr)>>2);
      if (deserialized != num) warnOnce(`writeI53ToI64() out of range: serialized JS Number ${num} to Wasm heap as bytes lo=${ptrToString(HEAPU32[offset])}, hi=${ptrToString(HEAPU32[offset+1])}, which deserializes back to ${deserialized} instead!`);
    };
  
  
  var webglGetExtensions = () => {
      var exts = getEmscriptenSupportedExtensions(GLctx);
      exts = exts.concat(exts.map((e) => "GL_" + e));
      return exts;
    };
  
  var emscriptenWebGLGet = (name_, p, type) => {
      // Guard against user passing a null pointer.
      // Note that GLES2 spec does not say anything about how passing a null
      // pointer should be treated.  Testing on desktop core GL 3, the application
      // crashes on glGetIntegerv to a null pointer, but better to report an error
      // instead of doing anything random.
      if (!p) {
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      var ret = undefined;
      switch (name_) { // Handle a few trivial GLES values
        case 0x8DFA: // GL_SHADER_COMPILER
          ret = 1;
          break;
        case 0x8DF8: // GL_SHADER_BINARY_FORMATS
          if (type != 0 && type != 1) {
            GL.recordError(0x500); // GL_INVALID_ENUM
          }
          // Do not write anything to the out pointer, since no binary formats are
          // supported.
          return;
        case 0x87FE: // GL_NUM_PROGRAM_BINARY_FORMATS
        case 0x8DF9: // GL_NUM_SHADER_BINARY_FORMATS
          ret = 0;
          break;
        case 0x86A2: // GL_NUM_COMPRESSED_TEXTURE_FORMATS
          // WebGL doesn't have GL_NUM_COMPRESSED_TEXTURE_FORMATS (it's obsolete
          // since GL_COMPRESSED_TEXTURE_FORMATS returns a JS array that can be
          // queried for length), so implement it ourselves to allow C++ GLES2
          // code get the length.
          var formats = GLctx.getParameter(0x86A3 /*GL_COMPRESSED_TEXTURE_FORMATS*/);
          ret = formats ? formats.length : 0;
          break;
  
        case 0x821D: // GL_NUM_EXTENSIONS
          if (GL.currentContext.version < 2) {
            // Calling GLES3/WebGL2 function with a GLES2/WebGL1 context
            GL.recordError(0x502 /* GL_INVALID_OPERATION */);
            return;
          }
          ret = webglGetExtensions().length;
          break;
        case 0x821B: // GL_MAJOR_VERSION
        case 0x821C: // GL_MINOR_VERSION
          if (GL.currentContext.version < 2) {
            GL.recordError(0x500); // GL_INVALID_ENUM
            return;
          }
          ret = name_ == 0x821B ? 3 : 0; // return version 3.0
          break;
      }
  
      if (ret === undefined) {
        var result = GLctx.getParameter(name_);
        switch (typeof result) {
          case "number":
            ret = result;
            break;
          case "boolean":
            ret = result ? 1 : 0;
            break;
          case "string":
            GL.recordError(0x500); // GL_INVALID_ENUM
            return;
          case "object":
            if (result === null) {
              // null is a valid result for some (e.g., which buffer is bound -
              // perhaps nothing is bound), but otherwise can mean an invalid
              // name_, which we need to report as an error
              switch (name_) {
                case 0x8894: // ARRAY_BUFFER_BINDING
                case 0x8B8D: // CURRENT_PROGRAM
                case 0x8895: // ELEMENT_ARRAY_BUFFER_BINDING
                case 0x8CA6: // FRAMEBUFFER_BINDING or DRAW_FRAMEBUFFER_BINDING
                case 0x8CA7: // RENDERBUFFER_BINDING
                case 0x8069: // TEXTURE_BINDING_2D
                case 0x85B5: // WebGL 2 GL_VERTEX_ARRAY_BINDING, or WebGL 1 extension OES_vertex_array_object GL_VERTEX_ARRAY_BINDING_OES
                case 0x8F36: // COPY_READ_BUFFER_BINDING or COPY_READ_BUFFER
                case 0x8F37: // COPY_WRITE_BUFFER_BINDING or COPY_WRITE_BUFFER
                case 0x88ED: // PIXEL_PACK_BUFFER_BINDING
                case 0x88EF: // PIXEL_UNPACK_BUFFER_BINDING
                case 0x8CAA: // READ_FRAMEBUFFER_BINDING
                case 0x8919: // SAMPLER_BINDING
                case 0x8C1D: // TEXTURE_BINDING_2D_ARRAY
                case 0x806A: // TEXTURE_BINDING_3D
                case 0x8E25: // TRANSFORM_FEEDBACK_BINDING
                case 0x8C8F: // TRANSFORM_FEEDBACK_BUFFER_BINDING
                case 0x8A28: // UNIFORM_BUFFER_BINDING
                case 0x8514: { // TEXTURE_BINDING_CUBE_MAP
                  ret = 0;
                  break;
                }
                default: {
                  GL.recordError(0x500); // GL_INVALID_ENUM
                  return;
                }
              }
            } else if (result instanceof Float32Array ||
                       result instanceof Uint32Array ||
                       result instanceof Int32Array ||
                       result instanceof Array) {
              for (var i = 0; i < result.length; ++i) {
                switch (type) {
                  case 0: HEAP32[(((p)+(i*4))>>2)] = result[i]; break;
                  case 2: HEAPF32[(((p)+(i*4))>>2)] = result[i]; break;
                  case 4: HEAP8[(p)+(i)] = result[i] ? 1 : 0; break;
                }
              }
              return;
            } else {
              try {
                ret = result.name | 0;
              } catch(e) {
                GL.recordError(0x500); // GL_INVALID_ENUM
                err(`GL_INVALID_ENUM in glGet${type}v: Unknown object returned from WebGL getParameter(${name_})! (error: ${e})`);
                return;
              }
            }
            break;
          default:
            GL.recordError(0x500); // GL_INVALID_ENUM
            err(`GL_INVALID_ENUM in glGet${type}v: Native code calling glGet${type}v(${name_}) and it returns ${result} of type ${typeof(result)}!`);
            return;
        }
      }
  
      switch (type) {
        case 1: writeI53ToI64(p, ret); break;
        case 0: HEAP32[((p)>>2)] = ret; break;
        case 2:   HEAPF32[((p)>>2)] = ret; break;
        case 4: HEAP8[p] = ret ? 1 : 0; break;
      }
    };
  
  /** @suppress {duplicate } */
  var _glGetBooleanv = (name_, p) => emscriptenWebGLGet(name_, p, 4);
  var _emscripten_glGetBooleanv = _glGetBooleanv;

  /** @suppress {duplicate } */
  var _glGetBufferParameteri64v = (target, value, data) => {
      if (!data) {
        // GLES2 specification does not specify how to behave if data is a null pointer. Since calling this function does not make sense
        // if data == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      writeI53ToI64(data, GLctx.getBufferParameter(target, value));
    };
  var _emscripten_glGetBufferParameteri64v = _glGetBufferParameteri64v;

  /** @suppress {duplicate } */
  var _glGetBufferParameteriv = (target, value, data) => {
      if (!data) {
        // GLES2 specification does not specify how to behave if data is a null
        // pointer. Since calling this function does not make sense if data ==
        // null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAP32[((data)>>2)] = GLctx.getBufferParameter(target, value);
    };
  var _emscripten_glGetBufferParameteriv = _glGetBufferParameteriv;

  /** @suppress {duplicate } */
  var _glGetError = () => {
      var error = GLctx.getError() || GL.lastError;
      GL.lastError = 0/*GL_NO_ERROR*/;
      return error;
    };
  var _emscripten_glGetError = _glGetError;

  
  /** @suppress {duplicate } */
  var _glGetFloatv = (name_, p) => emscriptenWebGLGet(name_, p, 2);
  var _emscripten_glGetFloatv = _glGetFloatv;

  /** @suppress {duplicate } */
  var _glGetFragDataLocation = (program, name) => {
      return GLctx.getFragDataLocation(GL.programs[program], UTF8ToString(name));
    };
  var _emscripten_glGetFragDataLocation = _glGetFragDataLocation;

  /** @suppress {duplicate } */
  var _glGetFramebufferAttachmentParameteriv = (target, attachment, pname, params) => {
      var result = GLctx.getFramebufferAttachmentParameter(target, attachment, pname);
      if (result instanceof WebGLRenderbuffer ||
          result instanceof WebGLTexture) {
        result = result.name | 0;
      }
      HEAP32[((params)>>2)] = result;
    };
  var _emscripten_glGetFramebufferAttachmentParameteriv = _glGetFramebufferAttachmentParameteriv;

  var emscriptenWebGLGetIndexed = (target, index, data, type) => {
      if (!data) {
        // GLES2 specification does not specify how to behave if data is a null pointer. Since calling this function does not make sense
        // if data == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      var result = GLctx.getIndexedParameter(target, index);
      var ret;
      switch (typeof result) {
        case 'boolean':
          ret = result ? 1 : 0;
          break;
        case 'number':
          ret = result;
          break;
        case 'object':
          if (result === null) {
            switch (target) {
              case 0x8C8F: // TRANSFORM_FEEDBACK_BUFFER_BINDING
              case 0x8A28: // UNIFORM_BUFFER_BINDING
                ret = 0;
                break;
              default: {
                GL.recordError(0x500); // GL_INVALID_ENUM
                return;
              }
            }
          } else if (result instanceof WebGLBuffer) {
            ret = result.name | 0;
          } else {
            GL.recordError(0x500); // GL_INVALID_ENUM
            return;
          }
          break;
        default:
          GL.recordError(0x500); // GL_INVALID_ENUM
          return;
      }
  
      switch (type) {
        case 1: writeI53ToI64(data, ret); break;
        case 0: HEAP32[((data)>>2)] = ret; break;
        case 2: HEAPF32[((data)>>2)] = ret; break;
        case 4: HEAP8[data] = ret ? 1 : 0; break;
        default: throw 'internal emscriptenWebGLGetIndexed() error, bad type: ' + type;
      }
    };
  /** @suppress {duplicate } */
  var _glGetInteger64i_v = (target, index, data) =>
      emscriptenWebGLGetIndexed(target, index, data, 1);
  var _emscripten_glGetInteger64i_v = _glGetInteger64i_v;

  /** @suppress {duplicate } */
  var _glGetInteger64v = (name_, p) => {
      emscriptenWebGLGet(name_, p, 1);
    };
  var _emscripten_glGetInteger64v = _glGetInteger64v;

  /** @suppress {duplicate } */
  var _glGetIntegeri_v = (target, index, data) =>
      emscriptenWebGLGetIndexed(target, index, data, 0);
  var _emscripten_glGetIntegeri_v = _glGetIntegeri_v;

  
  /** @suppress {duplicate } */
  var _glGetIntegerv = (name_, p) => emscriptenWebGLGet(name_, p, 0);
  var _emscripten_glGetIntegerv = _glGetIntegerv;

  /** @suppress {duplicate } */
  var _glGetInternalformativ = (target, internalformat, pname, bufSize, params) => {
      if (bufSize < 0) {
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      if (!params) {
        // GLES3 specification does not specify how to behave if values is a null pointer. Since calling this function does not make sense
        // if values == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      var ret = GLctx.getInternalformatParameter(target, internalformat, pname);
      if (ret === null) return;
      for (var i = 0; i < ret.length && i < bufSize; ++i) {
        HEAP32[(((params)+(i*4))>>2)] = ret[i];
      }
    };
  var _emscripten_glGetInternalformativ = _glGetInternalformativ;

  /** @suppress {duplicate } */
  var _glGetProgramBinary = (program, bufSize, length, binaryFormat, binary) => {
      GL.recordError(0x502/*GL_INVALID_OPERATION*/);
    };
  var _emscripten_glGetProgramBinary = _glGetProgramBinary;

  /** @suppress {duplicate } */
  var _glGetProgramInfoLog = (program, maxLength, length, infoLog) => {
      var log = GLctx.getProgramInfoLog(GL.programs[program]);
      if (log === null) log = '(unknown error)';
      var numBytesWrittenExclNull = (maxLength > 0 && infoLog) ? stringToUTF8(log, infoLog, maxLength) : 0;
      if (length) HEAP32[((length)>>2)] = numBytesWrittenExclNull;
    };
  var _emscripten_glGetProgramInfoLog = _glGetProgramInfoLog;

  /** @suppress {duplicate } */
  var _glGetProgramiv = (program, pname, p) => {
      if (!p) {
        // GLES2 specification does not specify how to behave if p is a null
        // pointer. Since calling this function does not make sense if p == null,
        // issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
  
      if (program >= GL.counter) {
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
  
      program = GL.programs[program];
  
      if (pname == 0x8B84) { // GL_INFO_LOG_LENGTH
        var log = GLctx.getProgramInfoLog(program);
        if (log === null) log = '(unknown error)';
        HEAP32[((p)>>2)] = log.length + 1;
      } else if (pname == 0x8B87 /* GL_ACTIVE_UNIFORM_MAX_LENGTH */) {
        if (!program.maxUniformLength) {
          var numActiveUniforms = GLctx.getProgramParameter(program, 0x8B86/*GL_ACTIVE_UNIFORMS*/);
          for (var i = 0; i < numActiveUniforms; ++i) {
            program.maxUniformLength = Math.max(program.maxUniformLength, GLctx.getActiveUniform(program, i).name.length+1);
          }
        }
        HEAP32[((p)>>2)] = program.maxUniformLength;
      } else if (pname == 0x8B8A /* GL_ACTIVE_ATTRIBUTE_MAX_LENGTH */) {
        if (!program.maxAttributeLength) {
          var numActiveAttributes = GLctx.getProgramParameter(program, 0x8B89/*GL_ACTIVE_ATTRIBUTES*/);
          for (var i = 0; i < numActiveAttributes; ++i) {
            program.maxAttributeLength = Math.max(program.maxAttributeLength, GLctx.getActiveAttrib(program, i).name.length+1);
          }
        }
        HEAP32[((p)>>2)] = program.maxAttributeLength;
      } else if (pname == 0x8A35 /* GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH */) {
        if (!program.maxUniformBlockNameLength) {
          var numActiveUniformBlocks = GLctx.getProgramParameter(program, 0x8A36/*GL_ACTIVE_UNIFORM_BLOCKS*/);
          for (var i = 0; i < numActiveUniformBlocks; ++i) {
            program.maxUniformBlockNameLength = Math.max(program.maxUniformBlockNameLength, GLctx.getActiveUniformBlockName(program, i).length+1);
          }
        }
        HEAP32[((p)>>2)] = program.maxUniformBlockNameLength;
      } else {
        HEAP32[((p)>>2)] = GLctx.getProgramParameter(program, pname);
      }
    };
  var _emscripten_glGetProgramiv = _glGetProgramiv;

  
  /** @suppress {duplicate } */
  var _glGetQueryObjecti64vEXT = (id, pname, params) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      var query = GL.queries[id];
      var param;
      if (GL.currentContext.version < 2)
      {
        param = GLctx.disjointTimerQueryExt['getQueryObjectEXT'](query, pname);
      }
      else {
        param = GLctx.getQueryParameter(query, pname);
      }
      var ret;
      if (typeof param == 'boolean') {
        ret = param ? 1 : 0;
      } else {
        ret = param;
      }
      writeI53ToI64(params, ret);
    };
  var _emscripten_glGetQueryObjecti64vEXT = _glGetQueryObjecti64vEXT;

  /** @suppress {duplicate } */
  var _glGetQueryObjectivEXT = (id, pname, params) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      var query = GL.queries[id];
      var param = GLctx.disjointTimerQueryExt['getQueryObjectEXT'](query, pname);
      var ret;
      if (typeof param == 'boolean') {
        ret = param ? 1 : 0;
      } else {
        ret = param;
      }
      HEAP32[((params)>>2)] = ret;
    };
  var _emscripten_glGetQueryObjectivEXT = _glGetQueryObjectivEXT;

  
  /** @suppress {duplicate } */
  var _glGetQueryObjectui64vEXT = _glGetQueryObjecti64vEXT;
  var _emscripten_glGetQueryObjectui64vEXT = _glGetQueryObjectui64vEXT;

  /** @suppress {duplicate } */
  var _glGetQueryObjectuiv = (id, pname, params) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      var query = GL.queries[id];
      var param = GLctx.getQueryParameter(query, pname);
      var ret;
      if (typeof param == 'boolean') {
        ret = param ? 1 : 0;
      } else {
        ret = param;
      }
      HEAP32[((params)>>2)] = ret;
    };
  var _emscripten_glGetQueryObjectuiv = _glGetQueryObjectuiv;

  
  /** @suppress {duplicate } */
  var _glGetQueryObjectuivEXT = _glGetQueryObjectivEXT;
  var _emscripten_glGetQueryObjectuivEXT = _glGetQueryObjectuivEXT;

  /** @suppress {duplicate } */
  var _glGetQueryiv = (target, pname, params) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAP32[((params)>>2)] = GLctx.getQuery(target, pname);
    };
  var _emscripten_glGetQueryiv = _glGetQueryiv;

  /** @suppress {duplicate } */
  var _glGetQueryivEXT = (target, pname, params) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAP32[((params)>>2)] = GLctx.disjointTimerQueryExt['getQueryEXT'](target, pname);
    };
  var _emscripten_glGetQueryivEXT = _glGetQueryivEXT;

  /** @suppress {duplicate } */
  var _glGetRenderbufferParameteriv = (target, pname, params) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if params == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAP32[((params)>>2)] = GLctx.getRenderbufferParameter(target, pname);
    };
  var _emscripten_glGetRenderbufferParameteriv = _glGetRenderbufferParameteriv;

  /** @suppress {duplicate } */
  var _glGetSamplerParameterfv = (sampler, pname, params) => {
      if (!params) {
        // GLES3 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAPF32[((params)>>2)] = GLctx.getSamplerParameter(GL.samplers[sampler], pname);
    };
  var _emscripten_glGetSamplerParameterfv = _glGetSamplerParameterfv;

  /** @suppress {duplicate } */
  var _glGetSamplerParameteriv = (sampler, pname, params) => {
      if (!params) {
        // GLES3 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
        // if p == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAP32[((params)>>2)] = GLctx.getSamplerParameter(GL.samplers[sampler], pname);
    };
  var _emscripten_glGetSamplerParameteriv = _glGetSamplerParameteriv;

  
  /** @suppress {duplicate } */
  var _glGetShaderInfoLog = (shader, maxLength, length, infoLog) => {
      var log = GLctx.getShaderInfoLog(GL.shaders[shader]);
      if (log === null) log = '(unknown error)';
      var numBytesWrittenExclNull = (maxLength > 0 && infoLog) ? stringToUTF8(log, infoLog, maxLength) : 0;
      if (length) HEAP32[((length)>>2)] = numBytesWrittenExclNull;
    };
  var _emscripten_glGetShaderInfoLog = _glGetShaderInfoLog;

  /** @suppress {duplicate } */
  var _glGetShaderPrecisionFormat = (shaderType, precisionType, range, precision) => {
      var result = GLctx.getShaderPrecisionFormat(shaderType, precisionType);
      HEAP32[((range)>>2)] = result.rangeMin;
      HEAP32[(((range)+(4))>>2)] = result.rangeMax;
      HEAP32[((precision)>>2)] = result.precision;
    };
  var _emscripten_glGetShaderPrecisionFormat = _glGetShaderPrecisionFormat;

  /** @suppress {duplicate } */
  var _glGetShaderSource = (shader, bufSize, length, source) => {
      var result = GLctx.getShaderSource(GL.shaders[shader]);
      if (!result) return; // If an error occurs, nothing will be written to length or source.
      var numBytesWrittenExclNull = (bufSize > 0 && source) ? stringToUTF8(result, source, bufSize) : 0;
      if (length) HEAP32[((length)>>2)] = numBytesWrittenExclNull;
    };
  var _emscripten_glGetShaderSource = _glGetShaderSource;

  /** @suppress {duplicate } */
  var _glGetShaderiv = (shader, pname, p) => {
      if (!p) {
        // GLES2 specification does not specify how to behave if p is a null
        // pointer. Since calling this function does not make sense if p == null,
        // issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      if (pname == 0x8B84) { // GL_INFO_LOG_LENGTH
        var log = GLctx.getShaderInfoLog(GL.shaders[shader]);
        if (log === null) log = '(unknown error)';
        // The GLES2 specification says that if the shader has an empty info log,
        // a value of 0 is returned. Otherwise the log has a null char appended.
        // (An empty string is falsey, so we can just check that instead of
        // looking at log.length.)
        var logLength = log ? log.length + 1 : 0;
        HEAP32[((p)>>2)] = logLength;
      } else if (pname == 0x8B88) { // GL_SHADER_SOURCE_LENGTH
        var source = GLctx.getShaderSource(GL.shaders[shader]);
        // source may be a null, or the empty string, both of which are falsey
        // values that we report a 0 length for.
        var sourceLength = source ? source.length + 1 : 0;
        HEAP32[((p)>>2)] = sourceLength;
      } else {
        HEAP32[((p)>>2)] = GLctx.getShaderParameter(GL.shaders[shader], pname);
      }
    };
  var _emscripten_glGetShaderiv = _glGetShaderiv;

  
  
  /** @suppress {duplicate } */
  var _glGetString = (name_) => {
      var ret = GL.stringCache[name_];
      if (!ret) {
        switch (name_) {
          case 0x1F03 /* GL_EXTENSIONS */:
            ret = stringToNewUTF8(webglGetExtensions().join(' '));
            break;
          case 0x1F00 /* GL_VENDOR */:
          case 0x1F01 /* GL_RENDERER */:
          case 0x9245 /* UNMASKED_VENDOR_WEBGL */:
          case 0x9246 /* UNMASKED_RENDERER_WEBGL */:
            var s = GLctx.getParameter(name_);
            if (!s) {
              GL.recordError(0x500/*GL_INVALID_ENUM*/);
            }
            ret = s ? stringToNewUTF8(s) : 0;
            break;
  
          case 0x1F02 /* GL_VERSION */:
            var webGLVersion = GLctx.getParameter(0x1F02 /*GL_VERSION*/);
            // return GLES version string corresponding to the version of the WebGL context
            var glVersion = `OpenGL ES 2.0 (${webGLVersion})`;
            if (true) glVersion = `OpenGL ES 3.0 (${webGLVersion})`;
            ret = stringToNewUTF8(glVersion);
            break;
          case 0x8B8C /* GL_SHADING_LANGUAGE_VERSION */:
            var glslVersion = GLctx.getParameter(0x8B8C /*GL_SHADING_LANGUAGE_VERSION*/);
            // extract the version number 'N.M' from the string 'WebGL GLSL ES N.M ...'
            var ver_re = /^WebGL GLSL ES ([0-9]\.[0-9][0-9]?)(?:$| .*)/;
            var ver_num = glslVersion.match(ver_re);
            if (ver_num !== null) {
              if (ver_num[1].length == 3) ver_num[1] = ver_num[1] + '0'; // ensure minor version has 2 digits
              glslVersion = `OpenGL ES GLSL ES ${ver_num[1]} (${glslVersion})`;
            }
            ret = stringToNewUTF8(glslVersion);
            break;
          default:
            GL.recordError(0x500/*GL_INVALID_ENUM*/);
            // fall through
        }
        GL.stringCache[name_] = ret;
      }
      return ret;
    };
  var _emscripten_glGetString = _glGetString;

  
  /** @suppress {duplicate } */
  var _glGetStringi = (name, index) => {
      if (GL.currentContext.version < 2) {
        GL.recordError(0x502 /* GL_INVALID_OPERATION */); // Calling GLES3/WebGL2 function with a GLES2/WebGL1 context
        return 0;
      }
      var stringiCache = GL.stringiCache[name];
      if (stringiCache) {
        if (index < 0 || index >= stringiCache.length) {
          GL.recordError(0x501/*GL_INVALID_VALUE*/);
          return 0;
        }
        return stringiCache[index];
      }
      switch (name) {
        case 0x1F03 /* GL_EXTENSIONS */:
          var exts = webglGetExtensions().map(stringToNewUTF8);
          stringiCache = GL.stringiCache[name] = exts;
          if (index < 0 || index >= stringiCache.length) {
            GL.recordError(0x501/*GL_INVALID_VALUE*/);
            return 0;
          }
          return stringiCache[index];
        default:
          GL.recordError(0x500/*GL_INVALID_ENUM*/);
          return 0;
      }
    };
  var _emscripten_glGetStringi = _glGetStringi;

  /** @suppress {duplicate } */
  var _glGetSynciv = (sync, pname, bufSize, length, values) => {
      if (bufSize < 0) {
        // GLES3 specification does not specify how to behave if bufSize < 0, however in the spec wording for glGetInternalformativ, it does say that GL_INVALID_VALUE should be raised,
        // so raise GL_INVALID_VALUE here as well.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      if (!values) {
        // GLES3 specification does not specify how to behave if values is a null pointer. Since calling this function does not make sense
        // if values == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      var ret = GLctx.getSyncParameter(GL.syncs[sync], pname);
      if (ret !== null) {
        HEAP32[((values)>>2)] = ret;
        if (length) HEAP32[((length)>>2)] = 1; // Report a single value outputted.
      }
    };
  var _emscripten_glGetSynciv = _glGetSynciv;

  /** @suppress {duplicate } */
  var _glGetTexParameterfv = (target, pname, params) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null
        // pointer. Since calling this function does not make sense if p == null,
        // issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAPF32[((params)>>2)] = GLctx.getTexParameter(target, pname);
    };
  var _emscripten_glGetTexParameterfv = _glGetTexParameterfv;

  /** @suppress {duplicate } */
  var _glGetTexParameteriv = (target, pname, params) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null
        // pointer. Since calling this function does not make sense if p == null,
        // issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAP32[((params)>>2)] = GLctx.getTexParameter(target, pname);
    };
  var _emscripten_glGetTexParameteriv = _glGetTexParameteriv;

  /** @suppress {duplicate } */
  var _glGetTransformFeedbackVarying = (program, index, bufSize, length, size, type, name) => {
      program = GL.programs[program];
      var info = GLctx.getTransformFeedbackVarying(program, index);
      if (!info) return; // If an error occurred, the return parameters length, size, type and name will be unmodified.
  
      if (name && bufSize > 0) {
        var numBytesWrittenExclNull = stringToUTF8(info.name, name, bufSize);
        if (length) HEAP32[((length)>>2)] = numBytesWrittenExclNull;
      } else {
        if (length) HEAP32[((length)>>2)] = 0;
      }
  
      if (size) HEAP32[((size)>>2)] = info.size;
      if (type) HEAP32[((type)>>2)] = info.type;
    };
  var _emscripten_glGetTransformFeedbackVarying = _glGetTransformFeedbackVarying;

  /** @suppress {duplicate } */
  var _glGetUniformBlockIndex = (program, uniformBlockName) => {
      return GLctx.getUniformBlockIndex(GL.programs[program], UTF8ToString(uniformBlockName));
    };
  var _emscripten_glGetUniformBlockIndex = _glGetUniformBlockIndex;

  /** @suppress {duplicate } */
  var _glGetUniformIndices = (program, uniformCount, uniformNames, uniformIndices) => {
      if (!uniformIndices) {
        // GLES2 specification does not specify how to behave if uniformIndices is a null pointer. Since calling this function does not make sense
        // if uniformIndices == null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      if (uniformCount > 0 && (uniformNames == 0 || uniformIndices == 0)) {
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      program = GL.programs[program];
      var names = [];
      for (var i = 0; i < uniformCount; i++)
        names.push(UTF8ToString(HEAP32[(((uniformNames)+(i*4))>>2)]));
  
      var result = GLctx.getUniformIndices(program, names);
      if (!result) return; // GL spec: If an error is generated, nothing is written out to uniformIndices.
  
      var len = result.length;
      for (var i = 0; i < len; i++) {
        HEAP32[(((uniformIndices)+(i*4))>>2)] = result[i];
      }
    };
  var _emscripten_glGetUniformIndices = _glGetUniformIndices;

  
  /** @noinline */
  var webglGetLeftBracePos = (name) => name.slice(-1) == ']' && name.lastIndexOf('[');
  
  var webglPrepareUniformLocationsBeforeFirstUse = (program) => {
      var uniformLocsById = program.uniformLocsById, // Maps GLuint -> WebGLUniformLocation
        uniformSizeAndIdsByName = program.uniformSizeAndIdsByName, // Maps name -> [uniform array length, GLuint]
        i, j;
  
      // On the first time invocation of glGetUniformLocation on this shader program:
      // initialize cache data structures and discover which uniforms are arrays.
      if (!uniformLocsById) {
        // maps GLint integer locations to WebGLUniformLocations
        program.uniformLocsById = uniformLocsById = {};
        // maps integer locations back to uniform name strings, so that we can lazily fetch uniform array locations
        program.uniformArrayNamesById = {};
  
        var numActiveUniforms = GLctx.getProgramParameter(program, 0x8B86/*GL_ACTIVE_UNIFORMS*/);
        for (i = 0; i < numActiveUniforms; ++i) {
          var u = GLctx.getActiveUniform(program, i);
          var nm = u.name;
          var sz = u.size;
          var lb = webglGetLeftBracePos(nm);
          var arrayName = lb > 0 ? nm.slice(0, lb) : nm;
  
          // Assign a new location.
          var id = program.uniformIdCounter;
          program.uniformIdCounter += sz;
          // Eagerly get the location of the uniformArray[0] base element.
          // The remaining indices >0 will be left for lazy evaluation to
          // improve performance. Those may never be needed to fetch, if the
          // application fills arrays always in full starting from the first
          // element of the array.
          uniformSizeAndIdsByName[arrayName] = [sz, id];
  
          // Store placeholder integers in place that highlight that these
          // >0 index locations are array indices pending population.
          for (j = 0; j < sz; ++j) {
            uniformLocsById[id] = j;
            program.uniformArrayNamesById[id++] = arrayName;
          }
        }
      }
    };
  
  
  
  /** @suppress {duplicate } */
  var _glGetUniformLocation = (program, name) => {
  
      name = UTF8ToString(name);
  
      if (program = GL.programs[program]) {
        webglPrepareUniformLocationsBeforeFirstUse(program);
        var uniformLocsById = program.uniformLocsById; // Maps GLuint -> WebGLUniformLocation
        var arrayIndex = 0;
        var uniformBaseName = name;
  
        // Invariant: when populating integer IDs for uniform locations, we must
        // maintain the precondition that arrays reside in contiguous addresses,
        // i.e. for a 'vec4 colors[10];', colors[4] must be at location
        // colors[0]+4.  However, user might call glGetUniformLocation(program,
        // "colors") for an array, so we cannot discover based on the user input
        // arguments whether the uniform we are dealing with is an array. The only
        // way to discover which uniforms are arrays is to enumerate over all the
        // active uniforms in the program.
        var leftBrace = webglGetLeftBracePos(name);
  
        // If user passed an array accessor "[index]", parse the array index off the accessor.
        if (leftBrace > 0) {
          arrayIndex = jstoi_q(name.slice(leftBrace + 1)) >>> 0; // "index]", coerce parseInt(']') with >>>0 to treat "foo[]" as "foo[0]" and foo[-1] as unsigned out-of-bounds.
          uniformBaseName = name.slice(0, leftBrace);
        }
  
        // Have we cached the location of this uniform before?
        // A pair [array length, GLint of the uniform location]
        var sizeAndId = program.uniformSizeAndIdsByName[uniformBaseName];
  
        // If an uniform with this name exists, and if its index is within the
        // array limits (if it's even an array), query the WebGLlocation, or
        // return an existing cached location.
        if (sizeAndId && arrayIndex < sizeAndId[0]) {
          arrayIndex += sizeAndId[1]; // Add the base location of the uniform to the array index offset.
          if ((uniformLocsById[arrayIndex] = uniformLocsById[arrayIndex] || GLctx.getUniformLocation(program, name))) {
            return arrayIndex;
          }
        }
      }
      else {
        // N.b. we are currently unable to distinguish between GL program IDs that
        // never existed vs GL program IDs that have been deleted, so report
        // GL_INVALID_VALUE in both cases.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
      }
      return -1;
    };
  var _emscripten_glGetUniformLocation = _glGetUniformLocation;

  var webglGetUniformLocation = (location) => {
      var p = GLctx.currentProgram;
  
      if (p) {
        var webglLoc = p.uniformLocsById[location];
        // p.uniformLocsById[location] stores either an integer, or a
        // WebGLUniformLocation.
        // If an integer, we have not yet bound the location, so do it now. The
        // integer value specifies the array index we should bind to.
        if (typeof webglLoc == 'number') {
          p.uniformLocsById[location] = webglLoc = GLctx.getUniformLocation(p, p.uniformArrayNamesById[location] + (webglLoc > 0 ? `[${webglLoc}]` : ''));
        }
        // Else an already cached WebGLUniformLocation, return it.
        return webglLoc;
      } else {
        GL.recordError(0x502/*GL_INVALID_OPERATION*/);
      }
    };
  
  
  /** @suppress{checkTypes} */
  var emscriptenWebGLGetUniform = (program, location, params, type) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null
        // pointer. Since calling this function does not make sense if params ==
        // null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      program = GL.programs[program];
      webglPrepareUniformLocationsBeforeFirstUse(program);
      var data = GLctx.getUniform(program, webglGetUniformLocation(location));
      if (typeof data == 'number' || typeof data == 'boolean') {
        switch (type) {
          case 0: HEAP32[((params)>>2)] = data; break;
          case 2: HEAPF32[((params)>>2)] = data; break;
        }
      } else {
        for (var i = 0; i < data.length; i++) {
          switch (type) {
            case 0: HEAP32[(((params)+(i*4))>>2)] = data[i]; break;
            case 2: HEAPF32[(((params)+(i*4))>>2)] = data[i]; break;
          }
        }
      }
    };
  
  /** @suppress {duplicate } */
  var _glGetUniformfv = (program, location, params) => {
      emscriptenWebGLGetUniform(program, location, params, 2);
    };
  var _emscripten_glGetUniformfv = _glGetUniformfv;

  
  /** @suppress {duplicate } */
  var _glGetUniformiv = (program, location, params) => {
      emscriptenWebGLGetUniform(program, location, params, 0);
    };
  var _emscripten_glGetUniformiv = _glGetUniformiv;

  /** @suppress {duplicate } */
  var _glGetUniformuiv = (program, location, params) =>
      emscriptenWebGLGetUniform(program, location, params, 0);
  var _emscripten_glGetUniformuiv = _glGetUniformuiv;

  /** @suppress{checkTypes} */
  var emscriptenWebGLGetVertexAttrib = (index, pname, params, type) => {
      if (!params) {
        // GLES2 specification does not specify how to behave if params is a null
        // pointer. Since calling this function does not make sense if params ==
        // null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      var data = GLctx.getVertexAttrib(index, pname);
      if (pname == 0x889F/*VERTEX_ATTRIB_ARRAY_BUFFER_BINDING*/) {
        HEAP32[((params)>>2)] = data && data["name"];
      } else if (typeof data == 'number' || typeof data == 'boolean') {
        switch (type) {
          case 0: HEAP32[((params)>>2)] = data; break;
          case 2: HEAPF32[((params)>>2)] = data; break;
          case 5: HEAP32[((params)>>2)] = Math.fround(data); break;
        }
      } else {
        for (var i = 0; i < data.length; i++) {
          switch (type) {
            case 0: HEAP32[(((params)+(i*4))>>2)] = data[i]; break;
            case 2: HEAPF32[(((params)+(i*4))>>2)] = data[i]; break;
            case 5: HEAP32[(((params)+(i*4))>>2)] = Math.fround(data[i]); break;
          }
        }
      }
    };
  /** @suppress {duplicate } */
  var _glGetVertexAttribIiv = (index, pname, params) => {
      // N.B. This function may only be called if the vertex attribute was specified using the function glVertexAttribI4iv(),
      // otherwise the results are undefined. (GLES3 spec 6.1.12)
      emscriptenWebGLGetVertexAttrib(index, pname, params, 0);
    };
  var _emscripten_glGetVertexAttribIiv = _glGetVertexAttribIiv;

  
  /** @suppress {duplicate } */
  var _glGetVertexAttribIuiv = _glGetVertexAttribIiv;
  var _emscripten_glGetVertexAttribIuiv = _glGetVertexAttribIuiv;

  /** @suppress {duplicate } */
  var _glGetVertexAttribPointerv = (index, pname, pointer) => {
      if (!pointer) {
        // GLES2 specification does not specify how to behave if pointer is a null
        // pointer. Since calling this function does not make sense if pointer ==
        // null, issue a GL error to notify user about it.
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
      HEAP32[((pointer)>>2)] = GLctx.getVertexAttribOffset(index, pname);
    };
  var _emscripten_glGetVertexAttribPointerv = _glGetVertexAttribPointerv;

  
  /** @suppress {duplicate } */
  var _glGetVertexAttribfv = (index, pname, params) => {
      // N.B. This function may only be called if the vertex attribute was
      // specified using the function glVertexAttrib*f(), otherwise the results
      // are undefined. (GLES3 spec 6.1.12)
      emscriptenWebGLGetVertexAttrib(index, pname, params, 2);
    };
  var _emscripten_glGetVertexAttribfv = _glGetVertexAttribfv;

  
  /** @suppress {duplicate } */
  var _glGetVertexAttribiv = (index, pname, params) => {
      // N.B. This function may only be called if the vertex attribute was
      // specified using the function glVertexAttrib*f(), otherwise the results
      // are undefined. (GLES3 spec 6.1.12)
      emscriptenWebGLGetVertexAttrib(index, pname, params, 5);
    };
  var _emscripten_glGetVertexAttribiv = _glGetVertexAttribiv;

  /** @suppress {duplicate } */
  var _glHint = (x0, x1) => GLctx.hint(x0, x1);
  var _emscripten_glHint = _glHint;

  /** @suppress {duplicate } */
  var _glInvalidateFramebuffer = (target, numAttachments, attachments) => {
      var list = tempFixedLengthArray[numAttachments];
      for (var i = 0; i < numAttachments; i++) {
        list[i] = HEAP32[(((attachments)+(i*4))>>2)];
      }
  
      GLctx.invalidateFramebuffer(target, list);
    };
  var _emscripten_glInvalidateFramebuffer = _glInvalidateFramebuffer;

  /** @suppress {duplicate } */
  var _glInvalidateSubFramebuffer = (target, numAttachments, attachments, x, y, width, height) => {
      var list = tempFixedLengthArray[numAttachments];
      for (var i = 0; i < numAttachments; i++) {
        list[i] = HEAP32[(((attachments)+(i*4))>>2)];
      }
  
      GLctx.invalidateSubFramebuffer(target, list, x, y, width, height);
    };
  var _emscripten_glInvalidateSubFramebuffer = _glInvalidateSubFramebuffer;

  /** @suppress {duplicate } */
  var _glIsBuffer = (buffer) => {
      var b = GL.buffers[buffer];
      if (!b) return 0;
      return GLctx.isBuffer(b);
    };
  var _emscripten_glIsBuffer = _glIsBuffer;

  /** @suppress {duplicate } */
  var _glIsEnabled = (x0) => GLctx.isEnabled(x0);
  var _emscripten_glIsEnabled = _glIsEnabled;

  /** @suppress {duplicate } */
  var _glIsFramebuffer = (framebuffer) => {
      var fb = GL.framebuffers[framebuffer];
      if (!fb) return 0;
      return GLctx.isFramebuffer(fb);
    };
  var _emscripten_glIsFramebuffer = _glIsFramebuffer;

  /** @suppress {duplicate } */
  var _glIsProgram = (program) => {
      program = GL.programs[program];
      if (!program) return 0;
      return GLctx.isProgram(program);
    };
  var _emscripten_glIsProgram = _glIsProgram;

  /** @suppress {duplicate } */
  var _glIsQuery = (id) => {
      var query = GL.queries[id];
      if (!query) return 0;
      return GLctx.isQuery(query);
    };
  var _emscripten_glIsQuery = _glIsQuery;

  /** @suppress {duplicate } */
  var _glIsQueryEXT = (id) => {
      var query = GL.queries[id];
      if (!query) return 0;
      return GLctx.disjointTimerQueryExt['isQueryEXT'](query);
    };
  var _emscripten_glIsQueryEXT = _glIsQueryEXT;

  /** @suppress {duplicate } */
  var _glIsRenderbuffer = (renderbuffer) => {
      var rb = GL.renderbuffers[renderbuffer];
      if (!rb) return 0;
      return GLctx.isRenderbuffer(rb);
    };
  var _emscripten_glIsRenderbuffer = _glIsRenderbuffer;

  /** @suppress {duplicate } */
  var _glIsSampler = (id) => {
      var sampler = GL.samplers[id];
      if (!sampler) return 0;
      return GLctx.isSampler(sampler);
    };
  var _emscripten_glIsSampler = _glIsSampler;

  /** @suppress {duplicate } */
  var _glIsShader = (shader) => {
      var s = GL.shaders[shader];
      if (!s) return 0;
      return GLctx.isShader(s);
    };
  var _emscripten_glIsShader = _glIsShader;

  /** @suppress {duplicate } */
  var _glIsSync = (sync) => GLctx.isSync(GL.syncs[sync]);
  var _emscripten_glIsSync = _glIsSync;

  /** @suppress {duplicate } */
  var _glIsTexture = (id) => {
      var texture = GL.textures[id];
      if (!texture) return 0;
      return GLctx.isTexture(texture);
    };
  var _emscripten_glIsTexture = _glIsTexture;

  /** @suppress {duplicate } */
  var _glIsTransformFeedback = (id) => GLctx.isTransformFeedback(GL.transformFeedbacks[id]);
  var _emscripten_glIsTransformFeedback = _glIsTransformFeedback;

  /** @suppress {duplicate } */
  var _glIsVertexArray = (array) => {
  
      var vao = GL.vaos[array];
      if (!vao) return 0;
      return GLctx.isVertexArray(vao);
    };
  var _emscripten_glIsVertexArray = _glIsVertexArray;

  
  /** @suppress {duplicate } */
  var _glIsVertexArrayOES = _glIsVertexArray;
  var _emscripten_glIsVertexArrayOES = _glIsVertexArrayOES;

  /** @suppress {duplicate } */
  var _glLineWidth = (x0) => GLctx.lineWidth(x0);
  var _emscripten_glLineWidth = _glLineWidth;

  /** @suppress {duplicate } */
  var _glLinkProgram = (program) => {
      program = GL.programs[program];
      GLctx.linkProgram(program);
      // Invalidate earlier computed uniform->ID mappings, those have now become stale
      program.uniformLocsById = 0; // Mark as null-like so that glGetUniformLocation() knows to populate this again.
      program.uniformSizeAndIdsByName = {};
  
    };
  var _emscripten_glLinkProgram = _glLinkProgram;

  /** @suppress {duplicate } */
  var _glPauseTransformFeedback = () => GLctx.pauseTransformFeedback();
  var _emscripten_glPauseTransformFeedback = _glPauseTransformFeedback;

  /** @suppress {duplicate } */
  var _glPixelStorei = (pname, param) => {
      if (pname == 3317) {
        GL.unpackAlignment = param;
      } else if (pname == 3314) {
        GL.unpackRowLength = param;
      }
      GLctx.pixelStorei(pname, param);
    };
  var _emscripten_glPixelStorei = _glPixelStorei;

  /** @suppress {duplicate } */
  var _glPolygonModeWEBGL = (face, mode) => {
      GLctx.webglPolygonMode['polygonModeWEBGL'](face, mode);
    };
  var _emscripten_glPolygonModeWEBGL = _glPolygonModeWEBGL;

  /** @suppress {duplicate } */
  var _glPolygonOffset = (x0, x1) => GLctx.polygonOffset(x0, x1);
  var _emscripten_glPolygonOffset = _glPolygonOffset;

  /** @suppress {duplicate } */
  var _glPolygonOffsetClampEXT = (factor, units, clamp) => {
      GLctx.extPolygonOffsetClamp['polygonOffsetClampEXT'](factor, units, clamp);
    };
  var _emscripten_glPolygonOffsetClampEXT = _glPolygonOffsetClampEXT;

  /** @suppress {duplicate } */
  var _glProgramBinary = (program, binaryFormat, binary, length) => {
      GL.recordError(0x500/*GL_INVALID_ENUM*/);
    };
  var _emscripten_glProgramBinary = _glProgramBinary;

  /** @suppress {duplicate } */
  var _glProgramParameteri = (program, pname, value) => {
      GL.recordError(0x500/*GL_INVALID_ENUM*/);
    };
  var _emscripten_glProgramParameteri = _glProgramParameteri;

  /** @suppress {duplicate } */
  var _glQueryCounterEXT = (id, target) => {
      GLctx.disjointTimerQueryExt['queryCounterEXT'](GL.queries[id], target);
    };
  var _emscripten_glQueryCounterEXT = _glQueryCounterEXT;

  /** @suppress {duplicate } */
  var _glReadBuffer = (x0) => GLctx.readBuffer(x0);
  var _emscripten_glReadBuffer = _glReadBuffer;

  var heapObjectForWebGLType = (type) => {
      // Micro-optimization for size: Subtract lowest GL enum number (0x1400/* GL_BYTE */) from type to compare
      // smaller values for the heap, for shorter generated code size.
      // Also the type HEAPU16 is not tested for explicitly, but any unrecognized type will return out HEAPU16.
      // (since most types are HEAPU16)
      type -= 0x1400;
      if (type == 0) return HEAP8;
  
      if (type == 1) return HEAPU8;
  
      if (type == 2) return HEAP16;
  
      if (type == 4) return HEAP32;
  
      if (type == 6) return HEAPF32;
  
      if (type == 5
        || type == 28922
        || type == 28520
        || type == 30779
        || type == 30782
        )
        return HEAPU32;
  
      return HEAPU16;
    };
  
  var toTypedArrayIndex = (pointer, heap) =>
      pointer >>> (31 - Math.clz32(heap.BYTES_PER_ELEMENT));
  
  /** @suppress {duplicate } */
  var _glReadPixels = (x, y, width, height, format, type, pixels) => {
      if (true) {
        if (GLctx.currentPixelPackBufferBinding) {
          GLctx.readPixels(x, y, width, height, format, type, pixels);
          return;
        }
        var heap = heapObjectForWebGLType(type);
        var target = toTypedArrayIndex(pixels, heap);
        GLctx.readPixels(x, y, width, height, format, type, heap, target);
        return;
      }
    };
  var _emscripten_glReadPixels = _glReadPixels;

  /** @suppress {duplicate } */
  var _glReleaseShaderCompiler = () => {
      // NOP (as allowed by GLES 2.0 spec)
    };
  var _emscripten_glReleaseShaderCompiler = _glReleaseShaderCompiler;

  /** @suppress {duplicate } */
  var _glRenderbufferStorage = (x0, x1, x2, x3) => GLctx.renderbufferStorage(x0, x1, x2, x3);
  var _emscripten_glRenderbufferStorage = _glRenderbufferStorage;

  /** @suppress {duplicate } */
  var _glRenderbufferStorageMultisample = (x0, x1, x2, x3, x4) => GLctx.renderbufferStorageMultisample(x0, x1, x2, x3, x4);
  var _emscripten_glRenderbufferStorageMultisample = _glRenderbufferStorageMultisample;

  /** @suppress {duplicate } */
  var _glResumeTransformFeedback = () => GLctx.resumeTransformFeedback();
  var _emscripten_glResumeTransformFeedback = _glResumeTransformFeedback;

  /** @suppress {duplicate } */
  var _glSampleCoverage = (value, invert) => {
      GLctx.sampleCoverage(value, !!invert);
    };
  var _emscripten_glSampleCoverage = _glSampleCoverage;

  /** @suppress {duplicate } */
  var _glSamplerParameterf = (sampler, pname, param) => {
      GLctx.samplerParameterf(GL.samplers[sampler], pname, param);
    };
  var _emscripten_glSamplerParameterf = _glSamplerParameterf;

  /** @suppress {duplicate } */
  var _glSamplerParameterfv = (sampler, pname, params) => {
      var param = HEAPF32[((params)>>2)];
      GLctx.samplerParameterf(GL.samplers[sampler], pname, param);
    };
  var _emscripten_glSamplerParameterfv = _glSamplerParameterfv;

  /** @suppress {duplicate } */
  var _glSamplerParameteri = (sampler, pname, param) => {
      GLctx.samplerParameteri(GL.samplers[sampler], pname, param);
    };
  var _emscripten_glSamplerParameteri = _glSamplerParameteri;

  /** @suppress {duplicate } */
  var _glSamplerParameteriv = (sampler, pname, params) => {
      var param = HEAP32[((params)>>2)];
      GLctx.samplerParameteri(GL.samplers[sampler], pname, param);
    };
  var _emscripten_glSamplerParameteriv = _glSamplerParameteriv;

  /** @suppress {duplicate } */
  var _glScissor = (x0, x1, x2, x3) => GLctx.scissor(x0, x1, x2, x3);
  var _emscripten_glScissor = _glScissor;

  /** @suppress {duplicate } */
  var _glShaderBinary = (count, shaders, binaryformat, binary, length) => {
      GL.recordError(0x500/*GL_INVALID_ENUM*/);
    };
  var _emscripten_glShaderBinary = _glShaderBinary;

  /** @suppress {duplicate } */
  var _glShaderSource = (shader, count, string, length) => {
      var source = GL.getSource(shader, count, string, length);
  
      GLctx.shaderSource(GL.shaders[shader], source);
    };
  var _emscripten_glShaderSource = _glShaderSource;

  /** @suppress {duplicate } */
  var _glStencilFunc = (x0, x1, x2) => GLctx.stencilFunc(x0, x1, x2);
  var _emscripten_glStencilFunc = _glStencilFunc;

  /** @suppress {duplicate } */
  var _glStencilFuncSeparate = (x0, x1, x2, x3) => GLctx.stencilFuncSeparate(x0, x1, x2, x3);
  var _emscripten_glStencilFuncSeparate = _glStencilFuncSeparate;

  /** @suppress {duplicate } */
  var _glStencilMask = (x0) => GLctx.stencilMask(x0);
  var _emscripten_glStencilMask = _glStencilMask;

  /** @suppress {duplicate } */
  var _glStencilMaskSeparate = (x0, x1) => GLctx.stencilMaskSeparate(x0, x1);
  var _emscripten_glStencilMaskSeparate = _glStencilMaskSeparate;

  /** @suppress {duplicate } */
  var _glStencilOp = (x0, x1, x2) => GLctx.stencilOp(x0, x1, x2);
  var _emscripten_glStencilOp = _glStencilOp;

  /** @suppress {duplicate } */
  var _glStencilOpSeparate = (x0, x1, x2, x3) => GLctx.stencilOpSeparate(x0, x1, x2, x3);
  var _emscripten_glStencilOpSeparate = _glStencilOpSeparate;

  var computeUnpackAlignedImageSize = (width, height, sizePerPixel) => {
      function roundedToNextMultipleOf(x, y) {
        return (x + y - 1) & -y;
      }
      var plainRowSize = (GL.unpackRowLength || width) * sizePerPixel;
      var alignedRowSize = roundedToNextMultipleOf(plainRowSize, GL.unpackAlignment);
      return height * alignedRowSize;
    };
  
  var colorChannelsInGlTextureFormat = (format) => {
      // Micro-optimizations for size: map format to size by subtracting smallest
      // enum value (0x1902) from all values first.  Also omit the most common
      // size value (1) from the list, which is assumed by formats not on the
      // list.
      var colorChannels = {
        // 0x1902 /* GL_DEPTH_COMPONENT */ - 0x1902: 1,
        // 0x1906 /* GL_ALPHA */ - 0x1902: 1,
        5: 3,
        6: 4,
        // 0x1909 /* GL_LUMINANCE */ - 0x1902: 1,
        8: 2,
        29502: 3,
        29504: 4,
        // 0x1903 /* GL_RED */ - 0x1902: 1,
        26917: 2,
        26918: 2,
        // 0x8D94 /* GL_RED_INTEGER */ - 0x1902: 1,
        29846: 3,
        29847: 4
      };
      return colorChannels[format - 0x1902]||1;
    };
  
  
  
  var emscriptenWebGLGetTexPixelData = (type, format, width, height, pixels, internalFormat) => {
      var heap = heapObjectForWebGLType(type);
      var sizePerPixel = colorChannelsInGlTextureFormat(format) * heap.BYTES_PER_ELEMENT;
      var bytes = computeUnpackAlignedImageSize(width, height, sizePerPixel);
      return heap.subarray(toTypedArrayIndex(pixels, heap), toTypedArrayIndex(pixels + bytes, heap));
    };
  
  
  
  /** @suppress {duplicate } */
  var _glTexImage2D = (target, level, internalFormat, width, height, border, format, type, pixels) => {
      if (true) {
        if (GLctx.currentPixelUnpackBufferBinding) {
          GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
          return;
        }
        if (pixels) {
          var heap = heapObjectForWebGLType(type);
          var index = toTypedArrayIndex(pixels, heap);
          GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, heap, index);
          return;
        }
      }
      var pixelData = pixels ? emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, internalFormat) : null;
      GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, pixelData);
    };
  var _emscripten_glTexImage2D = _glTexImage2D;

  
  /** @suppress {duplicate } */
  var _glTexImage3D = (target, level, internalFormat, width, height, depth, border, format, type, pixels) => {
      if (GLctx.currentPixelUnpackBufferBinding) {
        GLctx.texImage3D(target, level, internalFormat, width, height, depth, border, format, type, pixels);
      } else if (pixels) {
        var heap = heapObjectForWebGLType(type);
        GLctx.texImage3D(target, level, internalFormat, width, height, depth, border, format, type, heap, toTypedArrayIndex(pixels, heap));
      } else {
        GLctx.texImage3D(target, level, internalFormat, width, height, depth, border, format, type, null);
      }
    };
  var _emscripten_glTexImage3D = _glTexImage3D;

  /** @suppress {duplicate } */
  var _glTexParameterf = (x0, x1, x2) => GLctx.texParameterf(x0, x1, x2);
  var _emscripten_glTexParameterf = _glTexParameterf;

  /** @suppress {duplicate } */
  var _glTexParameterfv = (target, pname, params) => {
      var param = HEAPF32[((params)>>2)];
      GLctx.texParameterf(target, pname, param);
    };
  var _emscripten_glTexParameterfv = _glTexParameterfv;

  /** @suppress {duplicate } */
  var _glTexParameteri = (x0, x1, x2) => GLctx.texParameteri(x0, x1, x2);
  var _emscripten_glTexParameteri = _glTexParameteri;

  /** @suppress {duplicate } */
  var _glTexParameteriv = (target, pname, params) => {
      var param = HEAP32[((params)>>2)];
      GLctx.texParameteri(target, pname, param);
    };
  var _emscripten_glTexParameteriv = _glTexParameteriv;

  /** @suppress {duplicate } */
  var _glTexStorage2D = (x0, x1, x2, x3, x4) => GLctx.texStorage2D(x0, x1, x2, x3, x4);
  var _emscripten_glTexStorage2D = _glTexStorage2D;

  /** @suppress {duplicate } */
  var _glTexStorage3D = (x0, x1, x2, x3, x4, x5) => GLctx.texStorage3D(x0, x1, x2, x3, x4, x5);
  var _emscripten_glTexStorage3D = _glTexStorage3D;

  
  
  
  /** @suppress {duplicate } */
  var _glTexSubImage2D = (target, level, xoffset, yoffset, width, height, format, type, pixels) => {
      if (true) {
        if (GLctx.currentPixelUnpackBufferBinding) {
          GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
          return;
        }
        if (pixels) {
          var heap = heapObjectForWebGLType(type);
          GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, heap, toTypedArrayIndex(pixels, heap));
          return;
        }
      }
      var pixelData = pixels ? emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, 0) : null;
      GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixelData);
    };
  var _emscripten_glTexSubImage2D = _glTexSubImage2D;

  
  /** @suppress {duplicate } */
  var _glTexSubImage3D = (target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels) => {
      if (GLctx.currentPixelUnpackBufferBinding) {
        GLctx.texSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
      } else if (pixels) {
        var heap = heapObjectForWebGLType(type);
        GLctx.texSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, heap, toTypedArrayIndex(pixels, heap));
      } else {
        GLctx.texSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, null);
      }
    };
  var _emscripten_glTexSubImage3D = _glTexSubImage3D;

  /** @suppress {duplicate } */
  var _glTransformFeedbackVaryings = (program, count, varyings, bufferMode) => {
      program = GL.programs[program];
      var vars = [];
      for (var i = 0; i < count; i++)
        vars.push(UTF8ToString(HEAP32[(((varyings)+(i*4))>>2)]));
  
      GLctx.transformFeedbackVaryings(program, vars, bufferMode);
    };
  var _emscripten_glTransformFeedbackVaryings = _glTransformFeedbackVaryings;

  
  /** @suppress {duplicate } */
  var _glUniform1f = (location, v0) => {
      GLctx.uniform1f(webglGetUniformLocation(location), v0);
    };
  var _emscripten_glUniform1f = _glUniform1f;

  
  /** @suppress {duplicate } */
  var _glUniform1fv = (location, count, value) => {
  
      count && GLctx.uniform1fv(webglGetUniformLocation(location), HEAPF32, ((value)>>2), count);
    };
  var _emscripten_glUniform1fv = _glUniform1fv;

  
  /** @suppress {duplicate } */
  var _glUniform1i = (location, v0) => {
      GLctx.uniform1i(webglGetUniformLocation(location), v0);
    };
  var _emscripten_glUniform1i = _glUniform1i;

  
  /** @suppress {duplicate } */
  var _glUniform1iv = (location, count, value) => {
  
      count && GLctx.uniform1iv(webglGetUniformLocation(location), HEAP32, ((value)>>2), count);
    };
  var _emscripten_glUniform1iv = _glUniform1iv;

  /** @suppress {duplicate } */
  var _glUniform1ui = (location, v0) => {
      GLctx.uniform1ui(webglGetUniformLocation(location), v0);
    };
  var _emscripten_glUniform1ui = _glUniform1ui;

  /** @suppress {duplicate } */
  var _glUniform1uiv = (location, count, value) => {
      count && GLctx.uniform1uiv(webglGetUniformLocation(location), HEAPU32, ((value)>>2), count);
    };
  var _emscripten_glUniform1uiv = _glUniform1uiv;

  
  /** @suppress {duplicate } */
  var _glUniform2f = (location, v0, v1) => {
      GLctx.uniform2f(webglGetUniformLocation(location), v0, v1);
    };
  var _emscripten_glUniform2f = _glUniform2f;

  
  /** @suppress {duplicate } */
  var _glUniform2fv = (location, count, value) => {
  
      count && GLctx.uniform2fv(webglGetUniformLocation(location), HEAPF32, ((value)>>2), count*2);
    };
  var _emscripten_glUniform2fv = _glUniform2fv;

  
  /** @suppress {duplicate } */
  var _glUniform2i = (location, v0, v1) => {
      GLctx.uniform2i(webglGetUniformLocation(location), v0, v1);
    };
  var _emscripten_glUniform2i = _glUniform2i;

  
  /** @suppress {duplicate } */
  var _glUniform2iv = (location, count, value) => {
  
      count && GLctx.uniform2iv(webglGetUniformLocation(location), HEAP32, ((value)>>2), count*2);
    };
  var _emscripten_glUniform2iv = _glUniform2iv;

  /** @suppress {duplicate } */
  var _glUniform2ui = (location, v0, v1) => {
      GLctx.uniform2ui(webglGetUniformLocation(location), v0, v1);
    };
  var _emscripten_glUniform2ui = _glUniform2ui;

  /** @suppress {duplicate } */
  var _glUniform2uiv = (location, count, value) => {
      count && GLctx.uniform2uiv(webglGetUniformLocation(location), HEAPU32, ((value)>>2), count*2);
    };
  var _emscripten_glUniform2uiv = _glUniform2uiv;

  
  /** @suppress {duplicate } */
  var _glUniform3f = (location, v0, v1, v2) => {
      GLctx.uniform3f(webglGetUniformLocation(location), v0, v1, v2);
    };
  var _emscripten_glUniform3f = _glUniform3f;

  
  /** @suppress {duplicate } */
  var _glUniform3fv = (location, count, value) => {
  
      count && GLctx.uniform3fv(webglGetUniformLocation(location), HEAPF32, ((value)>>2), count*3);
    };
  var _emscripten_glUniform3fv = _glUniform3fv;

  
  /** @suppress {duplicate } */
  var _glUniform3i = (location, v0, v1, v2) => {
      GLctx.uniform3i(webglGetUniformLocation(location), v0, v1, v2);
    };
  var _emscripten_glUniform3i = _glUniform3i;

  
  /** @suppress {duplicate } */
  var _glUniform3iv = (location, count, value) => {
  
      count && GLctx.uniform3iv(webglGetUniformLocation(location), HEAP32, ((value)>>2), count*3);
    };
  var _emscripten_glUniform3iv = _glUniform3iv;

  /** @suppress {duplicate } */
  var _glUniform3ui = (location, v0, v1, v2) => {
      GLctx.uniform3ui(webglGetUniformLocation(location), v0, v1, v2);
    };
  var _emscripten_glUniform3ui = _glUniform3ui;

  /** @suppress {duplicate } */
  var _glUniform3uiv = (location, count, value) => {
      count && GLctx.uniform3uiv(webglGetUniformLocation(location), HEAPU32, ((value)>>2), count*3);
    };
  var _emscripten_glUniform3uiv = _glUniform3uiv;

  
  /** @suppress {duplicate } */
  var _glUniform4f = (location, v0, v1, v2, v3) => {
      GLctx.uniform4f(webglGetUniformLocation(location), v0, v1, v2, v3);
    };
  var _emscripten_glUniform4f = _glUniform4f;

  
  /** @suppress {duplicate } */
  var _glUniform4fv = (location, count, value) => {
  
      count && GLctx.uniform4fv(webglGetUniformLocation(location), HEAPF32, ((value)>>2), count*4);
    };
  var _emscripten_glUniform4fv = _glUniform4fv;

  
  /** @suppress {duplicate } */
  var _glUniform4i = (location, v0, v1, v2, v3) => {
      GLctx.uniform4i(webglGetUniformLocation(location), v0, v1, v2, v3);
    };
  var _emscripten_glUniform4i = _glUniform4i;

  
  /** @suppress {duplicate } */
  var _glUniform4iv = (location, count, value) => {
  
      count && GLctx.uniform4iv(webglGetUniformLocation(location), HEAP32, ((value)>>2), count*4);
    };
  var _emscripten_glUniform4iv = _glUniform4iv;

  /** @suppress {duplicate } */
  var _glUniform4ui = (location, v0, v1, v2, v3) => {
      GLctx.uniform4ui(webglGetUniformLocation(location), v0, v1, v2, v3);
    };
  var _emscripten_glUniform4ui = _glUniform4ui;

  /** @suppress {duplicate } */
  var _glUniform4uiv = (location, count, value) => {
      count && GLctx.uniform4uiv(webglGetUniformLocation(location), HEAPU32, ((value)>>2), count*4);
    };
  var _emscripten_glUniform4uiv = _glUniform4uiv;

  /** @suppress {duplicate } */
  var _glUniformBlockBinding = (program, uniformBlockIndex, uniformBlockBinding) => {
      program = GL.programs[program];
  
      GLctx.uniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
    };
  var _emscripten_glUniformBlockBinding = _glUniformBlockBinding;

  
  /** @suppress {duplicate } */
  var _glUniformMatrix2fv = (location, count, transpose, value) => {
  
      count && GLctx.uniformMatrix2fv(webglGetUniformLocation(location), !!transpose, HEAPF32, ((value)>>2), count*4);
    };
  var _emscripten_glUniformMatrix2fv = _glUniformMatrix2fv;

  /** @suppress {duplicate } */
  var _glUniformMatrix2x3fv = (location, count, transpose, value) => {
      count && GLctx.uniformMatrix2x3fv(webglGetUniformLocation(location), !!transpose, HEAPF32, ((value)>>2), count*6);
    };
  var _emscripten_glUniformMatrix2x3fv = _glUniformMatrix2x3fv;

  /** @suppress {duplicate } */
  var _glUniformMatrix2x4fv = (location, count, transpose, value) => {
      count && GLctx.uniformMatrix2x4fv(webglGetUniformLocation(location), !!transpose, HEAPF32, ((value)>>2), count*8);
    };
  var _emscripten_glUniformMatrix2x4fv = _glUniformMatrix2x4fv;

  
  /** @suppress {duplicate } */
  var _glUniformMatrix3fv = (location, count, transpose, value) => {
  
      count && GLctx.uniformMatrix3fv(webglGetUniformLocation(location), !!transpose, HEAPF32, ((value)>>2), count*9);
    };
  var _emscripten_glUniformMatrix3fv = _glUniformMatrix3fv;

  /** @suppress {duplicate } */
  var _glUniformMatrix3x2fv = (location, count, transpose, value) => {
      count && GLctx.uniformMatrix3x2fv(webglGetUniformLocation(location), !!transpose, HEAPF32, ((value)>>2), count*6);
    };
  var _emscripten_glUniformMatrix3x2fv = _glUniformMatrix3x2fv;

  /** @suppress {duplicate } */
  var _glUniformMatrix3x4fv = (location, count, transpose, value) => {
      count && GLctx.uniformMatrix3x4fv(webglGetUniformLocation(location), !!transpose, HEAPF32, ((value)>>2), count*12);
    };
  var _emscripten_glUniformMatrix3x4fv = _glUniformMatrix3x4fv;

  
  /** @suppress {duplicate } */
  var _glUniformMatrix4fv = (location, count, transpose, value) => {
  
      count && GLctx.uniformMatrix4fv(webglGetUniformLocation(location), !!transpose, HEAPF32, ((value)>>2), count*16);
    };
  var _emscripten_glUniformMatrix4fv = _glUniformMatrix4fv;

  /** @suppress {duplicate } */
  var _glUniformMatrix4x2fv = (location, count, transpose, value) => {
      count && GLctx.uniformMatrix4x2fv(webglGetUniformLocation(location), !!transpose, HEAPF32, ((value)>>2), count*8);
    };
  var _emscripten_glUniformMatrix4x2fv = _glUniformMatrix4x2fv;

  /** @suppress {duplicate } */
  var _glUniformMatrix4x3fv = (location, count, transpose, value) => {
      count && GLctx.uniformMatrix4x3fv(webglGetUniformLocation(location), !!transpose, HEAPF32, ((value)>>2), count*12);
    };
  var _emscripten_glUniformMatrix4x3fv = _glUniformMatrix4x3fv;

  /** @suppress {duplicate } */
  var _glUseProgram = (program) => {
      program = GL.programs[program];
      GLctx.useProgram(program);
      // Record the currently active program so that we can access the uniform
      // mapping table of that program.
      GLctx.currentProgram = program;
    };
  var _emscripten_glUseProgram = _glUseProgram;

  /** @suppress {duplicate } */
  var _glValidateProgram = (program) => {
      GLctx.validateProgram(GL.programs[program]);
    };
  var _emscripten_glValidateProgram = _glValidateProgram;

  /** @suppress {duplicate } */
  var _glVertexAttrib1f = (x0, x1) => GLctx.vertexAttrib1f(x0, x1);
  var _emscripten_glVertexAttrib1f = _glVertexAttrib1f;

  /** @suppress {duplicate } */
  var _glVertexAttrib1fv = (index, v) => {
  
      GLctx.vertexAttrib1f(index, HEAPF32[v>>2]);
    };
  var _emscripten_glVertexAttrib1fv = _glVertexAttrib1fv;

  /** @suppress {duplicate } */
  var _glVertexAttrib2f = (x0, x1, x2) => GLctx.vertexAttrib2f(x0, x1, x2);
  var _emscripten_glVertexAttrib2f = _glVertexAttrib2f;

  /** @suppress {duplicate } */
  var _glVertexAttrib2fv = (index, v) => {
  
      GLctx.vertexAttrib2f(index, HEAPF32[v>>2], HEAPF32[v+4>>2]);
    };
  var _emscripten_glVertexAttrib2fv = _glVertexAttrib2fv;

  /** @suppress {duplicate } */
  var _glVertexAttrib3f = (x0, x1, x2, x3) => GLctx.vertexAttrib3f(x0, x1, x2, x3);
  var _emscripten_glVertexAttrib3f = _glVertexAttrib3f;

  /** @suppress {duplicate } */
  var _glVertexAttrib3fv = (index, v) => {
  
      GLctx.vertexAttrib3f(index, HEAPF32[v>>2], HEAPF32[v+4>>2], HEAPF32[v+8>>2]);
    };
  var _emscripten_glVertexAttrib3fv = _glVertexAttrib3fv;

  /** @suppress {duplicate } */
  var _glVertexAttrib4f = (x0, x1, x2, x3, x4) => GLctx.vertexAttrib4f(x0, x1, x2, x3, x4);
  var _emscripten_glVertexAttrib4f = _glVertexAttrib4f;

  /** @suppress {duplicate } */
  var _glVertexAttrib4fv = (index, v) => {
  
      GLctx.vertexAttrib4f(index, HEAPF32[v>>2], HEAPF32[v+4>>2], HEAPF32[v+8>>2], HEAPF32[v+12>>2]);
    };
  var _emscripten_glVertexAttrib4fv = _glVertexAttrib4fv;

  /** @suppress {duplicate } */
  var _glVertexAttribDivisor = (index, divisor) => {
      GLctx.vertexAttribDivisor(index, divisor);
    };
  var _emscripten_glVertexAttribDivisor = _glVertexAttribDivisor;

  
  /** @suppress {duplicate } */
  var _glVertexAttribDivisorANGLE = _glVertexAttribDivisor;
  var _emscripten_glVertexAttribDivisorANGLE = _glVertexAttribDivisorANGLE;

  
  /** @suppress {duplicate } */
  var _glVertexAttribDivisorARB = _glVertexAttribDivisor;
  var _emscripten_glVertexAttribDivisorARB = _glVertexAttribDivisorARB;

  
  /** @suppress {duplicate } */
  var _glVertexAttribDivisorEXT = _glVertexAttribDivisor;
  var _emscripten_glVertexAttribDivisorEXT = _glVertexAttribDivisorEXT;

  
  /** @suppress {duplicate } */
  var _glVertexAttribDivisorNV = _glVertexAttribDivisor;
  var _emscripten_glVertexAttribDivisorNV = _glVertexAttribDivisorNV;

  /** @suppress {duplicate } */
  var _glVertexAttribI4i = (x0, x1, x2, x3, x4) => GLctx.vertexAttribI4i(x0, x1, x2, x3, x4);
  var _emscripten_glVertexAttribI4i = _glVertexAttribI4i;

  /** @suppress {duplicate } */
  var _glVertexAttribI4iv = (index, v) => {
      GLctx.vertexAttribI4i(index, HEAP32[v>>2], HEAP32[v+4>>2], HEAP32[v+8>>2], HEAP32[v+12>>2]);
    };
  var _emscripten_glVertexAttribI4iv = _glVertexAttribI4iv;

  /** @suppress {duplicate } */
  var _glVertexAttribI4ui = (x0, x1, x2, x3, x4) => GLctx.vertexAttribI4ui(x0, x1, x2, x3, x4);
  var _emscripten_glVertexAttribI4ui = _glVertexAttribI4ui;

  /** @suppress {duplicate } */
  var _glVertexAttribI4uiv = (index, v) => {
      GLctx.vertexAttribI4ui(index, HEAPU32[v>>2], HEAPU32[v+4>>2], HEAPU32[v+8>>2], HEAPU32[v+12>>2]);
    };
  var _emscripten_glVertexAttribI4uiv = _glVertexAttribI4uiv;

  /** @suppress {duplicate } */
  var _glVertexAttribIPointer = (index, size, type, stride, ptr) => {
      GLctx.vertexAttribIPointer(index, size, type, stride, ptr);
    };
  var _emscripten_glVertexAttribIPointer = _glVertexAttribIPointer;

  /** @suppress {duplicate } */
  var _glVertexAttribPointer = (index, size, type, normalized, stride, ptr) => {
      GLctx.vertexAttribPointer(index, size, type, !!normalized, stride, ptr);
    };
  var _emscripten_glVertexAttribPointer = _glVertexAttribPointer;

  /** @suppress {duplicate } */
  var _glViewport = (x0, x1, x2, x3) => GLctx.viewport(x0, x1, x2, x3);
  var _emscripten_glViewport = _glViewport;

  /** @suppress {duplicate } */
  var _glWaitSync = (sync, flags, timeout) => {
      // See WebGL2 vs GLES3 difference on GL_TIMEOUT_IGNORED above (https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.15)
      timeout = Number(timeout);
      GLctx.waitSync(GL.syncs[sync], flags, timeout);
    };
  var _emscripten_glWaitSync = _glWaitSync;

  var _emscripten_has_asyncify = () => 0;

  
  
  var doRequestFullscreen = (target, strategy) => {
      if (!JSEvents.fullscreenEnabled()) return -1;
      target = findEventTarget(target);
      if (!target) return -4;
  
      if (!target.requestFullscreen
        && !target.webkitRequestFullscreen
        ) {
        return -3;
      }
  
      // Queue this function call if we're not currently in an event handler and
      // the user saw it appropriate to do so.
      if (!JSEvents.canPerformEventHandlerRequests()) {
        if (strategy.deferUntilInEventHandler) {
          JSEvents.deferCall(JSEvents_requestFullscreen, 1 /* priority over pointer lock */, [target, strategy]);
          return 1;
        }
        return -2;
      }
  
      return JSEvents_requestFullscreen(target, strategy);
    };
  var _emscripten_request_fullscreen_strategy = (target, deferUntilInEventHandler, fullscreenStrategy) => {
      var strategy = {
        scaleMode: HEAP32[((fullscreenStrategy)>>2)],
        canvasResolutionScaleMode: HEAP32[(((fullscreenStrategy)+(4))>>2)],
        filteringMode: HEAP32[(((fullscreenStrategy)+(8))>>2)],
        deferUntilInEventHandler,
        canvasResizedCallback: HEAP32[(((fullscreenStrategy)+(12))>>2)],
        canvasResizedCallbackUserData: HEAP32[(((fullscreenStrategy)+(16))>>2)]
      };
  
      return doRequestFullscreen(target, strategy);
    };

  
  
  var _emscripten_request_pointerlock = (target, deferUntilInEventHandler) => {
      target = findEventTarget(target);
      if (!target) return -4;
      if (!target.requestPointerLock
        ) {
        return -1;
      }
  
      // Queue this function call if we're not currently in an event handler and
      // the user saw it appropriate to do so.
      if (!JSEvents.canPerformEventHandlerRequests()) {
        if (deferUntilInEventHandler) {
          JSEvents.deferCall(requestPointerLock, 2 /* priority below fullscreen */, [target]);
          return 1;
        }
        return -2;
      }
  
      return requestPointerLock(target);
    };

  
  
  var growMemory = (size) => {
      var b = wasmMemory.buffer;
      var pages = ((size - b.byteLength + 65535) / 65536) | 0;
      try {
        // round size grow request up to wasm page size (fixed 64KB per spec)
        wasmMemory.grow(pages); // .grow() takes a delta compared to the previous size
        updateMemoryViews();
        return 1 /*success*/;
      } catch(e) {
        err(`growMemory: Attempted to grow heap from ${b.byteLength} bytes to ${size} bytes, but got error: ${e}`);
      }
      // implicit 0 return to save code size (caller will cast "undefined" into 0
      // anyhow)
    };
  var _emscripten_resize_heap = (requestedSize) => {
      var oldSize = HEAPU8.length;
      // With CAN_ADDRESS_2GB or MEMORY64, pointers are already unsigned.
      requestedSize >>>= 0;
      // With multithreaded builds, races can happen (another thread might increase the size
      // in between), so return a failure, and let the caller retry.
      assert(requestedSize > oldSize);
  
      // Memory resize rules:
      // 1.  Always increase heap size to at least the requested size, rounded up
      //     to next page multiple.
      // 2a. If MEMORY_GROWTH_LINEAR_STEP == -1, excessively resize the heap
      //     geometrically: increase the heap size according to
      //     MEMORY_GROWTH_GEOMETRIC_STEP factor (default +20%), At most
      //     overreserve by MEMORY_GROWTH_GEOMETRIC_CAP bytes (default 96MB).
      // 2b. If MEMORY_GROWTH_LINEAR_STEP != -1, excessively resize the heap
      //     linearly: increase the heap size by at least
      //     MEMORY_GROWTH_LINEAR_STEP bytes.
      // 3.  Max size for the heap is capped at 2048MB-WASM_PAGE_SIZE, or by
      //     MAXIMUM_MEMORY, or by ASAN limit, depending on which is smallest
      // 4.  If we were unable to allocate as much memory, it may be due to
      //     over-eager decision to excessively reserve due to (3) above.
      //     Hence if an allocation fails, cut down on the amount of excess
      //     growth, in an attempt to succeed to perform a smaller allocation.
  
      // A limit is set for how much we can grow. We should not exceed that
      // (the wasm binary specifies it, so if we tried, we'd fail anyhow).
      var maxHeapSize = getHeapMax();
      if (requestedSize > maxHeapSize) {
        err(`Cannot enlarge memory, requested ${requestedSize} bytes, but the limit is ${maxHeapSize} bytes!`);
        return false;
      }
  
      // Loop through potential heap size increases. If we attempt a too eager
      // reservation that fails, cut down on the attempted size and reserve a
      // smaller bump instead. (max 3 times, chosen somewhat arbitrarily)
      for (var cutDown = 1; cutDown <= 4; cutDown *= 2) {
        var overGrownHeapSize = oldSize * (1 + 0.2 / cutDown); // ensure geometric growth
        // but limit overreserving (default to capping at +96MB overgrowth at most)
        overGrownHeapSize = Math.min(overGrownHeapSize, requestedSize + 100663296 );
  
        var newSize = Math.min(maxHeapSize, alignMemory(Math.max(requestedSize, overGrownHeapSize), 65536));
  
        var replacement = growMemory(newSize);
        if (replacement) {
  
          return true;
        }
      }
      err(`Failed to grow the heap from ${oldSize} bytes to ${newSize} bytes, not enough memory!`);
      return false;
    };

  /** @suppress {checkTypes} */
  var _emscripten_sample_gamepad_data = () => {
      try {
        if (navigator.getGamepads) return (JSEvents.lastGamepadState = navigator.getGamepads())
          ? 0 : -1;
      } catch(e) {
        err(`navigator.getGamepads() exists, but failed to execute with exception ${e}. Disabling Gamepad access.`);
        navigator.getGamepads = null; // Disable getGamepads() so that it won't be attempted to be used again.
      }
      return -1;
    };

  
  
  
  var registerBeforeUnloadEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString) => {
      var beforeUnloadEventHandlerFunc = (e = event) => {
        // Note: This is always called on the main browser thread, since it needs synchronously return a value!
        var confirmationMessage = getWasmTableEntry(callbackfunc)(eventTypeId, 0, userData);
  
        if (confirmationMessage) {
          confirmationMessage = UTF8ToString(confirmationMessage);
        }
        if (confirmationMessage) {
          e.preventDefault();
          e.returnValue = confirmationMessage;
          return confirmationMessage;
        }
      };
  
      var eventHandler = {
        target: findEventTarget(target),
        eventTypeString,
        callbackfunc,
        handlerFunc: beforeUnloadEventHandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  var _emscripten_set_beforeunload_callback_on_thread = (userData, callbackfunc, targetThread) => {
      if (typeof onbeforeunload == 'undefined') return -1;
      // beforeunload callback can only be registered on the main browser thread, because the page will go away immediately after returning from the handler,
      // and there is no time to start proxying it anywhere.
      if (targetThread !== 1) return -5;
      return registerBeforeUnloadEventCallback(2, userData, true, callbackfunc, 28, "beforeunload");
    };

  
  
  
  
  var registerFocusEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
      JSEvents.focusEvent ||= _malloc(256);
  
      var focusEventHandlerFunc = (e = event) => {
        var nodeName = JSEvents.getNodeNameForTarget(e.target);
        var id = e.target.id ? e.target.id : '';
  
        var focusEvent = JSEvents.focusEvent;
        stringToUTF8(nodeName, focusEvent + 0, 128);
        stringToUTF8(id, focusEvent + 128, 128);
  
        if (getWasmTableEntry(callbackfunc)(eventTypeId, focusEvent, userData)) e.preventDefault();
      };
  
      var eventHandler = {
        target: findEventTarget(target),
        eventTypeString,
        callbackfunc,
        handlerFunc: focusEventHandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  var _emscripten_set_blur_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerFocusEventCallback(target, userData, useCapture, callbackfunc, 12, "blur", targetThread);


  var _emscripten_set_element_css_size = (target, width, height) => {
      target = findEventTarget(target);
      if (!target) return -4;
  
      target.style.width = width + "px";
      target.style.height = height + "px";
  
      return 0;
    };

  var _emscripten_set_focus_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerFocusEventCallback(target, userData, useCapture, callbackfunc, 13, "focus", targetThread);

  
  
  
  var fillFullscreenChangeEventData = (eventStruct) => {
      var fullscreenElement = document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement;
      var isFullscreen = !!fullscreenElement;
      // Assigning a boolean to HEAP32 with expected type coercion.
      /** @suppress{checkTypes} */
      HEAP8[eventStruct] = isFullscreen;
      HEAP8[(eventStruct)+(1)] = JSEvents.fullscreenEnabled();
      // If transitioning to fullscreen, report info about the element that is now fullscreen.
      // If transitioning to windowed mode, report info about the element that just was fullscreen.
      var reportedElement = isFullscreen ? fullscreenElement : JSEvents.previousFullscreenElement;
      var nodeName = JSEvents.getNodeNameForTarget(reportedElement);
      var id = reportedElement?.id || '';
      stringToUTF8(nodeName, eventStruct + 2, 128);
      stringToUTF8(id, eventStruct + 130, 128);
      HEAP32[(((eventStruct)+(260))>>2)] = reportedElement ? reportedElement.clientWidth : 0;
      HEAP32[(((eventStruct)+(264))>>2)] = reportedElement ? reportedElement.clientHeight : 0;
      HEAP32[(((eventStruct)+(268))>>2)] = screen.width;
      HEAP32[(((eventStruct)+(272))>>2)] = screen.height;
      if (isFullscreen) {
        JSEvents.previousFullscreenElement = fullscreenElement;
      }
    };
  
  
  var registerFullscreenChangeEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
      JSEvents.fullscreenChangeEvent ||= _malloc(276);
  
      var fullscreenChangeEventhandlerFunc = (e = event) => {
        var fullscreenChangeEvent = JSEvents.fullscreenChangeEvent;
  
        fillFullscreenChangeEventData(fullscreenChangeEvent);
  
        if (getWasmTableEntry(callbackfunc)(eventTypeId, fullscreenChangeEvent, userData)) e.preventDefault();
      };
  
      var eventHandler = {
        target,
        eventTypeString,
        callbackfunc,
        handlerFunc: fullscreenChangeEventhandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  
  var _emscripten_set_fullscreenchange_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) => {
      if (!JSEvents.fullscreenEnabled()) return -1;
      target = findEventTarget(target);
      if (!target) return -4;
  
      // Unprefixed Fullscreen API shipped in Chromium 71 (https://bugs.chromium.org/p/chromium/issues/detail?id=383813)
      // As of Safari 13.0.3 on macOS Catalina 10.15.1 still ships with prefixed webkitfullscreenchange. TODO: revisit this check once Safari ships unprefixed version.
      registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, 19, "webkitfullscreenchange", targetThread);
  
      return registerFullscreenChangeEventCallback(target, userData, useCapture, callbackfunc, 19, "fullscreenchange", targetThread);
    };

  
  
  
  
  var registerGamepadEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
      JSEvents.gamepadEvent ||= _malloc(1240);
  
      var gamepadEventHandlerFunc = (e = event) => {
        var gamepadEvent = JSEvents.gamepadEvent;
        fillGamepadEventData(gamepadEvent, e["gamepad"]);
  
        if (getWasmTableEntry(callbackfunc)(eventTypeId, gamepadEvent, userData)) e.preventDefault();
      };
  
      var eventHandler = {
        target: findEventTarget(target),
        allowsDeferredCalls: true,
        eventTypeString,
        callbackfunc,
        handlerFunc: gamepadEventHandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  
  var _emscripten_set_gamepadconnected_callback_on_thread = (userData, useCapture, callbackfunc, targetThread) => {
      if (_emscripten_sample_gamepad_data()) return -1;
      return registerGamepadEventCallback(2, userData, useCapture, callbackfunc, 26, "gamepadconnected", targetThread);
    };

  
  var _emscripten_set_gamepaddisconnected_callback_on_thread = (userData, useCapture, callbackfunc, targetThread) => {
      if (_emscripten_sample_gamepad_data()) return -1;
      return registerGamepadEventCallback(2, userData, useCapture, callbackfunc, 27, "gamepaddisconnected", targetThread);
    };

  
  
  
  
  var registerKeyEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
      JSEvents.keyEvent ||= _malloc(160);
  
      var keyEventHandlerFunc = (e) => {
        assert(e);
  
        var keyEventData = JSEvents.keyEvent;
        HEAPF64[((keyEventData)>>3)] = e.timeStamp;
  
        var idx = ((keyEventData)>>2);
  
        HEAP32[idx + 2] = e.location;
        HEAP8[keyEventData + 12] = e.ctrlKey;
        HEAP8[keyEventData + 13] = e.shiftKey;
        HEAP8[keyEventData + 14] = e.altKey;
        HEAP8[keyEventData + 15] = e.metaKey;
        HEAP8[keyEventData + 16] = e.repeat;
        HEAP32[idx + 5] = e.charCode;
        HEAP32[idx + 6] = e.keyCode;
        HEAP32[idx + 7] = e.which;
        stringToUTF8(e.key || '', keyEventData + 32, 32);
        stringToUTF8(e.code || '', keyEventData + 64, 32);
        stringToUTF8(e.char || '', keyEventData + 96, 32);
        stringToUTF8(e.locale || '', keyEventData + 128, 32);
  
        if (getWasmTableEntry(callbackfunc)(eventTypeId, keyEventData, userData)) e.preventDefault();
      };
  
      var eventHandler = {
        target: findEventTarget(target),
        eventTypeString,
        callbackfunc,
        handlerFunc: keyEventHandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  var _emscripten_set_keydown_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerKeyEventCallback(target, userData, useCapture, callbackfunc, 2, "keydown", targetThread);

  var _emscripten_set_keypress_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerKeyEventCallback(target, userData, useCapture, callbackfunc, 1, "keypress", targetThread);

  var _emscripten_set_keyup_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerKeyEventCallback(target, userData, useCapture, callbackfunc, 3, "keyup", targetThread);

  
  var _emscripten_set_main_loop = (func, fps, simulateInfiniteLoop) => {
      var iterFunc = getWasmTableEntry(func);
      setMainLoop(iterFunc, fps, simulateInfiniteLoop);
    };

  
  var fillMouseEventData = (eventStruct, e, target) => {
      assert(eventStruct % 4 == 0);
      HEAPF64[((eventStruct)>>3)] = e.timeStamp;
      var idx = ((eventStruct)>>2);
      HEAP32[idx + 2] = e.screenX;
      HEAP32[idx + 3] = e.screenY;
      HEAP32[idx + 4] = e.clientX;
      HEAP32[idx + 5] = e.clientY;
      HEAP8[eventStruct + 24] = e.ctrlKey;
      HEAP8[eventStruct + 25] = e.shiftKey;
      HEAP8[eventStruct + 26] = e.altKey;
      HEAP8[eventStruct + 27] = e.metaKey;
      HEAP16[idx*2 + 14] = e.button;
      HEAP16[idx*2 + 15] = e.buttons;
  
      HEAP32[idx + 8] = e["movementX"]
        ;
  
      HEAP32[idx + 9] = e["movementY"]
        ;
  
      // Note: rect contains doubles (truncated to placate SAFE_HEAP, which is the same behaviour when writing to HEAP32 anyway)
      var rect = getBoundingClientRect(target);
      HEAP32[idx + 10] = e.clientX - (rect.left | 0);
      HEAP32[idx + 11] = e.clientY - (rect.top  | 0);
    };
  
  
  
  var registerMouseEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
      JSEvents.mouseEvent ||= _malloc(64);
      target = findEventTarget(target);
  
      var mouseEventHandlerFunc = (e = event) => {
        // TODO: Make this access thread safe, or this could update live while app is reading it.
        fillMouseEventData(JSEvents.mouseEvent, e, target);
  
        if (getWasmTableEntry(callbackfunc)(eventTypeId, JSEvents.mouseEvent, userData)) e.preventDefault();
      };
  
      var eventHandler = {
        target,
        allowsDeferredCalls: eventTypeString != 'mousemove' && eventTypeString != 'mouseenter' && eventTypeString != 'mouseleave', // Mouse move events do not allow fullscreen/pointer lock requests to be handled in them!
        eventTypeString,
        callbackfunc,
        handlerFunc: mouseEventHandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  var _emscripten_set_mousedown_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerMouseEventCallback(target, userData, useCapture, callbackfunc, 5, "mousedown", targetThread);

  var _emscripten_set_mouseenter_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerMouseEventCallback(target, userData, useCapture, callbackfunc, 33, "mouseenter", targetThread);

  var _emscripten_set_mouseleave_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerMouseEventCallback(target, userData, useCapture, callbackfunc, 34, "mouseleave", targetThread);

  var _emscripten_set_mousemove_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerMouseEventCallback(target, userData, useCapture, callbackfunc, 8, "mousemove", targetThread);

  var _emscripten_set_mouseup_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerMouseEventCallback(target, userData, useCapture, callbackfunc, 6, "mouseup", targetThread);

  
  
  var fillPointerlockChangeEventData = (eventStruct) => {
      var pointerLockElement = document.pointerLockElement || document.mozPointerLockElement || document.webkitPointerLockElement || document.msPointerLockElement;
      var isPointerlocked = !!pointerLockElement;
      // Assigning a boolean to HEAP32 with expected type coercion.
      /** @suppress{checkTypes} */
      HEAP8[eventStruct] = isPointerlocked;
      var nodeName = JSEvents.getNodeNameForTarget(pointerLockElement);
      var id = pointerLockElement?.id || '';
      stringToUTF8(nodeName, eventStruct + 1, 128);
      stringToUTF8(id, eventStruct + 129, 128);
    };
  
  
  var registerPointerlockChangeEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
      JSEvents.pointerlockChangeEvent ||= _malloc(257);
  
      var pointerlockChangeEventHandlerFunc = (e = event) => {
        var pointerlockChangeEvent = JSEvents.pointerlockChangeEvent;
        fillPointerlockChangeEventData(pointerlockChangeEvent);
  
        if (getWasmTableEntry(callbackfunc)(eventTypeId, pointerlockChangeEvent, userData)) e.preventDefault();
      };
  
      var eventHandler = {
        target,
        eventTypeString,
        callbackfunc,
        handlerFunc: pointerlockChangeEventHandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  
  /** @suppress {missingProperties} */
  var _emscripten_set_pointerlockchange_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) => {
      // TODO: Currently not supported in pthreads or in --proxy-to-worker mode. (In pthreads mode, document object is not defined)
      if (!document || !document.body || (!document.body.requestPointerLock && !document.body.mozRequestPointerLock && !document.body.webkitRequestPointerLock && !document.body.msRequestPointerLock)) {
        return -1;
      }
  
      target = findEventTarget(target);
      if (!target) return -4;
      registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, 20, "mozpointerlockchange", targetThread);
      registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, 20, "webkitpointerlockchange", targetThread);
      registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, 20, "mspointerlockchange", targetThread);
      return registerPointerlockChangeEventCallback(target, userData, useCapture, callbackfunc, 20, "pointerlockchange", targetThread);
    };

  
  
  
  var registerUiEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
      JSEvents.uiEvent ||= _malloc(36);
  
      target = findEventTarget(target);
  
      var uiEventHandlerFunc = (e = event) => {
        if (e.target != target) {
          // Never take ui events such as scroll via a 'bubbled' route, but always from the direct element that
          // was targeted. Otherwise e.g. if app logs a message in response to a page scroll, the Emscripten log
          // message box could cause to scroll, generating a new (bubbled) scroll message, causing a new log print,
          // causing a new scroll, etc..
          return;
        }
        var b = document.body; // Take document.body to a variable, Closure compiler does not outline access to it on its own.
        if (!b) {
          // During a page unload 'body' can be null, with "Cannot read property 'clientWidth' of null" being thrown
          return;
        }
        var uiEvent = JSEvents.uiEvent;
        HEAP32[((uiEvent)>>2)] = 0; // always zero for resize and scroll
        HEAP32[(((uiEvent)+(4))>>2)] = b.clientWidth;
        HEAP32[(((uiEvent)+(8))>>2)] = b.clientHeight;
        HEAP32[(((uiEvent)+(12))>>2)] = innerWidth;
        HEAP32[(((uiEvent)+(16))>>2)] = innerHeight;
        HEAP32[(((uiEvent)+(20))>>2)] = outerWidth;
        HEAP32[(((uiEvent)+(24))>>2)] = outerHeight;
        HEAP32[(((uiEvent)+(28))>>2)] = pageXOffset | 0; // scroll offsets are float
        HEAP32[(((uiEvent)+(32))>>2)] = pageYOffset | 0;
        if (getWasmTableEntry(callbackfunc)(eventTypeId, uiEvent, userData)) e.preventDefault();
      };
  
      var eventHandler = {
        target,
        eventTypeString,
        callbackfunc,
        handlerFunc: uiEventHandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  var _emscripten_set_resize_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerUiEventCallback(target, userData, useCapture, callbackfunc, 10, "resize", targetThread);

  
  
  
  
  var registerTouchEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
      JSEvents.touchEvent ||= _malloc(1552);
  
      target = findEventTarget(target);
  
      var touchEventHandlerFunc = (e) => {
        assert(e);
        var t, touches = {}, et = e.touches;
        // To ease marshalling different kinds of touches that browser reports (all touches are listed in e.touches,
        // only changed touches in e.changedTouches, and touches on target at a.targetTouches), mark a boolean in
        // each Touch object so that we can later loop only once over all touches we see to marshall over to Wasm.
  
        for (let t of et) {
          // Browser might recycle the generated Touch objects between each frame (Firefox on Android), so reset any
          // changed/target states we may have set from previous frame.
          t.isChanged = t.onTarget = 0;
          touches[t.identifier] = t;
        }
        // Mark which touches are part of the changedTouches list.
        for (let t of e.changedTouches) {
          t.isChanged = 1;
          touches[t.identifier] = t;
        }
        // Mark which touches are part of the targetTouches list.
        for (let t of e.targetTouches) {
          touches[t.identifier].onTarget = 1;
        }
  
        var touchEvent = JSEvents.touchEvent;
        HEAPF64[((touchEvent)>>3)] = e.timeStamp;
        HEAP8[touchEvent + 12] = e.ctrlKey;
        HEAP8[touchEvent + 13] = e.shiftKey;
        HEAP8[touchEvent + 14] = e.altKey;
        HEAP8[touchEvent + 15] = e.metaKey;
        var idx = touchEvent + 16;
        var targetRect = getBoundingClientRect(target);
        var numTouches = 0;
        for (let t of Object.values(touches)) {
          var idx32 = ((idx)>>2); // Pre-shift the ptr to index to HEAP32 to save code size
          HEAP32[idx32 + 0] = t.identifier;
          HEAP32[idx32 + 1] = t.screenX;
          HEAP32[idx32 + 2] = t.screenY;
          HEAP32[idx32 + 3] = t.clientX;
          HEAP32[idx32 + 4] = t.clientY;
          HEAP32[idx32 + 5] = t.pageX;
          HEAP32[idx32 + 6] = t.pageY;
          HEAP8[idx + 28] = t.isChanged;
          HEAP8[idx + 29] = t.onTarget;
          HEAP32[idx32 + 8] = t.clientX - (targetRect.left | 0);
          HEAP32[idx32 + 9] = t.clientY - (targetRect.top  | 0);
  
          idx += 48;
  
          if (++numTouches > 31) {
            break;
          }
        }
        HEAP32[(((touchEvent)+(8))>>2)] = numTouches;
  
        if (getWasmTableEntry(callbackfunc)(eventTypeId, touchEvent, userData)) e.preventDefault();
      };
  
      var eventHandler = {
        target,
        allowsDeferredCalls: eventTypeString == 'touchstart' || eventTypeString == 'touchend',
        eventTypeString,
        callbackfunc,
        handlerFunc: touchEventHandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  var _emscripten_set_touchcancel_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerTouchEventCallback(target, userData, useCapture, callbackfunc, 25, "touchcancel", targetThread);

  var _emscripten_set_touchend_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerTouchEventCallback(target, userData, useCapture, callbackfunc, 23, "touchend", targetThread);

  var _emscripten_set_touchmove_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerTouchEventCallback(target, userData, useCapture, callbackfunc, 24, "touchmove", targetThread);

  var _emscripten_set_touchstart_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) =>
      registerTouchEventCallback(target, userData, useCapture, callbackfunc, 22, "touchstart", targetThread);

  
  var fillVisibilityChangeEventData = (eventStruct) => {
      var visibilityStates = [ "hidden", "visible", "prerender", "unloaded" ];
      var visibilityState = visibilityStates.indexOf(document.visibilityState);
  
      // Assigning a boolean to HEAP32 with expected type coercion.
      /** @suppress{checkTypes} */
      HEAP8[eventStruct] = document.hidden;
      HEAP32[(((eventStruct)+(4))>>2)] = visibilityState;
    };
  
  
  var registerVisibilityChangeEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
      JSEvents.visibilityChangeEvent ||= _malloc(8);
  
      var visibilityChangeEventHandlerFunc = (e = event) => {
        var visibilityChangeEvent = JSEvents.visibilityChangeEvent;
  
        fillVisibilityChangeEventData(visibilityChangeEvent);
  
        if (getWasmTableEntry(callbackfunc)(eventTypeId, visibilityChangeEvent, userData)) e.preventDefault();
      };
  
      var eventHandler = {
        target,
        eventTypeString,
        callbackfunc,
        handlerFunc: visibilityChangeEventHandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  
  var _emscripten_set_visibilitychange_callback_on_thread = (userData, useCapture, callbackfunc, targetThread) => {
    if (!specialHTMLTargets[1]) {
      return -4;
    }
      return registerVisibilityChangeEventCallback(specialHTMLTargets[1], userData, useCapture, callbackfunc, 21, "visibilitychange", targetThread);
    };

  
  
  
  var registerWheelEventCallback = (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
      JSEvents.wheelEvent ||= _malloc(96);
  
      // The DOM Level 3 events spec event 'wheel'
      var wheelHandlerFunc = (e = event) => {
        var wheelEvent = JSEvents.wheelEvent;
        fillMouseEventData(wheelEvent, e, target);
        HEAPF64[(((wheelEvent)+(64))>>3)] = e["deltaX"];
        HEAPF64[(((wheelEvent)+(72))>>3)] = e["deltaY"];
        HEAPF64[(((wheelEvent)+(80))>>3)] = e["deltaZ"];
        HEAP32[(((wheelEvent)+(88))>>2)] = e["deltaMode"];
        if (getWasmTableEntry(callbackfunc)(eventTypeId, wheelEvent, userData)) e.preventDefault();
      };
  
      var eventHandler = {
        target,
        allowsDeferredCalls: true,
        eventTypeString,
        callbackfunc,
        handlerFunc: wheelHandlerFunc,
        useCapture
      };
      return JSEvents.registerOrRemoveHandler(eventHandler);
    };
  
  var _emscripten_set_wheel_callback_on_thread = (target, userData, useCapture, callbackfunc, targetThread) => {
      target = findEventTarget(target);
      if (!target) return -4;
      if (typeof target.onwheel != 'undefined') {
        return registerWheelEventCallback(target, userData, useCapture, callbackfunc, 9, "wheel", targetThread);
      } else {
        return -1;
      }
    };

  
  var _emscripten_set_window_title = (title) => document.title = UTF8ToString(title);

  var _emscripten_sleep = () => {
      throw 'Please compile your program with async support in order to use asynchronous operations like emscripten_sleep';
    };

  var ENV = {
  };
  
  var getExecutableName = () => thisProgram || './this.program';
  var getEnvStrings = () => {
      if (!getEnvStrings.strings) {
        // Default values.
        // Browser language detection #8751
        var lang = ((typeof navigator == 'object' && navigator.languages && navigator.languages[0]) || 'C').replace('-', '_') + '.UTF-8';
        var env = {
          'USER': 'web_user',
          'LOGNAME': 'web_user',
          'PATH': '/',
          'PWD': '/',
          'HOME': '/home/web_user',
          'LANG': lang,
          '_': getExecutableName()
        };
        // Apply the user-provided values, if any.
        for (var x in ENV) {
          // x is a key in ENV; if ENV[x] is undefined, that means it was
          // explicitly set to be so. We allow user code to do that to
          // force variables with default values to remain unset.
          if (ENV[x] === undefined) delete env[x];
          else env[x] = ENV[x];
        }
        var strings = [];
        for (var x in env) {
          strings.push(`${x}=${env[x]}`);
        }
        getEnvStrings.strings = strings;
      }
      return getEnvStrings.strings;
    };
  
  var stringToAscii = (str, buffer) => {
      for (var i = 0; i < str.length; ++i) {
        assert(str.charCodeAt(i) === (str.charCodeAt(i) & 0xff));
        HEAP8[buffer++] = str.charCodeAt(i);
      }
      // Null-terminate the string
      HEAP8[buffer] = 0;
    };
  var _environ_get = (__environ, environ_buf) => {
      var bufSize = 0;
      getEnvStrings().forEach((string, i) => {
        var ptr = environ_buf + bufSize;
        HEAPU32[(((__environ)+(i*4))>>2)] = ptr;
        stringToAscii(string, ptr);
        bufSize += string.length + 1;
      });
      return 0;
    };

  var _environ_sizes_get = (penviron_count, penviron_buf_size) => {
      var strings = getEnvStrings();
      HEAPU32[((penviron_count)>>2)] = strings.length;
      var bufSize = 0;
      strings.forEach((string) => bufSize += string.length + 1);
      HEAPU32[((penviron_buf_size)>>2)] = bufSize;
      return 0;
    };


  function _fd_close(fd) {
  try {
  
      var stream = SYSCALLS.getStreamFromFD(fd);
      FS.close(stream);
      return 0;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return e.errno;
  }
  }

  /** @param {number=} offset */
  var doReadv = (stream, iov, iovcnt, offset) => {
      var ret = 0;
      for (var i = 0; i < iovcnt; i++) {
        var ptr = HEAPU32[((iov)>>2)];
        var len = HEAPU32[(((iov)+(4))>>2)];
        iov += 8;
        var curr = FS.read(stream, HEAP8, ptr, len, offset);
        if (curr < 0) return -1;
        ret += curr;
        if (curr < len) break; // nothing more to read
        if (typeof offset != 'undefined') {
          offset += curr;
        }
      }
      return ret;
    };
  
  function _fd_read(fd, iov, iovcnt, pnum) {
  try {
  
      var stream = SYSCALLS.getStreamFromFD(fd);
      var num = doReadv(stream, iov, iovcnt);
      HEAPU32[((pnum)>>2)] = num;
      return 0;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return e.errno;
  }
  }

  
  function _fd_seek(fd, offset, whence, newOffset) {
    offset = bigintToI53Checked(offset);
  
  
  try {
  
      if (isNaN(offset)) return 61;
      var stream = SYSCALLS.getStreamFromFD(fd);
      FS.llseek(stream, offset, whence);
      HEAP64[((newOffset)>>3)] = BigInt(stream.position);
      if (stream.getdents && offset === 0 && whence === 0) stream.getdents = null; // reset readdir state
      return 0;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return e.errno;
  }
  ;
  }

  /** @param {number=} offset */
  var doWritev = (stream, iov, iovcnt, offset) => {
      var ret = 0;
      for (var i = 0; i < iovcnt; i++) {
        var ptr = HEAPU32[((iov)>>2)];
        var len = HEAPU32[(((iov)+(4))>>2)];
        iov += 8;
        var curr = FS.write(stream, HEAP8, ptr, len, offset);
        if (curr < 0) return -1;
        ret += curr;
        if (curr < len) {
          // No more space to write.
          break;
        }
        if (typeof offset != 'undefined') {
          offset += curr;
        }
      }
      return ret;
    };
  
  function _fd_write(fd, iov, iovcnt, pnum) {
  try {
  
      var stream = SYSCALLS.getStreamFromFD(fd);
      var num = doWritev(stream, iov, iovcnt);
      HEAPU32[((pnum)>>2)] = num;
      return 0;
    } catch (e) {
    if (typeof FS == 'undefined' || !(e.name === 'ErrnoError')) throw e;
    return e.errno;
  }
  }














































































  var dynCall = (sig, ptr, args = [], promising = false) => {
      assert(!promising, 'async dynCall is not supported in this mode')
      assert(getWasmTableEntry(ptr), `missing table entry in dynCall: ${ptr}`);
      var func = getWasmTableEntry(ptr);
      var rtn = func(...args);
      return rtn;
    };





  var FS_createPath = FS.createPath;



  var FS_unlink = (path) => FS.unlink(path);

  var FS_createLazyFile = FS.createLazyFile;

  var FS_createDevice = FS.createDevice;

  var incrementExceptionRefcount = (ptr) => ___cxa_increment_exception_refcount(ptr);
  Module['incrementExceptionRefcount'] = incrementExceptionRefcount;

  var decrementExceptionRefcount = (ptr) => ___cxa_decrement_exception_refcount(ptr);
  Module['decrementExceptionRefcount'] = decrementExceptionRefcount;

  
  
  
  
  
  var getExceptionMessageCommon = (ptr) => {
      var sp = stackSave();
      var type_addr_addr = stackAlloc(4);
      var message_addr_addr = stackAlloc(4);
      ___get_exception_message(ptr, type_addr_addr, message_addr_addr);
      var type_addr = HEAPU32[((type_addr_addr)>>2)];
      var message_addr = HEAPU32[((message_addr_addr)>>2)];
      var type = UTF8ToString(type_addr);
      _free(type_addr);
      var message;
      if (message_addr) {
        message = UTF8ToString(message_addr);
        _free(message_addr);
      }
      stackRestore(sp);
      return [type, message];
    };
  var getExceptionMessage = (ptr) => getExceptionMessageCommon(ptr);
  Module['getExceptionMessage'] = getExceptionMessage;

  FS.createPreloadedFile = FS_createPreloadedFile;
  FS.staticInit();
  // Set module methods based on EXPORTED_RUNTIME_METHODS
  Module['FS_createPath'] = FS.createPath;
  Module['FS_createDataFile'] = FS.createDataFile;
  Module['FS_createPreloadedFile'] = FS.createPreloadedFile;
  Module['FS_unlink'] = FS.unlink;
  Module['FS_createLazyFile'] = FS.createLazyFile;
  Module['FS_createDevice'] = FS.createDevice;
  ;

      Module['requestAnimationFrame'] = MainLoop.requestAnimationFrame;
      Module['pauseMainLoop'] = MainLoop.pause;
      Module['resumeMainLoop'] = MainLoop.resume;
      MainLoop.init();;

      // exports
      Module['requestFullscreen'] = Browser.requestFullscreen;
      Module['requestFullScreen'] = Browser.requestFullScreen;
      Module['setCanvasSize'] = Browser.setCanvasSize;
      Module['getUserMedia'] = Browser.getUserMedia;
      Module['createContext'] = Browser.createContext;
    ;
for (let i = 0; i < 32; ++i) tempFixedLengthArray.push(new Array(i));;
// End JS library code

function checkIncomingModuleAPI() {
  ignoredModuleProp('fetchSettings');
}
var ASM_CONSTS = {
  1506184: () => { if (typeof(AudioContext) !== 'undefined') { return true; } else if (typeof(webkitAudioContext) !== 'undefined') { return true; } return false; },  
 1506331: () => { if ((typeof(navigator.mediaDevices) !== 'undefined') && (typeof(navigator.mediaDevices.getUserMedia) !== 'undefined')) { return true; } else if (typeof(navigator.webkitGetUserMedia) !== 'undefined') { return true; } return false; },  
 1506565: ($0) => { if(typeof(Module['SDL2']) === 'undefined') { Module['SDL2'] = {}; } var SDL2 = Module['SDL2']; if (!$0) { SDL2.audio = {}; } else { SDL2.capture = {}; } if (!SDL2.audioContext) { if (typeof(AudioContext) !== 'undefined') { SDL2.audioContext = new AudioContext(); } else if (typeof(webkitAudioContext) !== 'undefined') { SDL2.audioContext = new webkitAudioContext(); } if (SDL2.audioContext) { if ((typeof navigator.userActivation) === 'undefined') { autoResumeAudioContext(SDL2.audioContext); } } } return SDL2.audioContext === undefined ? -1 : 0; },  
 1507117: () => { var SDL2 = Module['SDL2']; return SDL2.audioContext.sampleRate; },  
 1507185: ($0, $1, $2, $3) => { var SDL2 = Module['SDL2']; var have_microphone = function(stream) { if (SDL2.capture.silenceTimer !== undefined) { clearInterval(SDL2.capture.silenceTimer); SDL2.capture.silenceTimer = undefined; SDL2.capture.silenceBuffer = undefined } SDL2.capture.mediaStreamNode = SDL2.audioContext.createMediaStreamSource(stream); SDL2.capture.scriptProcessorNode = SDL2.audioContext.createScriptProcessor($1, $0, 1); SDL2.capture.scriptProcessorNode.onaudioprocess = function(audioProcessingEvent) { if ((SDL2 === undefined) || (SDL2.capture === undefined)) { return; } audioProcessingEvent.outputBuffer.getChannelData(0).fill(0.0); SDL2.capture.currentCaptureBuffer = audioProcessingEvent.inputBuffer; dynCall('vi', $2, [$3]); }; SDL2.capture.mediaStreamNode.connect(SDL2.capture.scriptProcessorNode); SDL2.capture.scriptProcessorNode.connect(SDL2.audioContext.destination); SDL2.capture.stream = stream; }; var no_microphone = function(error) { }; SDL2.capture.silenceBuffer = SDL2.audioContext.createBuffer($0, $1, SDL2.audioContext.sampleRate); SDL2.capture.silenceBuffer.getChannelData(0).fill(0.0); var silence_callback = function() { SDL2.capture.currentCaptureBuffer = SDL2.capture.silenceBuffer; dynCall('vi', $2, [$3]); }; SDL2.capture.silenceTimer = setInterval(silence_callback, ($1 / SDL2.audioContext.sampleRate) * 1000); if ((navigator.mediaDevices !== undefined) && (navigator.mediaDevices.getUserMedia !== undefined)) { navigator.mediaDevices.getUserMedia({ audio: true, video: false }).then(have_microphone).catch(no_microphone); } else if (navigator.webkitGetUserMedia !== undefined) { navigator.webkitGetUserMedia({ audio: true, video: false }, have_microphone, no_microphone); } },  
 1508878: ($0, $1, $2, $3) => { var SDL2 = Module['SDL2']; SDL2.audio.scriptProcessorNode = SDL2.audioContext['createScriptProcessor']($1, 0, $0); SDL2.audio.scriptProcessorNode['onaudioprocess'] = function (e) { if ((SDL2 === undefined) || (SDL2.audio === undefined)) { return; } if (SDL2.audio.silenceTimer !== undefined) { clearInterval(SDL2.audio.silenceTimer); SDL2.audio.silenceTimer = undefined; SDL2.audio.silenceBuffer = undefined; } SDL2.audio.currentOutputBuffer = e['outputBuffer']; dynCall('vi', $2, [$3]); }; SDL2.audio.scriptProcessorNode['connect'](SDL2.audioContext['destination']); if (SDL2.audioContext.state === 'suspended') { SDL2.audio.silenceBuffer = SDL2.audioContext.createBuffer($0, $1, SDL2.audioContext.sampleRate); SDL2.audio.silenceBuffer.getChannelData(0).fill(0.0); var silence_callback = function() { if ((typeof navigator.userActivation) !== 'undefined') { if (navigator.userActivation.hasBeenActive) { SDL2.audioContext.resume(); } } SDL2.audio.currentOutputBuffer = SDL2.audio.silenceBuffer; dynCall('vi', $2, [$3]); SDL2.audio.currentOutputBuffer = undefined; }; SDL2.audio.silenceTimer = setInterval(silence_callback, ($1 / SDL2.audioContext.sampleRate) * 1000); } },  
 1510053: ($0, $1) => { var SDL2 = Module['SDL2']; var numChannels = SDL2.capture.currentCaptureBuffer.numberOfChannels; for (var c = 0; c < numChannels; ++c) { var channelData = SDL2.capture.currentCaptureBuffer.getChannelData(c); if (channelData.length != $1) { throw 'Web Audio capture buffer length mismatch! Destination size: ' + channelData.length + ' samples vs expected ' + $1 + ' samples!'; } if (numChannels == 1) { for (var j = 0; j < $1; ++j) { setValue($0 + (j * 4), channelData[j], 'float'); } } else { for (var j = 0; j < $1; ++j) { setValue($0 + (((j * numChannels) + c) * 4), channelData[j], 'float'); } } } },  
 1510658: ($0, $1) => { var SDL2 = Module['SDL2']; var buf = $0 >>> 2; var numChannels = SDL2.audio.currentOutputBuffer['numberOfChannels']; for (var c = 0; c < numChannels; ++c) { var channelData = SDL2.audio.currentOutputBuffer['getChannelData'](c); if (channelData.length != $1) { throw 'Web Audio output buffer length mismatch! Destination size: ' + channelData.length + ' samples vs expected ' + $1 + ' samples!'; } for (var j = 0; j < $1; ++j) { channelData[j] = HEAPF32[buf + (j*numChannels + c)]; } } },  
 1511147: ($0) => { var SDL2 = Module['SDL2']; if ($0) { if (SDL2.capture.silenceTimer !== undefined) { clearInterval(SDL2.capture.silenceTimer); } if (SDL2.capture.stream !== undefined) { var tracks = SDL2.capture.stream.getAudioTracks(); for (var i = 0; i < tracks.length; i++) { SDL2.capture.stream.removeTrack(tracks[i]); } } if (SDL2.capture.scriptProcessorNode !== undefined) { SDL2.capture.scriptProcessorNode.onaudioprocess = function(audioProcessingEvent) {}; SDL2.capture.scriptProcessorNode.disconnect(); } if (SDL2.capture.mediaStreamNode !== undefined) { SDL2.capture.mediaStreamNode.disconnect(); } SDL2.capture = undefined; } else { if (SDL2.audio.scriptProcessorNode != undefined) { SDL2.audio.scriptProcessorNode.disconnect(); } if (SDL2.audio.silenceTimer !== undefined) { clearInterval(SDL2.audio.silenceTimer); } SDL2.audio = undefined; } if ((SDL2.audioContext !== undefined) && (SDL2.audio === undefined) && (SDL2.capture === undefined)) { SDL2.audioContext.close(); SDL2.audioContext = undefined; } },  
 1512153: ($0) => { var str = UTF8ToString($0) + '\n\n' + 'Abort/Retry/Ignore/AlwaysIgnore? [ariA] :'; var reply = window.prompt(str, "i"); if (reply === null) { reply = "i"; } return allocate(intArrayFromString(reply), 'i8', ALLOC_NORMAL); },  
 1512378: ($0, $1, $2) => { var w = $0; var h = $1; var pixels = $2; if (!Module['SDL2']) Module['SDL2'] = {}; var SDL2 = Module['SDL2']; if (SDL2.ctxCanvas !== Module['canvas']) { SDL2.ctx = Module['createContext'](Module['canvas'], false, true); SDL2.ctxCanvas = Module['canvas']; } if (SDL2.w !== w || SDL2.h !== h || SDL2.imageCtx !== SDL2.ctx) { SDL2.image = SDL2.ctx.createImageData(w, h); SDL2.w = w; SDL2.h = h; SDL2.imageCtx = SDL2.ctx; } var data = SDL2.image.data; var src = pixels / 4; var dst = 0; var num; if (typeof CanvasPixelArray !== 'undefined' && data instanceof CanvasPixelArray) { num = data.length; while (dst < num) { var val = HEAP32[src]; data[dst ] = val & 0xff; data[dst+1] = (val >> 8) & 0xff; data[dst+2] = (val >> 16) & 0xff; data[dst+3] = 0xff; src++; dst += 4; } } else { if (SDL2.data32Data !== data) { SDL2.data32 = new Int32Array(data.buffer); SDL2.data8 = new Uint8Array(data.buffer); SDL2.data32Data = data; } var data32 = SDL2.data32; num = data32.length; data32.set(HEAP32.subarray(src, src + num)); var data8 = SDL2.data8; var i = 3; var j = i + 4*num; if (num % 8 == 0) { while (i < j) { data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; } } else { while (i < j) { data8[i] = 0xff; i = i + 4 | 0; } } } SDL2.ctx.putImageData(SDL2.image, 0, 0); },  
 1513846: ($0, $1, $2, $3, $4) => { var w = $0; var h = $1; var hot_x = $2; var hot_y = $3; var pixels = $4; var canvas = document.createElement("canvas"); canvas.width = w; canvas.height = h; var ctx = canvas.getContext("2d"); var image = ctx.createImageData(w, h); var data = image.data; var src = pixels / 4; var dst = 0; var num; if (typeof CanvasPixelArray !== 'undefined' && data instanceof CanvasPixelArray) { num = data.length; while (dst < num) { var val = HEAP32[src]; data[dst ] = val & 0xff; data[dst+1] = (val >> 8) & 0xff; data[dst+2] = (val >> 16) & 0xff; data[dst+3] = (val >> 24) & 0xff; src++; dst += 4; } } else { var data32 = new Int32Array(data.buffer); num = data32.length; data32.set(HEAP32.subarray(src, src + num)); } ctx.putImageData(image, 0, 0); var url = hot_x === 0 && hot_y === 0 ? "url(" + canvas.toDataURL() + "), auto" : "url(" + canvas.toDataURL() + ") " + hot_x + " " + hot_y + ", auto"; var urlBuf = _malloc(url.length + 1); stringToUTF8(url, urlBuf, url.length + 1); return urlBuf; },  
 1514834: ($0) => { if (Module['canvas']) { Module['canvas'].style['cursor'] = UTF8ToString($0); } },  
 1514917: () => { if (Module['canvas']) { Module['canvas'].style['cursor'] = 'none'; } },  
 1514986: () => { return window.innerWidth; },  
 1515016: () => { return window.innerHeight; }
};
function canvas_get_width() { return canvas.width; }
function canvas_get_height() { return canvas.height; }
function release_cursor_js() { if (document.pointerLockElement === Module['canvas']) { document.exitPointerLock(); } }
function lock_cursor_js() { if (Module['canvas']) { Module['canvas'].requestPointerLock(); } }
function MountPersistentFS() { FS.mkdir('/save'); FS.mount(IDBFS, {}, '/save'); FS.syncfs(true, function(err) { if (err) console.error('IDBFS initial load failed:', err); }); }
function WriteFileJS(relPath,content) { var path = '/save/' + UTF8ToString(relPath); var data = UTF8ToString(content); (function ensureDir(fp) { var parts = fp.split('/'); parts.pop(); var cur = ""; for (var i = 1; i < parts.length; i++) { cur += '/' + parts[i]; try { FS.mkdir(cur); } catch (e) { } } })(path); FS.writeFile(path, data); FS.syncfs(false, function(err) { if (err) console.error('IDBFS write sync failed:', err); }); }
function ReadFileJS(relPath) { try { var path = '/save/' + UTF8ToString(relPath); var data = FS.readFile(path, { encoding: 'utf8' }); var len = lengthBytesUTF8(data) + 1; var buf = _malloc(len); stringToUTF8(data, buf, len); return buf; } catch (e) { return 0; } }
function ImGui_ImplSDL2_EmscriptenOpenURL(url) { url = url ? UTF8ToString(url) : null; if (url) window.open(url, '_blank'); }
var wasmImports = {
  /** @export */
  ImGui_ImplSDL2_EmscriptenOpenURL,
  /** @export */
  MountPersistentFS,
  /** @export */
  ReadFileJS,
  /** @export */
  WriteFileJS,
  /** @export */
  __assert_fail: ___assert_fail,
  /** @export */
  __cxa_begin_catch: ___cxa_begin_catch,
  /** @export */
  __cxa_end_catch: ___cxa_end_catch,
  /** @export */
  __cxa_find_matching_catch_2: ___cxa_find_matching_catch_2,
  /** @export */
  __cxa_find_matching_catch_3: ___cxa_find_matching_catch_3,
  /** @export */
  __cxa_rethrow: ___cxa_rethrow,
  /** @export */
  __cxa_throw: ___cxa_throw,
  /** @export */
  __cxa_uncaught_exceptions: ___cxa_uncaught_exceptions,
  /** @export */
  __resumeException: ___resumeException,
  /** @export */
  __syscall_chdir: ___syscall_chdir,
  /** @export */
  __syscall_fcntl64: ___syscall_fcntl64,
  /** @export */
  __syscall_fstat64: ___syscall_fstat64,
  /** @export */
  __syscall_getcwd: ___syscall_getcwd,
  /** @export */
  __syscall_ioctl: ___syscall_ioctl,
  /** @export */
  __syscall_lstat64: ___syscall_lstat64,
  /** @export */
  __syscall_mkdirat: ___syscall_mkdirat,
  /** @export */
  __syscall_newfstatat: ___syscall_newfstatat,
  /** @export */
  __syscall_openat: ___syscall_openat,
  /** @export */
  __syscall_readlinkat: ___syscall_readlinkat,
  /** @export */
  __syscall_rmdir: ___syscall_rmdir,
  /** @export */
  __syscall_stat64: ___syscall_stat64,
  /** @export */
  __syscall_unlinkat: ___syscall_unlinkat,
  /** @export */
  _abort_js: __abort_js,
  /** @export */
  _emscripten_throw_longjmp: __emscripten_throw_longjmp,
  /** @export */
  _mmap_js: __mmap_js,
  /** @export */
  _munmap_js: __munmap_js,
  /** @export */
  _tzset_js: __tzset_js,
  /** @export */
  alBufferData: _alBufferData,
  /** @export */
  alDistanceModel: _alDistanceModel,
  /** @export */
  alGenBuffers: _alGenBuffers,
  /** @export */
  alGenSources: _alGenSources,
  /** @export */
  alGetListenerfv: _alGetListenerfv,
  /** @export */
  alGetString: _alGetString,
  /** @export */
  alIsSource: _alIsSource,
  /** @export */
  alListener3f: _alListener3f,
  /** @export */
  alListenerfv: _alListenerfv,
  /** @export */
  alSource3f: _alSource3f,
  /** @export */
  alSourcePause: _alSourcePause,
  /** @export */
  alSourcePlay: _alSourcePlay,
  /** @export */
  alSourcef: _alSourcef,
  /** @export */
  alSourcei: _alSourcei,
  /** @export */
  alcCloseDevice: _alcCloseDevice,
  /** @export */
  alcCreateContext: _alcCreateContext,
  /** @export */
  alcDestroyContext: _alcDestroyContext,
  /** @export */
  alcIsExtensionPresent: _alcIsExtensionPresent,
  /** @export */
  alcMakeContextCurrent: _alcMakeContextCurrent,
  /** @export */
  alcOpenDevice: _alcOpenDevice,
  /** @export */
  canvas_get_height,
  /** @export */
  canvas_get_width,
  /** @export */
  clock_time_get: _clock_time_get,
  /** @export */
  eglBindAPI: _eglBindAPI,
  /** @export */
  eglChooseConfig: _eglChooseConfig,
  /** @export */
  eglCreateContext: _eglCreateContext,
  /** @export */
  eglCreateWindowSurface: _eglCreateWindowSurface,
  /** @export */
  eglDestroyContext: _eglDestroyContext,
  /** @export */
  eglDestroySurface: _eglDestroySurface,
  /** @export */
  eglGetConfigAttrib: _eglGetConfigAttrib,
  /** @export */
  eglGetDisplay: _eglGetDisplay,
  /** @export */
  eglGetError: _eglGetError,
  /** @export */
  eglInitialize: _eglInitialize,
  /** @export */
  eglMakeCurrent: _eglMakeCurrent,
  /** @export */
  eglQueryString: _eglQueryString,
  /** @export */
  eglSwapBuffers: _eglSwapBuffers,
  /** @export */
  eglSwapInterval: _eglSwapInterval,
  /** @export */
  eglTerminate: _eglTerminate,
  /** @export */
  eglWaitGL: _eglWaitGL,
  /** @export */
  eglWaitNative: _eglWaitNative,
  /** @export */
  emscripten_asm_const_int: _emscripten_asm_const_int,
  /** @export */
  emscripten_asm_const_int_sync_on_main_thread: _emscripten_asm_const_int_sync_on_main_thread,
  /** @export */
  emscripten_asm_const_ptr_sync_on_main_thread: _emscripten_asm_const_ptr_sync_on_main_thread,
  /** @export */
  emscripten_date_now: _emscripten_date_now,
  /** @export */
  emscripten_enter_soft_fullscreen: _emscripten_enter_soft_fullscreen,
  /** @export */
  emscripten_err: _emscripten_err,
  /** @export */
  emscripten_exit_fullscreen: _emscripten_exit_fullscreen,
  /** @export */
  emscripten_exit_pointerlock: _emscripten_exit_pointerlock,
  /** @export */
  emscripten_get_device_pixel_ratio: _emscripten_get_device_pixel_ratio,
  /** @export */
  emscripten_get_element_css_size: _emscripten_get_element_css_size,
  /** @export */
  emscripten_get_gamepad_status: _emscripten_get_gamepad_status,
  /** @export */
  emscripten_get_heap_max: _emscripten_get_heap_max,
  /** @export */
  emscripten_get_now: _emscripten_get_now,
  /** @export */
  emscripten_get_num_gamepads: _emscripten_get_num_gamepads,
  /** @export */
  emscripten_get_preloaded_image_data: _emscripten_get_preloaded_image_data,
  /** @export */
  emscripten_get_preloaded_image_data_from_FILE: _emscripten_get_preloaded_image_data_from_FILE,
  /** @export */
  emscripten_get_screen_size: _emscripten_get_screen_size,
  /** @export */
  emscripten_glActiveTexture: _emscripten_glActiveTexture,
  /** @export */
  emscripten_glAttachShader: _emscripten_glAttachShader,
  /** @export */
  emscripten_glBeginQuery: _emscripten_glBeginQuery,
  /** @export */
  emscripten_glBeginQueryEXT: _emscripten_glBeginQueryEXT,
  /** @export */
  emscripten_glBeginTransformFeedback: _emscripten_glBeginTransformFeedback,
  /** @export */
  emscripten_glBindAttribLocation: _emscripten_glBindAttribLocation,
  /** @export */
  emscripten_glBindBuffer: _emscripten_glBindBuffer,
  /** @export */
  emscripten_glBindBufferBase: _emscripten_glBindBufferBase,
  /** @export */
  emscripten_glBindBufferRange: _emscripten_glBindBufferRange,
  /** @export */
  emscripten_glBindFramebuffer: _emscripten_glBindFramebuffer,
  /** @export */
  emscripten_glBindRenderbuffer: _emscripten_glBindRenderbuffer,
  /** @export */
  emscripten_glBindSampler: _emscripten_glBindSampler,
  /** @export */
  emscripten_glBindTexture: _emscripten_glBindTexture,
  /** @export */
  emscripten_glBindTransformFeedback: _emscripten_glBindTransformFeedback,
  /** @export */
  emscripten_glBindVertexArray: _emscripten_glBindVertexArray,
  /** @export */
  emscripten_glBindVertexArrayOES: _emscripten_glBindVertexArrayOES,
  /** @export */
  emscripten_glBlendColor: _emscripten_glBlendColor,
  /** @export */
  emscripten_glBlendEquation: _emscripten_glBlendEquation,
  /** @export */
  emscripten_glBlendEquationSeparate: _emscripten_glBlendEquationSeparate,
  /** @export */
  emscripten_glBlendFunc: _emscripten_glBlendFunc,
  /** @export */
  emscripten_glBlendFuncSeparate: _emscripten_glBlendFuncSeparate,
  /** @export */
  emscripten_glBlitFramebuffer: _emscripten_glBlitFramebuffer,
  /** @export */
  emscripten_glBufferData: _emscripten_glBufferData,
  /** @export */
  emscripten_glBufferSubData: _emscripten_glBufferSubData,
  /** @export */
  emscripten_glCheckFramebufferStatus: _emscripten_glCheckFramebufferStatus,
  /** @export */
  emscripten_glClear: _emscripten_glClear,
  /** @export */
  emscripten_glClearBufferfi: _emscripten_glClearBufferfi,
  /** @export */
  emscripten_glClearBufferfv: _emscripten_glClearBufferfv,
  /** @export */
  emscripten_glClearBufferiv: _emscripten_glClearBufferiv,
  /** @export */
  emscripten_glClearBufferuiv: _emscripten_glClearBufferuiv,
  /** @export */
  emscripten_glClearColor: _emscripten_glClearColor,
  /** @export */
  emscripten_glClearDepthf: _emscripten_glClearDepthf,
  /** @export */
  emscripten_glClearStencil: _emscripten_glClearStencil,
  /** @export */
  emscripten_glClientWaitSync: _emscripten_glClientWaitSync,
  /** @export */
  emscripten_glClipControlEXT: _emscripten_glClipControlEXT,
  /** @export */
  emscripten_glColorMask: _emscripten_glColorMask,
  /** @export */
  emscripten_glCompileShader: _emscripten_glCompileShader,
  /** @export */
  emscripten_glCompressedTexImage2D: _emscripten_glCompressedTexImage2D,
  /** @export */
  emscripten_glCompressedTexImage3D: _emscripten_glCompressedTexImage3D,
  /** @export */
  emscripten_glCompressedTexSubImage2D: _emscripten_glCompressedTexSubImage2D,
  /** @export */
  emscripten_glCompressedTexSubImage3D: _emscripten_glCompressedTexSubImage3D,
  /** @export */
  emscripten_glCopyBufferSubData: _emscripten_glCopyBufferSubData,
  /** @export */
  emscripten_glCopyTexImage2D: _emscripten_glCopyTexImage2D,
  /** @export */
  emscripten_glCopyTexSubImage2D: _emscripten_glCopyTexSubImage2D,
  /** @export */
  emscripten_glCopyTexSubImage3D: _emscripten_glCopyTexSubImage3D,
  /** @export */
  emscripten_glCreateProgram: _emscripten_glCreateProgram,
  /** @export */
  emscripten_glCreateShader: _emscripten_glCreateShader,
  /** @export */
  emscripten_glCullFace: _emscripten_glCullFace,
  /** @export */
  emscripten_glDeleteBuffers: _emscripten_glDeleteBuffers,
  /** @export */
  emscripten_glDeleteFramebuffers: _emscripten_glDeleteFramebuffers,
  /** @export */
  emscripten_glDeleteProgram: _emscripten_glDeleteProgram,
  /** @export */
  emscripten_glDeleteQueries: _emscripten_glDeleteQueries,
  /** @export */
  emscripten_glDeleteQueriesEXT: _emscripten_glDeleteQueriesEXT,
  /** @export */
  emscripten_glDeleteRenderbuffers: _emscripten_glDeleteRenderbuffers,
  /** @export */
  emscripten_glDeleteSamplers: _emscripten_glDeleteSamplers,
  /** @export */
  emscripten_glDeleteShader: _emscripten_glDeleteShader,
  /** @export */
  emscripten_glDeleteSync: _emscripten_glDeleteSync,
  /** @export */
  emscripten_glDeleteTextures: _emscripten_glDeleteTextures,
  /** @export */
  emscripten_glDeleteTransformFeedbacks: _emscripten_glDeleteTransformFeedbacks,
  /** @export */
  emscripten_glDeleteVertexArrays: _emscripten_glDeleteVertexArrays,
  /** @export */
  emscripten_glDeleteVertexArraysOES: _emscripten_glDeleteVertexArraysOES,
  /** @export */
  emscripten_glDepthFunc: _emscripten_glDepthFunc,
  /** @export */
  emscripten_glDepthMask: _emscripten_glDepthMask,
  /** @export */
  emscripten_glDepthRangef: _emscripten_glDepthRangef,
  /** @export */
  emscripten_glDetachShader: _emscripten_glDetachShader,
  /** @export */
  emscripten_glDisable: _emscripten_glDisable,
  /** @export */
  emscripten_glDisableVertexAttribArray: _emscripten_glDisableVertexAttribArray,
  /** @export */
  emscripten_glDrawArrays: _emscripten_glDrawArrays,
  /** @export */
  emscripten_glDrawArraysInstanced: _emscripten_glDrawArraysInstanced,
  /** @export */
  emscripten_glDrawArraysInstancedANGLE: _emscripten_glDrawArraysInstancedANGLE,
  /** @export */
  emscripten_glDrawArraysInstancedARB: _emscripten_glDrawArraysInstancedARB,
  /** @export */
  emscripten_glDrawArraysInstancedEXT: _emscripten_glDrawArraysInstancedEXT,
  /** @export */
  emscripten_glDrawArraysInstancedNV: _emscripten_glDrawArraysInstancedNV,
  /** @export */
  emscripten_glDrawBuffers: _emscripten_glDrawBuffers,
  /** @export */
  emscripten_glDrawBuffersEXT: _emscripten_glDrawBuffersEXT,
  /** @export */
  emscripten_glDrawBuffersWEBGL: _emscripten_glDrawBuffersWEBGL,
  /** @export */
  emscripten_glDrawElements: _emscripten_glDrawElements,
  /** @export */
  emscripten_glDrawElementsInstanced: _emscripten_glDrawElementsInstanced,
  /** @export */
  emscripten_glDrawElementsInstancedANGLE: _emscripten_glDrawElementsInstancedANGLE,
  /** @export */
  emscripten_glDrawElementsInstancedARB: _emscripten_glDrawElementsInstancedARB,
  /** @export */
  emscripten_glDrawElementsInstancedEXT: _emscripten_glDrawElementsInstancedEXT,
  /** @export */
  emscripten_glDrawElementsInstancedNV: _emscripten_glDrawElementsInstancedNV,
  /** @export */
  emscripten_glDrawRangeElements: _emscripten_glDrawRangeElements,
  /** @export */
  emscripten_glEnable: _emscripten_glEnable,
  /** @export */
  emscripten_glEnableVertexAttribArray: _emscripten_glEnableVertexAttribArray,
  /** @export */
  emscripten_glEndQuery: _emscripten_glEndQuery,
  /** @export */
  emscripten_glEndQueryEXT: _emscripten_glEndQueryEXT,
  /** @export */
  emscripten_glEndTransformFeedback: _emscripten_glEndTransformFeedback,
  /** @export */
  emscripten_glFenceSync: _emscripten_glFenceSync,
  /** @export */
  emscripten_glFinish: _emscripten_glFinish,
  /** @export */
  emscripten_glFlush: _emscripten_glFlush,
  /** @export */
  emscripten_glFramebufferRenderbuffer: _emscripten_glFramebufferRenderbuffer,
  /** @export */
  emscripten_glFramebufferTexture2D: _emscripten_glFramebufferTexture2D,
  /** @export */
  emscripten_glFramebufferTextureLayer: _emscripten_glFramebufferTextureLayer,
  /** @export */
  emscripten_glFrontFace: _emscripten_glFrontFace,
  /** @export */
  emscripten_glGenBuffers: _emscripten_glGenBuffers,
  /** @export */
  emscripten_glGenFramebuffers: _emscripten_glGenFramebuffers,
  /** @export */
  emscripten_glGenQueries: _emscripten_glGenQueries,
  /** @export */
  emscripten_glGenQueriesEXT: _emscripten_glGenQueriesEXT,
  /** @export */
  emscripten_glGenRenderbuffers: _emscripten_glGenRenderbuffers,
  /** @export */
  emscripten_glGenSamplers: _emscripten_glGenSamplers,
  /** @export */
  emscripten_glGenTextures: _emscripten_glGenTextures,
  /** @export */
  emscripten_glGenTransformFeedbacks: _emscripten_glGenTransformFeedbacks,
  /** @export */
  emscripten_glGenVertexArrays: _emscripten_glGenVertexArrays,
  /** @export */
  emscripten_glGenVertexArraysOES: _emscripten_glGenVertexArraysOES,
  /** @export */
  emscripten_glGenerateMipmap: _emscripten_glGenerateMipmap,
  /** @export */
  emscripten_glGetActiveAttrib: _emscripten_glGetActiveAttrib,
  /** @export */
  emscripten_glGetActiveUniform: _emscripten_glGetActiveUniform,
  /** @export */
  emscripten_glGetActiveUniformBlockName: _emscripten_glGetActiveUniformBlockName,
  /** @export */
  emscripten_glGetActiveUniformBlockiv: _emscripten_glGetActiveUniformBlockiv,
  /** @export */
  emscripten_glGetActiveUniformsiv: _emscripten_glGetActiveUniformsiv,
  /** @export */
  emscripten_glGetAttachedShaders: _emscripten_glGetAttachedShaders,
  /** @export */
  emscripten_glGetAttribLocation: _emscripten_glGetAttribLocation,
  /** @export */
  emscripten_glGetBooleanv: _emscripten_glGetBooleanv,
  /** @export */
  emscripten_glGetBufferParameteri64v: _emscripten_glGetBufferParameteri64v,
  /** @export */
  emscripten_glGetBufferParameteriv: _emscripten_glGetBufferParameteriv,
  /** @export */
  emscripten_glGetError: _emscripten_glGetError,
  /** @export */
  emscripten_glGetFloatv: _emscripten_glGetFloatv,
  /** @export */
  emscripten_glGetFragDataLocation: _emscripten_glGetFragDataLocation,
  /** @export */
  emscripten_glGetFramebufferAttachmentParameteriv: _emscripten_glGetFramebufferAttachmentParameteriv,
  /** @export */
  emscripten_glGetInteger64i_v: _emscripten_glGetInteger64i_v,
  /** @export */
  emscripten_glGetInteger64v: _emscripten_glGetInteger64v,
  /** @export */
  emscripten_glGetIntegeri_v: _emscripten_glGetIntegeri_v,
  /** @export */
  emscripten_glGetIntegerv: _emscripten_glGetIntegerv,
  /** @export */
  emscripten_glGetInternalformativ: _emscripten_glGetInternalformativ,
  /** @export */
  emscripten_glGetProgramBinary: _emscripten_glGetProgramBinary,
  /** @export */
  emscripten_glGetProgramInfoLog: _emscripten_glGetProgramInfoLog,
  /** @export */
  emscripten_glGetProgramiv: _emscripten_glGetProgramiv,
  /** @export */
  emscripten_glGetQueryObjecti64vEXT: _emscripten_glGetQueryObjecti64vEXT,
  /** @export */
  emscripten_glGetQueryObjectivEXT: _emscripten_glGetQueryObjectivEXT,
  /** @export */
  emscripten_glGetQueryObjectui64vEXT: _emscripten_glGetQueryObjectui64vEXT,
  /** @export */
  emscripten_glGetQueryObjectuiv: _emscripten_glGetQueryObjectuiv,
  /** @export */
  emscripten_glGetQueryObjectuivEXT: _emscripten_glGetQueryObjectuivEXT,
  /** @export */
  emscripten_glGetQueryiv: _emscripten_glGetQueryiv,
  /** @export */
  emscripten_glGetQueryivEXT: _emscripten_glGetQueryivEXT,
  /** @export */
  emscripten_glGetRenderbufferParameteriv: _emscripten_glGetRenderbufferParameteriv,
  /** @export */
  emscripten_glGetSamplerParameterfv: _emscripten_glGetSamplerParameterfv,
  /** @export */
  emscripten_glGetSamplerParameteriv: _emscripten_glGetSamplerParameteriv,
  /** @export */
  emscripten_glGetShaderInfoLog: _emscripten_glGetShaderInfoLog,
  /** @export */
  emscripten_glGetShaderPrecisionFormat: _emscripten_glGetShaderPrecisionFormat,
  /** @export */
  emscripten_glGetShaderSource: _emscripten_glGetShaderSource,
  /** @export */
  emscripten_glGetShaderiv: _emscripten_glGetShaderiv,
  /** @export */
  emscripten_glGetString: _emscripten_glGetString,
  /** @export */
  emscripten_glGetStringi: _emscripten_glGetStringi,
  /** @export */
  emscripten_glGetSynciv: _emscripten_glGetSynciv,
  /** @export */
  emscripten_glGetTexParameterfv: _emscripten_glGetTexParameterfv,
  /** @export */
  emscripten_glGetTexParameteriv: _emscripten_glGetTexParameteriv,
  /** @export */
  emscripten_glGetTransformFeedbackVarying: _emscripten_glGetTransformFeedbackVarying,
  /** @export */
  emscripten_glGetUniformBlockIndex: _emscripten_glGetUniformBlockIndex,
  /** @export */
  emscripten_glGetUniformIndices: _emscripten_glGetUniformIndices,
  /** @export */
  emscripten_glGetUniformLocation: _emscripten_glGetUniformLocation,
  /** @export */
  emscripten_glGetUniformfv: _emscripten_glGetUniformfv,
  /** @export */
  emscripten_glGetUniformiv: _emscripten_glGetUniformiv,
  /** @export */
  emscripten_glGetUniformuiv: _emscripten_glGetUniformuiv,
  /** @export */
  emscripten_glGetVertexAttribIiv: _emscripten_glGetVertexAttribIiv,
  /** @export */
  emscripten_glGetVertexAttribIuiv: _emscripten_glGetVertexAttribIuiv,
  /** @export */
  emscripten_glGetVertexAttribPointerv: _emscripten_glGetVertexAttribPointerv,
  /** @export */
  emscripten_glGetVertexAttribfv: _emscripten_glGetVertexAttribfv,
  /** @export */
  emscripten_glGetVertexAttribiv: _emscripten_glGetVertexAttribiv,
  /** @export */
  emscripten_glHint: _emscripten_glHint,
  /** @export */
  emscripten_glInvalidateFramebuffer: _emscripten_glInvalidateFramebuffer,
  /** @export */
  emscripten_glInvalidateSubFramebuffer: _emscripten_glInvalidateSubFramebuffer,
  /** @export */
  emscripten_glIsBuffer: _emscripten_glIsBuffer,
  /** @export */
  emscripten_glIsEnabled: _emscripten_glIsEnabled,
  /** @export */
  emscripten_glIsFramebuffer: _emscripten_glIsFramebuffer,
  /** @export */
  emscripten_glIsProgram: _emscripten_glIsProgram,
  /** @export */
  emscripten_glIsQuery: _emscripten_glIsQuery,
  /** @export */
  emscripten_glIsQueryEXT: _emscripten_glIsQueryEXT,
  /** @export */
  emscripten_glIsRenderbuffer: _emscripten_glIsRenderbuffer,
  /** @export */
  emscripten_glIsSampler: _emscripten_glIsSampler,
  /** @export */
  emscripten_glIsShader: _emscripten_glIsShader,
  /** @export */
  emscripten_glIsSync: _emscripten_glIsSync,
  /** @export */
  emscripten_glIsTexture: _emscripten_glIsTexture,
  /** @export */
  emscripten_glIsTransformFeedback: _emscripten_glIsTransformFeedback,
  /** @export */
  emscripten_glIsVertexArray: _emscripten_glIsVertexArray,
  /** @export */
  emscripten_glIsVertexArrayOES: _emscripten_glIsVertexArrayOES,
  /** @export */
  emscripten_glLineWidth: _emscripten_glLineWidth,
  /** @export */
  emscripten_glLinkProgram: _emscripten_glLinkProgram,
  /** @export */
  emscripten_glPauseTransformFeedback: _emscripten_glPauseTransformFeedback,
  /** @export */
  emscripten_glPixelStorei: _emscripten_glPixelStorei,
  /** @export */
  emscripten_glPolygonModeWEBGL: _emscripten_glPolygonModeWEBGL,
  /** @export */
  emscripten_glPolygonOffset: _emscripten_glPolygonOffset,
  /** @export */
  emscripten_glPolygonOffsetClampEXT: _emscripten_glPolygonOffsetClampEXT,
  /** @export */
  emscripten_glProgramBinary: _emscripten_glProgramBinary,
  /** @export */
  emscripten_glProgramParameteri: _emscripten_glProgramParameteri,
  /** @export */
  emscripten_glQueryCounterEXT: _emscripten_glQueryCounterEXT,
  /** @export */
  emscripten_glReadBuffer: _emscripten_glReadBuffer,
  /** @export */
  emscripten_glReadPixels: _emscripten_glReadPixels,
  /** @export */
  emscripten_glReleaseShaderCompiler: _emscripten_glReleaseShaderCompiler,
  /** @export */
  emscripten_glRenderbufferStorage: _emscripten_glRenderbufferStorage,
  /** @export */
  emscripten_glRenderbufferStorageMultisample: _emscripten_glRenderbufferStorageMultisample,
  /** @export */
  emscripten_glResumeTransformFeedback: _emscripten_glResumeTransformFeedback,
  /** @export */
  emscripten_glSampleCoverage: _emscripten_glSampleCoverage,
  /** @export */
  emscripten_glSamplerParameterf: _emscripten_glSamplerParameterf,
  /** @export */
  emscripten_glSamplerParameterfv: _emscripten_glSamplerParameterfv,
  /** @export */
  emscripten_glSamplerParameteri: _emscripten_glSamplerParameteri,
  /** @export */
  emscripten_glSamplerParameteriv: _emscripten_glSamplerParameteriv,
  /** @export */
  emscripten_glScissor: _emscripten_glScissor,
  /** @export */
  emscripten_glShaderBinary: _emscripten_glShaderBinary,
  /** @export */
  emscripten_glShaderSource: _emscripten_glShaderSource,
  /** @export */
  emscripten_glStencilFunc: _emscripten_glStencilFunc,
  /** @export */
  emscripten_glStencilFuncSeparate: _emscripten_glStencilFuncSeparate,
  /** @export */
  emscripten_glStencilMask: _emscripten_glStencilMask,
  /** @export */
  emscripten_glStencilMaskSeparate: _emscripten_glStencilMaskSeparate,
  /** @export */
  emscripten_glStencilOp: _emscripten_glStencilOp,
  /** @export */
  emscripten_glStencilOpSeparate: _emscripten_glStencilOpSeparate,
  /** @export */
  emscripten_glTexImage2D: _emscripten_glTexImage2D,
  /** @export */
  emscripten_glTexImage3D: _emscripten_glTexImage3D,
  /** @export */
  emscripten_glTexParameterf: _emscripten_glTexParameterf,
  /** @export */
  emscripten_glTexParameterfv: _emscripten_glTexParameterfv,
  /** @export */
  emscripten_glTexParameteri: _emscripten_glTexParameteri,
  /** @export */
  emscripten_glTexParameteriv: _emscripten_glTexParameteriv,
  /** @export */
  emscripten_glTexStorage2D: _emscripten_glTexStorage2D,
  /** @export */
  emscripten_glTexStorage3D: _emscripten_glTexStorage3D,
  /** @export */
  emscripten_glTexSubImage2D: _emscripten_glTexSubImage2D,
  /** @export */
  emscripten_glTexSubImage3D: _emscripten_glTexSubImage3D,
  /** @export */
  emscripten_glTransformFeedbackVaryings: _emscripten_glTransformFeedbackVaryings,
  /** @export */
  emscripten_glUniform1f: _emscripten_glUniform1f,
  /** @export */
  emscripten_glUniform1fv: _emscripten_glUniform1fv,
  /** @export */
  emscripten_glUniform1i: _emscripten_glUniform1i,
  /** @export */
  emscripten_glUniform1iv: _emscripten_glUniform1iv,
  /** @export */
  emscripten_glUniform1ui: _emscripten_glUniform1ui,
  /** @export */
  emscripten_glUniform1uiv: _emscripten_glUniform1uiv,
  /** @export */
  emscripten_glUniform2f: _emscripten_glUniform2f,
  /** @export */
  emscripten_glUniform2fv: _emscripten_glUniform2fv,
  /** @export */
  emscripten_glUniform2i: _emscripten_glUniform2i,
  /** @export */
  emscripten_glUniform2iv: _emscripten_glUniform2iv,
  /** @export */
  emscripten_glUniform2ui: _emscripten_glUniform2ui,
  /** @export */
  emscripten_glUniform2uiv: _emscripten_glUniform2uiv,
  /** @export */
  emscripten_glUniform3f: _emscripten_glUniform3f,
  /** @export */
  emscripten_glUniform3fv: _emscripten_glUniform3fv,
  /** @export */
  emscripten_glUniform3i: _emscripten_glUniform3i,
  /** @export */
  emscripten_glUniform3iv: _emscripten_glUniform3iv,
  /** @export */
  emscripten_glUniform3ui: _emscripten_glUniform3ui,
  /** @export */
  emscripten_glUniform3uiv: _emscripten_glUniform3uiv,
  /** @export */
  emscripten_glUniform4f: _emscripten_glUniform4f,
  /** @export */
  emscripten_glUniform4fv: _emscripten_glUniform4fv,
  /** @export */
  emscripten_glUniform4i: _emscripten_glUniform4i,
  /** @export */
  emscripten_glUniform4iv: _emscripten_glUniform4iv,
  /** @export */
  emscripten_glUniform4ui: _emscripten_glUniform4ui,
  /** @export */
  emscripten_glUniform4uiv: _emscripten_glUniform4uiv,
  /** @export */
  emscripten_glUniformBlockBinding: _emscripten_glUniformBlockBinding,
  /** @export */
  emscripten_glUniformMatrix2fv: _emscripten_glUniformMatrix2fv,
  /** @export */
  emscripten_glUniformMatrix2x3fv: _emscripten_glUniformMatrix2x3fv,
  /** @export */
  emscripten_glUniformMatrix2x4fv: _emscripten_glUniformMatrix2x4fv,
  /** @export */
  emscripten_glUniformMatrix3fv: _emscripten_glUniformMatrix3fv,
  /** @export */
  emscripten_glUniformMatrix3x2fv: _emscripten_glUniformMatrix3x2fv,
  /** @export */
  emscripten_glUniformMatrix3x4fv: _emscripten_glUniformMatrix3x4fv,
  /** @export */
  emscripten_glUniformMatrix4fv: _emscripten_glUniformMatrix4fv,
  /** @export */
  emscripten_glUniformMatrix4x2fv: _emscripten_glUniformMatrix4x2fv,
  /** @export */
  emscripten_glUniformMatrix4x3fv: _emscripten_glUniformMatrix4x3fv,
  /** @export */
  emscripten_glUseProgram: _emscripten_glUseProgram,
  /** @export */
  emscripten_glValidateProgram: _emscripten_glValidateProgram,
  /** @export */
  emscripten_glVertexAttrib1f: _emscripten_glVertexAttrib1f,
  /** @export */
  emscripten_glVertexAttrib1fv: _emscripten_glVertexAttrib1fv,
  /** @export */
  emscripten_glVertexAttrib2f: _emscripten_glVertexAttrib2f,
  /** @export */
  emscripten_glVertexAttrib2fv: _emscripten_glVertexAttrib2fv,
  /** @export */
  emscripten_glVertexAttrib3f: _emscripten_glVertexAttrib3f,
  /** @export */
  emscripten_glVertexAttrib3fv: _emscripten_glVertexAttrib3fv,
  /** @export */
  emscripten_glVertexAttrib4f: _emscripten_glVertexAttrib4f,
  /** @export */
  emscripten_glVertexAttrib4fv: _emscripten_glVertexAttrib4fv,
  /** @export */
  emscripten_glVertexAttribDivisor: _emscripten_glVertexAttribDivisor,
  /** @export */
  emscripten_glVertexAttribDivisorANGLE: _emscripten_glVertexAttribDivisorANGLE,
  /** @export */
  emscripten_glVertexAttribDivisorARB: _emscripten_glVertexAttribDivisorARB,
  /** @export */
  emscripten_glVertexAttribDivisorEXT: _emscripten_glVertexAttribDivisorEXT,
  /** @export */
  emscripten_glVertexAttribDivisorNV: _emscripten_glVertexAttribDivisorNV,
  /** @export */
  emscripten_glVertexAttribI4i: _emscripten_glVertexAttribI4i,
  /** @export */
  emscripten_glVertexAttribI4iv: _emscripten_glVertexAttribI4iv,
  /** @export */
  emscripten_glVertexAttribI4ui: _emscripten_glVertexAttribI4ui,
  /** @export */
  emscripten_glVertexAttribI4uiv: _emscripten_glVertexAttribI4uiv,
  /** @export */
  emscripten_glVertexAttribIPointer: _emscripten_glVertexAttribIPointer,
  /** @export */
  emscripten_glVertexAttribPointer: _emscripten_glVertexAttribPointer,
  /** @export */
  emscripten_glViewport: _emscripten_glViewport,
  /** @export */
  emscripten_glWaitSync: _emscripten_glWaitSync,
  /** @export */
  emscripten_has_asyncify: _emscripten_has_asyncify,
  /** @export */
  emscripten_request_fullscreen_strategy: _emscripten_request_fullscreen_strategy,
  /** @export */
  emscripten_request_pointerlock: _emscripten_request_pointerlock,
  /** @export */
  emscripten_resize_heap: _emscripten_resize_heap,
  /** @export */
  emscripten_sample_gamepad_data: _emscripten_sample_gamepad_data,
  /** @export */
  emscripten_set_beforeunload_callback_on_thread: _emscripten_set_beforeunload_callback_on_thread,
  /** @export */
  emscripten_set_blur_callback_on_thread: _emscripten_set_blur_callback_on_thread,
  /** @export */
  emscripten_set_canvas_element_size: _emscripten_set_canvas_element_size,
  /** @export */
  emscripten_set_element_css_size: _emscripten_set_element_css_size,
  /** @export */
  emscripten_set_focus_callback_on_thread: _emscripten_set_focus_callback_on_thread,
  /** @export */
  emscripten_set_fullscreenchange_callback_on_thread: _emscripten_set_fullscreenchange_callback_on_thread,
  /** @export */
  emscripten_set_gamepadconnected_callback_on_thread: _emscripten_set_gamepadconnected_callback_on_thread,
  /** @export */
  emscripten_set_gamepaddisconnected_callback_on_thread: _emscripten_set_gamepaddisconnected_callback_on_thread,
  /** @export */
  emscripten_set_keydown_callback_on_thread: _emscripten_set_keydown_callback_on_thread,
  /** @export */
  emscripten_set_keypress_callback_on_thread: _emscripten_set_keypress_callback_on_thread,
  /** @export */
  emscripten_set_keyup_callback_on_thread: _emscripten_set_keyup_callback_on_thread,
  /** @export */
  emscripten_set_main_loop: _emscripten_set_main_loop,
  /** @export */
  emscripten_set_mousedown_callback_on_thread: _emscripten_set_mousedown_callback_on_thread,
  /** @export */
  emscripten_set_mouseenter_callback_on_thread: _emscripten_set_mouseenter_callback_on_thread,
  /** @export */
  emscripten_set_mouseleave_callback_on_thread: _emscripten_set_mouseleave_callback_on_thread,
  /** @export */
  emscripten_set_mousemove_callback_on_thread: _emscripten_set_mousemove_callback_on_thread,
  /** @export */
  emscripten_set_mouseup_callback_on_thread: _emscripten_set_mouseup_callback_on_thread,
  /** @export */
  emscripten_set_pointerlockchange_callback_on_thread: _emscripten_set_pointerlockchange_callback_on_thread,
  /** @export */
  emscripten_set_resize_callback_on_thread: _emscripten_set_resize_callback_on_thread,
  /** @export */
  emscripten_set_touchcancel_callback_on_thread: _emscripten_set_touchcancel_callback_on_thread,
  /** @export */
  emscripten_set_touchend_callback_on_thread: _emscripten_set_touchend_callback_on_thread,
  /** @export */
  emscripten_set_touchmove_callback_on_thread: _emscripten_set_touchmove_callback_on_thread,
  /** @export */
  emscripten_set_touchstart_callback_on_thread: _emscripten_set_touchstart_callback_on_thread,
  /** @export */
  emscripten_set_visibilitychange_callback_on_thread: _emscripten_set_visibilitychange_callback_on_thread,
  /** @export */
  emscripten_set_wheel_callback_on_thread: _emscripten_set_wheel_callback_on_thread,
  /** @export */
  emscripten_set_window_title: _emscripten_set_window_title,
  /** @export */
  emscripten_sleep: _emscripten_sleep,
  /** @export */
  environ_get: _environ_get,
  /** @export */
  environ_sizes_get: _environ_sizes_get,
  /** @export */
  exit: _exit,
  /** @export */
  fd_close: _fd_close,
  /** @export */
  fd_read: _fd_read,
  /** @export */
  fd_seek: _fd_seek,
  /** @export */
  fd_write: _fd_write,
  /** @export */
  glActiveTexture: _glActiveTexture,
  /** @export */
  glAttachShader: _glAttachShader,
  /** @export */
  glBindBuffer: _glBindBuffer,
  /** @export */
  glBindFramebuffer: _glBindFramebuffer,
  /** @export */
  glBindTexture: _glBindTexture,
  /** @export */
  glBindVertexArray: _glBindVertexArray,
  /** @export */
  glBindVertexArrayOES: _glBindVertexArrayOES,
  /** @export */
  glBlendEquation: _glBlendEquation,
  /** @export */
  glBlendEquationSeparate: _glBlendEquationSeparate,
  /** @export */
  glBlendFunc: _glBlendFunc,
  /** @export */
  glBlendFuncSeparate: _glBlendFuncSeparate,
  /** @export */
  glBlitFramebuffer: _glBlitFramebuffer,
  /** @export */
  glBufferData: _glBufferData,
  /** @export */
  glBufferSubData: _glBufferSubData,
  /** @export */
  glCheckFramebufferStatus: _glCheckFramebufferStatus,
  /** @export */
  glClear: _glClear,
  /** @export */
  glClearColor: _glClearColor,
  /** @export */
  glColorMask: _glColorMask,
  /** @export */
  glCompileShader: _glCompileShader,
  /** @export */
  glCreateProgram: _glCreateProgram,
  /** @export */
  glCreateShader: _glCreateShader,
  /** @export */
  glDeleteBuffers: _glDeleteBuffers,
  /** @export */
  glDeleteShader: _glDeleteShader,
  /** @export */
  glDeleteTextures: _glDeleteTextures,
  /** @export */
  glDeleteVertexArrays: _glDeleteVertexArrays,
  /** @export */
  glDeleteVertexArraysOES: _glDeleteVertexArraysOES,
  /** @export */
  glDepthFunc: _glDepthFunc,
  /** @export */
  glDepthMask: _glDepthMask,
  /** @export */
  glDetachShader: _glDetachShader,
  /** @export */
  glDisable: _glDisable,
  /** @export */
  glDrawArrays: _glDrawArrays,
  /** @export */
  glDrawElements: _glDrawElements,
  /** @export */
  glDrawElementsInstanced: _glDrawElementsInstanced,
  /** @export */
  glEnable: _glEnable,
  /** @export */
  glEnableVertexAttribArray: _glEnableVertexAttribArray,
  /** @export */
  glFramebufferTexture2D: _glFramebufferTexture2D,
  /** @export */
  glGenBuffers: _glGenBuffers,
  /** @export */
  glGenFramebuffers: _glGenFramebuffers,
  /** @export */
  glGenTextures: _glGenTextures,
  /** @export */
  glGenVertexArrays: _glGenVertexArrays,
  /** @export */
  glGenVertexArraysOES: _glGenVertexArraysOES,
  /** @export */
  glGenerateMipmap: _glGenerateMipmap,
  /** @export */
  glGetActiveAttrib: _glGetActiveAttrib,
  /** @export */
  glGetActiveUniform: _glGetActiveUniform,
  /** @export */
  glGetAttribLocation: _glGetAttribLocation,
  /** @export */
  glGetFloatv: _glGetFloatv,
  /** @export */
  glGetIntegerv: _glGetIntegerv,
  /** @export */
  glGetProgramInfoLog: _glGetProgramInfoLog,
  /** @export */
  glGetProgramiv: _glGetProgramiv,
  /** @export */
  glGetShaderInfoLog: _glGetShaderInfoLog,
  /** @export */
  glGetShaderiv: _glGetShaderiv,
  /** @export */
  glGetString: _glGetString,
  /** @export */
  glGetUniformLocation: _glGetUniformLocation,
  /** @export */
  glIsEnabled: _glIsEnabled,
  /** @export */
  glIsProgram: _glIsProgram,
  /** @export */
  glLinkProgram: _glLinkProgram,
  /** @export */
  glPixelStorei: _glPixelStorei,
  /** @export */
  glPolygonOffset: _glPolygonOffset,
  /** @export */
  glScissor: _glScissor,
  /** @export */
  glShaderSource: _glShaderSource,
  /** @export */
  glTexImage2D: _glTexImage2D,
  /** @export */
  glTexParameterf: _glTexParameterf,
  /** @export */
  glTexParameteri: _glTexParameteri,
  /** @export */
  glUniform1f: _glUniform1f,
  /** @export */
  glUniform1i: _glUniform1i,
  /** @export */
  glUniform3f: _glUniform3f,
  /** @export */
  glUniform4f: _glUniform4f,
  /** @export */
  glUniformMatrix4fv: _glUniformMatrix4fv,
  /** @export */
  glUseProgram: _glUseProgram,
  /** @export */
  glVertexAttribDivisor: _glVertexAttribDivisor,
  /** @export */
  glVertexAttribIPointer: _glVertexAttribIPointer,
  /** @export */
  glVertexAttribPointer: _glVertexAttribPointer,
  /** @export */
  glViewport: _glViewport,
  /** @export */
  invoke_diii,
  /** @export */
  invoke_fiii,
  /** @export */
  invoke_i,
  /** @export */
  invoke_ii,
  /** @export */
  invoke_iii,
  /** @export */
  invoke_iiii,
  /** @export */
  invoke_iiiii,
  /** @export */
  invoke_iiiiid,
  /** @export */
  invoke_iiiiii,
  /** @export */
  invoke_iiiiiii,
  /** @export */
  invoke_iiiiiiii,
  /** @export */
  invoke_iiiiiiiii,
  /** @export */
  invoke_iiiiiiiiii,
  /** @export */
  invoke_iiiiiiiiiii,
  /** @export */
  invoke_iiiiiiiiiiii,
  /** @export */
  invoke_iiiiiiiiiiiii,
  /** @export */
  invoke_iiiiij,
  /** @export */
  invoke_j,
  /** @export */
  invoke_ji,
  /** @export */
  invoke_jii,
  /** @export */
  invoke_jiiii,
  /** @export */
  invoke_jiji,
  /** @export */
  invoke_v,
  /** @export */
  invoke_vi,
  /** @export */
  invoke_vii,
  /** @export */
  invoke_viii,
  /** @export */
  invoke_viiii,
  /** @export */
  invoke_viiiiiii,
  /** @export */
  invoke_viiiiiiiiii,
  /** @export */
  invoke_viiiiiiiiiiiiiii,
  /** @export */
  invoke_viijii,
  /** @export */
  lock_cursor_js,
  /** @export */
  release_cursor_js
};
var wasmExports;
createWasm();
var ___wasm_call_ctors = createExportWrapper('__wasm_call_ctors', 0);
var _malloc = createExportWrapper('malloc', 1);
var _free = createExportWrapper('free', 1);
var _fflush = createExportWrapper('fflush', 1);
var _main = Module['_main'] = createExportWrapper('__main_argc_argv', 2);
var _strerror = createExportWrapper('strerror', 1);
var _fileno = createExportWrapper('fileno', 1);
var _emscripten_builtin_memalign = createExportWrapper('emscripten_builtin_memalign', 2);
var _setThrew = createExportWrapper('setThrew', 2);
var __emscripten_tempret_set = createExportWrapper('_emscripten_tempret_set', 1);
var _emscripten_stack_init = () => (_emscripten_stack_init = wasmExports['emscripten_stack_init'])();
var _emscripten_stack_get_free = () => (_emscripten_stack_get_free = wasmExports['emscripten_stack_get_free'])();
var _emscripten_stack_get_base = () => (_emscripten_stack_get_base = wasmExports['emscripten_stack_get_base'])();
var _emscripten_stack_get_end = () => (_emscripten_stack_get_end = wasmExports['emscripten_stack_get_end'])();
var __emscripten_stack_restore = (a0) => (__emscripten_stack_restore = wasmExports['_emscripten_stack_restore'])(a0);
var __emscripten_stack_alloc = (a0) => (__emscripten_stack_alloc = wasmExports['_emscripten_stack_alloc'])(a0);
var _emscripten_stack_get_current = () => (_emscripten_stack_get_current = wasmExports['emscripten_stack_get_current'])();
var ___cxa_decrement_exception_refcount = createExportWrapper('__cxa_decrement_exception_refcount', 1);
var ___cxa_increment_exception_refcount = createExportWrapper('__cxa_increment_exception_refcount', 1);
var ___cxa_free_exception = createExportWrapper('__cxa_free_exception', 1);
var ___get_exception_message = createExportWrapper('__get_exception_message', 3);
var ___cxa_can_catch = createExportWrapper('__cxa_can_catch', 3);
var ___cxa_get_exception_ptr = createExportWrapper('__cxa_get_exception_ptr', 1);

function invoke_ji(index,a1) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
    return 0n;
  }
}

function invoke_ii(index,a1) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiii(index,a1,a2,a3,a4) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiii(index,a1,a2,a3) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_vi(index,a1) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_jiji(index,a1,a2,a3) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
    return 0n;
  }
}

function invoke_iii(index,a1,a2) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viii(index,a1,a2,a3) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1,a2,a3);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_vii(index,a1,a2) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1,a2);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4,a5,a6,a7,a8,a9);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiii(index,a1,a2,a3,a4,a5) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4,a5);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4,a5,a6,a7,a8);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_i(index) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)();
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viiii(index,a1,a2,a3,a4) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1,a2,a3,a4);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_v(index) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)();
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_j(index) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)();
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
    return 0n;
  }
}

function invoke_iiiiiii(index,a1,a2,a3,a4,a5,a6) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4,a5,a6);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viijii(index,a1,a2,a3,a4,a5) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1,a2,a3,a4,a5);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiij(index,a1,a2,a3,a4,a5) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4,a5);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiid(index,a1,a2,a3,a4,a5) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4,a5);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiiiii(index,a1,a2,a3,a4,a5,a6,a7) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4,a5,a6,a7);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_jiiii(index,a1,a2,a3,a4) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
    return 0n;
  }
}

function invoke_iiiiiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_fiii(index,a1,a2,a3) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_diii(index,a1,a2,a3) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viiiiiii(index,a1,a2,a3,a4,a5,a6,a7) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1,a2,a3,a4,a5,a6,a7);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_iiiiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_viiiiiiiiiiiiiii(index,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
  }
}

function invoke_jii(index,a1,a2) {
  var sp = stackSave();
  try {
    return getWasmTableEntry(index)(a1,a2);
  } catch(e) {
    stackRestore(sp);
    if (!(e instanceof EmscriptenEH)) throw e;
    _setThrew(1, 0);
    return 0n;
  }
}


// include: postamble.js
// === Auto-generated postamble setup entry stuff ===

Module['addRunDependency'] = addRunDependency;
Module['removeRunDependency'] = removeRunDependency;
Module['FS_createPreloadedFile'] = FS_createPreloadedFile;
Module['FS_unlink'] = FS_unlink;
Module['FS_createPath'] = FS_createPath;
Module['FS_createDevice'] = FS_createDevice;
Module['FS_createDataFile'] = FS_createDataFile;
Module['FS_createLazyFile'] = FS_createLazyFile;
var missingLibrarySymbols = [
  'writeI53ToI64Clamped',
  'writeI53ToI64Signaling',
  'writeI53ToU64Clamped',
  'writeI53ToU64Signaling',
  'convertI32PairToI53',
  'convertI32PairToI53Checked',
  'convertU32PairToI53',
  'getTempRet0',
  'inetPton4',
  'inetNtop4',
  'inetPton6',
  'inetNtop6',
  'readSockaddr',
  'writeSockaddr',
  'emscriptenLog',
  'getDynCaller',
  'runtimeKeepalivePush',
  'runtimeKeepalivePop',
  'asmjsMangle',
  'HandleAllocator',
  'getNativeTypeSize',
  'addOnInit',
  'addOnPostCtor',
  'addOnPreMain',
  'STACK_SIZE',
  'STACK_ALIGN',
  'POINTER_SIZE',
  'ASSERTIONS',
  'getCFunc',
  'ccall',
  'cwrap',
  'uleb128Encode',
  'sigToWasmTypes',
  'generateFuncType',
  'convertJsFunctionToWasm',
  'getEmptyTableSlot',
  'updateTableMap',
  'getFunctionAddress',
  'addFunction',
  'removeFunction',
  'reallyNegative',
  'unSign',
  'strLen',
  'reSign',
  'formatString',
  'intArrayToString',
  'AsciiToString',
  'UTF16ToString',
  'stringToUTF16',
  'lengthBytesUTF16',
  'UTF32ToString',
  'stringToUTF32',
  'lengthBytesUTF32',
  'writeArrayToMemory',
  'fillDeviceOrientationEventData',
  'registerDeviceOrientationEventCallback',
  'fillDeviceMotionEventData',
  'registerDeviceMotionEventCallback',
  'screenOrientation',
  'fillOrientationChangeEventData',
  'registerOrientationChangeEventCallback',
  'registerPointerlockErrorEventCallback',
  'fillBatteryEventData',
  'battery',
  'registerBatteryEventCallback',
  'jsStackTrace',
  'getCallstack',
  'convertPCtoSourceLocation',
  'wasiRightsToMuslOFlags',
  'wasiOFlagsToMuslOFlags',
  'setImmediateWrapped',
  'safeRequestAnimationFrame',
  'clearImmediateWrapped',
  'registerPostMainLoop',
  'registerPreMainLoop',
  'getPromise',
  'makePromise',
  'idsToPromises',
  'makePromiseCallback',
  'Browser_asyncPrepareDataCounter',
  'isLeapYear',
  'ydayFromDate',
  'arraySum',
  'addDays',
  'getSocketFromFD',
  'getSocketAddress',
  'FS_mkdirTree',
  '_setNetworkCallback',
  'writeGLArray',
  'registerWebGlEventCallback',
  'runAndAbortIfError',
  'ALLOC_NORMAL',
  'ALLOC_STACK',
  'allocate',
  'writeStringToMemory',
  'writeAsciiToMemory',
  'demangle',
  'stackTrace',
];
missingLibrarySymbols.forEach(missingLibrarySymbol)

var unexportedSymbols = [
  'run',
  'out',
  'err',
  'callMain',
  'abort',
  'wasmMemory',
  'wasmExports',
  'writeStackCookie',
  'checkStackCookie',
  'writeI53ToI64',
  'readI53FromI64',
  'readI53FromU64',
  'INT53_MAX',
  'INT53_MIN',
  'bigintToI53Checked',
  'stackSave',
  'stackRestore',
  'stackAlloc',
  'setTempRet0',
  'ptrToString',
  'zeroMemory',
  'exitJS',
  'getHeapMax',
  'growMemory',
  'ENV',
  'ERRNO_CODES',
  'strError',
  'DNS',
  'Protocols',
  'Sockets',
  'timers',
  'warnOnce',
  'readEmAsmArgsArray',
  'readEmAsmArgs',
  'runEmAsmFunction',
  'runMainThreadEmAsm',
  'jstoi_q',
  'jstoi_s',
  'getExecutableName',
  'listenOnce',
  'autoResumeAudioContext',
  'dynCall',
  'handleException',
  'keepRuntimeAlive',
  'callUserCallback',
  'maybeExit',
  'asyncLoad',
  'alignMemory',
  'mmapAlloc',
  'wasmTable',
  'noExitRuntime',
  'addOnPreRun',
  'addOnExit',
  'addOnPostRun',
  'freeTableIndexes',
  'functionsInTableMap',
  'setValue',
  'getValue',
  'PATH',
  'PATH_FS',
  'UTF8Decoder',
  'UTF8ArrayToString',
  'UTF8ToString',
  'stringToUTF8Array',
  'stringToUTF8',
  'lengthBytesUTF8',
  'intArrayFromString',
  'stringToAscii',
  'UTF16Decoder',
  'stringToNewUTF8',
  'stringToUTF8OnStack',
  'JSEvents',
  'registerKeyEventCallback',
  'specialHTMLTargets',
  'maybeCStringToJsString',
  'findEventTarget',
  'findCanvasEventTarget',
  'getBoundingClientRect',
  'fillMouseEventData',
  'registerMouseEventCallback',
  'registerWheelEventCallback',
  'registerUiEventCallback',
  'registerFocusEventCallback',
  'fillFullscreenChangeEventData',
  'registerFullscreenChangeEventCallback',
  'JSEvents_requestFullscreen',
  'JSEvents_resizeCanvasForFullscreen',
  'registerRestoreOldStyle',
  'hideEverythingExceptGivenElement',
  'restoreHiddenElements',
  'setLetterbox',
  'currentFullscreenStrategy',
  'restoreOldWindowedStyle',
  'softFullscreenResizeWebGLRenderTarget',
  'doRequestFullscreen',
  'fillPointerlockChangeEventData',
  'registerPointerlockChangeEventCallback',
  'requestPointerLock',
  'fillVisibilityChangeEventData',
  'registerVisibilityChangeEventCallback',
  'registerTouchEventCallback',
  'fillGamepadEventData',
  'registerGamepadEventCallback',
  'registerBeforeUnloadEventCallback',
  'setCanvasElementSize',
  'getCanvasElementSize',
  'UNWIND_CACHE',
  'ExitStatus',
  'getEnvStrings',
  'checkWasiClock',
  'doReadv',
  'doWritev',
  'initRandomFill',
  'randomFill',
  'safeSetTimeout',
  'emSetImmediate',
  'emClearImmediate_deps',
  'emClearImmediate',
  'promiseMap',
  'uncaughtExceptionCount',
  'exceptionLast',
  'exceptionCaught',
  'ExceptionInfo',
  'findMatchingCatch',
  'getExceptionMessageCommon',
  'incrementExceptionRefcount',
  'decrementExceptionRefcount',
  'getExceptionMessage',
  'Browser',
  'getPreloadedImageData__data',
  'wget',
  'MONTH_DAYS_REGULAR',
  'MONTH_DAYS_LEAP',
  'MONTH_DAYS_REGULAR_CUMULATIVE',
  'MONTH_DAYS_LEAP_CUMULATIVE',
  'SYSCALLS',
  'preloadPlugins',
  'FS_modeStringToFlags',
  'FS_getMode',
  'FS_stdin_getChar_buffer',
  'FS_stdin_getChar',
  'FS_readFile',
  'FS',
  'MEMFS',
  'TTY',
  'PIPEFS',
  'SOCKFS',
  'tempFixedLengthArray',
  'miniTempWebGLFloatBuffers',
  'miniTempWebGLIntBuffers',
  'heapObjectForWebGLType',
  'toTypedArrayIndex',
  'webgl_enable_WEBGL_multi_draw',
  'webgl_enable_EXT_polygon_offset_clamp',
  'webgl_enable_EXT_clip_control',
  'webgl_enable_WEBGL_polygon_mode',
  'GL',
  'emscriptenWebGLGet',
  'computeUnpackAlignedImageSize',
  'colorChannelsInGlTextureFormat',
  'emscriptenWebGLGetTexPixelData',
  'emscriptenWebGLGetUniform',
  'webglGetUniformLocation',
  'webglPrepareUniformLocationsBeforeFirstUse',
  'webglGetLeftBracePos',
  'emscriptenWebGLGetVertexAttrib',
  '__glGetActiveAttribOrUniform',
  'AL',
  'GLUT',
  'EGL',
  'GLEW',
  'IDBStore',
  'emscriptenWebGLGetIndexed',
  'webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance',
  'webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance',
  'allocateUTF8',
  'allocateUTF8OnStack',
  'print',
  'printErr',
  'IDBFS',
];
unexportedSymbols.forEach(unexportedRuntimeSymbol);



var calledRun;

function callMain(args = []) {
  assert(runDependencies == 0, 'cannot call main when async dependencies remain! (listen on Module["onRuntimeInitialized"])');
  assert(typeof onPreRuns === 'undefined' || onPreRuns.length == 0, 'cannot call main when preRun functions remain to be called');

  var entryFunction = _main;

  args.unshift(thisProgram);

  var argc = args.length;
  var argv = stackAlloc((argc + 1) * 4);
  var argv_ptr = argv;
  args.forEach((arg) => {
    HEAPU32[((argv_ptr)>>2)] = stringToUTF8OnStack(arg);
    argv_ptr += 4;
  });
  HEAPU32[((argv_ptr)>>2)] = 0;

  try {

    var ret = entryFunction(argc, argv);

    // if we're not running an evented main loop, it's time to exit
    exitJS(ret, /* implicit = */ true);
    return ret;
  } catch (e) {
    return handleException(e);
  }
}

function stackCheckInit() {
  // This is normally called automatically during __wasm_call_ctors but need to
  // get these values before even running any of the ctors so we call it redundantly
  // here.
  _emscripten_stack_init();
  // TODO(sbc): Move writeStackCookie to native to to avoid this.
  writeStackCookie();
}

function run(args = arguments_) {

  if (runDependencies > 0) {
    dependenciesFulfilled = run;
    return;
  }

  stackCheckInit();

  preRun();

  // a preRun added a dependency, run will be called later
  if (runDependencies > 0) {
    dependenciesFulfilled = run;
    return;
  }

  function doRun() {
    // run may have just been called through dependencies being fulfilled just in this very frame,
    // or while the async setStatus time below was happening
    assert(!calledRun);
    calledRun = true;
    Module['calledRun'] = true;

    if (ABORT) return;

    initRuntime();

    preMain();

    Module['onRuntimeInitialized']?.();
    consumedModuleProp('onRuntimeInitialized');

    var noInitialRun = Module['noInitialRun'];legacyModuleProp('noInitialRun', 'noInitialRun');
    if (!noInitialRun) callMain(args);

    postRun();
  }

  if (Module['setStatus']) {
    Module['setStatus']('Running...');
    setTimeout(() => {
      setTimeout(() => Module['setStatus'](''), 1);
      doRun();
    }, 1);
  } else
  {
    doRun();
  }
  checkStackCookie();
}

function checkUnflushedContent() {
  // Compiler settings do not allow exiting the runtime, so flushing
  // the streams is not possible. but in ASSERTIONS mode we check
  // if there was something to flush, and if so tell the user they
  // should request that the runtime be exitable.
  // Normally we would not even include flush() at all, but in ASSERTIONS
  // builds we do so just for this check, and here we see if there is any
  // content to flush, that is, we check if there would have been
  // something a non-ASSERTIONS build would have not seen.
  // How we flush the streams depends on whether we are in SYSCALLS_REQUIRE_FILESYSTEM=0
  // mode (which has its own special function for this; otherwise, all
  // the code is inside libc)
  var oldOut = out;
  var oldErr = err;
  var has = false;
  out = err = (x) => {
    has = true;
  }
  try { // it doesn't matter if it fails
    _fflush(0);
    // also flush in the JS FS layer
    ['stdout', 'stderr'].forEach((name) => {
      var info = FS.analyzePath('/dev/' + name);
      if (!info) return;
      var stream = info.object;
      var rdev = stream.rdev;
      var tty = TTY.ttys[rdev];
      if (tty?.output?.length) {
        has = true;
      }
    });
  } catch(e) {}
  out = oldOut;
  err = oldErr;
  if (has) {
    warnOnce('stdio streams had content in them that was not flushed. you should set EXIT_RUNTIME to 1 (see the Emscripten FAQ), or make sure to emit a newline when you printf etc.');
  }
}

if (Module['preInit']) {
  if (typeof Module['preInit'] == 'function') Module['preInit'] = [Module['preInit']];
  while (Module['preInit'].length > 0) {
    Module['preInit'].pop()();
  }
}
consumedModuleProp('preInit');

run();

// end include: postamble.js

