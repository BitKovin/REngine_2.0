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
// include: C:\Users\Admin\AppData\Local\Temp\tmpewrflgvk.js

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
    loadPackage({"files": [{"filename": "/GameData/Fonts/Kingthings_Calligraphica_2.ttf", "start": 0, "end": 29804}, {"filename": "/GameData/Maps/autosave/test2.1.map", "start": 29804, "end": 686372}, {"filename": "/GameData/Maps/e1m1.map", "start": 686372, "end": 10697901}, {"filename": "/GameData/Maps/e1m1.obj", "start": 10697901, "end": 20542922}, {"filename": "/GameData/Maps/test.map", "start": 20542922, "end": 20558856}, {"filename": "/GameData/Maps/test.mtl", "start": 20558856, "end": 20559133}, {"filename": "/GameData/Maps/test.obj", "start": 20559133, "end": 20577653}, {"filename": "/GameData/Maps/test2.map", "start": 20577653, "end": 21235528}, {"filename": "/GameData/Maps/test2.mtl", "start": 21235528, "end": 21236223}, {"filename": "/GameData/Maps/test2.obj", "start": 21236223, "end": 22139520}, {"filename": "/GameData/Shaders/default_pixel.frag", "start": 22139520, "end": 22145729}, {"filename": "/GameData/Shaders/default_vertex.vert", "start": 22145729, "end": 22147932}, {"filename": "/GameData/Shaders/empty_pixel.frag", "start": 22147932, "end": 22147999}, {"filename": "/GameData/Shaders/fullscreen_vertex.vert", "start": 22147999, "end": 22148203}, {"filename": "/GameData/Shaders/instanced_bilboard_vertex.vert", "start": 22148203, "end": 22150142}, {"filename": "/GameData/Shaders/solidRed_pixel.frag", "start": 22150142, "end": 22150311}, {"filename": "/GameData/Shaders/texture_pixel.frag", "start": 22150311, "end": 22150499}, {"filename": "/GameData/Shaders/ui.vert", "start": 22150499, "end": 22150802}, {"filename": "/GameData/Shaders/ui_flatcolor.frag", "start": 22150802, "end": 22150938}, {"filename": "/GameData/Shaders/ui_textured.frag", "start": 22150938, "end": 22151195}, {"filename": "/GameData/Sounds/Dog/Death.wav", "start": 22151195, "end": 22224581, "audio": 1}, {"filename": "/GameData/Sounds/mew.wav", "start": 22224581, "end": 22297967, "audio": 1}, {"filename": "/GameData/Textures/M_Shotgun_Base_Color.png", "start": 22297967, "end": 22673109}, {"filename": "/GameData/Textures/arms.png", "start": 22673109, "end": 22713353}, {"filename": "/GameData/Textures/brushes/Ground/grass.png", "start": 22713353, "end": 22717137}, {"filename": "/GameData/Textures/brushes/Metal/metal1.png", "start": 22717137, "end": 23141193}, {"filename": "/GameData/Textures/brushes/Wall/brickWall1.png", "start": 23141193, "end": 23149409}, {"filename": "/GameData/Textures/brushes/Wall/brickWall2.png", "start": 23149409, "end": 23157533}, {"filename": "/GameData/Textures/brushes/Wall/brickWall3.png", "start": 23157533, "end": 23165416}, {"filename": "/GameData/Textures/brushes/Water/Water1_t.png", "start": 23165416, "end": 23478517}, {"filename": "/GameData/Textures/brushes/Wood/wood1.png", "start": 23478517, "end": 23826869}, {"filename": "/GameData/Textures/brushes/__TB_empty.png", "start": 23826869, "end": 23827627}, {"filename": "/GameData/Textures/brushes/brick.png", "start": 23827627, "end": 24340541}, {"filename": "/GameData/Textures/brushes/brickPBR.png", "start": 24340541, "end": 25270673}, {"filename": "/GameData/Textures/brushes/brickPBR_orm.png", "start": 25270673, "end": 25695524}, {"filename": "/GameData/Textures/brushes/bricks.png", "start": 25695524, "end": 25706444}, {"filename": "/GameData/Textures/brushes/cat.png", "start": 25706444, "end": 25955882}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door1a.png", "start": 25955882, "end": 25963666}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door1b.png", "start": 25963666, "end": 25972856}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door1c.png", "start": 25972856, "end": 25980707}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door1d.png", "start": 25980707, "end": 25989879}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door2a.png", "start": 25989879, "end": 25997603}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door2b.png", "start": 25997603, "end": 26006728}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door2c.png", "start": 26006728, "end": 26014756}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door2d.png", "start": 26014756, "end": 26023984}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door3a.png", "start": 26023984, "end": 26028132}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door3b.png", "start": 26028132, "end": 26032910}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door3c.png", "start": 26032910, "end": 26037060}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door3d.png", "start": 26037060, "end": 26041852}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door4a.png", "start": 26041852, "end": 26045973}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door4b.png", "start": 26045973, "end": 26050764}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door4c.png", "start": 26050764, "end": 26055034}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door4d.png", "start": 26055034, "end": 26059870}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric1a.png", "start": 26059870, "end": 26065246}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric1b.png", "start": 26065246, "end": 26071374}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric1c.png", "start": 26071374, "end": 26077668}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric1d.png", "start": 26077668, "end": 26081304}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric1e.png", "start": 26081304, "end": 26084990}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric2a.png", "start": 26084990, "end": 26089715}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric2b.png", "start": 26089715, "end": 26095166}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric2c.png", "start": 26095166, "end": 26100844}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric2d.png", "start": 26100844, "end": 26104130}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric2e.png", "start": 26104130, "end": 26107491}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric3a.png", "start": 26107491, "end": 26112445}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric3b.png", "start": 26112445, "end": 26118116}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric3c.png", "start": 26118116, "end": 26123941}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric3d.png", "start": 26123941, "end": 26127321}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric3e.png", "start": 26127321, "end": 26130763}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric4a.png", "start": 26130763, "end": 26140181}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric4b.png", "start": 26140181, "end": 26150146}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric4c.png", "start": 26150146, "end": 26160275}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric4d.png", "start": 26160275, "end": 26165799}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric4e.png", "start": 26165799, "end": 26171369}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric5a.png", "start": 26171369, "end": 26180436}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric5b.png", "start": 26180436, "end": 26190083}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric5c.png", "start": 26190083, "end": 26199883}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric5d.png", "start": 26199883, "end": 26205293}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric5e.png", "start": 26205293, "end": 26210727}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric6a.png", "start": 26210727, "end": 26217861}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric6b.png", "start": 26217861, "end": 26225589}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric6c.png", "start": 26225589, "end": 26233484}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric6d.png", "start": 26233484, "end": 26237884}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric6e.png", "start": 26237884, "end": 26242342}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground1a.png", "start": 26242342, "end": 26250264}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground1b.png", "start": 26250264, "end": 26259100}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground1c.png", "start": 26259100, "end": 26268596}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground1d.png", "start": 26268596, "end": 26278639}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground2a.png", "start": 26278639, "end": 26288507}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground2b.png", "start": 26288507, "end": 26297414}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground2c.png", "start": 26297414, "end": 26307139}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground2d.png", "start": 26307139, "end": 26317560}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground3a.png", "start": 26317560, "end": 26327619}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground3b.png", "start": 26327619, "end": 26337331}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground3c.png", "start": 26337331, "end": 26346767}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground4a.png", "start": 26346767, "end": 26356958}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground4b.png", "start": 26356958, "end": 26367408}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground4c.png", "start": 26367408, "end": 26377349}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litmetal1a.png", "start": 26377349, "end": 26381377}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litmetal1b.png", "start": 26381377, "end": 26386089}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litmetal1c.png", "start": 26386089, "end": 26390170}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litmetal2a.png", "start": 26390170, "end": 26394223}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litmetal2b.png", "start": 26394223, "end": 26398982}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litmetal2c.png", "start": 26398982, "end": 26402853}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litstone1a.png", "start": 26402853, "end": 26406231}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litstone1b.png", "start": 26406231, "end": 26410444}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litstone1c.png", "start": 26410444, "end": 26414075}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litstone2a.png", "start": 26414075, "end": 26418872}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litstone2b.png", "start": 26418872, "end": 26424398}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litstone2c.png", "start": 26424398, "end": 26428641}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalgen1.png", "start": 26428641, "end": 26434466}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalgen2.png", "start": 26434466, "end": 26441346}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan1a.png", "start": 26441346, "end": 26449404}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan1b.png", "start": 26449404, "end": 26456874}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan1c.png", "start": 26456874, "end": 26466076}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan2a.png", "start": 26466076, "end": 26474403}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan2b.png", "start": 26474403, "end": 26481815}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan2c.png", "start": 26481815, "end": 26491014}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan3a.png", "start": 26491014, "end": 26499621}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan3b.png", "start": 26499621, "end": 26507848}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan3c.png", "start": 26507848, "end": 26517150}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan4a.png", "start": 26517150, "end": 26525911}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan4b.png", "start": 26525911, "end": 26534236}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan4c.png", "start": 26534236, "end": 26543691}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip1a.png", "start": 26543691, "end": 26550335}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip1b.png", "start": 26550335, "end": 26556847}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip2a.png", "start": 26556847, "end": 26563980}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip2b.png", "start": 26563980, "end": 26570998}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip3a.png", "start": 26570998, "end": 26578269}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip3b.png", "start": 26578269, "end": 26585347}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip4a.png", "start": 26585347, "end": 26592915}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip4b.png", "start": 26592915, "end": 26600270}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm1a.png", "start": 26600270, "end": 26607268}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm1b.png", "start": 26607268, "end": 26613801}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm1c.png", "start": 26613801, "end": 26621989}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm2a.png", "start": 26621989, "end": 26629486}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm2b.png", "start": 26629486, "end": 26636074}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm2c.png", "start": 26636074, "end": 26644335}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm3a.png", "start": 26644335, "end": 26651611}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm3b.png", "start": 26651611, "end": 26658537}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm3c.png", "start": 26658537, "end": 26666532}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm4a.png", "start": 26666532, "end": 26673412}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm4b.png", "start": 26673412, "end": 26680120}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm4c.png", "start": 26680120, "end": 26687929}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm5.png", "start": 26687929, "end": 26691588}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm6.png", "start": 26691588, "end": 26695311}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr1a.png", "start": 26695311, "end": 26702103}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr1b.png", "start": 26702103, "end": 26710474}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr1c.png", "start": 26710474, "end": 26718144}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr1d.png", "start": 26718144, "end": 26726261}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr1e.png", "start": 26726261, "end": 26734153}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr2a.png", "start": 26734153, "end": 26742344}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr2b.png", "start": 26742344, "end": 26751856}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr2c.png", "start": 26751856, "end": 26760758}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr2d.png", "start": 26760758, "end": 26770009}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr2e.png", "start": 26770009, "end": 26779080}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr3a.png", "start": 26779080, "end": 26787211}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr3b.png", "start": 26787211, "end": 26796921}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr3c.png", "start": 26796921, "end": 26805893}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr3d.png", "start": 26805893, "end": 26815367}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr3e.png", "start": 26815367, "end": 26824594}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr4a.png", "start": 26824594, "end": 26833100}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr4b.png", "start": 26833100, "end": 26841560}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr4c.png", "start": 26841560, "end": 26850224}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr5a.png", "start": 26850224, "end": 26857403}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr5b.png", "start": 26857403, "end": 26864531}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr5c.png", "start": 26864531, "end": 26872671}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr6a.png", "start": 26872671, "end": 26881347}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr6b.png", "start": 26881347, "end": 26889762}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr6c.png", "start": 26889762, "end": 26898495}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slategen1.png", "start": 26898495, "end": 26904456}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slategen2.png", "start": 26904456, "end": 26912015}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk1a.png", "start": 26912015, "end": 26917965}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk1b.png", "start": 26917965, "end": 26923781}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk1c.png", "start": 26923781, "end": 26929725}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk1d.png", "start": 26929725, "end": 26935371}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk2a.png", "start": 26935371, "end": 26944243}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk2b.png", "start": 26944243, "end": 26952917}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk2c.png", "start": 26952917, "end": 26961714}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk2d.png", "start": 26961714, "end": 26970010}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk3a.png", "start": 26970010, "end": 26978457}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk3b.png", "start": 26978457, "end": 26987104}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk4a.png", "start": 26987104, "end": 26993723}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk4b.png", "start": 26993723, "end": 27000409}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk4c.png", "start": 27000409, "end": 27007026}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk4d.png", "start": 27007026, "end": 27013640}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk4e.png", "start": 27013640, "end": 27020224}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk4f.png", "start": 27020224, "end": 27026581}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk5a.png", "start": 27026581, "end": 27036058}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk5b.png", "start": 27036058, "end": 27045570}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk5c.png", "start": 27045570, "end": 27055068}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk5d.png", "start": 27055068, "end": 27064541}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk5e.png", "start": 27064541, "end": 27073932}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk5f.png", "start": 27073932, "end": 27083069}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk6a.png", "start": 27083069, "end": 27091994}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk6b.png", "start": 27091994, "end": 27101219}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr1a.png", "start": 27101219, "end": 27106649}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr1b.png", "start": 27106649, "end": 27113071}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr1c.png", "start": 27113071, "end": 27120094}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr2a.png", "start": 27120094, "end": 27127982}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr2b.png", "start": 27127982, "end": 27136797}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr2c.png", "start": 27136797, "end": 27146078}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr3a.png", "start": 27146078, "end": 27151781}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr3b.png", "start": 27151781, "end": 27158259}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr3c.png", "start": 27158259, "end": 27165251}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr4a.png", "start": 27165251, "end": 27173643}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr4b.png", "start": 27173643, "end": 27182712}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr4c.png", "start": 27182712, "end": 27191812}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr5a.png", "start": 27191812, "end": 27197902}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr5b.png", "start": 27197902, "end": 27204315}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr5c.png", "start": 27204315, "end": 27211233}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr6a.png", "start": 27211233, "end": 27219778}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr6b.png", "start": 27219778, "end": 27228598}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr6c.png", "start": 27228598, "end": 27237882}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonegen1.png", "start": 27237882, "end": 27242976}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonegen2.png", "start": 27242976, "end": 27250406}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonestep1a.png", "start": 27250406, "end": 27255849}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonestep1b.png", "start": 27255849, "end": 27261389}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonestep2a.png", "start": 27261389, "end": 27269238}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonestep2b.png", "start": 27269238, "end": 27276988}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm1a.png", "start": 27276988, "end": 27282278}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm1b.png", "start": 27282278, "end": 27288378}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm1c.png", "start": 27288378, "end": 27293954}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm2a.png", "start": 27293954, "end": 27301700}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm2b.png", "start": 27301700, "end": 27310530}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm2c.png", "start": 27310530, "end": 27318838}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm3a.png", "start": 27318838, "end": 27324138}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm3b.png", "start": 27324138, "end": 27330211}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm4a.png", "start": 27330211, "end": 27338033}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm4b.png", "start": 27338033, "end": 27346564}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm5.png", "start": 27346564, "end": 27349721}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm6.png", "start": 27349721, "end": 27354076}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonewaf1a.png", "start": 27354076, "end": 27359687}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonewaf1b.png", "start": 27359687, "end": 27365304}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonewaf2a.png", "start": 27365304, "end": 27373177}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonewaf2b.png", "start": 27373177, "end": 27381286}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood1a.png", "start": 27381286, "end": 27385985}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood1b.png", "start": 27385985, "end": 27390944}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood2a.png", "start": 27390944, "end": 27397640}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood2b.png", "start": 27397640, "end": 27404597}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood3a.png", "start": 27404597, "end": 27409801}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood3b.png", "start": 27409801, "end": 27415259}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood4a.png", "start": 27415259, "end": 27422091}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood4b.png", "start": 27422091, "end": 27429136}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5a.png", "start": 27429136, "end": 27435149}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5b.png", "start": 27435149, "end": 27441984}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5c.png", "start": 27441984, "end": 27448994}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5d.png", "start": 27448994, "end": 27456584}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5e.png", "start": 27456584, "end": 27465717}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5f.png", "start": 27465717, "end": 27474746}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5g.png", "start": 27474746, "end": 27483480}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5h.png", "start": 27483480, "end": 27492208}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6a.png", "start": 27492208, "end": 27499838}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6b.png", "start": 27499838, "end": 27508286}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6c.png", "start": 27508286, "end": 27515854}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6d.png", "start": 27515854, "end": 27524251}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6e.png", "start": 27524251, "end": 27534008}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6f.png", "start": 27534008, "end": 27543954}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6g.png", "start": 27543954, "end": 27553157}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6h.png", "start": 27553157, "end": 27562596}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_woodgen1.png", "start": 27562596, "end": 27567143}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_woodgen2.png", "start": 27567143, "end": 27573836}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry1a.png", "start": 27573836, "end": 27581623}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry1b.png", "start": 27581623, "end": 27588770}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry1c.png", "start": 27588770, "end": 27593040}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry1d.png", "start": 27593040, "end": 27597050}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry2a.png", "start": 27597050, "end": 27604189}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry2b.png", "start": 27604189, "end": 27610748}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry2c.png", "start": 27610748, "end": 27614569}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry2d.png", "start": 27614569, "end": 27618207}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry3a.png", "start": 27618207, "end": 27625336}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry3b.png", "start": 27625336, "end": 27631922}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry3c.png", "start": 27631922, "end": 27635895}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry3d.png", "start": 27635895, "end": 27639667}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry4a.png", "start": 27639667, "end": 27650549}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry4b.png", "start": 27650549, "end": 27659387}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry4c.png", "start": 27659387, "end": 27664889}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry4d.png", "start": 27664889, "end": 27669599}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry5a.png", "start": 27669599, "end": 27680183}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry5b.png", "start": 27680183, "end": 27688809}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry5c.png", "start": 27688809, "end": 27694118}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry5d.png", "start": 27694118, "end": 27698668}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry6a.png", "start": 27698668, "end": 27707517}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry6b.png", "start": 27707517, "end": 27714795}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry6c.png", "start": 27714795, "end": 27719413}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry6d.png", "start": 27719413, "end": 27723425}, {"filename": "/GameData/Textures/brushes/foil.png", "start": 27723425, "end": 27980643}, {"filename": "/GameData/Textures/brushes/grass.png", "start": 27980643, "end": 28108768}, {"filename": "/GameData/Textures/brushes/hole_t.png", "start": 28108768, "end": 28111197}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_1.png", "start": 28111197, "end": 28124310}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_10.png", "start": 28124310, "end": 28138424}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_2.png", "start": 28138424, "end": 28152465}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_3.png", "start": 28152465, "end": 28166975}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_4.png", "start": 28166975, "end": 28181361}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_5.png", "start": 28181361, "end": 28195810}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_6.png", "start": 28195810, "end": 28210427}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_7.png", "start": 28210427, "end": 28225024}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_8.png", "start": 28225024, "end": 28239372}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_9.png", "start": 28239372, "end": 28253530}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_a1.png", "start": 28253530, "end": 28267978}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_1.png", "start": 28267978, "end": 28277406}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_10.png", "start": 28277406, "end": 28286855}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_2.png", "start": 28286855, "end": 28296249}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_3.png", "start": 28296249, "end": 28305894}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_4.png", "start": 28305894, "end": 28315401}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_5.png", "start": 28315401, "end": 28324906}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_6.png", "start": 28324906, "end": 28334419}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_7.png", "start": 28334419, "end": 28343964}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_8.png", "start": 28343964, "end": 28353374}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_9.png", "start": 28353374, "end": 28362911}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_a1.png", "start": 28362911, "end": 28371941}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_1.png", "start": 28371941, "end": 28379805}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_10.png", "start": 28379805, "end": 28388741}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_2.png", "start": 28388741, "end": 28397135}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_3.png", "start": 28397135, "end": 28407362}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_4.png", "start": 28407362, "end": 28416641}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_5.png", "start": 28416641, "end": 28426371}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_6.png", "start": 28426371, "end": 28435931}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_7.png", "start": 28435931, "end": 28444756}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_8.png", "start": 28444756, "end": 28453537}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_9.png", "start": 28453537, "end": 28463013}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_a1.png", "start": 28463013, "end": 28471420}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_1.png", "start": 28471420, "end": 28477579}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_10.png", "start": 28477579, "end": 28483939}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_2.png", "start": 28483939, "end": 28490204}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_3.png", "start": 28490204, "end": 28497130}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_4.png", "start": 28497130, "end": 28503620}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_5.png", "start": 28503620, "end": 28510213}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_6.png", "start": 28510213, "end": 28516737}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_7.png", "start": 28516737, "end": 28523145}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_8.png", "start": 28523145, "end": 28529428}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_9.png", "start": 28529428, "end": 28536011}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_a1.png", "start": 28536011, "end": 28542080}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc5_1.png", "start": 28542080, "end": 28550842}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc5_10.png", "start": 28550842, "end": 28560457}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc5_2.png", "start": 28560457, "end": 28569773}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc5_8.png", "start": 28569773, "end": 28578853}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc5_9.png", "start": 28578853, "end": 28588567}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc5_a1.png", "start": 28588567, "end": 28596942}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_1.png", "start": 28596942, "end": 28603355}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_10.png", "start": 28603355, "end": 28609764}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_2.png", "start": 28609764, "end": 28616092}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_3.png", "start": 28616092, "end": 28622953}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_4.png", "start": 28622953, "end": 28629561}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_5.png", "start": 28629561, "end": 28636237}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_6.png", "start": 28636237, "end": 28643011}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_7.png", "start": 28643011, "end": 28649714}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_8.png", "start": 28649714, "end": 28656210}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_9.png", "start": 28656210, "end": 28662832}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_a1.png", "start": 28662832, "end": 28669476}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_1.png", "start": 28669476, "end": 28677266}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_10.png", "start": 28677266, "end": 28684944}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_2.png", "start": 28684944, "end": 28692570}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_3.png", "start": 28692570, "end": 28700514}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_4.png", "start": 28700514, "end": 28708282}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_5.png", "start": 28708282, "end": 28716106}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_6.png", "start": 28716106, "end": 28724105}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_7.png", "start": 28724105, "end": 28732085}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_8.png", "start": 28732085, "end": 28739909}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_9.png", "start": 28739909, "end": 28747709}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_a1.png", "start": 28747709, "end": 28755442}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr1_1.png", "start": 28755442, "end": 28757774}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr1_2.png", "start": 28757774, "end": 28761038}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr1_3.png", "start": 28761038, "end": 28764578}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr1_4.png", "start": 28764578, "end": 28767355}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr1_5.png", "start": 28767355, "end": 28770951}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr1_6.png", "start": 28770951, "end": 28774007}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_1.png", "start": 28774007, "end": 28776117}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_2.png", "start": 28776117, "end": 28779330}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_3.png", "start": 28779330, "end": 28782950}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_4.png", "start": 28782950, "end": 28786348}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_5.png", "start": 28786348, "end": 28789708}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_6.png", "start": 28789708, "end": 28792907}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_7.png", "start": 28792907, "end": 28796042}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_8.png", "start": 28796042, "end": 28798983}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/clip.png", "start": 28798983, "end": 28799489}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_blue_a.png", "start": 28799489, "end": 28800045}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_blue_b.png", "start": 28800045, "end": 28800600}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_blue_c.png", "start": 28800600, "end": 28801155}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_brown_a.png", "start": 28801155, "end": 28801710}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_brown_b.png", "start": 28801710, "end": 28802265}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_brown_c.png", "start": 28802265, "end": 28802820}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_green_a.png", "start": 28802820, "end": 28803376}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_green_b.png", "start": 28803376, "end": 28803931}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_green_c.png", "start": 28803931, "end": 28804486}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_grey_a.png", "start": 28804486, "end": 28805039}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_grey_b.png", "start": 28805039, "end": 28805595}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_grey_c.png", "start": 28805595, "end": 28806150}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_olive_a.png", "start": 28806150, "end": 28806705}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_olive_b.png", "start": 28806705, "end": 28807259}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_olive_c.png", "start": 28807259, "end": 28807814}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_orange_a.png", "start": 28807814, "end": 28808370}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_orange_b.png", "start": 28808370, "end": 28808925}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_orange_c.png", "start": 28808925, "end": 28809480}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_pink_a.png", "start": 28809480, "end": 28810036}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_pink_b.png", "start": 28810036, "end": 28810592}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_pink_c.png", "start": 28810592, "end": 28811147}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_purple_a.png", "start": 28811147, "end": 28811703}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_purple_b.png", "start": 28811703, "end": 28812258}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_purple_c.png", "start": 28812258, "end": 28812813}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_red_a.png", "start": 28812813, "end": 28813364}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_red_b.png", "start": 28813364, "end": 28813915}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_red_c.png", "start": 28813915, "end": 28814466}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_tan_a.png", "start": 28814466, "end": 28815022}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_tan_b.png", "start": 28815022, "end": 28815578}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_tan_c.png", "start": 28815578, "end": 28816133}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_yellow_a.png", "start": 28816133, "end": 28816689}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_yellow_b.png", "start": 28816689, "end": 28817245}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_yellow_c.png", "start": 28817245, "end": 28817799}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_blue_a.png", "start": 28817799, "end": 28818616}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_blue_b.png", "start": 28818616, "end": 28819434}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_blue_c.png", "start": 28819434, "end": 28820252}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_brown_a.png", "start": 28820252, "end": 28821074}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_brown_b.png", "start": 28821074, "end": 28821897}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_brown_c.png", "start": 28821897, "end": 28822719}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_green_a.png", "start": 28822719, "end": 28823540}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_green_b.png", "start": 28823540, "end": 28824362}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_green_c.png", "start": 28824362, "end": 28825184}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_grey_a.png", "start": 28825184, "end": 28825994}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_grey_b.png", "start": 28825994, "end": 28826807}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_grey_c.png", "start": 28826807, "end": 28827621}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_olive_a.png", "start": 28827621, "end": 28828439}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_olive_b.png", "start": 28828439, "end": 28829254}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_olive_c.png", "start": 28829254, "end": 28830071}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_orange_a.png", "start": 28830071, "end": 28830894}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_orange_b.png", "start": 28830894, "end": 28831717}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_orange_c.png", "start": 28831717, "end": 28832537}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_pink_a.png", "start": 28832537, "end": 28833360}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_pink_b.png", "start": 28833360, "end": 28834182}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_pink_c.png", "start": 28834182, "end": 28835004}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_purple_a.png", "start": 28835004, "end": 28835826}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_purple_b.png", "start": 28835826, "end": 28836648}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_purple_c.png", "start": 28836648, "end": 28837469}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_red_a.png", "start": 28837469, "end": 28838279}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_red_b.png", "start": 28838279, "end": 28839089}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_red_c.png", "start": 28839089, "end": 28839900}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_tan_a.png", "start": 28839900, "end": 28840720}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_tan_b.png", "start": 28840720, "end": 28841544}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_tan_c.png", "start": 28841544, "end": 28842365}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_yellow_a.png", "start": 28842365, "end": 28843186}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_yellow_b.png", "start": 28843186, "end": 28844008}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_yellow_c.png", "start": 28844008, "end": 28844827}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/hint.png", "start": 28844827, "end": 28845693}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/hintskip.png", "start": 28845693, "end": 28846586}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/key_gold_1.png", "start": 28846586, "end": 28847341}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/key_silver_1.png", "start": 28847341, "end": 28848095}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/light_fbr.png", "start": 28848095, "end": 28849011}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/origin.png", "start": 28849011, "end": 28849494}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/plus_0_button_fbr.png", "start": 28849494, "end": 28849693}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/plus_0_shoot_fbr.png", "start": 28849693, "end": 28849972}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/plus_1_button_fbr.png", "start": 28849972, "end": 28850173}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/plus_1_shoot_fbr.png", "start": 28850173, "end": 28850454}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/plus_a_button_fbr.png", "start": 28850454, "end": 28850655}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/plus_a_shoot_fbr.png", "start": 28850655, "end": 28850942}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/skip.png", "start": 28850942, "end": 28851430}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/sky_dev.png", "start": 28851430, "end": 28853402}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/sky_dev.png.bak", "start": 28853402, "end": 28865756}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/sky_dev_day_fbr.png", "start": 28865756, "end": 28869582}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/sky_dev_void.png", "start": 28869582, "end": 28870493}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_blood1.png", "start": 28870493, "end": 28870855}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_lava1.png", "start": 28870855, "end": 28871226}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_lavaskip.png", "start": 28871226, "end": 28872339}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_slime1.png", "start": 28872339, "end": 28872709}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_slimeskip.png", "start": 28872709, "end": 28873782}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_smile.png", "start": 28873782, "end": 28874262}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_teleport.png", "start": 28874262, "end": 28874586}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_water1.png", "start": 28874586, "end": 28874958}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_water2.png", "start": 28874958, "end": 28875327}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_waterskip.png", "start": 28875327, "end": 28877067}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/trigger.png", "start": 28877067, "end": 28877564}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_blue_a.png", "start": 28877564, "end": 28878244}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_blue_b.png", "start": 28878244, "end": 28878924}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_blue_c.png", "start": 28878924, "end": 28879604}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_brown_a.png", "start": 28879604, "end": 28880286}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_brown_b.png", "start": 28880286, "end": 28880968}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_brown_c.png", "start": 28880968, "end": 28881650}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_green_a.png", "start": 28881650, "end": 28882332}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_green_b.png", "start": 28882332, "end": 28883014}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_green_c.png", "start": 28883014, "end": 28883696}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_grey_a.png", "start": 28883696, "end": 28884370}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_grey_b.png", "start": 28884370, "end": 28885046}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_grey_c.png", "start": 28885046, "end": 28885723}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_olive_a.png", "start": 28885723, "end": 28886403}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_olive_b.png", "start": 28886403, "end": 28887080}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_olive_c.png", "start": 28887080, "end": 28887759}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_orange_a.png", "start": 28887759, "end": 28888441}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_orange_b.png", "start": 28888441, "end": 28889123}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_orange_c.png", "start": 28889123, "end": 28889804}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_pink_a.png", "start": 28889804, "end": 28890486}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_pink_b.png", "start": 28890486, "end": 28891168}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_pink_c.png", "start": 28891168, "end": 28891850}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_purple_a.png", "start": 28891850, "end": 28892533}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_purple_b.png", "start": 28892533, "end": 28893215}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_purple_c.png", "start": 28893215, "end": 28893897}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_red_a.png", "start": 28893897, "end": 28894570}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_red_b.png", "start": 28894570, "end": 28895243}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_red_c.png", "start": 28895243, "end": 28895917}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_tan_a.png", "start": 28895917, "end": 28896599}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_tan_b.png", "start": 28896599, "end": 28897281}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_tan_c.png", "start": 28897281, "end": 28897963}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_yellow_a.png", "start": 28897963, "end": 28898645}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_yellow_b.png", "start": 28898645, "end": 28899327}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_yellow_c.png", "start": 28899327, "end": 28900008}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_0_fbr.png", "start": 28900008, "end": 28900185}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_1_fbr.png", "start": 28900185, "end": 28900345}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_2_fbr.png", "start": 28900345, "end": 28900517}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_3_fbr.png", "start": 28900517, "end": 28900696}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_4_fbr.png", "start": 28900696, "end": 28900859}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_5_fbr.png", "start": 28900859, "end": 28901032}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_6_fbr.png", "start": 28901032, "end": 28901204}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_7_fbr.png", "start": 28901204, "end": 28901358}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_8_fbr.png", "start": 28901358, "end": 28901526}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_9_fbr.png", "start": 28901526, "end": 28901693}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_a_fbr.png", "start": 28901693, "end": 28901857}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_b_fbr.png", "start": 28901857, "end": 28902027}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_c_fbr.png", "start": 28902027, "end": 28902177}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_d_fbr.png", "start": 28902177, "end": 28902343}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_e_fbr.png", "start": 28902343, "end": 28902511}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_f_fbr.png", "start": 28902511, "end": 28902671}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_g_fbr.png", "start": 28902671, "end": 28902843}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_h_fbr.png", "start": 28902843, "end": 28903002}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_i_fbr.png", "start": 28903002, "end": 28903161}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_j_fbr.png", "start": 28903161, "end": 28903327}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_k_fbr.png", "start": 28903327, "end": 28903521}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_l_fbr.png", "start": 28903521, "end": 28903662}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_m_fbr.png", "start": 28903662, "end": 28903811}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_n_fbr.png", "start": 28903811, "end": 28903994}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_o_fbr.png", "start": 28903994, "end": 28904144}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_p_fbr.png", "start": 28904144, "end": 28904302}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_q_fbr.png", "start": 28904302, "end": 28904468}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_r_fbr.png", "start": 28904468, "end": 28904640}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_s_fbr.png", "start": 28904640, "end": 28904816}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_t_fbr.png", "start": 28904816, "end": 28904964}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_trans_fbr.png", "start": 28904964, "end": 28905087}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_u_fbr.png", "start": 28905087, "end": 28905233}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_v_fbr.png", "start": 28905233, "end": 28905401}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_w_fbr.png", "start": 28905401, "end": 28905550}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_x_fbr.png", "start": 28905550, "end": 28905732}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_y_fbr.png", "start": 28905732, "end": 28905901}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_z_fbr.png", "start": 28905901, "end": 28906076}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_a_fbr.png", "start": 28906076, "end": 28906231}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_b_fbr.png", "start": 28906231, "end": 28906390}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_c_fbr.png", "start": 28906390, "end": 28906544}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_d_fbr.png", "start": 28906544, "end": 28906705}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_e_fbr.png", "start": 28906705, "end": 28906859}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_f_fbr.png", "start": 28906859, "end": 28907019}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_g_fbr.png", "start": 28907019, "end": 28907177}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_h_fbr.png", "start": 28907177, "end": 28907331}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_i_fbr.png", "start": 28907331, "end": 28907477}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_j_fbr.png", "start": 28907477, "end": 28907637}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_k_fbr.png", "start": 28907637, "end": 28907816}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_l_fbr.png", "start": 28907816, "end": 28907954}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_m_fbr.png", "start": 28907954, "end": 28908107}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_n_fbr.png", "start": 28908107, "end": 28908257}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_o_fbr.png", "start": 28908257, "end": 28908411}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_p_fbr.png", "start": 28908411, "end": 28908566}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_q_fbr.png", "start": 28908566, "end": 28908716}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_r_fbr.png", "start": 28908716, "end": 28908869}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_s_fbr.png", "start": 28908869, "end": 28909021}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_t_fbr.png", "start": 28909021, "end": 28909187}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_u_fbr.png", "start": 28909187, "end": 28909338}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_v_fbr.png", "start": 28909338, "end": 28909512}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_w_fbr.png", "start": 28909512, "end": 28909666}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_x_fbr.png", "start": 28909666, "end": 28909853}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_y_fbr.png", "start": 28909853, "end": 28910027}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_z_fbr.png", "start": 28910027, "end": 28910198}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_add_fbr.png", "start": 28910198, "end": 28910361}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_and_fbr.png", "start": 28910361, "end": 28910553}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_ardown_fbr.png", "start": 28910553, "end": 28910738}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_arleft_fbr.png", "start": 28910738, "end": 28910922}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_arright_fbr.png", "start": 28910922, "end": 28911107}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_arup_fbr.png", "start": 28911107, "end": 28911285}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_at_fbr.png", "start": 28911285, "end": 28911442}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_brackc1_fbr.png", "start": 28911442, "end": 28911621}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_brackc2_fbr.png", "start": 28911621, "end": 28911805}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_brackr1_fbr.png", "start": 28911805, "end": 28911972}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_brackr2_fbr.png", "start": 28911972, "end": 28912142}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_bracks1_fbr.png", "start": 28912142, "end": 28912296}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_bracks2_fbr.png", "start": 28912296, "end": 28912449}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_caret_fbr.png", "start": 28912449, "end": 28912629}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_colon_fbr.png", "start": 28912629, "end": 28912781}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_colonsemi_fbr.png", "start": 28912781, "end": 28912948}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_comma_fbr.png", "start": 28912948, "end": 28913104}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_div_fbr.png", "start": 28913104, "end": 28913273}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_dollar_fbr.png", "start": 28913273, "end": 28913449}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_equ_fbr.png", "start": 28913449, "end": 28913608}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_exclam_fbr.png", "start": 28913608, "end": 28913752}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_grave_fbr.png", "start": 28913752, "end": 28913902}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_hash_fbr.png", "start": 28913902, "end": 28914087}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_heart_fbr.png", "start": 28914087, "end": 28914272}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_multi_fbr.png", "start": 28914272, "end": 28914441}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_percent_fbr.png", "start": 28914441, "end": 28914647}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_perio_fbr.png", "start": 28914647, "end": 28914784}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_pipe_fbr.png", "start": 28914784, "end": 28914931}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_quest_fbr.png", "start": 28914931, "end": 28915104}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_slaback_fbr.png", "start": 28915104, "end": 28915291}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_slafoward_fbr.png", "start": 28915291, "end": 28915470}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_smile_fbr.png", "start": 28915470, "end": 28915630}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_sub_fbr.png", "start": 28915630, "end": 28915777}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_sun_fbr.png", "start": 28915777, "end": 28915976}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_thngreater_fbr.png", "start": 28915976, "end": 28916169}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_thnless_fbr.png", "start": 28916169, "end": 28916356}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_tilde_fbr.png", "start": 28916356, "end": 28916525}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_unders_fbr.png", "start": 28916525, "end": 28916662}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/bone1_1.png", "start": 28916662, "end": 28924069}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/bone1_2.png", "start": 28924069, "end": 28933261}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/bone2_1.png", "start": 28933261, "end": 28943693}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/dopefish_fbr.png", "start": 28943693, "end": 28951923}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_gut1.png", "start": 28951923, "end": 28962238}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_gut2.png", "start": 28962238, "end": 28974038}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_1.png", "start": 28974038, "end": 28987788}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_2.png", "start": 28987788, "end": 29000768}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_3.png", "start": 29000768, "end": 29012801}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_4a.png", "start": 29012801, "end": 29026014}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_4b.png", "start": 29026014, "end": 29039154}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_5a.png", "start": 29039154, "end": 29052798}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_5b.png", "start": 29052798, "end": 29066371}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot2_1.png", "start": 29066371, "end": 29079597}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot3_1.png", "start": 29079597, "end": 29094543}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot3_2.png", "start": 29094543, "end": 29109582}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot4_1.png", "start": 29109582, "end": 29124219}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot5_1.png", "start": 29124219, "end": 29136487}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot6_1.png", "start": 29136487, "end": 29152894}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod1_1.png", "start": 29152894, "end": 29164412}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod1_2.png", "start": 29164412, "end": 29177474}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod1_3.png", "start": 29177474, "end": 29188477}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod1_4.png", "start": 29188477, "end": 29199889}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod1_5.png", "start": 29199889, "end": 29210488}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod2_1.png", "start": 29210488, "end": 29214144}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod2_2.png", "start": 29214144, "end": 29218241}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod2_3.png", "start": 29218241, "end": 29221617}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod2_4.png", "start": 29221617, "end": 29225215}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod2_5.png", "start": 29225215, "end": 29228605}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/fleshtile.png", "start": 29228605, "end": 29239574}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/marbred128.png", "start": 29239574, "end": 29251803}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_eat_eye2_fbr.png", "start": 29251803, "end": 29255676}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_eat_eye3_fbr.png", "start": 29255676, "end": 29259553}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_eat_eye_fbr.png", "start": 29259553, "end": 29263428}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_b.png", "start": 29263428, "end": 29276091}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_c.png", "start": 29276091, "end": 29288815}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_hol1.png", "start": 29288815, "end": 29293487}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_lit1_fbr.png", "start": 29293487, "end": 29297557}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_shut1.png", "start": 29297557, "end": 29312523}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_sp.png", "start": 29312523, "end": 29332665}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_sp2.png", "start": 29332665, "end": 29358288}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_te.png", "start": 29358288, "end": 29373394}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_tet.png", "start": 29373394, "end": 29386614}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh2_b.png", "start": 29386614, "end": 29400137}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh3_1a.png", "start": 29400137, "end": 29413863}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh3_1b.png", "start": 29413863, "end": 29450425}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh3_b.png", "start": 29450425, "end": 29464311}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh4_det.png", "start": 29464311, "end": 29474622}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh4a_det.png", "start": 29474622, "end": 29489483}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh5.png", "start": 29489483, "end": 29499819}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh5_1a.png", "start": 29499819, "end": 29554554}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh5_1b.png", "start": 29554554, "end": 29607272}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh5_1c.png", "start": 29607272, "end": 29650914}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh5_1lit_fbr.png", "start": 29650914, "end": 29667848}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh_dr1a.png", "start": 29667848, "end": 29724741}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_skin_eye.png", "start": 29724741, "end": 29735958}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/meat-teeth0.png", "start": 29735958, "end": 29779329}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/meat-teeth1.png", "start": 29779329, "end": 29824521}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/meat_det1.png", "start": 29824521, "end": 29843252}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/meat_det2.png", "start": 29843252, "end": 29865823}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/meat_pipe1.png", "start": 29865823, "end": 29890247}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_0eye.png", "start": 29890247, "end": 29894392}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_0flesh2_gl.png", "start": 29894392, "end": 29909572}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_0flesh_but1_fbr.png", "start": 29909572, "end": 29913867}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_0flesh_but2_fbr.png", "start": 29913867, "end": 29918056}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_0flsh_vent.png", "start": 29918056, "end": 29932307}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_1eye.png", "start": 29932307, "end": 29936451}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_1flesh2_gl.png", "start": 29936451, "end": 29951610}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_1flesh_but2_fbr.png", "start": 29951610, "end": 29955805}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_2eye.png", "start": 29955805, "end": 29959845}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_2flesh2_gl.png", "start": 29959845, "end": 29975021}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_2flesh_but2_fbr.png", "start": 29975021, "end": 29979204}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_3eye.png", "start": 29979204, "end": 29983252}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_3flesh2_gl.png", "start": 29983252, "end": 29998426}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_3flesh_but2_fbr.png", "start": 29998426, "end": 30002597}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_4eye.png", "start": 30002597, "end": 30006716}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_4flesh2_gl.png", "start": 30006716, "end": 30021896}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_5eye.png", "start": 30021896, "end": 30025949}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_6eye.png", "start": 30025949, "end": 30029997}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_7eye.png", "start": 30029997, "end": 30034068}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_8eye.png", "start": 30034068, "end": 30038176}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_9eye.png", "start": 30038176, "end": 30042324}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_aeye.png", "start": 30042324, "end": 30046316}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_aflesh_but1.png", "start": 30046316, "end": 30050882}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_aflesh_but2.png", "start": 30050882, "end": 30055411}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_aflsh_vent.png", "start": 30055411, "end": 30069811}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/black.png", "start": 30069811, "end": 30069956}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch1.png", "start": 30069956, "end": 30074973}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch1_2.png", "start": 30074973, "end": 30080599}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch1_a.png", "start": 30080599, "end": 30093374}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch1_b.png", "start": 30093374, "end": 30106834}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch1_c.png", "start": 30106834, "end": 30120007}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch1_d.png", "start": 30120007, "end": 30133524}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch2.png", "start": 30133524, "end": 30136806}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch2_2.png", "start": 30136806, "end": 30140128}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch2_a.png", "start": 30140128, "end": 30150531}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch2_b.png", "start": 30150531, "end": 30161048}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch2_c.png", "start": 30161048, "end": 30171177}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch_end.png", "start": 30171177, "end": 30171933}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch_tcap.png", "start": 30171933, "end": 30172647}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch_trim.png", "start": 30172647, "end": 30174119}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_arch1a.png", "start": 30174119, "end": 30181754}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_arch1b.png", "start": 30181754, "end": 30190289}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_brk1a.png", "start": 30190289, "end": 30192761}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_brk1b.png", "start": 30192761, "end": 30194899}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_brk2a.png", "start": 30194899, "end": 30197320}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_brk2b.png", "start": 30197320, "end": 30199780}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_flt1.png", "start": 30199780, "end": 30202004}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_fsh1.png", "start": 30202004, "end": 30204930}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_fsh2.png", "start": 30204930, "end": 30213022}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_pil1.png", "start": 30213022, "end": 30215809}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_trim1.png", "start": 30215809, "end": 30218221}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll1.png", "start": 30218221, "end": 30220941}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll2.png", "start": 30220941, "end": 30223667}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll3a.png", "start": 30223667, "end": 30226025}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll3b.png", "start": 30226025, "end": 30228493}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll4b.png", "start": 30228493, "end": 30231059}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll5a.png", "start": 30231059, "end": 30233276}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll5b.png", "start": 30233276, "end": 30235575}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll5c.png", "start": 30235575, "end": 30237898}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll5d.png", "start": 30237898, "end": 30240305}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk15.png", "start": 30240305, "end": 30249811}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk15_b.png", "start": 30249811, "end": 30259059}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk15_c.png", "start": 30259059, "end": 30275927}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk15_f.png", "start": 30275927, "end": 30284220}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk15_g.png", "start": 30284220, "end": 30291869}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk16.png", "start": 30291869, "end": 30301145}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk16_a.png", "start": 30301145, "end": 30310424}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk16_f.png", "start": 30310424, "end": 30320129}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk17.png", "start": 30320129, "end": 30355273}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk17_f.png", "start": 30355273, "end": 30367875}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_but1.png", "start": 30367875, "end": 30368824}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_det1.png", "start": 30368824, "end": 30371897}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_door1.png", "start": 30371897, "end": 30387004}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_door1_f.png", "start": 30387004, "end": 30397039}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_door2.png", "start": 30397039, "end": 30407899}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_door3.png", "start": 30407899, "end": 30420556}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_ebrick10.png", "start": 30420556, "end": 30431183}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_ebrick10_bl.png", "start": 30431183, "end": 30445717}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_ebrick22.png", "start": 30445717, "end": 30459583}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_ebrick23.png", "start": 30459583, "end": 30468949}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_ebrick24.png", "start": 30468949, "end": 30477142}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr1.png", "start": 30477142, "end": 30477473}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr2.png", "start": 30477473, "end": 30478327}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr3.png", "start": 30478327, "end": 30480923}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_1.png", "start": 30480923, "end": 30483053}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_2.png", "start": 30483053, "end": 30485315}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_3.png", "start": 30485315, "end": 30487678}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_4.png", "start": 30487678, "end": 30489973}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_5.png", "start": 30489973, "end": 30493605}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_6.png", "start": 30493605, "end": 30496611}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_8.png", "start": 30496611, "end": 30499946}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr5_1.png", "start": 30499946, "end": 30502533}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr5_2.png", "start": 30502533, "end": 30505027}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr5_3.png", "start": 30505027, "end": 30507769}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr5_4.png", "start": 30507769, "end": 30510072}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_key_gl2.png", "start": 30510072, "end": 30511096}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_key_sl2.png", "start": 30511096, "end": 30512097}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_lion1.png", "start": 30512097, "end": 30522571}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_lion2.png", "start": 30522571, "end": 30532238}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_lion3.png", "start": 30532238, "end": 30542781}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_lion4.png", "start": 30542781, "end": 30553933}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_met1.png", "start": 30553933, "end": 30564314}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_met1_trim.png", "start": 30564314, "end": 30575589}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_met1a_trim.png", "start": 30575589, "end": 30586557}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_met1b_trim.png", "start": 30586557, "end": 30600670}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_met2_trim.png", "start": 30600670, "end": 30613939}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_met_plt.png", "start": 30613939, "end": 30629330}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_mural1.png", "start": 30629330, "end": 30649188}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_mural2.png", "start": 30649188, "end": 30671905}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_mural3.png", "start": 30671905, "end": 30761185}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_pl1_a.png", "start": 30761185, "end": 30763001}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_pl1_b.png", "start": 30763001, "end": 30765084}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_pl2_a.png", "start": 30765084, "end": 30767316}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_pl2_b.png", "start": 30767316, "end": 30769528}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_plat1_side.png", "start": 30769528, "end": 30770745}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_plat1_top.png", "start": 30770745, "end": 30774642}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_tile2_1.png", "start": 30774642, "end": 30777076}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_tile2_2.png", "start": 30777076, "end": 30779429}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1.png", "start": 30779429, "end": 30786186}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_3.png", "start": 30786186, "end": 30792800}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_3_s.png", "start": 30792800, "end": 30794840}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_4_s.png", "start": 30794840, "end": 30796916}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_5.png", "start": 30796916, "end": 30803852}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_5_s.png", "start": 30803852, "end": 30806072}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_6_s.png", "start": 30806072, "end": 30808349}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_7_s.png", "start": 30808349, "end": 30811691}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim2.png", "start": 30811691, "end": 30814052}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_wall1.png", "start": 30814052, "end": 30817084}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_wall2.png", "start": 30817084, "end": 30819755}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_wall3.png", "start": 30819755, "end": 30825541}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_wall3b.png", "start": 30825541, "end": 30852575}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_win1_a.png", "start": 30852575, "end": 30861714}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_win1_b.png", "start": 30861714, "end": 30871057}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_0grk_but1_fbr.png", "start": 30871057, "end": 30872248}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_0grk_hbut_fbr.png", "start": 30872248, "end": 30873271}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_1grk_but1_fbr.png", "start": 30873271, "end": 30874446}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_1grk_hbut_fbr.png", "start": 30874446, "end": 30875527}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_2grk_but1_fbr.png", "start": 30875527, "end": 30876678}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_2grk_hbut_fbr.png", "start": 30876678, "end": 30877718}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_3grk_but1_fbr.png", "start": 30877718, "end": 30878871}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_3grk_hbut_fbr.png", "start": 30878871, "end": 30879913}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_agrk_but1.png", "start": 30879913, "end": 30880904}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_agrk_hbut.png", "start": 30880904, "end": 30881882}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_bottom.png", "start": 30881882, "end": 30882812}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_c_b1_fbr.png", "start": 30882812, "end": 30883420}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_c_b2_fbr.png", "start": 30883420, "end": 30884032}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_c_b3_fbr.png", "start": 30884032, "end": 30884839}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_c_s1_fbr.png", "start": 30884839, "end": 30885432}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_c_s2_fbr.png", "start": 30885432, "end": 30886422}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_c_s3_fbr.png", "start": 30886422, "end": 30887055}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_fl.png", "start": 30887055, "end": 30887614}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_fl2.png", "start": 30887614, "end": 30888169}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_n_b1_fbr.png", "start": 30888169, "end": 30888923}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_n_b2_fbr.png", "start": 30888923, "end": 30889608}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_n_b3.png", "start": 30889608, "end": 30890179}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_n_s1.png", "start": 30890179, "end": 30890750}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_n_s2_fbr.png", "start": 30890750, "end": 30891405}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_n_s3_fbr.png", "start": 30891405, "end": 30891942}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_r_b1_fbr.png", "start": 30891942, "end": 30892657}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_r_b2_fbr.png", "start": 30892657, "end": 30893283}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_r_b3_fbr.png", "start": 30893283, "end": 30893741}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_r_s1_fbr.png", "start": 30893741, "end": 30894355}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_r_s2_fbr.png", "start": 30894355, "end": 30894704}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_s_b1_fbr.png", "start": 30894704, "end": 30895363}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_s_b2_fbr.png", "start": 30895363, "end": 30895991}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_s_b3.png", "start": 30895991, "end": 30896768}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_s_s1_fbr.png", "start": 30896768, "end": 30897274}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_s_s2_fbr.png", "start": 30897274, "end": 30897852}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_s_s3_fbr.png", "start": 30897852, "end": 30898403}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammobotsmall.png", "start": 30898403, "end": 30899071}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammotop.png", "start": 30899071, "end": 30899816}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammotopsmall.png", "start": 30899816, "end": 30900367}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/boom.png", "start": 30900367, "end": 30901385}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/boomammo_bottom.png", "start": 30901385, "end": 30901751}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/boomammotop.png", "start": 30901751, "end": 30902093}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/boomsmall.png", "start": 30902093, "end": 30902768}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/epboxlarge_fbr.png", "start": 30902768, "end": 30903952}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/epboxsmall_fbr.png", "start": 30903952, "end": 30904849}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/explob_s2.png", "start": 30904849, "end": 30905345}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/hp15_side.png", "start": 30905345, "end": 30905930}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/hp25_top2.png", "start": 30905930, "end": 30906359}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/hp_bottom.png", "start": 30906359, "end": 30907266}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/hp_details.png", "start": 30907266, "end": 30907770}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/nails.png", "start": 30907770, "end": 30908817}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/nailssmall.png", "start": 30908817, "end": 30909558}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp100-winq_fbr.png", "start": 30909558, "end": 30912158}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp100_side_fbr.png", "start": 30912158, "end": 30913050}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp100_top_fbr.png", "start": 30913050, "end": 30913781}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp15_top_fbr.png", "start": 30913781, "end": 30914511}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp15winq_fbr.png", "start": 30914511, "end": 30917298}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp25-winq_fbr.png", "start": 30917298, "end": 30919852}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp25_side_fbr.png", "start": 30919852, "end": 30920674}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp25_top_fbr.png", "start": 30920674, "end": 30921408}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0explob2_s1_fbr.png", "start": 30921408, "end": 30922511}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0explob_s1_fbr.png", "start": 30922511, "end": 30923144}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp100-winq_fbr.png", "start": 30923144, "end": 30925749}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp100_side_fbr.png", "start": 30925749, "end": 30926643}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp100_top_fbr.png", "start": 30926643, "end": 30927377}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp15_top_fbr.png", "start": 30927377, "end": 30928111}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp15winq_fbr.png", "start": 30928111, "end": 30930897}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp25-winq_fbr.png", "start": 30930897, "end": 30933461}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp25_side_fbr.png", "start": 30933461, "end": 30934287}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp25_top_fbr.png", "start": 30934287, "end": 30935026}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1explob2_s1_fbr.png", "start": 30935026, "end": 30936133}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1explob_s1_fbr.png", "start": 30936133, "end": 30936766}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2_hp100-winq_fbr.png", "start": 30936766, "end": 30939370}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2_hp100_side_fbr.png", "start": 30939370, "end": 30940264}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2_hp25-winq_fbr.png", "start": 30940264, "end": 30942822}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2_hp25_side_fbr.png", "start": 30942822, "end": 30943647}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2_hp25_top_fbr.png", "start": 30943647, "end": 30944385}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2explob2_s1_fbr.png", "start": 30944385, "end": 30945481}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2explob_s1_fbr.png", "start": 30945481, "end": 30946123}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3_hp100-winq_fbr.png", "start": 30946123, "end": 30948733}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3_hp100_side_fbr.png", "start": 30948733, "end": 30949630}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3_hp25-winq_fbr.png", "start": 30949630, "end": 30952199}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3_hp25_side_fbr.png", "start": 30952199, "end": 30953030}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3_hp25_top_fbr.png", "start": 30953030, "end": 30953772}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3explob2_s1_fbr.png", "start": 30953772, "end": 30954868}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3explob_s1_fbr.png", "start": 30954868, "end": 30955510}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/shells.png", "start": 30955510, "end": 30956456}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/shellssmall.png", "start": 30956456, "end": 30957182}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/zap.png", "start": 30957182, "end": 30958175}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/zapsmall.png", "start": 30958175, "end": 30958939}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/brick7.png", "start": 30958939, "end": 30961872}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/brick8.png", "start": 30961872, "end": 30964959}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/button_0.png", "start": 30964959, "end": 30968431}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/button_0_grey.png", "start": 30968431, "end": 30971641}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/button_0_grn.png", "start": 30971641, "end": 30974976}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/button_1.png", "start": 30974976, "end": 30978431}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/button_1_grey.png", "start": 30978431, "end": 30981626}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/button_1_grn.png", "start": 30981626, "end": 30984927}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/floor_temp.png", "start": 30984927, "end": 30986985}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/flr.png", "start": 30986985, "end": 30989742}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/gardgrass_1.png", "start": 30989742, "end": 31009912}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/go-savgx.png", "start": 31009912, "end": 31011661}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grass.png", "start": 31011661, "end": 31015062}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_brk15_c_old.png", "start": 31015062, "end": 31025589}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_brk17_f_old.png", "start": 31025589, "end": 31034298}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_door1_old.png", "start": 31034298, "end": 31045315}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_door2_old.png", "start": 31045315, "end": 31054488}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_door3_old.png", "start": 31054488, "end": 31063294}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_ebrick22_old.png", "start": 31063294, "end": 31072611}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_trim1_7_s_old.png", "start": 31072611, "end": 31074816}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/ground_1.png", "start": 31074816, "end": 31076997}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/leaves.png", "start": 31076997, "end": 31080059}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/marble1_4.png", "start": 31080059, "end": 31082962}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/marble1_5.png", "start": 31082962, "end": 31086052}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_cflat1_3.png", "start": 31086052, "end": 31088533}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_csl_trm3.png", "start": 31088533, "end": 31089858}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_csl_trm3b.png", "start": 31089858, "end": 31090848}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_csl_trm3c.png", "start": 31090848, "end": 31092376}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_flat10.png", "start": 31092376, "end": 31096269}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_flat11.png", "start": 31096269, "end": 31099474}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_flat13.png", "start": 31099474, "end": 31110251}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_flat14.png", "start": 31110251, "end": 31120652}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_flat9a.png", "start": 31120652, "end": 31164232}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_flat9b.png", "start": 31164232, "end": 31206131}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_plaster1.png", "start": 31206131, "end": 31214173}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_ret_rock1.png", "start": 31214173, "end": 31217992}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_ret_wood1_old.png", "start": 31217992, "end": 31225073}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_rmet_key.png", "start": 31225073, "end": 31229034}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_rock6.png", "start": 31229034, "end": 31274637}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_rock7.png", "start": 31274637, "end": 31277521}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_rock8.png", "start": 31277521, "end": 31279406}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_rough_block.png", "start": 31279406, "end": 31328249}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_rough_block_f.png", "start": 31328249, "end": 31382794}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_tile.png", "start": 31382794, "end": 31384930}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_wall1.png", "start": 31384930, "end": 31394795}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/metground_1.png", "start": 31394795, "end": 31397726}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/note-savgx.png", "start": 31397726, "end": 31416746}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/plus_0button1.png", "start": 31416746, "end": 31420191}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/plus_0button2_fbr.png", "start": 31420191, "end": 31421678}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/plus_1button2_fbr.png", "start": 31421678, "end": 31423166}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/plus_1button3.png", "start": 31423166, "end": 31425046}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/plus_abutton1_fbr.png", "start": 31425046, "end": 31428648}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/plus_abutton2_fbr.png", "start": 31428648, "end": 31430146}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/readme.txt", "start": 31430146, "end": 31430222}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiflat.png", "start": 31430222, "end": 31444341}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiflat_blu.png", "start": 31444341, "end": 31457276}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiflat_grn.png", "start": 31457276, "end": 31468930}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoilava.png", "start": 31468930, "end": 31473434}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoislime.png", "start": 31473434, "end": 31477139}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoitrim.png", "start": 31477139, "end": 31478626}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoitrim__purp.png", "start": 31478626, "end": 31480342}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoitrim_blu.png", "start": 31480342, "end": 31482072}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiwall.png", "start": 31482072, "end": 31521869}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiwall__purp.png", "start": 31521869, "end": 31557942}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiwall_blu.png", "start": 31557942, "end": 31594003}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiwater.png", "start": 31594003, "end": 31596894}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/rune1_fbr.png", "start": 31596894, "end": 31599907}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/rune2_fbr.png", "start": 31599907, "end": 31602973}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/rune3_fbr.png", "start": 31602973, "end": 31606369}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/rune4_fbr.png", "start": 31606369, "end": 31608921}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_easy.png", "start": 31608921, "end": 31613873}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_empty.png", "start": 31613873, "end": 31618630}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_hard.png", "start": 31618630, "end": 31623503}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_medium.png", "start": 31623503, "end": 31628513}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_metal_1.png", "start": 31628513, "end": 31629725}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_metal_2.png", "start": 31629725, "end": 31631320}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_nmare.png", "start": 31631320, "end": 31636341}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky2.png", "start": 31636341, "end": 31651831}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky4.png", "start": 31651831, "end": 31661343}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky5_fbr.png", "start": 31661343, "end": 31678496}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky5a.png", "start": 31678496, "end": 31687507}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky7.png", "start": 31687507, "end": 31701422}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky8.png", "start": 31701422, "end": 31718095}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky8a_fbr.png", "start": 31718095, "end": 31726891}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/tile.png", "start": 31726891, "end": 31735707}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/tile_blu.png", "start": 31735707, "end": 31744713}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/tile_grn.png", "start": 31744713, "end": 31752976}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/trim2_blu.png", "start": 31752976, "end": 31754089}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/trim2_grn.png", "start": 31754089, "end": 31755153}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/ultrasteel1.png", "start": 31755153, "end": 31768043}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/ultrasteel2.png", "start": 31768043, "end": 31779837}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/vines1_old.png", "start": 31779837, "end": 31784691}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/wiz1_4.png", "start": 31784691, "end": 31788512}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/+0water_f3.png", "start": 31788512, "end": 31791453}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/+1water_f3.png", "start": 31791453, "end": 31794377}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/+2water_f3.png", "start": 31794377, "end": 31797295}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/+3water_f3.png", "start": 31797295, "end": 31800147}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_0blood_f1.png", "start": 31800147, "end": 31801936}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_0fslime.png", "start": 31801936, "end": 31815892}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_0lava_fall3_fbr.png", "start": 31815892, "end": 31824885}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_0water_f1.png", "start": 31824885, "end": 31826734}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_0water_f2.png", "start": 31826734, "end": 31828552}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_0wfall0.png", "start": 31828552, "end": 31838161}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_1blood_f1.png", "start": 31838161, "end": 31839966}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_1fslime.png", "start": 31839966, "end": 31853709}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_1lava_fall3_fbr.png", "start": 31853709, "end": 31863276}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_1water_f1.png", "start": 31863276, "end": 31864924}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_1water_f2.png", "start": 31864924, "end": 31866753}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_1wfall0.png", "start": 31866753, "end": 31876404}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_2blood_f1.png", "start": 31876404, "end": 31878262}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_2fslime.png", "start": 31878262, "end": 31892156}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_2lava_fall3_fbr.png", "start": 31892156, "end": 31901705}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_2water_f1.png", "start": 31901705, "end": 31903476}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_2water_f2.png", "start": 31903476, "end": 31905362}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_2wfall0.png", "start": 31905362, "end": 31915024}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_3blood_f1.png", "start": 31915024, "end": 31916824}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_3fslime.png", "start": 31916824, "end": 31930507}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_3lava_fall3_fbr.png", "start": 31930507, "end": 31939844}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_3water_f1.png", "start": 31939844, "end": 31941495}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_3water_f2.png", "start": 31941495, "end": 31943322}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_3wfall0.png", "start": 31943322, "end": 31953025}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_4fslime.png", "start": 31953025, "end": 31966689}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_4lava_fall3_fbr.png", "start": 31966689, "end": 31976062}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_4wfall0.png", "start": 31976062, "end": 31985641}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_5fslime.png", "start": 31985641, "end": 31999409}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_5lava_fall3_fbr.png", "start": 31999409, "end": 32008772}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_5wfall0.png", "start": 32008772, "end": 32018371}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_6fslime.png", "start": 32018371, "end": 32032223}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_6lava_fall3_fbr.png", "start": 32032223, "end": 32041608}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_6wfall0.png", "start": 32041608, "end": 32051193}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_7fslime.png", "start": 32051193, "end": 32065044}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_7lava_fall3_fbr.png", "start": 32065044, "end": 32074559}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_7wfall0.png", "start": 32074559, "end": 32084228}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_8wfall0.png", "start": 32084228, "end": 32093890}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_9wfall0.png", "start": 32093890, "end": 32103512}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky-test.png", "start": 32103512, "end": 32121108}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky-test.xcf", "start": 32121108, "end": 32254373}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky5_blu.png", "start": 32254373, "end": 32267686}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky5_dismal.png", "start": 32267686, "end": 32280869}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_galx_fbr.png", "start": 32280869, "end": 32303624}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_galx_spark_fbr.png", "start": 32303624, "end": 32323182}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_orng.png", "start": 32323182, "end": 32337821}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_pando.png", "start": 32337821, "end": 32353929}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_pando2.png", "start": 32353929, "end": 32370464}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_star.png", "start": 32370464, "end": 32372124}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_void.png", "start": 32372124, "end": 32372819}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_wfog_fbr.png", "start": 32372819, "end": 32373725}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_acid.png", "start": 32373725, "end": 32375904}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_blood1.png", "start": 32375904, "end": 32377977}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_lava1_fbr.png", "start": 32377977, "end": 32381182}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_lava2_fbr.png", "start": 32381182, "end": 32385264}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_lava3_fbr.png", "start": 32385264, "end": 32389233}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_lava_void_fbr.png", "start": 32389233, "end": 32392923}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_lavaskip.png", "start": 32392923, "end": 32394036}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_meatgoo2_fbr.png", "start": 32394036, "end": 32397224}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_meatgoo_fbr.png", "start": 32397224, "end": 32400021}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_slime1.png", "start": 32400021, "end": 32406312}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_slime2.png", "start": 32406312, "end": 32409501}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_slime3.png", "start": 32409501, "end": 32411533}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_slime_soul.png", "start": 32411533, "end": 32414323}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_slimeskip.png", "start": 32414323, "end": 32415396}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_soul_drain.png", "start": 32415396, "end": 32418364}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_tele1_fbr.png", "start": 32418364, "end": 32420244}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_tele2_fbr.png", "start": 32420244, "end": 32423045}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_tele3_fbr.png", "start": 32423045, "end": 32426081}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_tele4_fbr.png", "start": 32426081, "end": 32428773}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_water0.png", "start": 32428773, "end": 32431621}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_water1.png", "start": 32431621, "end": 32434730}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_water2.png", "start": 32434730, "end": 32436990}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_water3.png", "start": 32436990, "end": 32439371}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_water4.png", "start": 32439371, "end": 32444158}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_waterskip.png", "start": 32444158, "end": 32445898}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_wstill0.png", "start": 32445898, "end": 32448737}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/btn1.png", "start": 32448737, "end": 32451655}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_arrow.png", "start": 32451655, "end": 32452536}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok1_1.png", "start": 32452536, "end": 32455057}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok1_2.png", "start": 32455057, "end": 32457248}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok1_2_m.png", "start": 32457248, "end": 32460344}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok1_m.png", "start": 32460344, "end": 32469618}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok2_1.png", "start": 32469618, "end": 32472182}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok2_1_m.png", "start": 32472182, "end": 32475403}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok2_2.png", "start": 32475403, "end": 32485666}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok2_2_m.png", "start": 32485666, "end": 32498094}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blud1_1.png", "start": 32498094, "end": 32500686}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blud1_1m.png", "start": 32500686, "end": 32503906}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_1.png", "start": 32503906, "end": 32506341}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_1m.png", "start": 32506341, "end": 32509411}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_2.png", "start": 32509411, "end": 32511838}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_2m.png", "start": 32511838, "end": 32514866}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_3.png", "start": 32514866, "end": 32517125}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_3m.png", "start": 32517125, "end": 32520534}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_5.png", "start": 32520534, "end": 32523387}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_5m.png", "start": 32523387, "end": 32527036}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd_skull.png", "start": 32527036, "end": 32529668}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_0.png", "start": 32529668, "end": 32532772}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_0m.png", "start": 32532772, "end": 32535829}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_1.png", "start": 32535829, "end": 32538511}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_1m.png", "start": 32538511, "end": 32541710}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_2.png", "start": 32541710, "end": 32544661}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_2m.png", "start": 32544661, "end": 32548066}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_3.png", "start": 32548066, "end": 32551003}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_3m.png", "start": 32551003, "end": 32554179}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk2_0.png", "start": 32554179, "end": 32564034}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk2_0_m.png", "start": 32564034, "end": 32576367}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk_old.png", "start": 32576367, "end": 32578966}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk_oldm.png", "start": 32578966, "end": 32582305}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_deco1_1.png", "start": 32582305, "end": 32593534}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_deco1_1m.png", "start": 32593534, "end": 32607569}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_deco1_2.png", "start": 32607569, "end": 32610089}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_deco1_2my.png", "start": 32610089, "end": 32613477}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_deco1_3.png", "start": 32613477, "end": 32632050}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_deco1_3m.png", "start": 32632050, "end": 32652401}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_door1_1.png", "start": 32652401, "end": 32661868}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_door1_1m.png", "start": 32661868, "end": 32672959}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_door2_1.png", "start": 32672959, "end": 32684927}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_door2_2.png", "start": 32684927, "end": 32696705}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_drt2_1.png", "start": 32696705, "end": 32699542}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_flr1_1.png", "start": 32699542, "end": 32702447}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_flr1_2.png", "start": 32702447, "end": 32705670}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_flt1_1.png", "start": 32705670, "end": 32707959}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_flt1_1m.png", "start": 32707959, "end": 32711407}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_key1_1.png", "start": 32711407, "end": 32712994}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_key1_2.png", "start": 32712994, "end": 32714991}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_lite1_1_fbr.png", "start": 32714991, "end": 32715921}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_lite1_2.png", "start": 32715921, "end": 32716284}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_lite2_1.png", "start": 32716284, "end": 32719221}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_lite2_2.png", "start": 32719221, "end": 32719707}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_lite3_1_fbr.png", "start": 32719707, "end": 32720664}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_lite_f1.png", "start": 32720664, "end": 32721150}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_oldmtomb1_1_fbr.png", "start": 32721150, "end": 32735002}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_oldtomb1_2_fbr.png", "start": 32735002, "end": 32748894}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_plat_stem_m.png", "start": 32748894, "end": 32749760}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_plats.png", "start": 32749760, "end": 32752422}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_platst.png", "start": 32752422, "end": 32754849}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_platt.png", "start": 32754849, "end": 32757472}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_plr1_1.png", "start": 32757472, "end": 32760666}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_tomb1_1_fbr.png", "start": 32760666, "end": 32774518}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_tomb1_2_fbr.png", "start": 32774518, "end": 32788410}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_trm1_1.png", "start": 32788410, "end": 32797338}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_trm1_2.png", "start": 32797338, "end": 32800043}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_trm1_a.png", "start": 32800043, "end": 32802385}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_tskull.png", "start": 32802385, "end": 32813391}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_wall1_1.png", "start": 32813391, "end": 32816373}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_wall1_2.png", "start": 32816373, "end": 32819339}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_wall1_3.png", "start": 32819339, "end": 32822533}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_wall1_3a.png", "start": 32822533, "end": 32825114}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_wall1_4.png", "start": 32825114, "end": 32828102}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_wall1_4a.png", "start": 32828102, "end": 32830501}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/maya_end_dr1.png", "start": 32830501, "end": 32834887}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/maya_end_dr2.png", "start": 32834887, "end": 32839272}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/maya_end_trim1.png", "start": 32839272, "end": 32843662}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_0_may_btn1.png", "start": 32843662, "end": 32846579}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_0_may_mpiloilon_fbr.png", "start": 32846579, "end": 32848137}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_0_may_mpilon_fbr.png", "start": 32848137, "end": 32849712}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_0_may_mshoohoot_fbr.png", "start": 32849712, "end": 32850657}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_0_may_mshoot_fbr.png", "start": 32850657, "end": 32851600}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_1_may_btn1.png", "start": 32851600, "end": 32854508}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_1_may_mpiloilon_fbr.png", "start": 32854508, "end": 32856081}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_1_may_mpilon_fbr.png", "start": 32856081, "end": 32857674}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_1_may_mshoohoot_fbr.png", "start": 32857674, "end": 32858659}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_1_may_mshoot_fbr.png", "start": 32858659, "end": 32859643}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_2_may_btn1.png", "start": 32859643, "end": 32862534}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_2_may_mpilon.png", "start": 32862534, "end": 32864111}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_2_may_mshoohoot_fbr.png", "start": 32864111, "end": 32865136}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_2_may_mshoot_fbr.png", "start": 32865136, "end": 32866163}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_3_may_btn1.png", "start": 32866163, "end": 32869225}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_3_may_mpiloilon_fbr.png", "start": 32869225, "end": 32870798}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_3_may_mpilon_fbr.png", "start": 32870798, "end": 32872391}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_3_may_mshoohoot_fbr.png", "start": 32872391, "end": 32873376}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_3_may_mshoot_fbr.png", "start": 32873376, "end": 32874360}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_4_may_btn1.png", "start": 32874360, "end": 32877423}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_5_may_btn1.png", "start": 32877423, "end": 32880164}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_a_may_btn1.png", "start": 32880164, "end": 32882887}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_a_may_mpiloilon_fbr.png", "start": 32882887, "end": 32884464}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_a_may_mpilon_fbr.png", "start": 32884464, "end": 32886048}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_a_may_mshoot.png", "start": 32886048, "end": 32886997}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+0med_but1.png", "start": 32886997, "end": 32891252}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+0med_but2.png", "start": 32891252, "end": 32892964}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+0med_but3.png", "start": 32892964, "end": 32894667}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+0med_but_s1.png", "start": 32894667, "end": 32898913}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+0med_sht_but1.png", "start": 32898913, "end": 32900206}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+1med_but3.png", "start": 32900206, "end": 32901908}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+1med_but_s1.png", "start": 32901908, "end": 32906127}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+1med_sht_but1.png", "start": 32906127, "end": 32907440}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+2med_but_s1.png", "start": 32907440, "end": 32911714}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+3med_but_s1.png", "start": 32911714, "end": 32915933}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+amed_but1.png", "start": 32915933, "end": 32920016}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+amed_but2.png", "start": 32920016, "end": 32921734}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+amed_but3.png", "start": 32921734, "end": 32923344}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+amed_but_s1.png", "start": 32923344, "end": 32927557}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+amed_sht_but1.png", "start": 32927557, "end": 32928713}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/Art1.png", "start": 32928713, "end": 33061706}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/afloor1_4.png", "start": 33061706, "end": 33064707}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/afloor1_8.png", "start": 33064707, "end": 33067547}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/afloor3_1.png", "start": 33067547, "end": 33070357}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/altar1_1.png", "start": 33070357, "end": 33074245}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/altar1_3.png", "start": 33074245, "end": 33078726}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/altar1_4.png", "start": 33078726, "end": 33082779}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/brick0.png", "start": 33082779, "end": 33091001}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/brick1.png", "start": 33091001, "end": 33101755}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/brick4_s.png", "start": 33101755, "end": 33103688}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/brown1.png", "start": 33103688, "end": 33112864}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1a.png", "start": 33112864, "end": 33159219}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1b.png", "start": 33159219, "end": 33242583}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1c.png", "start": 33242583, "end": 33340754}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1d.png", "start": 33340754, "end": 33449423}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1e.png", "start": 33449423, "end": 33557916}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1f.png", "start": 33557916, "end": 33625775}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1s.png", "start": 33625775, "end": 33743226}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2a.png", "start": 33743226, "end": 33820610}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2b.png", "start": 33820610, "end": 33898049}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2c.png", "start": 33898049, "end": 33986924}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2d.png", "start": 33986924, "end": 34085118}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2e.png", "start": 34085118, "end": 34183355}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2f.png", "start": 34183355, "end": 34247899}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2s.png", "start": 34247899, "end": 34353651}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_ceil1a.png", "start": 34353651, "end": 34365459}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_ceil1b.png", "start": 34365459, "end": 34377994}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_wal2a.png", "start": 34377994, "end": 34390891}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_wal2b.png", "start": 34390891, "end": 34404147}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_wal2c.png", "start": 34404147, "end": 34416589}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_win1.png", "start": 34416589, "end": 34459587}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_win1b.png", "start": 34459587, "end": 34504913}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_but_side.png", "start": 34504913, "end": 34505911}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet1.png", "start": 34505911, "end": 34509035}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet2a.png", "start": 34509035, "end": 34512186}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet2b.png", "start": 34512186, "end": 34515248}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet2c.png", "start": 34515248, "end": 34518385}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet3a.png", "start": 34518385, "end": 34521657}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet3b.png", "start": 34521657, "end": 34524839}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet4.png", "start": 34524839, "end": 34527680}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet5a.png", "start": 34527680, "end": 34530818}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet5c.png", "start": 34530818, "end": 34533961}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk10.png", "start": 34533961, "end": 34548687}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk10_f.png", "start": 34548687, "end": 34560975}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk10b.png", "start": 34560975, "end": 34575538}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk11.png", "start": 34575538, "end": 34586786}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk12.png", "start": 34586786, "end": 34603735}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk12_f.png", "start": 34603735, "end": 34621068}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk13.png", "start": 34621068, "end": 34677185}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk14.png", "start": 34677185, "end": 34693039}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk14_f.png", "start": 34693039, "end": 34706008}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk14b.png", "start": 34706008, "end": 34719498}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk15.png", "start": 34719498, "end": 34733324}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk15b.png", "start": 34733324, "end": 34747254}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk15f.png", "start": 34747254, "end": 34760452}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk16.png", "start": 34760452, "end": 34776673}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk16b.png", "start": 34776673, "end": 34794265}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk16f.png", "start": 34794265, "end": 34811536}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk17.png", "start": 34811536, "end": 34821643}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk17_f.png", "start": 34821643, "end": 34830668}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk17b.png", "start": 34830668, "end": 34840805}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18_f.png", "start": 34840805, "end": 34849438}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18_gb.png", "start": 34849438, "end": 34852186}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18_gt.png", "start": 34852186, "end": 34854923}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18_t.png", "start": 34854923, "end": 34863109}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18_tb.png", "start": 34863109, "end": 34873735}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18_tc.png", "start": 34873735, "end": 34878264}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18b.png", "start": 34878264, "end": 34886437}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk19_f.png", "start": 34886437, "end": 34898138}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk19_t.png", "start": 34898138, "end": 34909529}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk19b.png", "start": 34909529, "end": 34920849}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk1_1.png", "start": 34920849, "end": 34923334}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk1_2.png", "start": 34923334, "end": 34926505}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk1_3.png", "start": 34926505, "end": 34928748}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk2_1.png", "start": 34928748, "end": 34931297}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk2_2.png", "start": 34931297, "end": 34934670}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk5.png", "start": 34934670, "end": 34937837}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk6_1.png", "start": 34937837, "end": 34940480}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk6_2.png", "start": 34940480, "end": 34945542}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk7_1.png", "start": 34945542, "end": 34948182}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk7_1b.png", "start": 34948182, "end": 34950726}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk7_2.png", "start": 34950726, "end": 34953892}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk8_1c.png", "start": 34953892, "end": 34966497}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk8_1d.png", "start": 34966497, "end": 34979100}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk9_1.png", "start": 34979100, "end": 34992310}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk9_1b.png", "start": 34992310, "end": 35005967}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk9_f.png", "start": 35005967, "end": 35018754}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr1_1.png", "start": 35018754, "end": 35028701}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr2_1.png", "start": 35028701, "end": 35030987}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr2_2.png", "start": 35030987, "end": 35033160}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr4_1.png", "start": 35033160, "end": 35036900}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr4_3.png", "start": 35036900, "end": 35040334}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr4_4.png", "start": 35040334, "end": 35053390}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr4_5.png", "start": 35053390, "end": 35067908}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr5_1.png", "start": 35067908, "end": 35082838}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr5_2.png", "start": 35082838, "end": 35098366}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_stp1.png", "start": 35098366, "end": 35100963}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_stp2.png", "start": 35100963, "end": 35103506}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_trm1.png", "start": 35103506, "end": 35104831}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick1.png", "start": 35104831, "end": 35126304}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick1_t.png", "start": 35126304, "end": 35137691}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick1_t2.png", "start": 35137691, "end": 35150853}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick1_t2b_fbr.png", "start": 35150853, "end": 35164803}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick1_t3.png", "start": 35164803, "end": 35176543}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick1_t4.png", "start": 35176543, "end": 35189319}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick2.png", "start": 35189319, "end": 35194853}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick3.png", "start": 35194853, "end": 35200227}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick4.png", "start": 35200227, "end": 35205535}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick5.png", "start": 35205535, "end": 35211357}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick6.png", "start": 35211357, "end": 35220518}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick6b.png", "start": 35220518, "end": 35229526}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick6f.png", "start": 35229526, "end": 35238816}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_door1.png", "start": 35238816, "end": 35243499}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_door2.png", "start": 35243499, "end": 35247998}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_door3.png", "start": 35247998, "end": 35252647}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_door3b.png", "start": 35252647, "end": 35255188}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_door4.png", "start": 35255188, "end": 35259752}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_door4b.png", "start": 35259752, "end": 35262348}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr1a.png", "start": 35262348, "end": 35281132}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr1a_blu.png", "start": 35281132, "end": 35299496}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr1b.png", "start": 35299496, "end": 35323645}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr1b_blu.png", "start": 35323645, "end": 35343264}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr2a.png", "start": 35343264, "end": 35367641}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr2a_blu.png", "start": 35367641, "end": 35387535}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr3a.png", "start": 35387535, "end": 35401018}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr3a_blu.png", "start": 35401018, "end": 35414308}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr3b.png", "start": 35414308, "end": 35428616}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr3b_blu.png", "start": 35428616, "end": 35442822}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr3c.png", "start": 35442822, "end": 35455665}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr3c_blu.png", "start": 35455665, "end": 35468789}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dwall1.png", "start": 35468789, "end": 35471265}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick1.png", "start": 35471265, "end": 35483847}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick10.png", "start": 35483847, "end": 35494732}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick10b.png", "start": 35494732, "end": 35505218}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick11.png", "start": 35505218, "end": 35520477}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick12.png", "start": 35520477, "end": 35537894}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick12b.png", "start": 35537894, "end": 35551133}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick13.png", "start": 35551133, "end": 35561031}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick14.png", "start": 35561031, "end": 35570046}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick15.png", "start": 35570046, "end": 35584847}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick16.png", "start": 35584847, "end": 35596211}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick16b.png", "start": 35596211, "end": 35604529}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick17.png", "start": 35604529, "end": 35617663}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick17b.png", "start": 35617663, "end": 35629929}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick17c.png", "start": 35629929, "end": 35643188}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick18.png", "start": 35643188, "end": 35659188}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick18b.png", "start": 35659188, "end": 35673576}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick2.png", "start": 35673576, "end": 35686068}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick20.png", "start": 35686068, "end": 35699468}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick21.png", "start": 35699468, "end": 35711610}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick22.png", "start": 35711610, "end": 35723771}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick3.png", "start": 35723771, "end": 35736231}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick4.png", "start": 35736231, "end": 35751781}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick5.png", "start": 35751781, "end": 35765597}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick6.png", "start": 35765597, "end": 35780308}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick7.png", "start": 35780308, "end": 35793953}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick8.png", "start": 35793953, "end": 35806692}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick9.png", "start": 35806692, "end": 35821247}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_etrim1.png", "start": 35821247, "end": 35824576}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_glass1.png", "start": 35824576, "end": 35836955}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_glass2.png", "start": 35836955, "end": 35848612}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_glass3.png", "start": 35848612, "end": 35859627}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_glass4.png", "start": 35859627, "end": 35891446}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_glass5.png", "start": 35891446, "end": 35901228}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_dec1.png", "start": 35901228, "end": 35905368}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_key1a.png", "start": 35905368, "end": 35907128}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_key1b.png", "start": 35907128, "end": 35908462}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_key2a.png", "start": 35908462, "end": 35910222}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_key2b.png", "start": 35910222, "end": 35911928}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_trim1.png", "start": 35911928, "end": 35912955}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_trim2.png", "start": 35912955, "end": 35913902}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_trim3.png", "start": 35913902, "end": 35914776}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_metw1a.png", "start": 35914776, "end": 35927084}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_metw1b.png", "start": 35927084, "end": 35935791}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_metw2a.png", "start": 35935791, "end": 35945698}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_metw2b.png", "start": 35945698, "end": 35958076}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_rmet.png", "start": 35958076, "end": 35971177}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_rmet_slat.png", "start": 35971177, "end": 35985488}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_rmet_tile.png", "start": 35985488, "end": 35999420}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_rmet_trim32.png", "start": 35999420, "end": 36012629}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_roof1.png", "start": 36012629, "end": 36024544}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_roof2.png", "start": 36024544, "end": 36039617}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_roof3.png", "start": 36039617, "end": 36054546}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_roof4.png", "start": 36054546, "end": 36072360}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_roof5.png", "start": 36072360, "end": 36082866}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall1.png", "start": 36082866, "end": 36124544}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall2.png", "start": 36124544, "end": 36184321}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall3.png", "start": 36184321, "end": 36234756}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall4.png", "start": 36234756, "end": 36283897}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall4_f.png", "start": 36283897, "end": 36327850}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall6.png", "start": 36327850, "end": 36372395}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall7.png", "start": 36372395, "end": 36417988}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall8.png", "start": 36417988, "end": 36455586}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall9.png", "start": 36455586, "end": 36504666}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall9_f.png", "start": 36504666, "end": 36548645}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_telepad.png", "start": 36548645, "end": 36553272}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tile1.png", "start": 36553272, "end": 36625883}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_lit1_fbr.png", "start": 36625883, "end": 36627486}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_lit3_fbr.png", "start": 36627486, "end": 36628791}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_tele.png", "start": 36628791, "end": 36631973}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_trim1.png", "start": 36631973, "end": 36635139}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_trim1b.png", "start": 36635139, "end": 36637194}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_trim2.png", "start": 36637194, "end": 36640734}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_trim3.png", "start": 36640734, "end": 36644211}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_trim4.png", "start": 36644211, "end": 36647324}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim1_1.png", "start": 36647324, "end": 36656028}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim2_1.png", "start": 36656028, "end": 36657142}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim3_1.png", "start": 36657142, "end": 36660737}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim3_2.png", "start": 36660737, "end": 36664296}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim3_3.png", "start": 36664296, "end": 36667993}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim3_4.png", "start": 36667993, "end": 36671364}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim4_1.png", "start": 36671364, "end": 36674788}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim4_2.png", "start": 36674788, "end": 36678312}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim4_3.png", "start": 36678312, "end": 36681672}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim4_4.png", "start": 36681672, "end": 36684895}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/plus_0_csl_brk14.png", "start": 36684895, "end": 36701208}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/plus_1_csl_brk14.png", "start": 36701208, "end": 36717501}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/plus_2_csl_brk14.png", "start": 36717501, "end": 36733800}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/plus_3_csl_brk14.png", "start": 36733800, "end": 36750042}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/plus_4_csl_brk14.png", "start": 36750042, "end": 36766383}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/sidewalk.png", "start": 36766383, "end": 36775139}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/sq_trim1_2.png", "start": 36775139, "end": 36783421}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/sq_trim1_2_s.png", "start": 36783421, "end": 36785996}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/tile.png", "start": 36785996, "end": 36789121}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/tile1.png", "start": 36789121, "end": 36861734}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/wall14_5.png", "start": 36861734, "end": 36865814}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/wbrick1_5.png", "start": 36865814, "end": 36869580}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/wswamp2_1.png", "start": 36869580, "end": 36872706}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/wswamp2_2.png", "start": 36872706, "end": 36876483}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig1_bone.png", "start": 36876483, "end": 36886164}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig1_bone_l.png", "start": 36886164, "end": 36916825}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig1_skull.png", "start": 36916825, "end": 36919489}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig1_skull_l.png", "start": 36919489, "end": 36928274}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig1_spine.png", "start": 36928274, "end": 36976975}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig2_bone.png", "start": 36976975, "end": 36985398}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig2_bone_l.png", "start": 36985398, "end": 37011146}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig2_bone_s.png", "start": 37011146, "end": 37013759}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig2_mouth_s.png", "start": 37013759, "end": 37016174}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/med_flat8.png", "start": 37016174, "end": 37019032}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/med_flat9.png", "start": 37019032, "end": 37022020}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_block.png", "start": 37022020, "end": 37025359}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_diam.png", "start": 37025359, "end": 37028249}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_trim28.png", "start": 37028249, "end": 37033292}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_trim32.png", "start": 37033292, "end": 37036438}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_trim32r.png", "start": 37036438, "end": 37039693}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_trim32s.png", "start": 37039693, "end": 37042793}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_trim64.png", "start": 37042793, "end": 37045936}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_block.png", "start": 37045936, "end": 37050956}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_det1.png", "start": 37050956, "end": 37052112}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_diam.png", "start": 37052112, "end": 37055009}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_diam2.png", "start": 37055009, "end": 37057804}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_diamc.png", "start": 37057804, "end": 37061335}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_door1.png", "start": 37061335, "end": 37066018}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_door2.png", "start": 37066018, "end": 37070517}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_door3.png", "start": 37070517, "end": 37075166}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_door4.png", "start": 37075166, "end": 37079730}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_door5.png", "start": 37079730, "end": 37082673}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_door6.png", "start": 37082673, "end": 37085269}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_fac1.png", "start": 37085269, "end": 37087671}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_flat.png", "start": 37087671, "end": 37091105}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_flatst.png", "start": 37091105, "end": 37093334}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_gig1.png", "start": 37093334, "end": 37097453}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_gig2.png", "start": 37097453, "end": 37101066}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_gig2b.png", "start": 37101066, "end": 37107260}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_grate.png", "start": 37107260, "end": 37110315}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_grate2.png", "start": 37110315, "end": 37113146}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_grate3.png", "start": 37113146, "end": 37114806}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_lit1_fbr.png", "start": 37114806, "end": 37116877}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_lit2_fbr.png", "start": 37116877, "end": 37118066}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_lit3.png", "start": 37118066, "end": 37119252}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_lit4.png", "start": 37119252, "end": 37120513}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_lit5.png", "start": 37120513, "end": 37122606}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_pan1.png", "start": 37122606, "end": 37126088}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_pan2.png", "start": 37126088, "end": 37128374}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_pan3.png", "start": 37128374, "end": 37131801}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_rect.png", "start": 37131801, "end": 37135580}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_rivg.png", "start": 37135580, "end": 37139501}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_rivs.png", "start": 37139501, "end": 37142859}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_slat.png", "start": 37142859, "end": 37146645}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_sqr.png", "start": 37146645, "end": 37150416}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_sqrd.png", "start": 37150416, "end": 37154254}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_sqrs.png", "start": 37154254, "end": 37158008}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_stile.png", "start": 37158008, "end": 37161235}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_tile.png", "start": 37161235, "end": 37164684}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim16.png", "start": 37164684, "end": 37168388}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim16g.png", "start": 37168388, "end": 37172138}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim16h.png", "start": 37172138, "end": 37175755}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim16s.png", "start": 37175755, "end": 37178923}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim28.png", "start": 37178923, "end": 37184220}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim32.png", "start": 37184220, "end": 37187670}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim32r.png", "start": 37187670, "end": 37191326}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim32s.png", "start": 37191326, "end": 37194650}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim64.png", "start": 37194650, "end": 37198127}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_vtrim.png", "start": 37198127, "end": 37201697}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn2_pat.png", "start": 37201697, "end": 37205546}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_block.png", "start": 37205546, "end": 37209257}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_blockl.png", "start": 37209257, "end": 37212786}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_det1.png", "start": 37212786, "end": 37213949}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_flat.png", "start": 37213949, "end": 37217470}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_grate.png", "start": 37217470, "end": 37220858}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_grate2.png", "start": 37220858, "end": 37224364}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_grate3.png", "start": 37224364, "end": 37226382}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_lit1_fbr.png", "start": 37226382, "end": 37228495}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_lit2_fbr.png", "start": 37228495, "end": 37229683}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_lit3.png", "start": 37229683, "end": 37230854}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_lit4.png", "start": 37230854, "end": 37232064}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_lit5.png", "start": 37232064, "end": 37234117}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_pan1.png", "start": 37234117, "end": 37237580}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_pan2.png", "start": 37237580, "end": 37239856}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_pan3.png", "start": 37239856, "end": 37243333}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_pan4.png", "start": 37243333, "end": 37247196}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_rect.png", "start": 37247196, "end": 37250874}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_rivg.png", "start": 37250874, "end": 37254902}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_rivs.png", "start": 37254902, "end": 37258325}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_signs.png", "start": 37258325, "end": 37263159}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_slat.png", "start": 37263159, "end": 37266802}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_sqr.png", "start": 37266802, "end": 37270493}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_sqrd.png", "start": 37270493, "end": 37274301}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_sqrs.png", "start": 37274301, "end": 37278005}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_stile.png", "start": 37278005, "end": 37282090}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_tile.png", "start": 37282090, "end": 37285471}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_tile2.png", "start": 37285471, "end": 37288804}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim16.png", "start": 37288804, "end": 37292910}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim16g.png", "start": 37292910, "end": 37297114}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim16h.png", "start": 37297114, "end": 37301311}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim16s.png", "start": 37301311, "end": 37304934}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim32.png", "start": 37304934, "end": 37308421}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim32s.png", "start": 37308421, "end": 37311807}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim64.png", "start": 37311807, "end": 37315447}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_vtrim.png", "start": 37315447, "end": 37318900}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_cop_flat.png", "start": 37318900, "end": 37321977}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_cop_riv.png", "start": 37321977, "end": 37325578}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_dbrn_flat.png", "start": 37325578, "end": 37328958}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_dbrn_rect.png", "start": 37328958, "end": 37332843}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_dbrn_slat.png", "start": 37332843, "end": 37336658}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grate.png", "start": 37336658, "end": 37340008}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_block.png", "start": 37340008, "end": 37343933}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_blockl.png", "start": 37343933, "end": 37347662}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_det1.png", "start": 37347662, "end": 37348913}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_fac1.png", "start": 37348913, "end": 37351823}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_flat.png", "start": 37351823, "end": 37355708}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_grate.png", "start": 37355708, "end": 37358770}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_grate2.png", "start": 37358770, "end": 37361753}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_grate3.png", "start": 37361753, "end": 37363497}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_lit1_fbr.png", "start": 37363497, "end": 37365631}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_lit2_fbr.png", "start": 37365631, "end": 37366832}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_lit3.png", "start": 37366832, "end": 37368044}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_lit4.png", "start": 37368044, "end": 37369330}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_lit5.png", "start": 37369330, "end": 37371522}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_pan1.png", "start": 37371522, "end": 37375238}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_pan2.png", "start": 37375238, "end": 37377603}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_pan3.png", "start": 37377603, "end": 37381203}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_rect.png", "start": 37381203, "end": 37385240}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_rivg.png", "start": 37385240, "end": 37389652}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_rivs.png", "start": 37389652, "end": 37393419}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_slat.png", "start": 37393419, "end": 37397460}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_sqr.png", "start": 37397460, "end": 37401458}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_sqrd.png", "start": 37401458, "end": 37405568}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_sqrs.png", "start": 37405568, "end": 37409598}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_stile.png", "start": 37409598, "end": 37412882}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_tile.png", "start": 37412882, "end": 37416143}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim16.png", "start": 37416143, "end": 37419755}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim16g.png", "start": 37419755, "end": 37423909}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim16h.png", "start": 37423909, "end": 37427458}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim16s.png", "start": 37427458, "end": 37430969}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim28.png", "start": 37430969, "end": 37435904}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim28r.png", "start": 37435904, "end": 37440930}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim32.png", "start": 37440930, "end": 37444755}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim32r.png", "start": 37444755, "end": 37448625}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim32s.png", "start": 37448625, "end": 37452271}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim64.png", "start": 37452271, "end": 37455928}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_vtrim.png", "start": 37455928, "end": 37459178}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_beam.png", "start": 37459178, "end": 37462192}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_block.png", "start": 37462192, "end": 37465215}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_flat.png", "start": 37465215, "end": 37467949}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_lit1_fbr.png", "start": 37467949, "end": 37469760}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_lit2_fbr.png", "start": 37469760, "end": 37470883}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_lit2b.png", "start": 37470883, "end": 37474333}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_pan1.png", "start": 37474333, "end": 37477429}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_pan2.png", "start": 37477429, "end": 37479582}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_pan3.png", "start": 37479582, "end": 37482685}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_rect.png", "start": 37482685, "end": 37485882}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_signs.png", "start": 37485882, "end": 37489712}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_slat.png", "start": 37489712, "end": 37493128}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_sqr.png", "start": 37493128, "end": 37495962}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_sqrd.png", "start": 37495962, "end": 37499210}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_sqrs.png", "start": 37499210, "end": 37502171}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_trim64.png", "start": 37502171, "end": 37505872}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_lbrn_flat.png", "start": 37505872, "end": 37509298}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_lbrn_rect.png", "start": 37509298, "end": 37513186}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_lbrn_slat.png", "start": 37513186, "end": 37517140}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_lift.png", "start": 37517140, "end": 37520185}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_met7_1.png", "start": 37520185, "end": 37522658}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mix_beam.png", "start": 37522658, "end": 37526310}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mix_diam.png", "start": 37526310, "end": 37529499}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mix_diam2.png", "start": 37529499, "end": 37532698}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mix_diamc.png", "start": 37532698, "end": 37536798}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt1_flat.png", "start": 37536798, "end": 37551088}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt1_rect.png", "start": 37551088, "end": 37566798}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt1_slat.png", "start": 37566798, "end": 37582554}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt1_sqr.png", "start": 37582554, "end": 37589094}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt2_flat.png", "start": 37589094, "end": 37605308}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt2_rect.png", "start": 37605308, "end": 37622034}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt2_slat.png", "start": 37622034, "end": 37638672}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt2_sqr.png", "start": 37638672, "end": 37647070}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt3_flat.png", "start": 37647070, "end": 37663670}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt3_rect.png", "start": 37663670, "end": 37680578}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt3_slat.png", "start": 37680578, "end": 37697489}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt3_sqr.png", "start": 37697489, "end": 37706024}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_ora_trim64.png", "start": 37706024, "end": 37708803}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_rail_flat.png", "start": 37708803, "end": 37710950}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_rune1_fbr.png", "start": 37710950, "end": 37713754}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_rune_trim32.png", "start": 37713754, "end": 37718095}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_set1.png", "start": 37718095, "end": 37787940}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_shm_flat.png", "start": 37787940, "end": 37791601}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_shm_rect.png", "start": 37791601, "end": 37795473}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_shm_slat.png", "start": 37795473, "end": 37799524}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_shm_sqr.png", "start": 37799524, "end": 37802986}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_teal_block.png", "start": 37802986, "end": 37806364}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_teal_flat.png", "start": 37806364, "end": 37809710}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_teal_trim32.png", "start": 37809710, "end": 37812986}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_teal_trim32r.png", "start": 37812986, "end": 37815961}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_teal_trim64.png", "start": 37815961, "end": 37819445}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_wall3_1.png", "start": 37819445, "end": 37830711}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_wall3_1_s.png", "start": 37830711, "end": 37833995}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/metal4_4.png", "start": 37833995, "end": 37838336}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0_sqbut1.png", "start": 37838336, "end": 37839418}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0_sqbut2_fbr.png", "start": 37839418, "end": 37842654}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0_sqshoot1_fbr.png", "start": 37842654, "end": 37843761}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0gig2a_fbr.png", "start": 37843761, "end": 37844234}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0gig_shot_fbr.png", "start": 37844234, "end": 37844710}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0gig_sshot_fbr.png", "start": 37844710, "end": 37845154}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0gig_ye_fbr.png", "start": 37845154, "end": 37845594}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0met_blu_keyg_fbr.png", "start": 37845594, "end": 37846494}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0met_blu_keys_fbr.png", "start": 37846494, "end": 37847377}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_1_sqbut1.png", "start": 37847377, "end": 37848543}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_1_sqbut2_fbr.png", "start": 37848543, "end": 37851731}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_1_sqshoot1.png", "start": 37851731, "end": 37852845}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_1met_blu_keyg_fbr.png", "start": 37852845, "end": 37853746}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_1met_blu_keys_fbr.png", "start": 37853746, "end": 37854623}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_2met_blu_keyg_fbr.png", "start": 37854623, "end": 37855532}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_2met_blu_keys_fbr.png", "start": 37855532, "end": 37856401}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_3met_blu_keyg_fbr.png", "start": 37856401, "end": 37857309}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_3met_blu_keys_fbr.png", "start": 37857309, "end": 37858165}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_4met_blu_keyg_fbr.png", "start": 37858165, "end": 37859073}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_4met_blu_keys_fbr.png", "start": 37859073, "end": 37859929}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_5met_blu_keyg_fbr.png", "start": 37859929, "end": 37860838}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_5met_blu_keys_fbr.png", "start": 37860838, "end": 37861707}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_6met_blu_keyg_fbr.png", "start": 37861707, "end": 37862608}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_6met_blu_keys_fbr.png", "start": 37862608, "end": 37863485}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_a_sqbut1.png", "start": 37863485, "end": 37864567}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_a_sqbut2_fbr.png", "start": 37864567, "end": 37867803}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_a_sqshoot1_fbr.png", "start": 37867803, "end": 37868910}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_agig2a.png", "start": 37868910, "end": 37869280}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_agig_shot_fbr.png", "start": 37869280, "end": 37869655}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_agig_sshot_fbr.png", "start": 37869655, "end": 37870018}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_agig_ye.png", "start": 37870018, "end": 37870430}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_amet_blu_keyg.png", "start": 37870430, "end": 37871300}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_amet_blu_keys.png", "start": 37871300, "end": 37872170}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/ret_metal1_tile.png", "start": 37872170, "end": 37886102}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/sq_lit1_fbr.png", "start": 37886102, "end": 37886475}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/sq_lit2_fbr.png", "start": 37886475, "end": 37886736}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_a.png", "start": 37886736, "end": 37887284}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_b.png", "start": 37887284, "end": 37887810}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_c.png", "start": 37887810, "end": 37888336}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_d.png", "start": 37888336, "end": 37888862}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_e.png", "start": 37888862, "end": 37889388}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_f.png", "start": 37889388, "end": 37889914}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_g.png", "start": 37889914, "end": 37890440}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_h.png", "start": 37890440, "end": 37890966}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_i.png", "start": 37890966, "end": 37891493}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_j.png", "start": 37891493, "end": 37892020}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_k.png", "start": 37892020, "end": 37892547}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_l.png", "start": 37892547, "end": 37893074}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_m.png", "start": 37893074, "end": 37893601}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_n.png", "start": 37893601, "end": 37894126}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_o.png", "start": 37894126, "end": 37894651}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_p.png", "start": 37894651, "end": 37895176}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_a.png", "start": 37895176, "end": 37895702}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_b.png", "start": 37895702, "end": 37896228}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_c.png", "start": 37896228, "end": 37896754}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_d.png", "start": 37896754, "end": 37897280}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_e.png", "start": 37897280, "end": 37897806}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_f.png", "start": 37897806, "end": 37898332}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_g.png", "start": 37898332, "end": 37898858}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_h.png", "start": 37898858, "end": 37899384}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_i.png", "start": 37899384, "end": 37899910}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_j.png", "start": 37899910, "end": 37900436}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_k.png", "start": 37900436, "end": 37900962}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_l.png", "start": 37900962, "end": 37901488}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_m.png", "start": 37901488, "end": 37902014}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_n.png", "start": 37902014, "end": 37902540}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_o.png", "start": 37902540, "end": 37903066}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_p.png", "start": 37903066, "end": 37903592}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_a.png", "start": 37903592, "end": 37904118}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_b.png", "start": 37904118, "end": 37904644}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_c.png", "start": 37904644, "end": 37905170}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_d.png", "start": 37905170, "end": 37905696}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_e.png", "start": 37905696, "end": 37906222}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_f.png", "start": 37906222, "end": 37906748}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_g.png", "start": 37906748, "end": 37907274}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_h.png", "start": 37907274, "end": 37907800}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_i.png", "start": 37907800, "end": 37908326}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_j.png", "start": 37908326, "end": 37908852}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_k.png", "start": 37908852, "end": 37909378}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_l.png", "start": 37909378, "end": 37909904}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_m.png", "start": 37909904, "end": 37910431}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_n.png", "start": 37910431, "end": 37910958}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_o.png", "start": 37910958, "end": 37911485}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_p.png", "start": 37911485, "end": 37912012}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_a.png", "start": 37912012, "end": 37912560}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_b.png", "start": 37912560, "end": 37913086}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_c.png", "start": 37913086, "end": 37913612}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_d.png", "start": 37913612, "end": 37914138}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_e.png", "start": 37914138, "end": 37914664}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_f.png", "start": 37914664, "end": 37915190}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_g.png", "start": 37915190, "end": 37915716}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_h.png", "start": 37915716, "end": 37916242}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_i.png", "start": 37916242, "end": 37916768}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_j.png", "start": 37916768, "end": 37917294}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_k.png", "start": 37917294, "end": 37917820}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_l.png", "start": 37917820, "end": 37918346}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_m.png", "start": 37918346, "end": 37918872}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_n.png", "start": 37918872, "end": 37919398}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_o.png", "start": 37919398, "end": 37919924}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_p.png", "start": 37919924, "end": 37920450}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_a.png", "start": 37920450, "end": 37920976}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_b.png", "start": 37920976, "end": 37921502}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_c.png", "start": 37921502, "end": 37922028}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_d.png", "start": 37922028, "end": 37922554}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_e.png", "start": 37922554, "end": 37923080}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_f.png", "start": 37923080, "end": 37923606}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_g.png", "start": 37923606, "end": 37924132}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_h.png", "start": 37924132, "end": 37924658}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_i.png", "start": 37924658, "end": 37925184}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_j.png", "start": 37925184, "end": 37925710}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_k.png", "start": 37925710, "end": 37926236}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_l.png", "start": 37926236, "end": 37926762}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_m.png", "start": 37926762, "end": 37927288}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_n.png", "start": 37927288, "end": 37927814}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_o.png", "start": 37927814, "end": 37928340}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_p.png", "start": 37928340, "end": 37928866}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_a.png", "start": 37928866, "end": 37929392}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_b.png", "start": 37929392, "end": 37929918}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_c.png", "start": 37929918, "end": 37930444}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_d.png", "start": 37930444, "end": 37930970}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_e.png", "start": 37930970, "end": 37931496}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_f.png", "start": 37931496, "end": 37932022}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_g.png", "start": 37932022, "end": 37932548}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_h.png", "start": 37932548, "end": 37933074}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_i.png", "start": 37933074, "end": 37933600}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_j.png", "start": 37933600, "end": 37934126}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_k.png", "start": 37934126, "end": 37934652}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_l.png", "start": 37934652, "end": 37935178}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_m.png", "start": 37935178, "end": 37935704}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_n.png", "start": 37935704, "end": 37936230}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_o.png", "start": 37936230, "end": 37936756}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_p.png", "start": 37936756, "end": 37937282}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_a.png", "start": 37937282, "end": 37937808}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_b.png", "start": 37937808, "end": 37938334}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_c.png", "start": 37938334, "end": 37938860}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_d.png", "start": 37938860, "end": 37939386}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_e.png", "start": 37939386, "end": 37939912}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_f.png", "start": 37939912, "end": 37940438}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_g.png", "start": 37940438, "end": 37940964}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_h.png", "start": 37940964, "end": 37941490}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_i.png", "start": 37941490, "end": 37942016}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_j.png", "start": 37942016, "end": 37942542}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_k.png", "start": 37942542, "end": 37943068}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_l.png", "start": 37943068, "end": 37943594}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_m.png", "start": 37943594, "end": 37944120}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_n.png", "start": 37944120, "end": 37944646}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_o.png", "start": 37944646, "end": 37945172}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_p.png", "start": 37945172, "end": 37945698}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_a.png", "start": 37945698, "end": 37946224}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_b.png", "start": 37946224, "end": 37946750}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_c.png", "start": 37946750, "end": 37947276}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_d.png", "start": 37947276, "end": 37947802}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_e.png", "start": 37947802, "end": 37948328}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_f.png", "start": 37948328, "end": 37948854}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_g.png", "start": 37948854, "end": 37949380}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_h.png", "start": 37949380, "end": 37949906}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_i.png", "start": 37949906, "end": 37950432}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_j.png", "start": 37950432, "end": 37950958}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_k.png", "start": 37950958, "end": 37951484}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_l.png", "start": 37951484, "end": 37952010}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_m.png", "start": 37952010, "end": 37952536}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_n.png", "start": 37952536, "end": 37953063}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_o.png", "start": 37953063, "end": 37953590}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_p.png", "start": 37953590, "end": 37954117}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_a.png", "start": 37954117, "end": 37954644}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_b.png", "start": 37954644, "end": 37955171}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_c.png", "start": 37955171, "end": 37955698}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_d.png", "start": 37955698, "end": 37956224}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_e.png", "start": 37956224, "end": 37956750}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_f.png", "start": 37956750, "end": 37957276}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_g.png", "start": 37957276, "end": 37957802}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_h.png", "start": 37957802, "end": 37958328}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_i.png", "start": 37958328, "end": 37958854}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_j.png", "start": 37958854, "end": 37959380}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_k.png", "start": 37959380, "end": 37959906}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_l.png", "start": 37959906, "end": 37960432}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_m.png", "start": 37960432, "end": 37960958}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_n.png", "start": 37960958, "end": 37961484}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_o.png", "start": 37961484, "end": 37962010}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_p.png", "start": 37962010, "end": 37962536}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_a.png", "start": 37962536, "end": 37963063}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_b.png", "start": 37963063, "end": 37963589}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_c.png", "start": 37963589, "end": 37964115}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_d.png", "start": 37964115, "end": 37964641}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_e.png", "start": 37964641, "end": 37965167}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_f.png", "start": 37965167, "end": 37965693}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_g.png", "start": 37965693, "end": 37966219}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_h.png", "start": 37966219, "end": 37966745}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_i.png", "start": 37966745, "end": 37967271}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_j.png", "start": 37967271, "end": 37967797}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_k.png", "start": 37967797, "end": 37968323}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_l.png", "start": 37968323, "end": 37968849}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_m.png", "start": 37968849, "end": 37969375}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_n.png", "start": 37969375, "end": 37969901}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_o.png", "start": 37969901, "end": 37970427}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_p.png", "start": 37970427, "end": 37970953}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_a.png", "start": 37970953, "end": 37971480}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_b.png", "start": 37971480, "end": 37972007}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_c.png", "start": 37972007, "end": 37972534}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_d.png", "start": 37972534, "end": 37973061}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_e.png", "start": 37973061, "end": 37973588}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_f.png", "start": 37973588, "end": 37974114}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_g.png", "start": 37974114, "end": 37974640}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_h.png", "start": 37974640, "end": 37975166}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_i.png", "start": 37975166, "end": 37975692}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_j.png", "start": 37975692, "end": 37976218}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_k.png", "start": 37976218, "end": 37976744}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_l.png", "start": 37976744, "end": 37977270}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_m.png", "start": 37977270, "end": 37977796}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_n.png", "start": 37977796, "end": 37978322}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_o.png", "start": 37978322, "end": 37978848}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_p.png", "start": 37978848, "end": 37979374}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_a.png", "start": 37979374, "end": 37979900}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_b.png", "start": 37979900, "end": 37980426}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_c.png", "start": 37980426, "end": 37980952}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_d.png", "start": 37980952, "end": 37981478}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_e.png", "start": 37981478, "end": 37982004}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_f.png", "start": 37982004, "end": 37982530}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_g.png", "start": 37982530, "end": 37983056}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_h.png", "start": 37983056, "end": 37983582}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_i.png", "start": 37983582, "end": 37984108}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_j.png", "start": 37984108, "end": 37984634}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_k.png", "start": 37984634, "end": 37985160}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_l.png", "start": 37985160, "end": 37985686}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_m.png", "start": 37985686, "end": 37986212}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_n.png", "start": 37986212, "end": 37986738}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_o.png", "start": 37986738, "end": 37987264}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_p.png", "start": 37987264, "end": 37987790}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_a.png", "start": 37987790, "end": 37988316}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_b.png", "start": 37988316, "end": 37988842}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_c.png", "start": 37988842, "end": 37989368}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_d.png", "start": 37989368, "end": 37989894}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_e.png", "start": 37989894, "end": 37990420}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_f.png", "start": 37990420, "end": 37990946}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_g.png", "start": 37990946, "end": 37991472}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_h.png", "start": 37991472, "end": 37991998}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_i.png", "start": 37991998, "end": 37992524}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_j.png", "start": 37992524, "end": 37993050}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_k.png", "start": 37993050, "end": 37993576}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_l.png", "start": 37993576, "end": 37994102}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_m.png", "start": 37994102, "end": 37994628}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_n.png", "start": 37994628, "end": 37995154}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_o.png", "start": 37995154, "end": 37995680}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_p.png", "start": 37995680, "end": 37996206}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_a.png", "start": 37996206, "end": 37996754}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_b.png", "start": 37996754, "end": 37997280}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_c.png", "start": 37997280, "end": 37997806}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_d.png", "start": 37997806, "end": 37998332}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_e.png", "start": 37998332, "end": 37998858}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_f.png", "start": 37998858, "end": 37999384}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_g.png", "start": 37999384, "end": 37999910}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_h.png", "start": 37999910, "end": 38000436}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_i.png", "start": 38000436, "end": 38000962}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_j.png", "start": 38000962, "end": 38001488}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_k.png", "start": 38001488, "end": 38002014}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_l.png", "start": 38002014, "end": 38002540}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_m.png", "start": 38002540, "end": 38003066}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_n.png", "start": 38003066, "end": 38003592}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_o.png", "start": 38003592, "end": 38004118}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_p.png", "start": 38004118, "end": 38004644}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_a_fbr.png", "start": 38004644, "end": 38005170}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_b_fbr.png", "start": 38005170, "end": 38005696}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_c_fbr.png", "start": 38005696, "end": 38006222}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_d_fbr.png", "start": 38006222, "end": 38006748}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_e_fbr.png", "start": 38006748, "end": 38007274}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_f_fbr.png", "start": 38007274, "end": 38007800}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_g_fbr.png", "start": 38007800, "end": 38008326}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_h_fbr.png", "start": 38008326, "end": 38008852}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_i_fbr.png", "start": 38008852, "end": 38009378}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_j_fbr.png", "start": 38009378, "end": 38009904}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_k_fbr.png", "start": 38009904, "end": 38010430}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_l_fbr.png", "start": 38010430, "end": 38010956}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_m_fbr.png", "start": 38010956, "end": 38011482}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_n_fbr.png", "start": 38011482, "end": 38012008}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_o_fbr.png", "start": 38012008, "end": 38012535}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_p_fbr.png", "start": 38012535, "end": 38013062}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_a_fbr.png", "start": 38013062, "end": 38013588}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_b_fbr.png", "start": 38013588, "end": 38014114}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_c_fbr.png", "start": 38014114, "end": 38014640}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_d_fbr.png", "start": 38014640, "end": 38015166}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_e_fbr.png", "start": 38015166, "end": 38015693}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_f_fbr.png", "start": 38015693, "end": 38016220}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_g_fbr.png", "start": 38016220, "end": 38016745}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_h_fbr.png", "start": 38016745, "end": 38017271}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_i_fbr.png", "start": 38017271, "end": 38017797}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_j_fbr.png", "start": 38017797, "end": 38018323}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_k_fbr.png", "start": 38018323, "end": 38018849}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_l_fbr.png", "start": 38018849, "end": 38019375}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_m_fbr.png", "start": 38019375, "end": 38019902}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_n_fbr.png", "start": 38019902, "end": 38020429}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_o_fbr.png", "start": 38020429, "end": 38020954}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_p_fbr.png", "start": 38020954, "end": 38021480}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/JarBod1.png", "start": 38021480, "end": 38021827}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/JarBod2.png", "start": 38021827, "end": 38022145}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/JarTop1.png", "start": 38022145, "end": 38022370}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/JarTop2.png", "start": 38022370, "end": 38022510}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/crate-door-brn.png", "start": 38022510, "end": 38031148}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/crate-door-grn.png", "start": 38031148, "end": 38041204}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/crate-door-orn.png", "start": 38041204, "end": 38050303}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/crate-side-brn.png", "start": 38050303, "end": 38070429}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/crate-side-grn.png", "start": 38070429, "end": 38093342}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/crate-side-orn.png", "start": 38093342, "end": 38114671}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/go-ep0_fbr.png", "start": 38114671, "end": 38116446}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_book_blue.png", "start": 38116446, "end": 38117026}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_book_green.png", "start": 38117026, "end": 38117649}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_book_pink.png", "start": 38117649, "end": 38118304}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_book_red.png", "start": 38118304, "end": 38119021}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_book_teal.png", "start": 38119021, "end": 38119608}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_books_wood.png", "start": 38119608, "end": 38132538}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_dbrick4_p1.png", "start": 38132538, "end": 38164375}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_dbrick4_p2.png", "start": 38164375, "end": 38197796}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_ebrick9_p1.png", "start": 38197796, "end": 38229687}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_ebrick9_p2.png", "start": 38229687, "end": 38261053}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/note-e0_fbr.png", "start": 38261053, "end": 38280482}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_0blink_fbr.png", "start": 38280482, "end": 38280711}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_0tvnoise.png", "start": 38280711, "end": 38281697}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_1blink_fbr.png", "start": 38281697, "end": 38281926}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_1tvnoise.png", "start": 38281926, "end": 38282904}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_2blink_fbr.png", "start": 38282904, "end": 38283135}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_2tvnoise.png", "start": 38283135, "end": 38284108}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_3blink_fbr.png", "start": 38284108, "end": 38284339}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_3tvnoise.png", "start": 38284339, "end": 38285321}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_4blink_fbr.png", "start": 38285321, "end": 38285551}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_4tvnoise.png", "start": 38285551, "end": 38286535}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_5tvnoise.png", "start": 38286535, "end": 38287499}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_6tvnoise.png", "start": 38287499, "end": 38288489}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_7tvnoise.png", "start": 38288489, "end": 38289463}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_8tvnoise.png", "start": 38289463, "end": 38290464}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_9tvnoise.png", "start": 38290464, "end": 38291441}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_ablink_fbr.png", "start": 38291441, "end": 38291671}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_atvnoise.png", "start": 38291671, "end": 38292231}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_atvnoise64.png", "start": 38292231, "end": 38293501}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/qr.png", "start": 38293501, "end": 38294921}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/radio16.png", "start": 38294921, "end": 38295689}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/radio32.png", "start": 38295689, "end": 38296679}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/radio64.png", "start": 38296679, "end": 38298437}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/radiowood.png", "start": 38298437, "end": 38300580}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/secret_gem_1.png", "start": 38300580, "end": 38303616}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/secret_gem_2.png", "start": 38303616, "end": 38306320}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/secret_gem_3.png", "start": 38306320, "end": 38309380}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/secret_gem_4.png", "start": 38309380, "end": 38313138}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/secret_gem_h.png", "start": 38313138, "end": 38313689}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/_t_fence01_fbr.png", "start": 38313689, "end": 38319406}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/_t_flare01_fbr.png", "start": 38319406, "end": 38319792}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqconc03.png", "start": 38319792, "end": 38330352}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqconc04.png", "start": 38330352, "end": 38341408}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqconc05.png", "start": 38341408, "end": 38345999}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqf006b.png", "start": 38345999, "end": 38348626}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqf032.png", "start": 38348626, "end": 38349897}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqf049.png", "start": 38349897, "end": 38352174}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqf074.png", "start": 38352174, "end": 38355596}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqf075.png", "start": 38355596, "end": 38358755}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqmetl01.png", "start": 38358755, "end": 38361483}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqmetl07.png", "start": 38361483, "end": 38366254}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqmetl14.png", "start": 38366254, "end": 38373986}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqmetl28.png", "start": 38373986, "end": 38375638}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqmetl30.png", "start": 38375638, "end": 38377290}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqmetl33.png", "start": 38377290, "end": 38379726}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpanl09.png", "start": 38379726, "end": 38382518}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpanl10.png", "start": 38382518, "end": 38387412}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe01.png", "start": 38387412, "end": 38391600}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe04.png", "start": 38391600, "end": 38394064}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe05.png", "start": 38394064, "end": 38397230}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe08.png", "start": 38397230, "end": 38402862}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe09.png", "start": 38402862, "end": 38410966}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe12.png", "start": 38410966, "end": 38419086}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe13.png", "start": 38419086, "end": 38425397}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe14.png", "start": 38425397, "end": 38432750}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust01.png", "start": 38432750, "end": 38435381}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust02.png", "start": 38435381, "end": 38440682}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust03.png", "start": 38440682, "end": 38445985}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust03b.png", "start": 38445985, "end": 38448715}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust04.png", "start": 38448715, "end": 38450108}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust09.png", "start": 38450108, "end": 38453996}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust10.png", "start": 38453996, "end": 38457162}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsect14.png", "start": 38457162, "end": 38459538}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsect15.png", "start": 38459538, "end": 38462485}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsect16.png", "start": 38462485, "end": 38465731}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsect16b.png", "start": 38465731, "end": 38470688}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp01.png", "start": 38470688, "end": 38473334}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp02.png", "start": 38473334, "end": 38478199}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp03.png", "start": 38478199, "end": 38482101}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp04.png", "start": 38482101, "end": 38483644}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp06.png", "start": 38483644, "end": 38485023}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp07.png", "start": 38485023, "end": 38486378}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp08.png", "start": 38486378, "end": 38488580}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp09.png", "start": 38488580, "end": 38491433}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqtrim01.png", "start": 38491433, "end": 38492660}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqtrim02.png", "start": 38492660, "end": 38493584}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqtrim03.png", "start": 38493584, "end": 38494155}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqtrim08.png", "start": 38494155, "end": 38494887}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/butmet.png", "start": 38494887, "end": 38496798}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_1.png", "start": 38496798, "end": 38500154}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_2.png", "start": 38500154, "end": 38502833}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_3.png", "start": 38502833, "end": 38505322}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_3b.png", "start": 38505322, "end": 38507950}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_4.png", "start": 38507950, "end": 38509555}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_5.png", "start": 38509555, "end": 38512654}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_6.png", "start": 38512654, "end": 38516096}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_7.png", "start": 38516096, "end": 38518441}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_8.png", "start": 38518441, "end": 38520770}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/compbase.png", "start": 38520770, "end": 38523189}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate.png", "start": 38523189, "end": 38526306}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_bottom.png", "start": 38526306, "end": 38528422}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_s_bottom.png", "start": 38528422, "end": 38529510}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_s_sside.png", "start": 38529510, "end": 38530455}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_s_top.png", "start": 38530455, "end": 38531894}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_s_tside.png", "start": 38531894, "end": 38533557}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_side.png", "start": 38533557, "end": 38537025}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_top.png", "start": 38537025, "end": 38539771}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_xs_bot.png", "start": 38539771, "end": 38540438}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_xs_sside.png", "start": 38540438, "end": 38541383}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_xs_top.png", "start": 38541383, "end": 38542253}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_xs_tside.png", "start": 38542253, "end": 38543193}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_bottom.png", "start": 38543193, "end": 38545157}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_s_bottom.png", "start": 38545157, "end": 38546187}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_s_sside.png", "start": 38546187, "end": 38547065}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_s_top.png", "start": 38547065, "end": 38548198}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_s_tside.png", "start": 38548198, "end": 38549663}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_side.png", "start": 38549663, "end": 38552780}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_top.png", "start": 38552780, "end": 38554901}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_xs_bot.png", "start": 38554901, "end": 38555555}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_xs_sside.png", "start": 38555555, "end": 38556433}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_xs_top.png", "start": 38556433, "end": 38557140}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_xs_tside.png", "start": 38557140, "end": 38557978}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/dem4_1.png", "start": 38557978, "end": 38567377}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/dem4_4.png", "start": 38567377, "end": 38575423}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/dem5_3_fbr.png", "start": 38575423, "end": 38584240}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/door02_1.png", "start": 38584240, "end": 38588372}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/doorr02_1.png", "start": 38588372, "end": 38589938}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/doortrak1.png", "start": 38589938, "end": 38591549}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/doortrak2-corn.png", "start": 38591549, "end": 38592846}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/doortrak2.png", "start": 38592846, "end": 38594511}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/ecop1_1.png", "start": 38594511, "end": 38597588}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/ecop1_4.png", "start": 38597588, "end": 38601189}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/edoor01_1.png", "start": 38601189, "end": 38613335}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/edoor02.png", "start": 38613335, "end": 38625386}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/edoor02.png.png", "start": 38625386, "end": 38637437}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/fddoor01.png", "start": 38637437, "end": 38646191}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/fddoor01b.png", "start": 38646191, "end": 38656874}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/fdoor02.png", "start": 38656874, "end": 38665428}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/flat4.png", "start": 38665428, "end": 38666601}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/floor5_2.png", "start": 38666601, "end": 38669327}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/floor5_3.png", "start": 38669327, "end": 38671507}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/laserfield1_fbr.png", "start": 38671507, "end": 38682516}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/light2.png", "start": 38682516, "end": 38683114}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/lit8nb.png", "start": 38683114, "end": 38683428}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/lit8sfb_fbr.png", "start": 38683428, "end": 38683725}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/met2.png", "start": 38683725, "end": 38696049}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/metalstrip_1.png", "start": 38696049, "end": 38698332}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_side1.png", "start": 38698332, "end": 38699097}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_stem.png", "start": 38699097, "end": 38699743}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_top1.png", "start": 38699743, "end": 38703143}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_top2.png", "start": 38703143, "end": 38706858}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_top3.png", "start": 38706858, "end": 38710266}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_top4.png", "start": 38710266, "end": 38712962}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_top5.png", "start": 38712962, "end": 38715057}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0_gkey.png", "start": 38715057, "end": 38716121}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0_skey.png", "start": 38716121, "end": 38717137}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0_tscrn0.png", "start": 38717137, "end": 38719250}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0_tscrn1.png", "start": 38719250, "end": 38721375}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0arrow2_d_fbr.png", "start": 38721375, "end": 38722629}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0arrow2_h_fbr.png", "start": 38722629, "end": 38723898}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0arrow2_u_fbr.png", "start": 38723898, "end": 38725156}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0arrow_d_fbr.png", "start": 38725156, "end": 38726038}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0arrow_h_fbr.png", "start": 38726038, "end": 38726917}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0arrow_u_fbr.png", "start": 38726917, "end": 38727792}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0basebtn1b_fbr.png", "start": 38727792, "end": 38728681}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0basebtn2_fbr.png", "start": 38728681, "end": 38729027}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0basebtn2b_fbr.png", "start": 38729027, "end": 38729367}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0basebtn_fbr.png", "start": 38729367, "end": 38730648}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0button3_fbr.png", "start": 38730648, "end": 38732524}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0lit8s.png", "start": 38732524, "end": 38732838}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0planet_a_fbr.png", "start": 38732838, "end": 38734429}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0planet_b_fbr.png", "start": 38734429, "end": 38735539}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0planet_c_fbr.png", "start": 38735539, "end": 38736639}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0slipbot.png", "start": 38736639, "end": 38739922}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0sliptop.png", "start": 38739922, "end": 38743565}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0tek_jump1_fbr.png", "start": 38743565, "end": 38746048}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0term128.png", "start": 38746048, "end": 38748852}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0term64.png", "start": 38748852, "end": 38750044}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0tlight1.png", "start": 38750044, "end": 38750642}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0tlight2.png", "start": 38750642, "end": 38751255}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0tlight3.png", "start": 38751255, "end": 38751826}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1_gkey.png", "start": 38751826, "end": 38752876}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1_skey.png", "start": 38752876, "end": 38753881}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1arrow2_d_fbr.png", "start": 38753881, "end": 38755137}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1arrow2_h_fbr.png", "start": 38755137, "end": 38756404}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1arrow2_u_fbr.png", "start": 38756404, "end": 38757662}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1arrow_d_fbr.png", "start": 38757662, "end": 38758548}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1arrow_h_fbr.png", "start": 38758548, "end": 38759430}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1arrow_u_fbr.png", "start": 38759430, "end": 38760308}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1basebtn.png", "start": 38760308, "end": 38761587}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1basebtn1b.png", "start": 38761587, "end": 38762465}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1basebtn2.png", "start": 38762465, "end": 38762815}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1basebtn2b.png", "start": 38762815, "end": 38763173}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1planet_a_fbr.png", "start": 38763173, "end": 38764794}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1planet_b_fbr.png", "start": 38764794, "end": 38765879}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1planet_c_fbr.png", "start": 38765879, "end": 38766951}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1tek_jump1_fbr.png", "start": 38766951, "end": 38769434}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1term128.png", "start": 38769434, "end": 38772237}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1term64.png", "start": 38772237, "end": 38773429}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2_gkey.png", "start": 38773429, "end": 38774491}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2_skey.png", "start": 38774491, "end": 38775497}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2arrow2_d_fbr.png", "start": 38775497, "end": 38776740}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2arrow2_h_fbr.png", "start": 38776740, "end": 38778001}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2arrow2_u_fbr.png", "start": 38778001, "end": 38779247}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2arrow_d_fbr.png", "start": 38779247, "end": 38780145}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2arrow_h_fbr.png", "start": 38780145, "end": 38781035}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2arrow_u_fbr.png", "start": 38781035, "end": 38781922}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2planet_a_fbr.png", "start": 38781922, "end": 38783525}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2planet_b_fbr.png", "start": 38783525, "end": 38784627}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2planet_c_fbr.png", "start": 38784627, "end": 38785745}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_3planet_a_fbr.png", "start": 38785745, "end": 38787335}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_3planet_b_fbr.png", "start": 38787335, "end": 38788429}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_3planet_c_fbr.png", "start": 38788429, "end": 38789552}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_4planet_a_fbr.png", "start": 38789552, "end": 38791161}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_4planet_b_fbr.png", "start": 38791161, "end": 38792276}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_4planet_c_fbr.png", "start": 38792276, "end": 38793423}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_5planet_a_fbr.png", "start": 38793423, "end": 38795055}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_5planet_b_fbr.png", "start": 38795055, "end": 38796182}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_5planet_c_fbr.png", "start": 38796182, "end": 38797289}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_6planet_a_fbr.png", "start": 38797289, "end": 38798910}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_6planet_b_fbr.png", "start": 38798910, "end": 38800034}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_6planet_c_fbr.png", "start": 38800034, "end": 38801128}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_7planet_a_fbr.png", "start": 38801128, "end": 38802716}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_7planet_b_fbr.png", "start": 38802716, "end": 38803829}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_7planet_c_fbr.png", "start": 38803829, "end": 38804921}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_8planet_a_fbr.png", "start": 38804921, "end": 38806543}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_8planet_b_fbr.png", "start": 38806543, "end": 38807670}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_8planet_c_fbr.png", "start": 38807670, "end": 38808759}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_9planet_a_fbr.png", "start": 38808759, "end": 38810343}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_9planet_b_fbr.png", "start": 38810343, "end": 38811431}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_9planet_c_fbr.png", "start": 38811431, "end": 38812507}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_A_tscrn0.png", "start": 38812507, "end": 38814130}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_A_tscrn1.png", "start": 38814130, "end": 38816639}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_A_tscrn2.png", "start": 38816639, "end": 38818239}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_abasebtn.png", "start": 38818239, "end": 38819528}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_abasebtn1b.png", "start": 38819528, "end": 38820817}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_abasebtn2.png", "start": 38820817, "end": 38821188}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_abasebtn2b.png", "start": 38821188, "end": 38821559}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_abasebtnb.png", "start": 38821559, "end": 38821930}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_abutton3_fbr.png", "start": 38821930, "end": 38823818}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_alit8s_fbr.png", "start": 38823818, "end": 38824115}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_atek_jump1_fbr.png", "start": 38824115, "end": 38826608}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_atlight1_fbr.png", "start": 38826608, "end": 38827198}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_atlight2_fbr.png", "start": 38827198, "end": 38827771}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_atlight3_fbr.png", "start": 38827771, "end": 38828373}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_1.png", "start": 38828373, "end": 38833938}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_2.png", "start": 38833938, "end": 38839792}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_3.png", "start": 38839792, "end": 38845538}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_4.png", "start": 38845538, "end": 38848104}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_4b_l.png", "start": 38848104, "end": 38851064}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_5.png", "start": 38851064, "end": 38853936}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_flat.png", "start": 38853936, "end": 38859887}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_lit.png", "start": 38859887, "end": 38860711}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_1.png", "start": 38860711, "end": 38866174}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_2.png", "start": 38866174, "end": 38872016}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_3.png", "start": 38872016, "end": 38878003}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_4.png", "start": 38878003, "end": 38880765}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_5.png", "start": 38880765, "end": 38883814}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_flat.png", "start": 38883814, "end": 38890012}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_lit.png", "start": 38890012, "end": 38890964}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_1.png", "start": 38890964, "end": 38896576}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_2.png", "start": 38896576, "end": 38902706}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_3.png", "start": 38902706, "end": 38908601}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_4.png", "start": 38908601, "end": 38915612}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_trim1.png", "start": 38915612, "end": 38918120}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_trim2.png", "start": 38918120, "end": 38919903}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_trim3.png", "start": 38919903, "end": 38922394}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw39_1_fbr.png", "start": 38922394, "end": 38928339}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/spotlight_fbr.png", "start": 38928339, "end": 38930951}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/star_lasergrid.png", "start": 38930951, "end": 38931331}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_band1a.png", "start": 38931331, "end": 38933835}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_band1b.png", "start": 38933835, "end": 38936443}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok01.png", "start": 38936443, "end": 38939121}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok01a.png", "start": 38939121, "end": 38941893}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok02.png", "start": 38941893, "end": 38947020}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok02a.png", "start": 38947020, "end": 38951879}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok03.png", "start": 38951879, "end": 38954321}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok03a.png", "start": 38954321, "end": 38956176}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok04.png", "start": 38956176, "end": 38959242}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok04h.png", "start": 38959242, "end": 38961966}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok05.png", "start": 38961966, "end": 38966254}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok06.png", "start": 38966254, "end": 38969104}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok06h.png", "start": 38969104, "end": 38971081}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok07.png", "start": 38971081, "end": 38973867}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok07a.png", "start": 38973867, "end": 38976653}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok08.png", "start": 38976653, "end": 38981160}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok09.png", "start": 38981160, "end": 38984480}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok10.png", "start": 38984480, "end": 38990491}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok10b.png", "start": 38990491, "end": 38996199}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok10c.png", "start": 38996199, "end": 38999417}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok11.png", "start": 38999417, "end": 39005358}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok11b.png", "start": 39005358, "end": 39010963}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok12c.png", "start": 39010963, "end": 39014170}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flat01.png", "start": 39014170, "end": 39016604}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flat02.png", "start": 39016604, "end": 39018969}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flat05.png", "start": 39018969, "end": 39027165}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flor1a.png", "start": 39027165, "end": 39030777}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flor1b.png", "start": 39030777, "end": 39034395}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flor2a.png", "start": 39034395, "end": 39036989}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flor2b.png", "start": 39036989, "end": 39038925}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flor2c.png", "start": 39038925, "end": 39040405}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flor2d.png", "start": 39040405, "end": 39040909}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit01_fbr.png", "start": 39040909, "end": 39041089}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit02_fbr.png", "start": 39041089, "end": 39041231}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit03_fbr.png", "start": 39041231, "end": 39041366}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit04_fbr.png", "start": 39041366, "end": 39041498}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit05_fbr.png", "start": 39041498, "end": 39041664}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit06_fbr.png", "start": 39041664, "end": 39041820}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit07_fbr.png", "start": 39041820, "end": 39044123}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit08_fbr.png", "start": 39044123, "end": 39044340}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_metalsheeta.png", "start": 39044340, "end": 39050939}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_metalsheetb.png", "start": 39050939, "end": 39062452}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_0_fbr.png", "start": 39062452, "end": 39063226}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_1_fbr.png", "start": 39063226, "end": 39063988}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_2_fbr.png", "start": 39063988, "end": 39064783}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_3_fbr.png", "start": 39064783, "end": 39065541}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_4_fbr.png", "start": 39065541, "end": 39066346}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_5_fbr.png", "start": 39066346, "end": 39067144}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_6_fbr.png", "start": 39067144, "end": 39067941}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_7_fbr.png", "start": 39067941, "end": 39068715}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_8_fbr.png", "start": 39068715, "end": 39069495}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_9_fbr.png", "start": 39069495, "end": 39070288}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_x.png", "start": 39070288, "end": 39071096}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_rivs01.png", "start": 39071096, "end": 39073524}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_rivs01a.png", "start": 39073524, "end": 39075978}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_sign1.png", "start": 39075978, "end": 39078928}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tech01.png", "start": 39078928, "end": 39082979}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tech02.png", "start": 39082979, "end": 39086082}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tech03.png", "start": 39086082, "end": 39102073}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tech04.png", "start": 39102073, "end": 39104928}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tech05.png", "start": 39104928, "end": 39107707}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tech06.png", "start": 39107707, "end": 39110519}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1a.png", "start": 39110519, "end": 39113007}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1aa.png", "start": 39113007, "end": 39115642}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1b.png", "start": 39115642, "end": 39118373}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1ba.png", "start": 39118373, "end": 39120997}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1c.png", "start": 39120997, "end": 39123782}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1ca.png", "start": 39123782, "end": 39126608}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1d.png", "start": 39126608, "end": 39128908}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1e.png", "start": 39128908, "end": 39131053}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2a.png", "start": 39131053, "end": 39134126}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2aa.png", "start": 39134126, "end": 39136778}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2b.png", "start": 39136778, "end": 39139406}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2ba.png", "start": 39139406, "end": 39142094}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2c.png", "start": 39142094, "end": 39144885}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2ca.png", "start": 39144885, "end": 39147764}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2d.png", "start": 39147764, "end": 39150226}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2e.png", "start": 39150226, "end": 39152733}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tris02.png", "start": 39152733, "end": 39155647}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall05.png", "start": 39155647, "end": 39158973}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall1a.png", "start": 39158973, "end": 39169500}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall1aa.png", "start": 39169500, "end": 39179821}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall1b.png", "start": 39179821, "end": 39190921}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall1ba.png", "start": 39190921, "end": 39201760}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall2a.png", "start": 39201760, "end": 39212544}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall2aa.png", "start": 39212544, "end": 39223564}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall2ab.png", "start": 39223564, "end": 39235321}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall2b.png", "start": 39235321, "end": 39249559}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall2ba.png", "start": 39249559, "end": 39263010}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall3a.png", "start": 39263010, "end": 39270977}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall3aa.png", "start": 39270977, "end": 39279244}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall3b.png", "start": 39279244, "end": 39287566}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall3ba.png", "start": 39287566, "end": 39296529}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall6a.png", "start": 39296529, "end": 39299763}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall6b.png", "start": 39299763, "end": 39302777}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall6c.png", "start": 39302777, "end": 39306453}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall6d.png", "start": 39306453, "end": 39310156}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall6e.png", "start": 39310156, "end": 39313952}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall7a.png", "start": 39313952, "end": 39324844}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall7b.png", "start": 39324844, "end": 39332285}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wire01.png", "start": 39332285, "end": 39335221}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wire02.png", "start": 39335221, "end": 39338756}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wire03.png", "start": 39338756, "end": 39342356}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tech04_1.png", "start": 39342356, "end": 39343179}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tech04_3.png", "start": 39343179, "end": 39344648}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tech08_1.png", "start": 39344648, "end": 39355701}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tech08_2.png", "start": 39355701, "end": 39366754}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tech10_3.png", "start": 39366754, "end": 39370610}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tech14-1.png", "start": 39370610, "end": 39380881}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/techbasetextures.txt", "start": 39380881, "end": 39381391}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/techeye1_fbr.png", "start": 39381391, "end": 39384735}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/techeye2_fbr.png", "start": 39384735, "end": 39388137}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_door1.png", "start": 39388137, "end": 39400283}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_door2.png", "start": 39400283, "end": 39412334}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_flr3.png", "start": 39412334, "end": 39415715}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_grate.png", "start": 39415715, "end": 39418593}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_lit1_fbr.png", "start": 39418593, "end": 39420228}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_lit2_fbr.png", "start": 39420228, "end": 39421214}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_lit3_fbr.png", "start": 39421214, "end": 39423056}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_lit4_fbr.png", "start": 39423056, "end": 39424183}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_pip1_fbr.png", "start": 39424183, "end": 39427265}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_pipe1.png", "start": 39427265, "end": 39430065}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_pipe2.png", "start": 39430065, "end": 39431724}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_trm1.png", "start": 39431724, "end": 39434182}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_trm3.png", "start": 39434182, "end": 39436792}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_wall4_1.png", "start": 39436792, "end": 39451237}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tele_frame1.png", "start": 39451237, "end": 39457392}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tele_frame2.png", "start": 39457392, "end": 39459343}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tele_frame3.png", "start": 39459343, "end": 39463330}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/telepad1_fbr.png", "start": 39463330, "end": 39465306}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlight11_fbr.png", "start": 39465306, "end": 39466932}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlight12_fbr.png", "start": 39466932, "end": 39468627}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlight13_fbr.png", "start": 39468627, "end": 39470686}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlightblfb_fbr.png", "start": 39470686, "end": 39471259}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlightfb_fbr.png", "start": 39471259, "end": 39471849}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlightnb.png", "start": 39471849, "end": 39472447}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlightrdfb_fbr.png", "start": 39472447, "end": 39473049}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/treadplatemetal.png", "start": 39473049, "end": 39487803}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/twall2_3.png", "start": 39487803, "end": 39491311}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/w17_1.png", "start": 39491311, "end": 39508335}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/w94_1.png", "start": 39508335, "end": 39521283}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/z_exit_fbr.png", "start": 39521283, "end": 39522844}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/afloor1_3.png", "start": 39522844, "end": 39525842}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/asphalt.png", "start": 39525842, "end": 39543302}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/azfloor1_1.png", "start": 39543302, "end": 39546321}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/badlawn.png", "start": 39546321, "end": 39588130}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/cracks1-1.png", "start": 39588130, "end": 39591149}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/darkrock.png", "start": 39591149, "end": 39613718}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/grass1.png", "start": 39613718, "end": 39624289}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/gravel1.png", "start": 39624289, "end": 39637643}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/gravel2.png", "start": 39637643, "end": 39652540}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/grk_leaf1_1.png", "start": 39652540, "end": 39656533}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/grk_leaf1_2.png", "start": 39656533, "end": 39660270}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/marbbrn128.png", "start": 39660270, "end": 39670512}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_drt1_1.png", "start": 39670512, "end": 39673408}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_drt1_2.png", "start": 39673408, "end": 39676262}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_drt2_2.png", "start": 39676262, "end": 39679006}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_rck1_1.png", "start": 39679006, "end": 39693106}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_rck1_2.png", "start": 39693106, "end": 39703784}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_rck1_3.png", "start": 39703784, "end": 39716627}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_slat1_1.png", "start": 39716627, "end": 39719475}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_bigdirt.png", "start": 39719475, "end": 39874058}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_bigdirt2.png", "start": 39874058, "end": 40028209}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_bigdirt3.png", "start": 40028209, "end": 40182160}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn1_1.png", "start": 40182160, "end": 40196541}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn1_1a.png", "start": 40196541, "end": 40213515}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn1_2.png", "start": 40213515, "end": 40227954}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn1_2a.png", "start": 40227954, "end": 40244769}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn2_1.png", "start": 40244769, "end": 40258738}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn2_1a.png", "start": 40258738, "end": 40276049}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn2_2.png", "start": 40276049, "end": 40291123}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn2_2a.png", "start": 40291123, "end": 40308227}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cracks1.png", "start": 40308227, "end": 40321689}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat1.png", "start": 40321689, "end": 40334598}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat12.png", "start": 40334598, "end": 40345986}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat15.png", "start": 40345986, "end": 40359087}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat16.png", "start": 40359087, "end": 40371929}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat2.png", "start": 40371929, "end": 40383206}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat3.png", "start": 40383206, "end": 40396571}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat4.png", "start": 40396571, "end": 40406520}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat5.png", "start": 40406520, "end": 40417605}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat5a.png", "start": 40417605, "end": 40428349}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat6.png", "start": 40428349, "end": 40437334}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat7.png", "start": 40437334, "end": 40448792}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_plaster2.png", "start": 40448792, "end": 40458444}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock1.png", "start": 40458444, "end": 40509701}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock10.png", "start": 40509701, "end": 40518996}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock10a.png", "start": 40518996, "end": 40528310}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock10b.png", "start": 40528310, "end": 40541675}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock10c.png", "start": 40541675, "end": 40554010}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock2.png", "start": 40554010, "end": 40595887}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock3.png", "start": 40595887, "end": 40641831}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock3_bump.png", "start": 40641831, "end": 40757925}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock4.png", "start": 40757925, "end": 40769281}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock5.png", "start": 40769281, "end": 40781813}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock9.png", "start": 40781813, "end": 40792837}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/ret_plaster1.png", "start": 40792837, "end": 40804114}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/rock1_1.png", "start": 40804114, "end": 40854485}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/rock1_1b.png", "start": 40854485, "end": 40865500}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/rock1_2.png", "start": 40865500, "end": 40907543}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/rocks07.png", "start": 40907543, "end": 40918567}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/rocks11d.png", "start": 40918567, "end": 40927881}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/rocks11e.png", "start": 40927881, "end": 40937176}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/sand.png", "start": 40937176, "end": 40963263}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/snow1.png", "start": 40963263, "end": 40965126}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/uwall1_2.png", "start": 40965126, "end": 40992777}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/vines1.png", "start": 40992777, "end": 40997832}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/black.png", "start": 40997832, "end": 40998380}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/clip.png", "start": 40998380, "end": 40998886}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/hint.png", "start": 40998886, "end": 40999752}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/hintskip.png", "start": 40999752, "end": 41000645}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/light_fbr.png", "start": 41000645, "end": 41001561}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/origin.png", "start": 41001561, "end": 41002044}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/skip.png", "start": 41002044, "end": 41002532}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/star_lavaskip.png", "start": 41002532, "end": 41003645}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/star_slimeskip.png", "start": 41003645, "end": 41004718}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/star_waterskip.png", "start": 41004718, "end": 41006458}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/trigger.png", "start": 41006458, "end": 41006955}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/crate4.png", "start": 41006955, "end": 41010097}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/crwdh6.png", "start": 41010097, "end": 41015105}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/crwdl12.png", "start": 41015105, "end": 41017538}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/crwds6.png", "start": 41017538, "end": 41018564}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/may_crate3-small.png", "start": 41018564, "end": 41019555}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/may_crate3.png", "start": 41019555, "end": 41022618}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/may_wood1_1.png", "start": 41022618, "end": 41025458}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/may_wood1_2.png", "start": 41025458, "end": 41028307}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_ret_wood1.png", "start": 41028307, "end": 41035276}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood1.png", "start": 41035276, "end": 41062644}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood2.png", "start": 41062644, "end": 41070157}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood2_plk1.png", "start": 41070157, "end": 41081855}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood2_plk2.png", "start": 41081855, "end": 41094347}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood3.png", "start": 41094347, "end": 41100347}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood3_plk1.png", "start": 41100347, "end": 41113562}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood4.png", "start": 41113562, "end": 41120686}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood5.png", "start": 41120686, "end": 41128484}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood6.png", "start": 41128484, "end": 41135264}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood7.png", "start": 41135264, "end": 41144303}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood8.png", "start": 41144303, "end": 41152267}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood_riv1.png", "start": 41152267, "end": 41158892}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood_riv1b.png", "start": 41158892, "end": 41164971}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood_riv1c.png", "start": 41164971, "end": 41170095}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood_riv2.png", "start": 41170095, "end": 41176505}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood_riv2b.png", "start": 41176505, "end": 41182925}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood_riv2c.png", "start": 41182925, "end": 41188297}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank1.png", "start": 41188297, "end": 41194711}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank1s.png", "start": 41194711, "end": 41196592}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank2.png", "start": 41196592, "end": 41203172}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank2s.png", "start": 41203172, "end": 41205061}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank3.png", "start": 41205061, "end": 41211921}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank3s.png", "start": 41211921, "end": 41213902}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank4.png", "start": 41213902, "end": 41220348}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank4s.png", "start": 41220348, "end": 41222238}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank5.png", "start": 41222238, "end": 41227887}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/sq_wood_1.png", "start": 41227887, "end": 41243928}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/sq_wood_2.png", "start": 41243928, "end": 41246816}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/sq_wood_2a.png", "start": 41246816, "end": 41250185}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/wood_1.png", "start": 41250185, "end": 41254363}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/wood_2.png", "start": 41254363, "end": 41258263}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodbark128.png", "start": 41258263, "end": 41268470}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodbark1m28.png", "start": 41268470, "end": 41277919}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodbark64.png", "start": 41277919, "end": 41280856}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodbarkA128.png", "start": 41280856, "end": 41289554}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodbarkm64.png", "start": 41289554, "end": 41292231}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodend.png", "start": 41292231, "end": 41294734}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodring128.png", "start": 41294734, "end": 41303158}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodring64.png", "start": 41303158, "end": 41305619}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodringm128.png", "start": 41305619, "end": 41315918}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodringm64.png", "start": 41315918, "end": 41319177}, {"filename": "/GameData/Textures/brushes/light.png", "start": 41319177, "end": 41319305}, {"filename": "/GameData/Textures/brushes/light_em.png", "start": 41319305, "end": 41319433}, {"filename": "/GameData/Textures/brushes/mask_test_m.png", "start": 41319433, "end": 41323495}, {"filename": "/GameData/Textures/brushes/mirror.png", "start": 41323495, "end": 41323615}, {"filename": "/GameData/Textures/brushes/mirror_orm.png", "start": 41323615, "end": 41323735}, {"filename": "/GameData/Textures/brushes/null_m.png", "start": 41323735, "end": 41325532}, {"filename": "/GameData/Textures/brushes/tormentPack/+0str_bloodfall.png", "start": 41325532, "end": 41327609}, {"filename": "/GameData/Textures/brushes/tormentPack/+1str_bloodfall.png", "start": 41327609, "end": 41329643}, {"filename": "/GameData/Textures/brushes/tormentPack/+2str_bloodfall.png", "start": 41329643, "end": 41331741}, {"filename": "/GameData/Textures/brushes/tormentPack/+3str_bloodfall.png", "start": 41331741, "end": 41333812}, {"filename": "/GameData/Textures/brushes/tormentPack/+4str_bloodfall.png", "start": 41333812, "end": 41335872}, {"filename": "/GameData/Textures/brushes/tormentPack/+5str_bloodfall.png", "start": 41335872, "end": 41337893}, {"filename": "/GameData/Textures/brushes/tormentPack/+6str_bloodfall.png", "start": 41337893, "end": 41339939}, {"filename": "/GameData/Textures/brushes/tormentPack/+7str_bloodfall.png", "start": 41339939, "end": 41342008}, {"filename": "/GameData/Textures/brushes/tormentPack/str_blood.png", "start": 41342008, "end": 41344016}, {"filename": "/GameData/Textures/brushes/tormentPack/str_blood_large.png", "start": 41344016, "end": 41366566}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein1.png", "start": 41366566, "end": 41393241}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein2.png", "start": 41393241, "end": 41425274}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein3.png", "start": 41425274, "end": 41458945}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein4.png", "start": 41458945, "end": 41490587}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein5.png", "start": 41490587, "end": 41524078}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein6.png", "start": 41524078, "end": 41558552}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein7.png", "start": 41558552, "end": 41593443}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein8.png", "start": 41593443, "end": 41621592}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein9.png", "start": 41621592, "end": 41657045}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr1.png", "start": 41657045, "end": 41693739}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr2.png", "start": 41693739, "end": 41730314}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr3.png", "start": 41730314, "end": 41761307}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr4.png", "start": 41761307, "end": 41791477}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr5.png", "start": 41791477, "end": 41830927}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr6.png", "start": 41830927, "end": 41870087}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr7.png", "start": 41870087, "end": 41911779}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr8.png", "start": 41911779, "end": 41952197}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgen1.png", "start": 41952197, "end": 41981467}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgen2.png", "start": 41981467, "end": 42010983}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgen3.png", "start": 42010983, "end": 42040459}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgen4.png", "start": 42040459, "end": 42065282}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgen5.png", "start": 42065282, "end": 42090152}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgen6.png", "start": 42090152, "end": 42114242}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl1.png", "start": 42114242, "end": 42143627}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl2.png", "start": 42143627, "end": 42176357}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl3.png", "start": 42176357, "end": 42202975}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl4.png", "start": 42202975, "end": 42232239}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl5.png", "start": 42232239, "end": 42266614}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl6.png", "start": 42266614, "end": 42305109}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl7.png", "start": 42305109, "end": 42341259}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl8.png", "start": 42341259, "end": 42379252}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan1.png", "start": 42379252, "end": 42414112}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan2.png", "start": 42414112, "end": 42449653}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan3.png", "start": 42449653, "end": 42480089}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan4.png", "start": 42480089, "end": 42511049}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan5.png", "start": 42511049, "end": 42547421}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan6.png", "start": 42547421, "end": 42585463}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan7.png", "start": 42585463, "end": 42624692}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan8.png", "start": 42624692, "end": 42663786}, {"filename": "/GameData/Textures/brushes/tormentPack/str_rotwoodgen1.png", "start": 42663786, "end": 42682365}, {"filename": "/GameData/Textures/brushes/tormentPack/str_rotwoodgen2.png", "start": 42682365, "end": 42701661}, {"filename": "/GameData/Textures/brushes/tormentPack/str_rotwoodgen3.png", "start": 42701661, "end": 42721715}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk1.png", "start": 42721715, "end": 42747107}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk2.png", "start": 42747107, "end": 42774109}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk3.png", "start": 42774109, "end": 42796739}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk4.png", "start": 42796739, "end": 42820792}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk5.png", "start": 42820792, "end": 42851876}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk6.png", "start": 42851876, "end": 42884548}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk7.png", "start": 42884548, "end": 42913072}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk8.png", "start": 42913072, "end": 42943324}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stoneflr1.png", "start": 42943324, "end": 42967480}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stoneflr2.png", "start": 42967480, "end": 42995386}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stoneflr3.png", "start": 42995386, "end": 43016391}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stoneflr4.png", "start": 43016391, "end": 43037535}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stoneflr5.png", "start": 43037535, "end": 43069067}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stoneflr6.png", "start": 43069067, "end": 43103350}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonegen1.png", "start": 43103350, "end": 43122373}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonegen2.png", "start": 43122373, "end": 43141744}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonegen3.png", "start": 43141744, "end": 43163516}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonegen4.png", "start": 43163516, "end": 43188561}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonegen5.png", "start": 43188561, "end": 43213949}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonegen6.png", "start": 43213949, "end": 43241386}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonerubble.png", "start": 43241386, "end": 43269780}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall1.png", "start": 43269780, "end": 43293037}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall2.png", "start": 43293037, "end": 43317842}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall3.png", "start": 43317842, "end": 43342505}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall4.png", "start": 43342505, "end": 43368975}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall5.png", "start": 43368975, "end": 43396442}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall6.png", "start": 43396442, "end": 43424610}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall7.png", "start": 43424610, "end": 43453999}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall8.png", "start": 43453999, "end": 43484895}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_bloodgunk.png", "start": 43484895, "end": 43511344}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_bloodweb1.png", "start": 43511344, "end": 43528584}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_bloodweb2.png", "start": 43528584, "end": 43537535}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_bloodweb3.png", "start": 43537535, "end": 43543546}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_grating1.png", "start": 43543546, "end": 43552127}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_grating2.png", "start": 43552127, "end": 43567330}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_grating3.png", "start": 43567330, "end": 43586678}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_grating4.png", "start": 43586678, "end": 43598835}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_grating5.png", "start": 43598835, "end": 43620340}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_grating6.png", "start": 43620340, "end": 43648586}, {"filename": "/GameData/Textures/brushes/trigger.png", "start": 43648586, "end": 43660462}, {"filename": "/GameData/Textures/brushes/trigger_t.png", "start": 43660462, "end": 43672338}, {"filename": "/GameData/Textures/brushes/white.png", "start": 43672338, "end": 43672458}, {"filename": "/GameData/Textures/gloves.png", "start": 43672458, "end": 43782133}, {"filename": "/GameData/Textures/jacket.png", "start": 43782133, "end": 43945945}, {"filename": "/GameData/Textures/muzzle_t.png", "start": 43945945, "end": 43955353}, {"filename": "/GameData/Textures/particles/blood.png", "start": 43955353, "end": 43959066}, {"filename": "/GameData/Textures/particles/smoke.png", "start": 43959066, "end": 43962936}, {"filename": "/GameData/Textures/particles/trail.png", "start": 43962936, "end": 43985297}, {"filename": "/GameData/Textures/particles/wood.png", "start": 43985297, "end": 44000780}, {"filename": "/GameData/Textures/shirt.png", "start": 44000780, "end": 44192531}, {"filename": "/GameData/arms.glb", "start": 44192531, "end": 44372295}, {"filename": "/GameData/bass_beat.ogg", "start": 44372295, "end": 44739171, "audio": 1}, {"filename": "/GameData/bass_beat.wav", "start": 44739171, "end": 50435505, "audio": 1}, {"filename": "/GameData/cat.png", "start": 50435505, "end": 50684943}, {"filename": "/GameData/cube.mtl", "start": 50684943, "end": 50684994}, {"filename": "/GameData/cube.obj", "start": 50684994, "end": 50685913}, {"filename": "/GameData/dog.bin", "start": 50685913, "end": 50751957}, {"filename": "/GameData/dog.dae", "start": 50751957, "end": 51151029}, {"filename": "/GameData/dog.fbx", "start": 51151029, "end": 52065361}, {"filename": "/GameData/dog.glb", "start": 52065361, "end": 52208149}, {"filename": "/GameData/dog.gltf", "start": 52208149, "end": 52299276}, {"filename": "/GameData/happy_hog.png", "start": 52299276, "end": 53216401}, {"filename": "/GameData/hog_sheet.png", "start": 53216401, "end": 55071477}, {"filename": "/GameData/level_1_bg_crop.png", "start": 55071477, "end": 55678656}, {"filename": "/GameData/mini_hog.ico", "start": 55678656, "end": 55682942}, {"filename": "/GameData/monkey.fbx", "start": 55682942, "end": 55714362}, {"filename": "/GameData/sound_test.ogg", "start": 55714362, "end": 55743470, "audio": 1}, {"filename": "/GameData/sprite_sheet_sticher.py", "start": 55743470, "end": 55743996}, {"filename": "/GameData/testViewmodel.glb", "start": 55743996, "end": 56558968}, {"filename": "/GameData/title_bg_crop.png", "start": 56558968, "end": 57474076}, {"filename": "/GameData/yummy.ogg", "start": 57474076, "end": 57509667, "audio": 1}], "remote_package_size": 57509667});

  })();

// end include: C:\Users\Admin\AppData\Local\Temp\tmpewrflgvk.js
// include: C:\Users\Admin\AppData\Local\Temp\tmpd9pi365q.js

    // All the pre-js content up to here must remain later on, we need to run
    // it.
    if (Module['$ww'] || (typeof ENVIRONMENT_IS_PTHREAD != 'undefined' && ENVIRONMENT_IS_PTHREAD)) Module['preRun'] = [];
    var necessaryPreJSTasks = Module['preRun'].slice();
  // end include: C:\Users\Admin\AppData\Local\Temp\tmpd9pi365q.js
// include: C:\Users\Admin\AppData\Local\Temp\tmpvbxhkvzt.js

    if (!Module['preRun']) throw 'Module.preRun should exist because file support used it; did a pre-js delete it?';
    necessaryPreJSTasks.forEach((task) => {
      if (Module['preRun'].indexOf(task) < 0) throw 'All preRun tasks that exist before user pre-js code should remain after; did you replace Module or modify Module.preRun?';
    });
  // end include: C:\Users\Admin\AppData\Local\Temp\tmpvbxhkvzt.js


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
  1502888: () => { if (typeof(AudioContext) !== 'undefined') { return true; } else if (typeof(webkitAudioContext) !== 'undefined') { return true; } return false; },  
 1503035: () => { if ((typeof(navigator.mediaDevices) !== 'undefined') && (typeof(navigator.mediaDevices.getUserMedia) !== 'undefined')) { return true; } else if (typeof(navigator.webkitGetUserMedia) !== 'undefined') { return true; } return false; },  
 1503269: ($0) => { if(typeof(Module['SDL2']) === 'undefined') { Module['SDL2'] = {}; } var SDL2 = Module['SDL2']; if (!$0) { SDL2.audio = {}; } else { SDL2.capture = {}; } if (!SDL2.audioContext) { if (typeof(AudioContext) !== 'undefined') { SDL2.audioContext = new AudioContext(); } else if (typeof(webkitAudioContext) !== 'undefined') { SDL2.audioContext = new webkitAudioContext(); } if (SDL2.audioContext) { if ((typeof navigator.userActivation) === 'undefined') { autoResumeAudioContext(SDL2.audioContext); } } } return SDL2.audioContext === undefined ? -1 : 0; },  
 1503821: () => { var SDL2 = Module['SDL2']; return SDL2.audioContext.sampleRate; },  
 1503889: ($0, $1, $2, $3) => { var SDL2 = Module['SDL2']; var have_microphone = function(stream) { if (SDL2.capture.silenceTimer !== undefined) { clearInterval(SDL2.capture.silenceTimer); SDL2.capture.silenceTimer = undefined; SDL2.capture.silenceBuffer = undefined } SDL2.capture.mediaStreamNode = SDL2.audioContext.createMediaStreamSource(stream); SDL2.capture.scriptProcessorNode = SDL2.audioContext.createScriptProcessor($1, $0, 1); SDL2.capture.scriptProcessorNode.onaudioprocess = function(audioProcessingEvent) { if ((SDL2 === undefined) || (SDL2.capture === undefined)) { return; } audioProcessingEvent.outputBuffer.getChannelData(0).fill(0.0); SDL2.capture.currentCaptureBuffer = audioProcessingEvent.inputBuffer; dynCall('vi', $2, [$3]); }; SDL2.capture.mediaStreamNode.connect(SDL2.capture.scriptProcessorNode); SDL2.capture.scriptProcessorNode.connect(SDL2.audioContext.destination); SDL2.capture.stream = stream; }; var no_microphone = function(error) { }; SDL2.capture.silenceBuffer = SDL2.audioContext.createBuffer($0, $1, SDL2.audioContext.sampleRate); SDL2.capture.silenceBuffer.getChannelData(0).fill(0.0); var silence_callback = function() { SDL2.capture.currentCaptureBuffer = SDL2.capture.silenceBuffer; dynCall('vi', $2, [$3]); }; SDL2.capture.silenceTimer = setInterval(silence_callback, ($1 / SDL2.audioContext.sampleRate) * 1000); if ((navigator.mediaDevices !== undefined) && (navigator.mediaDevices.getUserMedia !== undefined)) { navigator.mediaDevices.getUserMedia({ audio: true, video: false }).then(have_microphone).catch(no_microphone); } else if (navigator.webkitGetUserMedia !== undefined) { navigator.webkitGetUserMedia({ audio: true, video: false }, have_microphone, no_microphone); } },  
 1505582: ($0, $1, $2, $3) => { var SDL2 = Module['SDL2']; SDL2.audio.scriptProcessorNode = SDL2.audioContext['createScriptProcessor']($1, 0, $0); SDL2.audio.scriptProcessorNode['onaudioprocess'] = function (e) { if ((SDL2 === undefined) || (SDL2.audio === undefined)) { return; } if (SDL2.audio.silenceTimer !== undefined) { clearInterval(SDL2.audio.silenceTimer); SDL2.audio.silenceTimer = undefined; SDL2.audio.silenceBuffer = undefined; } SDL2.audio.currentOutputBuffer = e['outputBuffer']; dynCall('vi', $2, [$3]); }; SDL2.audio.scriptProcessorNode['connect'](SDL2.audioContext['destination']); if (SDL2.audioContext.state === 'suspended') { SDL2.audio.silenceBuffer = SDL2.audioContext.createBuffer($0, $1, SDL2.audioContext.sampleRate); SDL2.audio.silenceBuffer.getChannelData(0).fill(0.0); var silence_callback = function() { if ((typeof navigator.userActivation) !== 'undefined') { if (navigator.userActivation.hasBeenActive) { SDL2.audioContext.resume(); } } SDL2.audio.currentOutputBuffer = SDL2.audio.silenceBuffer; dynCall('vi', $2, [$3]); SDL2.audio.currentOutputBuffer = undefined; }; SDL2.audio.silenceTimer = setInterval(silence_callback, ($1 / SDL2.audioContext.sampleRate) * 1000); } },  
 1506757: ($0, $1) => { var SDL2 = Module['SDL2']; var numChannels = SDL2.capture.currentCaptureBuffer.numberOfChannels; for (var c = 0; c < numChannels; ++c) { var channelData = SDL2.capture.currentCaptureBuffer.getChannelData(c); if (channelData.length != $1) { throw 'Web Audio capture buffer length mismatch! Destination size: ' + channelData.length + ' samples vs expected ' + $1 + ' samples!'; } if (numChannels == 1) { for (var j = 0; j < $1; ++j) { setValue($0 + (j * 4), channelData[j], 'float'); } } else { for (var j = 0; j < $1; ++j) { setValue($0 + (((j * numChannels) + c) * 4), channelData[j], 'float'); } } } },  
 1507362: ($0, $1) => { var SDL2 = Module['SDL2']; var buf = $0 >>> 2; var numChannels = SDL2.audio.currentOutputBuffer['numberOfChannels']; for (var c = 0; c < numChannels; ++c) { var channelData = SDL2.audio.currentOutputBuffer['getChannelData'](c); if (channelData.length != $1) { throw 'Web Audio output buffer length mismatch! Destination size: ' + channelData.length + ' samples vs expected ' + $1 + ' samples!'; } for (var j = 0; j < $1; ++j) { channelData[j] = HEAPF32[buf + (j*numChannels + c)]; } } },  
 1507851: ($0) => { var SDL2 = Module['SDL2']; if ($0) { if (SDL2.capture.silenceTimer !== undefined) { clearInterval(SDL2.capture.silenceTimer); } if (SDL2.capture.stream !== undefined) { var tracks = SDL2.capture.stream.getAudioTracks(); for (var i = 0; i < tracks.length; i++) { SDL2.capture.stream.removeTrack(tracks[i]); } } if (SDL2.capture.scriptProcessorNode !== undefined) { SDL2.capture.scriptProcessorNode.onaudioprocess = function(audioProcessingEvent) {}; SDL2.capture.scriptProcessorNode.disconnect(); } if (SDL2.capture.mediaStreamNode !== undefined) { SDL2.capture.mediaStreamNode.disconnect(); } SDL2.capture = undefined; } else { if (SDL2.audio.scriptProcessorNode != undefined) { SDL2.audio.scriptProcessorNode.disconnect(); } if (SDL2.audio.silenceTimer !== undefined) { clearInterval(SDL2.audio.silenceTimer); } SDL2.audio = undefined; } if ((SDL2.audioContext !== undefined) && (SDL2.audio === undefined) && (SDL2.capture === undefined)) { SDL2.audioContext.close(); SDL2.audioContext = undefined; } },  
 1508857: ($0) => { var str = UTF8ToString($0) + '\n\n' + 'Abort/Retry/Ignore/AlwaysIgnore? [ariA] :'; var reply = window.prompt(str, "i"); if (reply === null) { reply = "i"; } return allocate(intArrayFromString(reply), 'i8', ALLOC_NORMAL); },  
 1509082: ($0, $1, $2) => { var w = $0; var h = $1; var pixels = $2; if (!Module['SDL2']) Module['SDL2'] = {}; var SDL2 = Module['SDL2']; if (SDL2.ctxCanvas !== Module['canvas']) { SDL2.ctx = Module['createContext'](Module['canvas'], false, true); SDL2.ctxCanvas = Module['canvas']; } if (SDL2.w !== w || SDL2.h !== h || SDL2.imageCtx !== SDL2.ctx) { SDL2.image = SDL2.ctx.createImageData(w, h); SDL2.w = w; SDL2.h = h; SDL2.imageCtx = SDL2.ctx; } var data = SDL2.image.data; var src = pixels / 4; var dst = 0; var num; if (typeof CanvasPixelArray !== 'undefined' && data instanceof CanvasPixelArray) { num = data.length; while (dst < num) { var val = HEAP32[src]; data[dst ] = val & 0xff; data[dst+1] = (val >> 8) & 0xff; data[dst+2] = (val >> 16) & 0xff; data[dst+3] = 0xff; src++; dst += 4; } } else { if (SDL2.data32Data !== data) { SDL2.data32 = new Int32Array(data.buffer); SDL2.data8 = new Uint8Array(data.buffer); SDL2.data32Data = data; } var data32 = SDL2.data32; num = data32.length; data32.set(HEAP32.subarray(src, src + num)); var data8 = SDL2.data8; var i = 3; var j = i + 4*num; if (num % 8 == 0) { while (i < j) { data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; } } else { while (i < j) { data8[i] = 0xff; i = i + 4 | 0; } } } SDL2.ctx.putImageData(SDL2.image, 0, 0); },  
 1510550: ($0, $1, $2, $3, $4) => { var w = $0; var h = $1; var hot_x = $2; var hot_y = $3; var pixels = $4; var canvas = document.createElement("canvas"); canvas.width = w; canvas.height = h; var ctx = canvas.getContext("2d"); var image = ctx.createImageData(w, h); var data = image.data; var src = pixels / 4; var dst = 0; var num; if (typeof CanvasPixelArray !== 'undefined' && data instanceof CanvasPixelArray) { num = data.length; while (dst < num) { var val = HEAP32[src]; data[dst ] = val & 0xff; data[dst+1] = (val >> 8) & 0xff; data[dst+2] = (val >> 16) & 0xff; data[dst+3] = (val >> 24) & 0xff; src++; dst += 4; } } else { var data32 = new Int32Array(data.buffer); num = data32.length; data32.set(HEAP32.subarray(src, src + num)); } ctx.putImageData(image, 0, 0); var url = hot_x === 0 && hot_y === 0 ? "url(" + canvas.toDataURL() + "), auto" : "url(" + canvas.toDataURL() + ") " + hot_x + " " + hot_y + ", auto"; var urlBuf = _malloc(url.length + 1); stringToUTF8(url, urlBuf, url.length + 1); return urlBuf; },  
 1511538: ($0) => { if (Module['canvas']) { Module['canvas'].style['cursor'] = UTF8ToString($0); } },  
 1511621: () => { if (Module['canvas']) { Module['canvas'].style['cursor'] = 'none'; } },  
 1511690: () => { return window.innerWidth; },  
 1511720: () => { return window.innerHeight; }
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

