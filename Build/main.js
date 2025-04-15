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
// include: C:\Users\Admin\AppData\Local\Temp\tmp6k4tdppf.js

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
Module['FS_createPath']("/GameData", "Shaders", true, true);
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
    loadPackage({"files": [{"filename": "/GameData/Fonts/Kingthings_Calligraphica_2.ttf", "start": 0, "end": 29804}, {"filename": "/GameData/Maps/e1m1.map", "start": 29804, "end": 10041333}, {"filename": "/GameData/Maps/e1m1.obj", "start": 10041333, "end": 19886354}, {"filename": "/GameData/Maps/test.map", "start": 19886354, "end": 19902308}, {"filename": "/GameData/Maps/test.mtl", "start": 19902308, "end": 19902585}, {"filename": "/GameData/Maps/test.obj", "start": 19902585, "end": 19921146}, {"filename": "/GameData/Maps/test2.map", "start": 19921146, "end": 20579021}, {"filename": "/GameData/Maps/test2.mtl", "start": 20579021, "end": 20579716}, {"filename": "/GameData/Maps/test2.obj", "start": 20579716, "end": 21483013}, {"filename": "/GameData/Shaders/default_pixel.frag", "start": 21483013, "end": 21483280}, {"filename": "/GameData/Shaders/default_vertex.vert", "start": 21483280, "end": 21483612}, {"filename": "/GameData/Shaders/empty_pixel.frag", "start": 21483612, "end": 21483679}, {"filename": "/GameData/Shaders/skeletal.vert", "start": 21483679, "end": 21484966}, {"filename": "/GameData/Shaders/solidRed_pixel.frag", "start": 21484966, "end": 21485135}, {"filename": "/GameData/Shaders/ui.vert", "start": 21485135, "end": 21485438}, {"filename": "/GameData/Shaders/ui_flatcolor.frag", "start": 21485438, "end": 21485574}, {"filename": "/GameData/Shaders/ui_textured.frag", "start": 21485574, "end": 21485831}, {"filename": "/GameData/Textures/M_Shotgun_Base_Color.png", "start": 21485831, "end": 21860973}, {"filename": "/GameData/Textures/arms.png", "start": 21860973, "end": 21901217}, {"filename": "/GameData/Textures/brushes/Ground/grass.png", "start": 21901217, "end": 21905001}, {"filename": "/GameData/Textures/brushes/Metal/metal1.png", "start": 21905001, "end": 22329057}, {"filename": "/GameData/Textures/brushes/Wall/brickWall1.png", "start": 22329057, "end": 22337273}, {"filename": "/GameData/Textures/brushes/Wall/brickWall2.png", "start": 22337273, "end": 22345397}, {"filename": "/GameData/Textures/brushes/Wall/brickWall3.png", "start": 22345397, "end": 22353280}, {"filename": "/GameData/Textures/brushes/Water/Water1_t.png", "start": 22353280, "end": 22666381}, {"filename": "/GameData/Textures/brushes/Wood/wood1.png", "start": 22666381, "end": 23014733}, {"filename": "/GameData/Textures/brushes/__TB_empty.png", "start": 23014733, "end": 23015491}, {"filename": "/GameData/Textures/brushes/brick.png", "start": 23015491, "end": 23528405}, {"filename": "/GameData/Textures/brushes/brickPBR.png", "start": 23528405, "end": 24458537}, {"filename": "/GameData/Textures/brushes/brickPBR_orm.png", "start": 24458537, "end": 24883388}, {"filename": "/GameData/Textures/brushes/bricks.png", "start": 24883388, "end": 24894308}, {"filename": "/GameData/Textures/brushes/cat.png", "start": 24894308, "end": 25143746}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door1a.png", "start": 25143746, "end": 25151530}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door1b.png", "start": 25151530, "end": 25160720}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door1c.png", "start": 25160720, "end": 25168571}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door1d.png", "start": 25168571, "end": 25177743}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door2a.png", "start": 25177743, "end": 25185467}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door2b.png", "start": 25185467, "end": 25194592}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door2c.png", "start": 25194592, "end": 25202620}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door2d.png", "start": 25202620, "end": 25211848}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door3a.png", "start": 25211848, "end": 25215996}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door3b.png", "start": 25215996, "end": 25220774}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door3c.png", "start": 25220774, "end": 25224924}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door3d.png", "start": 25224924, "end": 25229716}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door4a.png", "start": 25229716, "end": 25233837}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door4b.png", "start": 25233837, "end": 25238628}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door4c.png", "start": 25238628, "end": 25242898}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_door4d.png", "start": 25242898, "end": 25247734}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric1a.png", "start": 25247734, "end": 25253110}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric1b.png", "start": 25253110, "end": 25259238}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric1c.png", "start": 25259238, "end": 25265532}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric1d.png", "start": 25265532, "end": 25269168}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric1e.png", "start": 25269168, "end": 25272854}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric2a.png", "start": 25272854, "end": 25277579}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric2b.png", "start": 25277579, "end": 25283030}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric2c.png", "start": 25283030, "end": 25288708}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric2d.png", "start": 25288708, "end": 25291994}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric2e.png", "start": 25291994, "end": 25295355}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric3a.png", "start": 25295355, "end": 25300309}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric3b.png", "start": 25300309, "end": 25305980}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric3c.png", "start": 25305980, "end": 25311805}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric3d.png", "start": 25311805, "end": 25315185}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric3e.png", "start": 25315185, "end": 25318627}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric4a.png", "start": 25318627, "end": 25328045}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric4b.png", "start": 25328045, "end": 25338010}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric4c.png", "start": 25338010, "end": 25348139}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric4d.png", "start": 25348139, "end": 25353663}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric4e.png", "start": 25353663, "end": 25359233}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric5a.png", "start": 25359233, "end": 25368300}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric5b.png", "start": 25368300, "end": 25377947}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric5c.png", "start": 25377947, "end": 25387747}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric5d.png", "start": 25387747, "end": 25393157}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric5e.png", "start": 25393157, "end": 25398591}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric6a.png", "start": 25398591, "end": 25405725}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric6b.png", "start": 25405725, "end": 25413453}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric6c.png", "start": 25413453, "end": 25421348}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric6d.png", "start": 25421348, "end": 25425748}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_fabric6e.png", "start": 25425748, "end": 25430206}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground1a.png", "start": 25430206, "end": 25438128}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground1b.png", "start": 25438128, "end": 25446964}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground1c.png", "start": 25446964, "end": 25456460}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground1d.png", "start": 25456460, "end": 25466503}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground2a.png", "start": 25466503, "end": 25476371}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground2b.png", "start": 25476371, "end": 25485278}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground2c.png", "start": 25485278, "end": 25495003}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground2d.png", "start": 25495003, "end": 25505424}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground3a.png", "start": 25505424, "end": 25515483}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground3b.png", "start": 25515483, "end": 25525195}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground3c.png", "start": 25525195, "end": 25534631}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground4a.png", "start": 25534631, "end": 25544822}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground4b.png", "start": 25544822, "end": 25555272}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_ground4c.png", "start": 25555272, "end": 25565213}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litmetal1a.png", "start": 25565213, "end": 25569241}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litmetal1b.png", "start": 25569241, "end": 25573953}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litmetal1c.png", "start": 25573953, "end": 25578034}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litmetal2a.png", "start": 25578034, "end": 25582087}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litmetal2b.png", "start": 25582087, "end": 25586846}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litmetal2c.png", "start": 25586846, "end": 25590717}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litstone1a.png", "start": 25590717, "end": 25594095}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litstone1b.png", "start": 25594095, "end": 25598308}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litstone1c.png", "start": 25598308, "end": 25601939}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litstone2a.png", "start": 25601939, "end": 25606736}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litstone2b.png", "start": 25606736, "end": 25612262}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_litstone2c.png", "start": 25612262, "end": 25616505}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalgen1.png", "start": 25616505, "end": 25622330}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalgen2.png", "start": 25622330, "end": 25629210}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan1a.png", "start": 25629210, "end": 25637268}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan1b.png", "start": 25637268, "end": 25644738}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan1c.png", "start": 25644738, "end": 25653940}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan2a.png", "start": 25653940, "end": 25662267}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan2b.png", "start": 25662267, "end": 25669679}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan2c.png", "start": 25669679, "end": 25678878}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan3a.png", "start": 25678878, "end": 25687485}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan3b.png", "start": 25687485, "end": 25695712}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan3c.png", "start": 25695712, "end": 25705014}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan4a.png", "start": 25705014, "end": 25713775}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan4b.png", "start": 25713775, "end": 25722100}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpan4c.png", "start": 25722100, "end": 25731555}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip1a.png", "start": 25731555, "end": 25738199}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip1b.png", "start": 25738199, "end": 25744711}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip2a.png", "start": 25744711, "end": 25751844}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip2b.png", "start": 25751844, "end": 25758862}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip3a.png", "start": 25758862, "end": 25766133}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip3b.png", "start": 25766133, "end": 25773211}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip4a.png", "start": 25773211, "end": 25780779}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metalpip4b.png", "start": 25780779, "end": 25788134}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm1a.png", "start": 25788134, "end": 25795132}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm1b.png", "start": 25795132, "end": 25801665}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm1c.png", "start": 25801665, "end": 25809853}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm2a.png", "start": 25809853, "end": 25817350}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm2b.png", "start": 25817350, "end": 25823938}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm2c.png", "start": 25823938, "end": 25832199}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm3a.png", "start": 25832199, "end": 25839475}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm3b.png", "start": 25839475, "end": 25846401}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm3c.png", "start": 25846401, "end": 25854396}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm4a.png", "start": 25854396, "end": 25861276}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm4b.png", "start": 25861276, "end": 25867984}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm4c.png", "start": 25867984, "end": 25875793}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm5.png", "start": 25875793, "end": 25879452}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_metaltrm6.png", "start": 25879452, "end": 25883175}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr1a.png", "start": 25883175, "end": 25889967}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr1b.png", "start": 25889967, "end": 25898338}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr1c.png", "start": 25898338, "end": 25906008}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr1d.png", "start": 25906008, "end": 25914125}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr1e.png", "start": 25914125, "end": 25922017}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr2a.png", "start": 25922017, "end": 25930208}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr2b.png", "start": 25930208, "end": 25939720}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr2c.png", "start": 25939720, "end": 25948622}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr2d.png", "start": 25948622, "end": 25957873}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr2e.png", "start": 25957873, "end": 25966944}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr3a.png", "start": 25966944, "end": 25975075}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr3b.png", "start": 25975075, "end": 25984785}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr3c.png", "start": 25984785, "end": 25993757}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr3d.png", "start": 25993757, "end": 26003231}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr3e.png", "start": 26003231, "end": 26012458}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr4a.png", "start": 26012458, "end": 26020964}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr4b.png", "start": 26020964, "end": 26029424}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr4c.png", "start": 26029424, "end": 26038088}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr5a.png", "start": 26038088, "end": 26045267}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr5b.png", "start": 26045267, "end": 26052395}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr5c.png", "start": 26052395, "end": 26060535}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr6a.png", "start": 26060535, "end": 26069211}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr6b.png", "start": 26069211, "end": 26077626}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slateflr6c.png", "start": 26077626, "end": 26086359}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slategen1.png", "start": 26086359, "end": 26092320}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_slategen2.png", "start": 26092320, "end": 26099879}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk1a.png", "start": 26099879, "end": 26105829}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk1b.png", "start": 26105829, "end": 26111645}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk1c.png", "start": 26111645, "end": 26117589}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk1d.png", "start": 26117589, "end": 26123235}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk2a.png", "start": 26123235, "end": 26132107}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk2b.png", "start": 26132107, "end": 26140781}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk2c.png", "start": 26140781, "end": 26149578}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk2d.png", "start": 26149578, "end": 26157874}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk3a.png", "start": 26157874, "end": 26166321}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk3b.png", "start": 26166321, "end": 26174968}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk4a.png", "start": 26174968, "end": 26181587}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk4b.png", "start": 26181587, "end": 26188273}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk4c.png", "start": 26188273, "end": 26194890}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk4d.png", "start": 26194890, "end": 26201504}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk4e.png", "start": 26201504, "end": 26208088}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk4f.png", "start": 26208088, "end": 26214445}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk5a.png", "start": 26214445, "end": 26223922}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk5b.png", "start": 26223922, "end": 26233434}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk5c.png", "start": 26233434, "end": 26242932}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk5d.png", "start": 26242932, "end": 26252405}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk5e.png", "start": 26252405, "end": 26261796}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk5f.png", "start": 26261796, "end": 26270933}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk6a.png", "start": 26270933, "end": 26279858}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonebrk6b.png", "start": 26279858, "end": 26289083}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr1a.png", "start": 26289083, "end": 26294513}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr1b.png", "start": 26294513, "end": 26300935}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr1c.png", "start": 26300935, "end": 26307958}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr2a.png", "start": 26307958, "end": 26315846}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr2b.png", "start": 26315846, "end": 26324661}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr2c.png", "start": 26324661, "end": 26333942}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr3a.png", "start": 26333942, "end": 26339645}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr3b.png", "start": 26339645, "end": 26346123}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr3c.png", "start": 26346123, "end": 26353115}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr4a.png", "start": 26353115, "end": 26361507}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr4b.png", "start": 26361507, "end": 26370576}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr4c.png", "start": 26370576, "end": 26379676}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr5a.png", "start": 26379676, "end": 26385766}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr5b.png", "start": 26385766, "end": 26392179}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr5c.png", "start": 26392179, "end": 26399097}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr6a.png", "start": 26399097, "end": 26407642}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr6b.png", "start": 26407642, "end": 26416462}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stoneflr6c.png", "start": 26416462, "end": 26425746}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonegen1.png", "start": 26425746, "end": 26430840}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonegen2.png", "start": 26430840, "end": 26438270}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonestep1a.png", "start": 26438270, "end": 26443713}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonestep1b.png", "start": 26443713, "end": 26449253}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonestep2a.png", "start": 26449253, "end": 26457102}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonestep2b.png", "start": 26457102, "end": 26464852}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm1a.png", "start": 26464852, "end": 26470142}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm1b.png", "start": 26470142, "end": 26476242}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm1c.png", "start": 26476242, "end": 26481818}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm2a.png", "start": 26481818, "end": 26489564}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm2b.png", "start": 26489564, "end": 26498394}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm2c.png", "start": 26498394, "end": 26506702}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm3a.png", "start": 26506702, "end": 26512002}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm3b.png", "start": 26512002, "end": 26518075}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm4a.png", "start": 26518075, "end": 26525897}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm4b.png", "start": 26525897, "end": 26534428}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm5.png", "start": 26534428, "end": 26537585}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonetrm6.png", "start": 26537585, "end": 26541940}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonewaf1a.png", "start": 26541940, "end": 26547551}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonewaf1b.png", "start": 26547551, "end": 26553168}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonewaf2a.png", "start": 26553168, "end": 26561041}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_stonewaf2b.png", "start": 26561041, "end": 26569150}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood1a.png", "start": 26569150, "end": 26573849}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood1b.png", "start": 26573849, "end": 26578808}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood2a.png", "start": 26578808, "end": 26585504}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood2b.png", "start": 26585504, "end": 26592461}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood3a.png", "start": 26592461, "end": 26597665}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood3b.png", "start": 26597665, "end": 26603123}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood4a.png", "start": 26603123, "end": 26609955}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood4b.png", "start": 26609955, "end": 26617000}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5a.png", "start": 26617000, "end": 26623013}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5b.png", "start": 26623013, "end": 26629848}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5c.png", "start": 26629848, "end": 26636858}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5d.png", "start": 26636858, "end": 26644448}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5e.png", "start": 26644448, "end": 26653581}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5f.png", "start": 26653581, "end": 26662610}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5g.png", "start": 26662610, "end": 26671344}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood5h.png", "start": 26671344, "end": 26680072}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6a.png", "start": 26680072, "end": 26687702}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6b.png", "start": 26687702, "end": 26696150}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6c.png", "start": 26696150, "end": 26703718}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6d.png", "start": 26703718, "end": 26712115}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6e.png", "start": 26712115, "end": 26721872}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6f.png", "start": 26721872, "end": 26731818}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6g.png", "start": 26731818, "end": 26741021}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_wood6h.png", "start": 26741021, "end": 26750460}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_woodgen1.png", "start": 26750460, "end": 26755007}, {"filename": "/GameData/Textures/brushes/delvenPack/dlv_woodgen2.png", "start": 26755007, "end": 26761700}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry1a.png", "start": 26761700, "end": 26769487}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry1b.png", "start": 26769487, "end": 26776634}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry1c.png", "start": 26776634, "end": 26780904}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry1d.png", "start": 26780904, "end": 26784914}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry2a.png", "start": 26784914, "end": 26792053}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry2b.png", "start": 26792053, "end": 26798612}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry2c.png", "start": 26798612, "end": 26802433}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry2d.png", "start": 26802433, "end": 26806071}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry3a.png", "start": 26806071, "end": 26813200}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry3b.png", "start": 26813200, "end": 26819786}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry3c.png", "start": 26819786, "end": 26823759}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry3d.png", "start": 26823759, "end": 26827531}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry4a.png", "start": 26827531, "end": 26838413}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry4b.png", "start": 26838413, "end": 26847251}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry4c.png", "start": 26847251, "end": 26852753}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry4d.png", "start": 26852753, "end": 26857463}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry5a.png", "start": 26857463, "end": 26868047}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry5b.png", "start": 26868047, "end": 26876673}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry5c.png", "start": 26876673, "end": 26881982}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry5d.png", "start": 26881982, "end": 26886532}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry6a.png", "start": 26886532, "end": 26895381}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry6b.png", "start": 26895381, "end": 26902659}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry6c.png", "start": 26902659, "end": 26907277}, {"filename": "/GameData/Textures/brushes/delvenPack/{dlv_tapestry6d.png", "start": 26907277, "end": 26911289}, {"filename": "/GameData/Textures/brushes/foil.png", "start": 26911289, "end": 27168507}, {"filename": "/GameData/Textures/brushes/grass.png", "start": 27168507, "end": 27296632}, {"filename": "/GameData/Textures/brushes/hole_t.png", "start": 27296632, "end": 27299061}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_1.png", "start": 27299061, "end": 27312174}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_10.png", "start": 27312174, "end": 27326288}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_2.png", "start": 27326288, "end": 27340329}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_3.png", "start": 27340329, "end": 27354839}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_4.png", "start": 27354839, "end": 27369225}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_5.png", "start": 27369225, "end": 27383674}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_6.png", "start": 27383674, "end": 27398291}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_7.png", "start": 27398291, "end": 27412888}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_8.png", "start": 27412888, "end": 27427236}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_9.png", "start": 27427236, "end": 27441394}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc1_a1.png", "start": 27441394, "end": 27455842}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_1.png", "start": 27455842, "end": 27465270}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_10.png", "start": 27465270, "end": 27474719}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_2.png", "start": 27474719, "end": 27484113}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_3.png", "start": 27484113, "end": 27493758}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_4.png", "start": 27493758, "end": 27503265}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_5.png", "start": 27503265, "end": 27512770}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_6.png", "start": 27512770, "end": 27522283}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_7.png", "start": 27522283, "end": 27531828}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_8.png", "start": 27531828, "end": 27541238}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_9.png", "start": 27541238, "end": 27550775}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc2_a1.png", "start": 27550775, "end": 27559805}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_1.png", "start": 27559805, "end": 27567669}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_10.png", "start": 27567669, "end": 27576605}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_2.png", "start": 27576605, "end": 27584999}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_3.png", "start": 27584999, "end": 27595226}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_4.png", "start": 27595226, "end": 27604505}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_5.png", "start": 27604505, "end": 27614235}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_6.png", "start": 27614235, "end": 27623795}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_7.png", "start": 27623795, "end": 27632620}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_8.png", "start": 27632620, "end": 27641401}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_9.png", "start": 27641401, "end": 27650877}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc3_a1.png", "start": 27650877, "end": 27659284}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_1.png", "start": 27659284, "end": 27665443}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_10.png", "start": 27665443, "end": 27671803}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_2.png", "start": 27671803, "end": 27678068}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_3.png", "start": 27678068, "end": 27684994}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_4.png", "start": 27684994, "end": 27691484}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_5.png", "start": 27691484, "end": 27698077}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_6.png", "start": 27698077, "end": 27704601}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_7.png", "start": 27704601, "end": 27711009}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_8.png", "start": 27711009, "end": 27717292}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_9.png", "start": 27717292, "end": 27723875}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc4_a1.png", "start": 27723875, "end": 27729944}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc5_1.png", "start": 27729944, "end": 27738706}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc5_10.png", "start": 27738706, "end": 27748321}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc5_2.png", "start": 27748321, "end": 27757637}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc5_8.png", "start": 27757637, "end": 27766717}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc5_9.png", "start": 27766717, "end": 27776431}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc5_a1.png", "start": 27776431, "end": 27784806}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_1.png", "start": 27784806, "end": 27791219}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_10.png", "start": 27791219, "end": 27797628}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_2.png", "start": 27797628, "end": 27803956}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_3.png", "start": 27803956, "end": 27810817}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_4.png", "start": 27810817, "end": 27817425}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_5.png", "start": 27817425, "end": 27824101}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_6.png", "start": 27824101, "end": 27830875}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_7.png", "start": 27830875, "end": 27837578}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_8.png", "start": 27837578, "end": 27844074}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_9.png", "start": 27844074, "end": 27850696}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc6_a1.png", "start": 27850696, "end": 27857340}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_1.png", "start": 27857340, "end": 27865130}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_10.png", "start": 27865130, "end": 27872808}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_2.png", "start": 27872808, "end": 27880434}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_3.png", "start": 27880434, "end": 27888378}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_4.png", "start": 27888378, "end": 27896146}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_5.png", "start": 27896146, "end": 27903970}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_6.png", "start": 27903970, "end": 27911969}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_7.png", "start": 27911969, "end": 27919949}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_8.png", "start": 27919949, "end": 27927773}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_9.png", "start": 27927773, "end": 27935573}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/conc7_a1.png", "start": 27935573, "end": 27943306}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr1_1.png", "start": 27943306, "end": 27945638}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr1_2.png", "start": 27945638, "end": 27948902}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr1_3.png", "start": 27948902, "end": 27952442}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr1_4.png", "start": 27952442, "end": 27955219}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr1_5.png", "start": 27955219, "end": 27958815}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr1_6.png", "start": 27958815, "end": 27961871}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_1.png", "start": 27961871, "end": 27963981}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_2.png", "start": 27963981, "end": 27967194}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_3.png", "start": 27967194, "end": 27970814}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_4.png", "start": 27970814, "end": 27974212}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_5.png", "start": 27974212, "end": 27977572}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_6.png", "start": 27977572, "end": 27980771}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_7.png", "start": 27980771, "end": 27983906}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_conc/flr2_8.png", "start": 27983906, "end": 27986847}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/clip.png", "start": 27986847, "end": 27987353}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_blue_a.png", "start": 27987353, "end": 27987909}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_blue_b.png", "start": 27987909, "end": 27988464}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_blue_c.png", "start": 27988464, "end": 27989019}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_brown_a.png", "start": 27989019, "end": 27989574}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_brown_b.png", "start": 27989574, "end": 27990129}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_brown_c.png", "start": 27990129, "end": 27990684}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_green_a.png", "start": 27990684, "end": 27991240}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_green_b.png", "start": 27991240, "end": 27991795}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_green_c.png", "start": 27991795, "end": 27992350}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_grey_a.png", "start": 27992350, "end": 27992903}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_grey_b.png", "start": 27992903, "end": 27993459}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_grey_c.png", "start": 27993459, "end": 27994014}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_olive_a.png", "start": 27994014, "end": 27994569}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_olive_b.png", "start": 27994569, "end": 27995123}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_olive_c.png", "start": 27995123, "end": 27995678}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_orange_a.png", "start": 27995678, "end": 27996234}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_orange_b.png", "start": 27996234, "end": 27996789}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_orange_c.png", "start": 27996789, "end": 27997344}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_pink_a.png", "start": 27997344, "end": 27997900}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_pink_b.png", "start": 27997900, "end": 27998456}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_pink_c.png", "start": 27998456, "end": 27999011}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_purple_a.png", "start": 27999011, "end": 27999567}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_purple_b.png", "start": 27999567, "end": 28000122}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_purple_c.png", "start": 28000122, "end": 28000677}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_red_a.png", "start": 28000677, "end": 28001228}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_red_b.png", "start": 28001228, "end": 28001779}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_red_c.png", "start": 28001779, "end": 28002330}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_tan_a.png", "start": 28002330, "end": 28002886}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_tan_b.png", "start": 28002886, "end": 28003442}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_tan_c.png", "start": 28003442, "end": 28003997}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_yellow_a.png", "start": 28003997, "end": 28004553}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_yellow_b.png", "start": 28004553, "end": 28005109}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/dot_yellow_c.png", "start": 28005109, "end": 28005663}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_blue_a.png", "start": 28005663, "end": 28006480}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_blue_b.png", "start": 28006480, "end": 28007298}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_blue_c.png", "start": 28007298, "end": 28008116}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_brown_a.png", "start": 28008116, "end": 28008938}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_brown_b.png", "start": 28008938, "end": 28009761}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_brown_c.png", "start": 28009761, "end": 28010583}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_green_a.png", "start": 28010583, "end": 28011404}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_green_b.png", "start": 28011404, "end": 28012226}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_green_c.png", "start": 28012226, "end": 28013048}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_grey_a.png", "start": 28013048, "end": 28013858}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_grey_b.png", "start": 28013858, "end": 28014671}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_grey_c.png", "start": 28014671, "end": 28015485}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_olive_a.png", "start": 28015485, "end": 28016303}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_olive_b.png", "start": 28016303, "end": 28017118}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_olive_c.png", "start": 28017118, "end": 28017935}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_orange_a.png", "start": 28017935, "end": 28018758}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_orange_b.png", "start": 28018758, "end": 28019581}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_orange_c.png", "start": 28019581, "end": 28020401}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_pink_a.png", "start": 28020401, "end": 28021224}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_pink_b.png", "start": 28021224, "end": 28022046}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_pink_c.png", "start": 28022046, "end": 28022868}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_purple_a.png", "start": 28022868, "end": 28023690}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_purple_b.png", "start": 28023690, "end": 28024512}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_purple_c.png", "start": 28024512, "end": 28025333}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_red_a.png", "start": 28025333, "end": 28026143}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_red_b.png", "start": 28026143, "end": 28026953}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_red_c.png", "start": 28026953, "end": 28027764}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_tan_a.png", "start": 28027764, "end": 28028584}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_tan_b.png", "start": 28028584, "end": 28029408}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_tan_c.png", "start": 28029408, "end": 28030229}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_yellow_a.png", "start": 28030229, "end": 28031050}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_yellow_b.png", "start": 28031050, "end": 28031872}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/floor_yellow_c.png", "start": 28031872, "end": 28032691}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/hint.png", "start": 28032691, "end": 28033557}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/hintskip.png", "start": 28033557, "end": 28034450}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/key_gold_1.png", "start": 28034450, "end": 28035205}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/key_silver_1.png", "start": 28035205, "end": 28035959}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/light_fbr.png", "start": 28035959, "end": 28036875}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/origin.png", "start": 28036875, "end": 28037358}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/plus_0_button_fbr.png", "start": 28037358, "end": 28037557}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/plus_0_shoot_fbr.png", "start": 28037557, "end": 28037836}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/plus_1_button_fbr.png", "start": 28037836, "end": 28038037}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/plus_1_shoot_fbr.png", "start": 28038037, "end": 28038318}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/plus_a_button_fbr.png", "start": 28038318, "end": 28038519}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/plus_a_shoot_fbr.png", "start": 28038519, "end": 28038806}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/skip.png", "start": 28038806, "end": 28039294}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/sky_dev.png", "start": 28039294, "end": 28041266}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/sky_dev.png.bak", "start": 28041266, "end": 28053620}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/sky_dev_day_fbr.png", "start": 28053620, "end": 28057446}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/sky_dev_void.png", "start": 28057446, "end": 28058357}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_blood1.png", "start": 28058357, "end": 28058719}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_lava1.png", "start": 28058719, "end": 28059090}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_lavaskip.png", "start": 28059090, "end": 28060203}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_slime1.png", "start": 28060203, "end": 28060573}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_slimeskip.png", "start": 28060573, "end": 28061646}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_smile.png", "start": 28061646, "end": 28062126}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_teleport.png", "start": 28062126, "end": 28062450}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_water1.png", "start": 28062450, "end": 28062822}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_water2.png", "start": 28062822, "end": 28063191}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/star_waterskip.png", "start": 28063191, "end": 28064931}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/trigger.png", "start": 28064931, "end": 28065428}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_blue_a.png", "start": 28065428, "end": 28066108}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_blue_b.png", "start": 28066108, "end": 28066788}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_blue_c.png", "start": 28066788, "end": 28067468}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_brown_a.png", "start": 28067468, "end": 28068150}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_brown_b.png", "start": 28068150, "end": 28068832}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_brown_c.png", "start": 28068832, "end": 28069514}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_green_a.png", "start": 28069514, "end": 28070196}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_green_b.png", "start": 28070196, "end": 28070878}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_green_c.png", "start": 28070878, "end": 28071560}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_grey_a.png", "start": 28071560, "end": 28072234}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_grey_b.png", "start": 28072234, "end": 28072910}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_grey_c.png", "start": 28072910, "end": 28073587}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_olive_a.png", "start": 28073587, "end": 28074267}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_olive_b.png", "start": 28074267, "end": 28074944}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_olive_c.png", "start": 28074944, "end": 28075623}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_orange_a.png", "start": 28075623, "end": 28076305}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_orange_b.png", "start": 28076305, "end": 28076987}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_orange_c.png", "start": 28076987, "end": 28077668}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_pink_a.png", "start": 28077668, "end": 28078350}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_pink_b.png", "start": 28078350, "end": 28079032}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_pink_c.png", "start": 28079032, "end": 28079714}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_purple_a.png", "start": 28079714, "end": 28080397}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_purple_b.png", "start": 28080397, "end": 28081079}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_purple_c.png", "start": 28081079, "end": 28081761}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_red_a.png", "start": 28081761, "end": 28082434}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_red_b.png", "start": 28082434, "end": 28083107}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_red_c.png", "start": 28083107, "end": 28083781}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_tan_a.png", "start": 28083781, "end": 28084463}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_tan_b.png", "start": 28084463, "end": 28085145}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_tan_c.png", "start": 28085145, "end": 28085827}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_yellow_a.png", "start": 28085827, "end": 28086509}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_yellow_b.png", "start": 28086509, "end": 28087191}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/wall_yellow_c.png", "start": 28087191, "end": 28087872}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_0_fbr.png", "start": 28087872, "end": 28088049}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_1_fbr.png", "start": 28088049, "end": 28088209}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_2_fbr.png", "start": 28088209, "end": 28088381}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_3_fbr.png", "start": 28088381, "end": 28088560}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_4_fbr.png", "start": 28088560, "end": 28088723}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_5_fbr.png", "start": 28088723, "end": 28088896}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_6_fbr.png", "start": 28088896, "end": 28089068}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_7_fbr.png", "start": 28089068, "end": 28089222}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_8_fbr.png", "start": 28089222, "end": 28089390}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_9_fbr.png", "start": 28089390, "end": 28089557}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_a_fbr.png", "start": 28089557, "end": 28089721}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_b_fbr.png", "start": 28089721, "end": 28089891}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_c_fbr.png", "start": 28089891, "end": 28090041}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_d_fbr.png", "start": 28090041, "end": 28090207}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_e_fbr.png", "start": 28090207, "end": 28090375}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_f_fbr.png", "start": 28090375, "end": 28090535}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_g_fbr.png", "start": 28090535, "end": 28090707}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_h_fbr.png", "start": 28090707, "end": 28090866}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_i_fbr.png", "start": 28090866, "end": 28091025}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_j_fbr.png", "start": 28091025, "end": 28091191}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_k_fbr.png", "start": 28091191, "end": 28091385}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_l_fbr.png", "start": 28091385, "end": 28091526}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_m_fbr.png", "start": 28091526, "end": 28091675}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_n_fbr.png", "start": 28091675, "end": 28091858}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_o_fbr.png", "start": 28091858, "end": 28092008}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_p_fbr.png", "start": 28092008, "end": 28092166}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_q_fbr.png", "start": 28092166, "end": 28092332}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_r_fbr.png", "start": 28092332, "end": 28092504}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_s_fbr.png", "start": 28092504, "end": 28092680}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_t_fbr.png", "start": 28092680, "end": 28092828}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_trans_fbr.png", "start": 28092828, "end": 28092951}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_u_fbr.png", "start": 28092951, "end": 28093097}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_v_fbr.png", "start": 28093097, "end": 28093265}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_w_fbr.png", "start": 28093265, "end": 28093414}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_x_fbr.png", "start": 28093414, "end": 28093596}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_y_fbr.png", "start": 28093596, "end": 28093765}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{char_z_fbr.png", "start": 28093765, "end": 28093940}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_a_fbr.png", "start": 28093940, "end": 28094095}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_b_fbr.png", "start": 28094095, "end": 28094254}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_c_fbr.png", "start": 28094254, "end": 28094408}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_d_fbr.png", "start": 28094408, "end": 28094569}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_e_fbr.png", "start": 28094569, "end": 28094723}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_f_fbr.png", "start": 28094723, "end": 28094883}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_g_fbr.png", "start": 28094883, "end": 28095041}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_h_fbr.png", "start": 28095041, "end": 28095195}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_i_fbr.png", "start": 28095195, "end": 28095341}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_j_fbr.png", "start": 28095341, "end": 28095501}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_k_fbr.png", "start": 28095501, "end": 28095680}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_l_fbr.png", "start": 28095680, "end": 28095818}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_m_fbr.png", "start": 28095818, "end": 28095971}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_n_fbr.png", "start": 28095971, "end": 28096121}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_o_fbr.png", "start": 28096121, "end": 28096275}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_p_fbr.png", "start": 28096275, "end": 28096430}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_q_fbr.png", "start": 28096430, "end": 28096580}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_r_fbr.png", "start": 28096580, "end": 28096733}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_s_fbr.png", "start": 28096733, "end": 28096885}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_t_fbr.png", "start": 28096885, "end": 28097051}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_u_fbr.png", "start": 28097051, "end": 28097202}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_v_fbr.png", "start": 28097202, "end": 28097376}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_w_fbr.png", "start": 28097376, "end": 28097530}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_x_fbr.png", "start": 28097530, "end": 28097717}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_y_fbr.png", "start": 28097717, "end": 28097891}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{charlow_z_fbr.png", "start": 28097891, "end": 28098062}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_add_fbr.png", "start": 28098062, "end": 28098225}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_and_fbr.png", "start": 28098225, "end": 28098417}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_ardown_fbr.png", "start": 28098417, "end": 28098602}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_arleft_fbr.png", "start": 28098602, "end": 28098786}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_arright_fbr.png", "start": 28098786, "end": 28098971}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_arup_fbr.png", "start": 28098971, "end": 28099149}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_at_fbr.png", "start": 28099149, "end": 28099306}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_brackc1_fbr.png", "start": 28099306, "end": 28099485}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_brackc2_fbr.png", "start": 28099485, "end": 28099669}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_brackr1_fbr.png", "start": 28099669, "end": 28099836}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_brackr2_fbr.png", "start": 28099836, "end": 28100006}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_bracks1_fbr.png", "start": 28100006, "end": 28100160}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_bracks2_fbr.png", "start": 28100160, "end": 28100313}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_caret_fbr.png", "start": 28100313, "end": 28100493}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_colon_fbr.png", "start": 28100493, "end": 28100645}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_colonsemi_fbr.png", "start": 28100645, "end": 28100812}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_comma_fbr.png", "start": 28100812, "end": 28100968}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_div_fbr.png", "start": 28100968, "end": 28101137}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_dollar_fbr.png", "start": 28101137, "end": 28101313}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_equ_fbr.png", "start": 28101313, "end": 28101472}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_exclam_fbr.png", "start": 28101472, "end": 28101616}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_grave_fbr.png", "start": 28101616, "end": 28101766}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_hash_fbr.png", "start": 28101766, "end": 28101951}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_heart_fbr.png", "start": 28101951, "end": 28102136}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_multi_fbr.png", "start": 28102136, "end": 28102305}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_percent_fbr.png", "start": 28102305, "end": 28102511}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_perio_fbr.png", "start": 28102511, "end": 28102648}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_pipe_fbr.png", "start": 28102648, "end": 28102795}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_quest_fbr.png", "start": 28102795, "end": 28102968}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_slaback_fbr.png", "start": 28102968, "end": 28103155}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_slafoward_fbr.png", "start": 28103155, "end": 28103334}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_smile_fbr.png", "start": 28103334, "end": 28103494}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_sub_fbr.png", "start": 28103494, "end": 28103641}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_sun_fbr.png", "start": 28103641, "end": 28103840}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_thngreater_fbr.png", "start": 28103840, "end": 28104033}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_thnless_fbr.png", "start": 28104033, "end": 28104220}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_tilde_fbr.png", "start": 28104220, "end": 28104389}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_dev/{chars_unders_fbr.png", "start": 28104389, "end": 28104526}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/bone1_1.png", "start": 28104526, "end": 28111933}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/bone1_2.png", "start": 28111933, "end": 28121125}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/bone2_1.png", "start": 28121125, "end": 28131557}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/dopefish_fbr.png", "start": 28131557, "end": 28139787}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_gut1.png", "start": 28139787, "end": 28150102}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_gut2.png", "start": 28150102, "end": 28161902}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_1.png", "start": 28161902, "end": 28175652}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_2.png", "start": 28175652, "end": 28188632}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_3.png", "start": 28188632, "end": 28200665}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_4a.png", "start": 28200665, "end": 28213878}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_4b.png", "start": 28213878, "end": 28227018}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_5a.png", "start": 28227018, "end": 28240662}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot1_5b.png", "start": 28240662, "end": 28254235}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot2_1.png", "start": 28254235, "end": 28267461}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot3_1.png", "start": 28267461, "end": 28282407}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot3_2.png", "start": 28282407, "end": 28297446}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot4_1.png", "start": 28297446, "end": 28312083}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot5_1.png", "start": 28312083, "end": 28324351}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_rot6_1.png", "start": 28324351, "end": 28340758}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod1_1.png", "start": 28340758, "end": 28352276}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod1_2.png", "start": 28352276, "end": 28365338}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod1_3.png", "start": 28365338, "end": 28376341}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod1_4.png", "start": 28376341, "end": 28387753}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod1_5.png", "start": 28387753, "end": 28398352}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod2_1.png", "start": 28398352, "end": 28402008}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod2_2.png", "start": 28402008, "end": 28406105}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod2_3.png", "start": 28406105, "end": 28409481}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod2_4.png", "start": 28409481, "end": 28413079}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/flesh_wod2_5.png", "start": 28413079, "end": 28416469}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/fleshtile.png", "start": 28416469, "end": 28427438}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/marbred128.png", "start": 28427438, "end": 28439667}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_eat_eye2_fbr.png", "start": 28439667, "end": 28443540}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_eat_eye3_fbr.png", "start": 28443540, "end": 28447417}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_eat_eye_fbr.png", "start": 28447417, "end": 28451292}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_b.png", "start": 28451292, "end": 28463955}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_c.png", "start": 28463955, "end": 28476679}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_hol1.png", "start": 28476679, "end": 28481351}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_lit1_fbr.png", "start": 28481351, "end": 28485421}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_shut1.png", "start": 28485421, "end": 28500387}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_sp.png", "start": 28500387, "end": 28520529}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_sp2.png", "start": 28520529, "end": 28546152}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_te.png", "start": 28546152, "end": 28561258}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh1_tet.png", "start": 28561258, "end": 28574478}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh2_b.png", "start": 28574478, "end": 28588001}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh3_1a.png", "start": 28588001, "end": 28601727}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh3_1b.png", "start": 28601727, "end": 28638289}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh3_b.png", "start": 28638289, "end": 28652175}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh4_det.png", "start": 28652175, "end": 28662486}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh4a_det.png", "start": 28662486, "end": 28677347}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh5.png", "start": 28677347, "end": 28687683}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh5_1a.png", "start": 28687683, "end": 28742418}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh5_1b.png", "start": 28742418, "end": 28795136}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh5_1c.png", "start": 28795136, "end": 28838778}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh5_1lit_fbr.png", "start": 28838778, "end": 28855712}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_flesh_dr1a.png", "start": 28855712, "end": 28912605}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/may_skin_eye.png", "start": 28912605, "end": 28923822}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/meat-teeth0.png", "start": 28923822, "end": 28967193}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/meat-teeth1.png", "start": 28967193, "end": 29012385}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/meat_det1.png", "start": 29012385, "end": 29031116}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/meat_det2.png", "start": 29031116, "end": 29053687}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/meat_pipe1.png", "start": 29053687, "end": 29078111}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_0eye.png", "start": 29078111, "end": 29082256}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_0flesh2_gl.png", "start": 29082256, "end": 29097436}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_0flesh_but1_fbr.png", "start": 29097436, "end": 29101731}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_0flesh_but2_fbr.png", "start": 29101731, "end": 29105920}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_0flsh_vent.png", "start": 29105920, "end": 29120171}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_1eye.png", "start": 29120171, "end": 29124315}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_1flesh2_gl.png", "start": 29124315, "end": 29139474}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_1flesh_but2_fbr.png", "start": 29139474, "end": 29143669}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_2eye.png", "start": 29143669, "end": 29147709}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_2flesh2_gl.png", "start": 29147709, "end": 29162885}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_2flesh_but2_fbr.png", "start": 29162885, "end": 29167068}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_3eye.png", "start": 29167068, "end": 29171116}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_3flesh2_gl.png", "start": 29171116, "end": 29186290}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_3flesh_but2_fbr.png", "start": 29186290, "end": 29190461}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_4eye.png", "start": 29190461, "end": 29194580}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_4flesh2_gl.png", "start": 29194580, "end": 29209760}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_5eye.png", "start": 29209760, "end": 29213813}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_6eye.png", "start": 29213813, "end": 29217861}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_7eye.png", "start": 29217861, "end": 29221932}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_8eye.png", "start": 29221932, "end": 29226040}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_9eye.png", "start": 29226040, "end": 29230188}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_aeye.png", "start": 29230188, "end": 29234180}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_aflesh_but1.png", "start": 29234180, "end": 29238746}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_aflesh_but2.png", "start": 29238746, "end": 29243275}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_flesh/plus_aflsh_vent.png", "start": 29243275, "end": 29257675}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/black.png", "start": 29257675, "end": 29257820}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch1.png", "start": 29257820, "end": 29262837}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch1_2.png", "start": 29262837, "end": 29268463}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch1_a.png", "start": 29268463, "end": 29281238}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch1_b.png", "start": 29281238, "end": 29294698}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch1_c.png", "start": 29294698, "end": 29307871}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch1_d.png", "start": 29307871, "end": 29321388}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch2.png", "start": 29321388, "end": 29324670}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch2_2.png", "start": 29324670, "end": 29327992}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch2_a.png", "start": 29327992, "end": 29338395}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch2_b.png", "start": 29338395, "end": 29348912}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch2_c.png", "start": 29348912, "end": 29359041}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch_end.png", "start": 29359041, "end": 29359797}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch_tcap.png", "start": 29359797, "end": 29360511}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_arch_trim.png", "start": 29360511, "end": 29361983}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_arch1a.png", "start": 29361983, "end": 29369618}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_arch1b.png", "start": 29369618, "end": 29378153}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_brk1a.png", "start": 29378153, "end": 29380625}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_brk1b.png", "start": 29380625, "end": 29382763}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_brk2a.png", "start": 29382763, "end": 29385184}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_brk2b.png", "start": 29385184, "end": 29387644}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_flt1.png", "start": 29387644, "end": 29389868}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_fsh1.png", "start": 29389868, "end": 29392794}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_fsh2.png", "start": 29392794, "end": 29400886}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_pil1.png", "start": 29400886, "end": 29403673}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_trim1.png", "start": 29403673, "end": 29406085}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll1.png", "start": 29406085, "end": 29408805}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll2.png", "start": 29408805, "end": 29411531}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll3a.png", "start": 29411531, "end": 29413889}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll3b.png", "start": 29413889, "end": 29416357}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll4b.png", "start": 29416357, "end": 29418923}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll5a.png", "start": 29418923, "end": 29421140}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll5b.png", "start": 29421140, "end": 29423439}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll5c.png", "start": 29423439, "end": 29425762}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_bl_wll5d.png", "start": 29425762, "end": 29428169}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk15.png", "start": 29428169, "end": 29437675}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk15_b.png", "start": 29437675, "end": 29446923}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk15_c.png", "start": 29446923, "end": 29463791}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk15_f.png", "start": 29463791, "end": 29472084}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk15_g.png", "start": 29472084, "end": 29479733}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk16.png", "start": 29479733, "end": 29489009}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk16_a.png", "start": 29489009, "end": 29498288}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk16_f.png", "start": 29498288, "end": 29507993}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk17.png", "start": 29507993, "end": 29543137}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_brk17_f.png", "start": 29543137, "end": 29555739}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_but1.png", "start": 29555739, "end": 29556688}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_det1.png", "start": 29556688, "end": 29559761}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_door1.png", "start": 29559761, "end": 29574868}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_door1_f.png", "start": 29574868, "end": 29584903}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_door2.png", "start": 29584903, "end": 29595763}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_door3.png", "start": 29595763, "end": 29608420}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_ebrick10.png", "start": 29608420, "end": 29619047}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_ebrick10_bl.png", "start": 29619047, "end": 29633581}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_ebrick22.png", "start": 29633581, "end": 29647447}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_ebrick23.png", "start": 29647447, "end": 29656813}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_ebrick24.png", "start": 29656813, "end": 29665006}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr1.png", "start": 29665006, "end": 29665337}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr2.png", "start": 29665337, "end": 29666191}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr3.png", "start": 29666191, "end": 29668787}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_1.png", "start": 29668787, "end": 29670917}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_2.png", "start": 29670917, "end": 29673179}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_3.png", "start": 29673179, "end": 29675542}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_4.png", "start": 29675542, "end": 29677837}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_5.png", "start": 29677837, "end": 29681469}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_6.png", "start": 29681469, "end": 29684475}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr4_8.png", "start": 29684475, "end": 29687810}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr5_1.png", "start": 29687810, "end": 29690397}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr5_2.png", "start": 29690397, "end": 29692891}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr5_3.png", "start": 29692891, "end": 29695633}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_flr5_4.png", "start": 29695633, "end": 29697936}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_key_gl2.png", "start": 29697936, "end": 29698960}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_key_sl2.png", "start": 29698960, "end": 29699961}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_lion1.png", "start": 29699961, "end": 29710435}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_lion2.png", "start": 29710435, "end": 29720102}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_lion3.png", "start": 29720102, "end": 29730645}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_lion4.png", "start": 29730645, "end": 29741797}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_met1.png", "start": 29741797, "end": 29752178}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_met1_trim.png", "start": 29752178, "end": 29763453}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_met1a_trim.png", "start": 29763453, "end": 29774421}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_met1b_trim.png", "start": 29774421, "end": 29788534}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_met2_trim.png", "start": 29788534, "end": 29801803}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_met_plt.png", "start": 29801803, "end": 29817194}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_mural1.png", "start": 29817194, "end": 29837052}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_mural2.png", "start": 29837052, "end": 29859769}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_mural3.png", "start": 29859769, "end": 29949049}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_pl1_a.png", "start": 29949049, "end": 29950865}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_pl1_b.png", "start": 29950865, "end": 29952948}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_pl2_a.png", "start": 29952948, "end": 29955180}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_pl2_b.png", "start": 29955180, "end": 29957392}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_plat1_side.png", "start": 29957392, "end": 29958609}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_plat1_top.png", "start": 29958609, "end": 29962506}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_tile2_1.png", "start": 29962506, "end": 29964940}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_tile2_2.png", "start": 29964940, "end": 29967293}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1.png", "start": 29967293, "end": 29974050}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_3.png", "start": 29974050, "end": 29980664}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_3_s.png", "start": 29980664, "end": 29982704}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_4_s.png", "start": 29982704, "end": 29984780}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_5.png", "start": 29984780, "end": 29991716}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_5_s.png", "start": 29991716, "end": 29993936}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_6_s.png", "start": 29993936, "end": 29996213}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim1_7_s.png", "start": 29996213, "end": 29999555}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_trim2.png", "start": 29999555, "end": 30001916}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_wall1.png", "start": 30001916, "end": 30004948}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_wall2.png", "start": 30004948, "end": 30007619}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_wall3.png", "start": 30007619, "end": 30013405}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_wall3b.png", "start": 30013405, "end": 30040439}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_win1_a.png", "start": 30040439, "end": 30049578}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/grk_win1_b.png", "start": 30049578, "end": 30058921}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_0grk_but1_fbr.png", "start": 30058921, "end": 30060112}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_0grk_hbut_fbr.png", "start": 30060112, "end": 30061135}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_1grk_but1_fbr.png", "start": 30061135, "end": 30062310}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_1grk_hbut_fbr.png", "start": 30062310, "end": 30063391}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_2grk_but1_fbr.png", "start": 30063391, "end": 30064542}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_2grk_hbut_fbr.png", "start": 30064542, "end": 30065582}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_3grk_but1_fbr.png", "start": 30065582, "end": 30066735}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_3grk_hbut_fbr.png", "start": 30066735, "end": 30067777}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_agrk_but1.png", "start": 30067777, "end": 30068768}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_greek/plus_agrk_hbut.png", "start": 30068768, "end": 30069746}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_bottom.png", "start": 30069746, "end": 30070676}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_c_b1_fbr.png", "start": 30070676, "end": 30071284}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_c_b2_fbr.png", "start": 30071284, "end": 30071896}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_c_b3_fbr.png", "start": 30071896, "end": 30072703}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_c_s1_fbr.png", "start": 30072703, "end": 30073296}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_c_s2_fbr.png", "start": 30073296, "end": 30074286}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_c_s3_fbr.png", "start": 30074286, "end": 30074919}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_fl.png", "start": 30074919, "end": 30075478}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_fl2.png", "start": 30075478, "end": 30076033}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_n_b1_fbr.png", "start": 30076033, "end": 30076787}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_n_b2_fbr.png", "start": 30076787, "end": 30077472}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_n_b3.png", "start": 30077472, "end": 30078043}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_n_s1.png", "start": 30078043, "end": 30078614}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_n_s2_fbr.png", "start": 30078614, "end": 30079269}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_n_s3_fbr.png", "start": 30079269, "end": 30079806}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_r_b1_fbr.png", "start": 30079806, "end": 30080521}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_r_b2_fbr.png", "start": 30080521, "end": 30081147}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_r_b3_fbr.png", "start": 30081147, "end": 30081605}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_r_s1_fbr.png", "start": 30081605, "end": 30082219}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_r_s2_fbr.png", "start": 30082219, "end": 30082568}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_s_b1_fbr.png", "start": 30082568, "end": 30083227}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_s_b2_fbr.png", "start": 30083227, "end": 30083855}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_s_b3.png", "start": 30083855, "end": 30084632}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_s_s1_fbr.png", "start": 30084632, "end": 30085138}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_s_s2_fbr.png", "start": 30085138, "end": 30085716}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammo_s_s3_fbr.png", "start": 30085716, "end": 30086267}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammobotsmall.png", "start": 30086267, "end": 30086935}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammotop.png", "start": 30086935, "end": 30087680}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/ammotopsmall.png", "start": 30087680, "end": 30088231}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/boom.png", "start": 30088231, "end": 30089249}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/boomammo_bottom.png", "start": 30089249, "end": 30089615}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/boomammotop.png", "start": 30089615, "end": 30089957}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/boomsmall.png", "start": 30089957, "end": 30090632}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/epboxlarge_fbr.png", "start": 30090632, "end": 30091816}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/epboxsmall_fbr.png", "start": 30091816, "end": 30092713}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/explob_s2.png", "start": 30092713, "end": 30093209}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/hp15_side.png", "start": 30093209, "end": 30093794}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/hp25_top2.png", "start": 30093794, "end": 30094223}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/hp_bottom.png", "start": 30094223, "end": 30095130}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/hp_details.png", "start": 30095130, "end": 30095634}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/nails.png", "start": 30095634, "end": 30096681}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/nailssmall.png", "start": 30096681, "end": 30097422}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp100-winq_fbr.png", "start": 30097422, "end": 30100022}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp100_side_fbr.png", "start": 30100022, "end": 30100914}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp100_top_fbr.png", "start": 30100914, "end": 30101645}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp15_top_fbr.png", "start": 30101645, "end": 30102375}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp15winq_fbr.png", "start": 30102375, "end": 30105162}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp25-winq_fbr.png", "start": 30105162, "end": 30107716}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp25_side_fbr.png", "start": 30107716, "end": 30108538}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0_hp25_top_fbr.png", "start": 30108538, "end": 30109272}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0explob2_s1_fbr.png", "start": 30109272, "end": 30110375}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_0explob_s1_fbr.png", "start": 30110375, "end": 30111008}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp100-winq_fbr.png", "start": 30111008, "end": 30113613}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp100_side_fbr.png", "start": 30113613, "end": 30114507}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp100_top_fbr.png", "start": 30114507, "end": 30115241}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp15_top_fbr.png", "start": 30115241, "end": 30115975}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp15winq_fbr.png", "start": 30115975, "end": 30118761}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp25-winq_fbr.png", "start": 30118761, "end": 30121325}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp25_side_fbr.png", "start": 30121325, "end": 30122151}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1_hp25_top_fbr.png", "start": 30122151, "end": 30122890}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1explob2_s1_fbr.png", "start": 30122890, "end": 30123997}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_1explob_s1_fbr.png", "start": 30123997, "end": 30124630}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2_hp100-winq_fbr.png", "start": 30124630, "end": 30127234}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2_hp100_side_fbr.png", "start": 30127234, "end": 30128128}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2_hp25-winq_fbr.png", "start": 30128128, "end": 30130686}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2_hp25_side_fbr.png", "start": 30130686, "end": 30131511}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2_hp25_top_fbr.png", "start": 30131511, "end": 30132249}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2explob2_s1_fbr.png", "start": 30132249, "end": 30133345}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_2explob_s1_fbr.png", "start": 30133345, "end": 30133987}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3_hp100-winq_fbr.png", "start": 30133987, "end": 30136597}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3_hp100_side_fbr.png", "start": 30136597, "end": 30137494}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3_hp25-winq_fbr.png", "start": 30137494, "end": 30140063}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3_hp25_side_fbr.png", "start": 30140063, "end": 30140894}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3_hp25_top_fbr.png", "start": 30140894, "end": 30141636}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3explob2_s1_fbr.png", "start": 30141636, "end": 30142732}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/plus_3explob_s1_fbr.png", "start": 30142732, "end": 30143374}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/shells.png", "start": 30143374, "end": 30144320}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/shellssmall.png", "start": 30144320, "end": 30145046}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/zap.png", "start": 30145046, "end": 30146039}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_health_ammo/zapsmall.png", "start": 30146039, "end": 30146803}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/brick7.png", "start": 30146803, "end": 30149736}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/brick8.png", "start": 30149736, "end": 30152823}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/button_0.png", "start": 30152823, "end": 30156295}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/button_0_grey.png", "start": 30156295, "end": 30159505}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/button_0_grn.png", "start": 30159505, "end": 30162840}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/button_1.png", "start": 30162840, "end": 30166295}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/button_1_grey.png", "start": 30166295, "end": 30169490}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/button_1_grn.png", "start": 30169490, "end": 30172791}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/floor_temp.png", "start": 30172791, "end": 30174849}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/flr.png", "start": 30174849, "end": 30177606}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/gardgrass_1.png", "start": 30177606, "end": 30197776}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/go-savgx.png", "start": 30197776, "end": 30199525}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grass.png", "start": 30199525, "end": 30202926}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_brk15_c_old.png", "start": 30202926, "end": 30213453}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_brk17_f_old.png", "start": 30213453, "end": 30222162}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_door1_old.png", "start": 30222162, "end": 30233179}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_door2_old.png", "start": 30233179, "end": 30242352}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_door3_old.png", "start": 30242352, "end": 30251158}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_ebrick22_old.png", "start": 30251158, "end": 30260475}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/grk_trim1_7_s_old.png", "start": 30260475, "end": 30262680}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/ground_1.png", "start": 30262680, "end": 30264861}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/leaves.png", "start": 30264861, "end": 30267923}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/marble1_4.png", "start": 30267923, "end": 30270826}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/marble1_5.png", "start": 30270826, "end": 30273916}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_cflat1_3.png", "start": 30273916, "end": 30276397}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_csl_trm3.png", "start": 30276397, "end": 30277722}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_csl_trm3b.png", "start": 30277722, "end": 30278712}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_csl_trm3c.png", "start": 30278712, "end": 30280240}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_flat10.png", "start": 30280240, "end": 30284133}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_flat11.png", "start": 30284133, "end": 30287338}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_flat13.png", "start": 30287338, "end": 30298115}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_flat14.png", "start": 30298115, "end": 30308516}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_flat9a.png", "start": 30308516, "end": 30352096}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_flat9b.png", "start": 30352096, "end": 30393995}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_plaster1.png", "start": 30393995, "end": 30402037}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_ret_rock1.png", "start": 30402037, "end": 30405856}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_ret_wood1_old.png", "start": 30405856, "end": 30412937}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_rmet_key.png", "start": 30412937, "end": 30416898}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_rock6.png", "start": 30416898, "end": 30462501}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_rock7.png", "start": 30462501, "end": 30465385}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_rock8.png", "start": 30465385, "end": 30467270}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_rough_block.png", "start": 30467270, "end": 30516113}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_rough_block_f.png", "start": 30516113, "end": 30570658}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_tile.png", "start": 30570658, "end": 30572794}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/med_wall1.png", "start": 30572794, "end": 30582659}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/metground_1.png", "start": 30582659, "end": 30585590}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/note-savgx.png", "start": 30585590, "end": 30604610}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/plus_0button1.png", "start": 30604610, "end": 30608055}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/plus_0button2_fbr.png", "start": 30608055, "end": 30609542}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/plus_1button2_fbr.png", "start": 30609542, "end": 30611030}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/plus_1button3.png", "start": 30611030, "end": 30612910}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/plus_abutton1_fbr.png", "start": 30612910, "end": 30616512}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/plus_abutton2_fbr.png", "start": 30616512, "end": 30618010}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/readme.txt", "start": 30618010, "end": 30618086}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiflat.png", "start": 30618086, "end": 30632205}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiflat_blu.png", "start": 30632205, "end": 30645140}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiflat_grn.png", "start": 30645140, "end": 30656794}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoilava.png", "start": 30656794, "end": 30661298}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoislime.png", "start": 30661298, "end": 30665003}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoitrim.png", "start": 30665003, "end": 30666490}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoitrim__purp.png", "start": 30666490, "end": 30668206}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoitrim_blu.png", "start": 30668206, "end": 30669936}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiwall.png", "start": 30669936, "end": 30709733}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiwall__purp.png", "start": 30709733, "end": 30745806}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiwall_blu.png", "start": 30745806, "end": 30781867}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/riktoiwater.png", "start": 30781867, "end": 30784758}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/rune1_fbr.png", "start": 30784758, "end": 30787771}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/rune2_fbr.png", "start": 30787771, "end": 30790837}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/rune3_fbr.png", "start": 30790837, "end": 30794233}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/rune4_fbr.png", "start": 30794233, "end": 30796785}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_easy.png", "start": 30796785, "end": 30801737}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_empty.png", "start": 30801737, "end": 30806494}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_hard.png", "start": 30806494, "end": 30811367}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_medium.png", "start": 30811367, "end": 30816377}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_metal_1.png", "start": 30816377, "end": 30817589}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_metal_2.png", "start": 30817589, "end": 30819184}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sign_nmare.png", "start": 30819184, "end": 30824205}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky2.png", "start": 30824205, "end": 30839695}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky4.png", "start": 30839695, "end": 30849207}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky5_fbr.png", "start": 30849207, "end": 30866360}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky5a.png", "start": 30866360, "end": 30875371}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky7.png", "start": 30875371, "end": 30889286}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky8.png", "start": 30889286, "end": 30905959}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/sky8a_fbr.png", "start": 30905959, "end": 30914755}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/tile.png", "start": 30914755, "end": 30923571}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/tile_blu.png", "start": 30923571, "end": 30932577}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/tile_grn.png", "start": 30932577, "end": 30940840}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/trim2_blu.png", "start": 30940840, "end": 30941953}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/trim2_grn.png", "start": 30941953, "end": 30943017}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/ultrasteel1.png", "start": 30943017, "end": 30955907}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/ultrasteel2.png", "start": 30955907, "end": 30967701}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/vines1_old.png", "start": 30967701, "end": 30972555}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_legacy/wiz1_4.png", "start": 30972555, "end": 30976376}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/+0water_f3.png", "start": 30976376, "end": 30979317}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/+1water_f3.png", "start": 30979317, "end": 30982241}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/+2water_f3.png", "start": 30982241, "end": 30985159}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/+3water_f3.png", "start": 30985159, "end": 30988011}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_0blood_f1.png", "start": 30988011, "end": 30989800}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_0fslime.png", "start": 30989800, "end": 31003756}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_0lava_fall3_fbr.png", "start": 31003756, "end": 31012749}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_0water_f1.png", "start": 31012749, "end": 31014598}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_0water_f2.png", "start": 31014598, "end": 31016416}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_0wfall0.png", "start": 31016416, "end": 31026025}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_1blood_f1.png", "start": 31026025, "end": 31027830}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_1fslime.png", "start": 31027830, "end": 31041573}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_1lava_fall3_fbr.png", "start": 31041573, "end": 31051140}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_1water_f1.png", "start": 31051140, "end": 31052788}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_1water_f2.png", "start": 31052788, "end": 31054617}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_1wfall0.png", "start": 31054617, "end": 31064268}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_2blood_f1.png", "start": 31064268, "end": 31066126}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_2fslime.png", "start": 31066126, "end": 31080020}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_2lava_fall3_fbr.png", "start": 31080020, "end": 31089569}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_2water_f1.png", "start": 31089569, "end": 31091340}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_2water_f2.png", "start": 31091340, "end": 31093226}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_2wfall0.png", "start": 31093226, "end": 31102888}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_3blood_f1.png", "start": 31102888, "end": 31104688}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_3fslime.png", "start": 31104688, "end": 31118371}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_3lava_fall3_fbr.png", "start": 31118371, "end": 31127708}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_3water_f1.png", "start": 31127708, "end": 31129359}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_3water_f2.png", "start": 31129359, "end": 31131186}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_3wfall0.png", "start": 31131186, "end": 31140889}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_4fslime.png", "start": 31140889, "end": 31154553}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_4lava_fall3_fbr.png", "start": 31154553, "end": 31163926}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_4wfall0.png", "start": 31163926, "end": 31173505}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_5fslime.png", "start": 31173505, "end": 31187273}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_5lava_fall3_fbr.png", "start": 31187273, "end": 31196636}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_5wfall0.png", "start": 31196636, "end": 31206235}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_6fslime.png", "start": 31206235, "end": 31220087}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_6lava_fall3_fbr.png", "start": 31220087, "end": 31229472}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_6wfall0.png", "start": 31229472, "end": 31239057}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_7fslime.png", "start": 31239057, "end": 31252908}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_7lava_fall3_fbr.png", "start": 31252908, "end": 31262423}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_7wfall0.png", "start": 31262423, "end": 31272092}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_8wfall0.png", "start": 31272092, "end": 31281754}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/plus_9wfall0.png", "start": 31281754, "end": 31291376}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky-test.png", "start": 31291376, "end": 31308972}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky-test.xcf", "start": 31308972, "end": 31442237}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky5_blu.png", "start": 31442237, "end": 31455550}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky5_dismal.png", "start": 31455550, "end": 31468733}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_galx_fbr.png", "start": 31468733, "end": 31491488}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_galx_spark_fbr.png", "start": 31491488, "end": 31511046}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_orng.png", "start": 31511046, "end": 31525685}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_pando.png", "start": 31525685, "end": 31541793}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_pando2.png", "start": 31541793, "end": 31558328}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_star.png", "start": 31558328, "end": 31559988}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_void.png", "start": 31559988, "end": 31560683}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/sky_wfog_fbr.png", "start": 31560683, "end": 31561589}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_acid.png", "start": 31561589, "end": 31563768}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_blood1.png", "start": 31563768, "end": 31565841}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_lava1_fbr.png", "start": 31565841, "end": 31569046}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_lava2_fbr.png", "start": 31569046, "end": 31573128}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_lava3_fbr.png", "start": 31573128, "end": 31577097}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_lava_void_fbr.png", "start": 31577097, "end": 31580787}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_lavaskip.png", "start": 31580787, "end": 31581900}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_meatgoo2_fbr.png", "start": 31581900, "end": 31585088}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_meatgoo_fbr.png", "start": 31585088, "end": 31587885}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_slime1.png", "start": 31587885, "end": 31594176}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_slime2.png", "start": 31594176, "end": 31597365}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_slime3.png", "start": 31597365, "end": 31599397}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_slime_soul.png", "start": 31599397, "end": 31602187}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_slimeskip.png", "start": 31602187, "end": 31603260}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_soul_drain.png", "start": 31603260, "end": 31606228}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_tele1_fbr.png", "start": 31606228, "end": 31608108}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_tele2_fbr.png", "start": 31608108, "end": 31610909}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_tele3_fbr.png", "start": 31610909, "end": 31613945}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_tele4_fbr.png", "start": 31613945, "end": 31616637}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_water0.png", "start": 31616637, "end": 31619485}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_water1.png", "start": 31619485, "end": 31622594}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_water2.png", "start": 31622594, "end": 31624854}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_water3.png", "start": 31624854, "end": 31627235}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_water4.png", "start": 31627235, "end": 31632022}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_waterskip.png", "start": 31632022, "end": 31633762}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_liquidsky/star_wstill0.png", "start": 31633762, "end": 31636601}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/btn1.png", "start": 31636601, "end": 31639519}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_arrow.png", "start": 31639519, "end": 31640400}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok1_1.png", "start": 31640400, "end": 31642921}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok1_2.png", "start": 31642921, "end": 31645112}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok1_2_m.png", "start": 31645112, "end": 31648208}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok1_m.png", "start": 31648208, "end": 31657482}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok2_1.png", "start": 31657482, "end": 31660046}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok2_1_m.png", "start": 31660046, "end": 31663267}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok2_2.png", "start": 31663267, "end": 31673530}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blok2_2_m.png", "start": 31673530, "end": 31685958}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blud1_1.png", "start": 31685958, "end": 31688550}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_blud1_1m.png", "start": 31688550, "end": 31691770}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_1.png", "start": 31691770, "end": 31694205}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_1m.png", "start": 31694205, "end": 31697275}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_2.png", "start": 31697275, "end": 31699702}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_2m.png", "start": 31699702, "end": 31702730}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_3.png", "start": 31702730, "end": 31704989}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_3m.png", "start": 31704989, "end": 31708398}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_5.png", "start": 31708398, "end": 31711251}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd1_5m.png", "start": 31711251, "end": 31714900}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_bnd_skull.png", "start": 31714900, "end": 31717532}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_0.png", "start": 31717532, "end": 31720636}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_0m.png", "start": 31720636, "end": 31723693}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_1.png", "start": 31723693, "end": 31726375}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_1m.png", "start": 31726375, "end": 31729574}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_2.png", "start": 31729574, "end": 31732525}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_2m.png", "start": 31732525, "end": 31735930}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_3.png", "start": 31735930, "end": 31738867}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk1_3m.png", "start": 31738867, "end": 31742043}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk2_0.png", "start": 31742043, "end": 31751898}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk2_0_m.png", "start": 31751898, "end": 31764231}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk_old.png", "start": 31764231, "end": 31766830}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_brk_oldm.png", "start": 31766830, "end": 31770169}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_deco1_1.png", "start": 31770169, "end": 31781398}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_deco1_1m.png", "start": 31781398, "end": 31795433}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_deco1_2.png", "start": 31795433, "end": 31797953}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_deco1_2my.png", "start": 31797953, "end": 31801341}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_deco1_3.png", "start": 31801341, "end": 31819914}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_deco1_3m.png", "start": 31819914, "end": 31840265}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_door1_1.png", "start": 31840265, "end": 31849732}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_door1_1m.png", "start": 31849732, "end": 31860823}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_door2_1.png", "start": 31860823, "end": 31872791}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_door2_2.png", "start": 31872791, "end": 31884569}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_drt2_1.png", "start": 31884569, "end": 31887406}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_flr1_1.png", "start": 31887406, "end": 31890311}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_flr1_2.png", "start": 31890311, "end": 31893534}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_flt1_1.png", "start": 31893534, "end": 31895823}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_flt1_1m.png", "start": 31895823, "end": 31899271}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_key1_1.png", "start": 31899271, "end": 31900858}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_key1_2.png", "start": 31900858, "end": 31902855}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_lite1_1_fbr.png", "start": 31902855, "end": 31903785}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_lite1_2.png", "start": 31903785, "end": 31904148}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_lite2_1.png", "start": 31904148, "end": 31907085}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_lite2_2.png", "start": 31907085, "end": 31907571}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_lite3_1_fbr.png", "start": 31907571, "end": 31908528}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_lite_f1.png", "start": 31908528, "end": 31909014}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_oldmtomb1_1_fbr.png", "start": 31909014, "end": 31922866}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_oldtomb1_2_fbr.png", "start": 31922866, "end": 31936758}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_plat_stem_m.png", "start": 31936758, "end": 31937624}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_plats.png", "start": 31937624, "end": 31940286}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_platst.png", "start": 31940286, "end": 31942713}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_platt.png", "start": 31942713, "end": 31945336}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_plr1_1.png", "start": 31945336, "end": 31948530}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_tomb1_1_fbr.png", "start": 31948530, "end": 31962382}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_tomb1_2_fbr.png", "start": 31962382, "end": 31976274}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_trm1_1.png", "start": 31976274, "end": 31985202}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_trm1_2.png", "start": 31985202, "end": 31987907}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_trm1_a.png", "start": 31987907, "end": 31990249}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_tskull.png", "start": 31990249, "end": 32001255}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_wall1_1.png", "start": 32001255, "end": 32004237}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_wall1_2.png", "start": 32004237, "end": 32007203}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_wall1_3.png", "start": 32007203, "end": 32010397}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_wall1_3a.png", "start": 32010397, "end": 32012978}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_wall1_4.png", "start": 32012978, "end": 32015966}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/may_wall1_4a.png", "start": 32015966, "end": 32018365}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/maya_end_dr1.png", "start": 32018365, "end": 32022751}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/maya_end_dr2.png", "start": 32022751, "end": 32027136}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/maya_end_trim1.png", "start": 32027136, "end": 32031526}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_0_may_btn1.png", "start": 32031526, "end": 32034443}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_0_may_mpiloilon_fbr.png", "start": 32034443, "end": 32036001}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_0_may_mpilon_fbr.png", "start": 32036001, "end": 32037576}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_0_may_mshoohoot_fbr.png", "start": 32037576, "end": 32038521}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_0_may_mshoot_fbr.png", "start": 32038521, "end": 32039464}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_1_may_btn1.png", "start": 32039464, "end": 32042372}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_1_may_mpiloilon_fbr.png", "start": 32042372, "end": 32043945}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_1_may_mpilon_fbr.png", "start": 32043945, "end": 32045538}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_1_may_mshoohoot_fbr.png", "start": 32045538, "end": 32046523}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_1_may_mshoot_fbr.png", "start": 32046523, "end": 32047507}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_2_may_btn1.png", "start": 32047507, "end": 32050398}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_2_may_mpilon.png", "start": 32050398, "end": 32051975}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_2_may_mshoohoot_fbr.png", "start": 32051975, "end": 32053000}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_2_may_mshoot_fbr.png", "start": 32053000, "end": 32054027}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_3_may_btn1.png", "start": 32054027, "end": 32057089}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_3_may_mpiloilon_fbr.png", "start": 32057089, "end": 32058662}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_3_may_mpilon_fbr.png", "start": 32058662, "end": 32060255}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_3_may_mshoohoot_fbr.png", "start": 32060255, "end": 32061240}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_3_may_mshoot_fbr.png", "start": 32061240, "end": 32062224}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_4_may_btn1.png", "start": 32062224, "end": 32065287}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_5_may_btn1.png", "start": 32065287, "end": 32068028}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_a_may_btn1.png", "start": 32068028, "end": 32070751}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_a_may_mpiloilon_fbr.png", "start": 32070751, "end": 32072328}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_a_may_mpilon_fbr.png", "start": 32072328, "end": 32073912}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_mayan/plus_a_may_mshoot.png", "start": 32073912, "end": 32074861}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+0med_but1.png", "start": 32074861, "end": 32079116}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+0med_but2.png", "start": 32079116, "end": 32080828}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+0med_but3.png", "start": 32080828, "end": 32082531}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+0med_but_s1.png", "start": 32082531, "end": 32086777}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+0med_sht_but1.png", "start": 32086777, "end": 32088070}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+1med_but3.png", "start": 32088070, "end": 32089772}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+1med_but_s1.png", "start": 32089772, "end": 32093991}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+1med_sht_but1.png", "start": 32093991, "end": 32095304}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+2med_but_s1.png", "start": 32095304, "end": 32099578}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+3med_but_s1.png", "start": 32099578, "end": 32103797}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+amed_but1.png", "start": 32103797, "end": 32107880}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+amed_but2.png", "start": 32107880, "end": 32109598}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+amed_but3.png", "start": 32109598, "end": 32111208}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+amed_but_s1.png", "start": 32111208, "end": 32115421}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/+amed_sht_but1.png", "start": 32115421, "end": 32116577}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/Art1.png", "start": 32116577, "end": 32249570}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/afloor1_4.png", "start": 32249570, "end": 32252571}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/afloor1_8.png", "start": 32252571, "end": 32255411}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/afloor3_1.png", "start": 32255411, "end": 32258221}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/altar1_1.png", "start": 32258221, "end": 32262109}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/altar1_3.png", "start": 32262109, "end": 32266590}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/altar1_4.png", "start": 32266590, "end": 32270643}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/brick0.png", "start": 32270643, "end": 32278865}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/brick1.png", "start": 32278865, "end": 32289619}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/brick4_s.png", "start": 32289619, "end": 32291552}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/brown1.png", "start": 32291552, "end": 32300728}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1a.png", "start": 32300728, "end": 32347083}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1b.png", "start": 32347083, "end": 32430447}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1c.png", "start": 32430447, "end": 32528618}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1d.png", "start": 32528618, "end": 32637287}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1e.png", "start": 32637287, "end": 32745780}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1f.png", "start": 32745780, "end": 32813639}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_1s.png", "start": 32813639, "end": 32931090}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2a.png", "start": 32931090, "end": 33008474}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2b.png", "start": 33008474, "end": 33085913}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2c.png", "start": 33085913, "end": 33174788}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2d.png", "start": 33174788, "end": 33272982}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2e.png", "start": 33272982, "end": 33371219}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2f.png", "start": 33371219, "end": 33435763}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_block_2s.png", "start": 33435763, "end": 33541515}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_ceil1a.png", "start": 33541515, "end": 33553323}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_ceil1b.png", "start": 33553323, "end": 33565858}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_wal2a.png", "start": 33565858, "end": 33578755}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_wal2b.png", "start": 33578755, "end": 33592011}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_wal2c.png", "start": 33592011, "end": 33604453}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_win1.png", "start": 33604453, "end": 33647451}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_brk9_win1b.png", "start": 33647451, "end": 33692777}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_but_side.png", "start": 33692777, "end": 33693775}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet1.png", "start": 33693775, "end": 33696899}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet2a.png", "start": 33696899, "end": 33700050}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet2b.png", "start": 33700050, "end": 33703112}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet2c.png", "start": 33703112, "end": 33706249}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet3a.png", "start": 33706249, "end": 33709521}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet3b.png", "start": 33709521, "end": 33712703}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet4.png", "start": 33712703, "end": 33715544}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet5a.png", "start": 33715544, "end": 33718682}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_cmet5c.png", "start": 33718682, "end": 33721825}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk10.png", "start": 33721825, "end": 33736551}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk10_f.png", "start": 33736551, "end": 33748839}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk10b.png", "start": 33748839, "end": 33763402}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk11.png", "start": 33763402, "end": 33774650}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk12.png", "start": 33774650, "end": 33791599}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk12_f.png", "start": 33791599, "end": 33808932}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk13.png", "start": 33808932, "end": 33865049}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk14.png", "start": 33865049, "end": 33880903}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk14_f.png", "start": 33880903, "end": 33893872}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk14b.png", "start": 33893872, "end": 33907362}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk15.png", "start": 33907362, "end": 33921188}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk15b.png", "start": 33921188, "end": 33935118}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk15f.png", "start": 33935118, "end": 33948316}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk16.png", "start": 33948316, "end": 33964537}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk16b.png", "start": 33964537, "end": 33982129}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk16f.png", "start": 33982129, "end": 33999400}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk17.png", "start": 33999400, "end": 34009507}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk17_f.png", "start": 34009507, "end": 34018532}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk17b.png", "start": 34018532, "end": 34028669}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18_f.png", "start": 34028669, "end": 34037302}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18_gb.png", "start": 34037302, "end": 34040050}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18_gt.png", "start": 34040050, "end": 34042787}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18_t.png", "start": 34042787, "end": 34050973}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18_tb.png", "start": 34050973, "end": 34061599}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18_tc.png", "start": 34061599, "end": 34066128}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk18b.png", "start": 34066128, "end": 34074301}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk19_f.png", "start": 34074301, "end": 34086002}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk19_t.png", "start": 34086002, "end": 34097393}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk19b.png", "start": 34097393, "end": 34108713}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk1_1.png", "start": 34108713, "end": 34111198}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk1_2.png", "start": 34111198, "end": 34114369}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk1_3.png", "start": 34114369, "end": 34116612}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk2_1.png", "start": 34116612, "end": 34119161}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk2_2.png", "start": 34119161, "end": 34122534}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk5.png", "start": 34122534, "end": 34125701}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk6_1.png", "start": 34125701, "end": 34128344}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk6_2.png", "start": 34128344, "end": 34133406}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk7_1.png", "start": 34133406, "end": 34136046}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk7_1b.png", "start": 34136046, "end": 34138590}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk7_2.png", "start": 34138590, "end": 34141756}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk8_1c.png", "start": 34141756, "end": 34154361}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk8_1d.png", "start": 34154361, "end": 34166964}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk9_1.png", "start": 34166964, "end": 34180174}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk9_1b.png", "start": 34180174, "end": 34193831}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_brk9_f.png", "start": 34193831, "end": 34206618}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr1_1.png", "start": 34206618, "end": 34216565}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr2_1.png", "start": 34216565, "end": 34218851}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr2_2.png", "start": 34218851, "end": 34221024}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr4_1.png", "start": 34221024, "end": 34224764}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr4_3.png", "start": 34224764, "end": 34228198}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr4_4.png", "start": 34228198, "end": 34241254}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr4_5.png", "start": 34241254, "end": 34255772}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr5_1.png", "start": 34255772, "end": 34270702}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_flr5_2.png", "start": 34270702, "end": 34286230}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_stp1.png", "start": 34286230, "end": 34288827}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_stp2.png", "start": 34288827, "end": 34291370}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_csl_trm1.png", "start": 34291370, "end": 34292695}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick1.png", "start": 34292695, "end": 34314168}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick1_t.png", "start": 34314168, "end": 34325555}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick1_t2.png", "start": 34325555, "end": 34338717}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick1_t2b_fbr.png", "start": 34338717, "end": 34352667}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick1_t3.png", "start": 34352667, "end": 34364407}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick1_t4.png", "start": 34364407, "end": 34377183}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick2.png", "start": 34377183, "end": 34382717}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick3.png", "start": 34382717, "end": 34388091}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick4.png", "start": 34388091, "end": 34393399}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick5.png", "start": 34393399, "end": 34399221}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick6.png", "start": 34399221, "end": 34408382}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick6b.png", "start": 34408382, "end": 34417390}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dbrick6f.png", "start": 34417390, "end": 34426680}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_door1.png", "start": 34426680, "end": 34431363}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_door2.png", "start": 34431363, "end": 34435862}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_door3.png", "start": 34435862, "end": 34440511}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_door3b.png", "start": 34440511, "end": 34443052}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_door4.png", "start": 34443052, "end": 34447616}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_door4b.png", "start": 34447616, "end": 34450212}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr1a.png", "start": 34450212, "end": 34468996}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr1a_blu.png", "start": 34468996, "end": 34487360}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr1b.png", "start": 34487360, "end": 34511509}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr1b_blu.png", "start": 34511509, "end": 34531128}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr2a.png", "start": 34531128, "end": 34555505}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr2a_blu.png", "start": 34555505, "end": 34575399}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr3a.png", "start": 34575399, "end": 34588882}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr3a_blu.png", "start": 34588882, "end": 34602172}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr3b.png", "start": 34602172, "end": 34616480}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr3b_blu.png", "start": 34616480, "end": 34630686}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr3c.png", "start": 34630686, "end": 34643529}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dr3c_blu.png", "start": 34643529, "end": 34656653}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_dwall1.png", "start": 34656653, "end": 34659129}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick1.png", "start": 34659129, "end": 34671711}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick10.png", "start": 34671711, "end": 34682596}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick10b.png", "start": 34682596, "end": 34693082}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick11.png", "start": 34693082, "end": 34708341}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick12.png", "start": 34708341, "end": 34725758}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick12b.png", "start": 34725758, "end": 34738997}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick13.png", "start": 34738997, "end": 34748895}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick14.png", "start": 34748895, "end": 34757910}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick15.png", "start": 34757910, "end": 34772711}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick16.png", "start": 34772711, "end": 34784075}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick16b.png", "start": 34784075, "end": 34792393}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick17.png", "start": 34792393, "end": 34805527}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick17b.png", "start": 34805527, "end": 34817793}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick17c.png", "start": 34817793, "end": 34831052}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick18.png", "start": 34831052, "end": 34847052}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick18b.png", "start": 34847052, "end": 34861440}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick2.png", "start": 34861440, "end": 34873932}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick20.png", "start": 34873932, "end": 34887332}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick21.png", "start": 34887332, "end": 34899474}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick22.png", "start": 34899474, "end": 34911635}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick3.png", "start": 34911635, "end": 34924095}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick4.png", "start": 34924095, "end": 34939645}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick5.png", "start": 34939645, "end": 34953461}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick6.png", "start": 34953461, "end": 34968172}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick7.png", "start": 34968172, "end": 34981817}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick8.png", "start": 34981817, "end": 34994556}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_ebrick9.png", "start": 34994556, "end": 35009111}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_etrim1.png", "start": 35009111, "end": 35012440}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_glass1.png", "start": 35012440, "end": 35024819}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_glass2.png", "start": 35024819, "end": 35036476}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_glass3.png", "start": 35036476, "end": 35047491}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_glass4.png", "start": 35047491, "end": 35079310}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_glass5.png", "start": 35079310, "end": 35089092}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_dec1.png", "start": 35089092, "end": 35093232}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_key1a.png", "start": 35093232, "end": 35094992}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_key1b.png", "start": 35094992, "end": 35096326}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_key2a.png", "start": 35096326, "end": 35098086}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_key2b.png", "start": 35098086, "end": 35099792}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_trim1.png", "start": 35099792, "end": 35100819}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_trim2.png", "start": 35100819, "end": 35101766}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_met_trim3.png", "start": 35101766, "end": 35102640}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_metw1a.png", "start": 35102640, "end": 35114948}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_metw1b.png", "start": 35114948, "end": 35123655}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_metw2a.png", "start": 35123655, "end": 35133562}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_metw2b.png", "start": 35133562, "end": 35145940}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_rmet.png", "start": 35145940, "end": 35159041}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_rmet_slat.png", "start": 35159041, "end": 35173352}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_rmet_tile.png", "start": 35173352, "end": 35187284}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_rmet_trim32.png", "start": 35187284, "end": 35200493}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_roof1.png", "start": 35200493, "end": 35212408}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_roof2.png", "start": 35212408, "end": 35227481}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_roof3.png", "start": 35227481, "end": 35242410}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_roof4.png", "start": 35242410, "end": 35260224}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_roof5.png", "start": 35260224, "end": 35270730}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall1.png", "start": 35270730, "end": 35312408}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall2.png", "start": 35312408, "end": 35372185}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall3.png", "start": 35372185, "end": 35422620}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall4.png", "start": 35422620, "end": 35471761}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall4_f.png", "start": 35471761, "end": 35515714}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall6.png", "start": 35515714, "end": 35560259}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall7.png", "start": 35560259, "end": 35605852}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall8.png", "start": 35605852, "end": 35643450}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall9.png", "start": 35643450, "end": 35692530}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tanwall9_f.png", "start": 35692530, "end": 35736509}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_telepad.png", "start": 35736509, "end": 35741136}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tile1.png", "start": 35741136, "end": 35813747}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_lit1_fbr.png", "start": 35813747, "end": 35815350}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_lit3_fbr.png", "start": 35815350, "end": 35816655}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_tele.png", "start": 35816655, "end": 35819837}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_trim1.png", "start": 35819837, "end": 35823003}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_trim1b.png", "start": 35823003, "end": 35825058}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_trim2.png", "start": 35825058, "end": 35828598}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_trim3.png", "start": 35828598, "end": 35832075}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_tmpl_trim4.png", "start": 35832075, "end": 35835188}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim1_1.png", "start": 35835188, "end": 35843892}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim2_1.png", "start": 35843892, "end": 35845006}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim3_1.png", "start": 35845006, "end": 35848601}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim3_2.png", "start": 35848601, "end": 35852160}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim3_3.png", "start": 35852160, "end": 35855857}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim3_4.png", "start": 35855857, "end": 35859228}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim4_1.png", "start": 35859228, "end": 35862652}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim4_2.png", "start": 35862652, "end": 35866176}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim4_3.png", "start": 35866176, "end": 35869536}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/med_trim4_4.png", "start": 35869536, "end": 35872759}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/plus_0_csl_brk14.png", "start": 35872759, "end": 35889072}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/plus_1_csl_brk14.png", "start": 35889072, "end": 35905365}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/plus_2_csl_brk14.png", "start": 35905365, "end": 35921664}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/plus_3_csl_brk14.png", "start": 35921664, "end": 35937906}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/plus_4_csl_brk14.png", "start": 35937906, "end": 35954247}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/sidewalk.png", "start": 35954247, "end": 35963003}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/sq_trim1_2.png", "start": 35963003, "end": 35971285}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/sq_trim1_2_s.png", "start": 35971285, "end": 35973860}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/tile.png", "start": 35973860, "end": 35976985}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/tile1.png", "start": 35976985, "end": 36049598}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/wall14_5.png", "start": 36049598, "end": 36053678}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/wbrick1_5.png", "start": 36053678, "end": 36057444}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/wswamp2_1.png", "start": 36057444, "end": 36060570}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_medieval/wswamp2_2.png", "start": 36060570, "end": 36064347}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig1_bone.png", "start": 36064347, "end": 36074028}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig1_bone_l.png", "start": 36074028, "end": 36104689}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig1_skull.png", "start": 36104689, "end": 36107353}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig1_skull_l.png", "start": 36107353, "end": 36116138}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig1_spine.png", "start": 36116138, "end": 36164839}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig2_bone.png", "start": 36164839, "end": 36173262}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig2_bone_l.png", "start": 36173262, "end": 36199010}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig2_bone_s.png", "start": 36199010, "end": 36201623}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/gig2_mouth_s.png", "start": 36201623, "end": 36204038}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/med_flat8.png", "start": 36204038, "end": 36206896}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/med_flat9.png", "start": 36206896, "end": 36209884}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_block.png", "start": 36209884, "end": 36213223}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_diam.png", "start": 36213223, "end": 36216113}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_trim28.png", "start": 36216113, "end": 36221156}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_trim32.png", "start": 36221156, "end": 36224302}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_trim32r.png", "start": 36224302, "end": 36227557}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_trim32s.png", "start": 36227557, "end": 36230657}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blc_trim64.png", "start": 36230657, "end": 36233800}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_block.png", "start": 36233800, "end": 36238820}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_det1.png", "start": 36238820, "end": 36239976}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_diam.png", "start": 36239976, "end": 36242873}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_diam2.png", "start": 36242873, "end": 36245668}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_diamc.png", "start": 36245668, "end": 36249199}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_door1.png", "start": 36249199, "end": 36253882}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_door2.png", "start": 36253882, "end": 36258381}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_door3.png", "start": 36258381, "end": 36263030}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_door4.png", "start": 36263030, "end": 36267594}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_door5.png", "start": 36267594, "end": 36270537}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_door6.png", "start": 36270537, "end": 36273133}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_fac1.png", "start": 36273133, "end": 36275535}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_flat.png", "start": 36275535, "end": 36278969}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_flatst.png", "start": 36278969, "end": 36281198}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_gig1.png", "start": 36281198, "end": 36285317}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_gig2.png", "start": 36285317, "end": 36288930}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_gig2b.png", "start": 36288930, "end": 36295124}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_grate.png", "start": 36295124, "end": 36298179}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_grate2.png", "start": 36298179, "end": 36301010}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_grate3.png", "start": 36301010, "end": 36302670}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_lit1_fbr.png", "start": 36302670, "end": 36304741}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_lit2_fbr.png", "start": 36304741, "end": 36305930}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_lit3.png", "start": 36305930, "end": 36307116}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_lit4.png", "start": 36307116, "end": 36308377}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_lit5.png", "start": 36308377, "end": 36310470}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_pan1.png", "start": 36310470, "end": 36313952}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_pan2.png", "start": 36313952, "end": 36316238}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_pan3.png", "start": 36316238, "end": 36319665}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_rect.png", "start": 36319665, "end": 36323444}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_rivg.png", "start": 36323444, "end": 36327365}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_rivs.png", "start": 36327365, "end": 36330723}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_slat.png", "start": 36330723, "end": 36334509}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_sqr.png", "start": 36334509, "end": 36338280}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_sqrd.png", "start": 36338280, "end": 36342118}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_sqrs.png", "start": 36342118, "end": 36345872}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_stile.png", "start": 36345872, "end": 36349099}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_tile.png", "start": 36349099, "end": 36352548}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim16.png", "start": 36352548, "end": 36356252}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim16g.png", "start": 36356252, "end": 36360002}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim16h.png", "start": 36360002, "end": 36363619}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim16s.png", "start": 36363619, "end": 36366787}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim28.png", "start": 36366787, "end": 36372084}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim32.png", "start": 36372084, "end": 36375534}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim32r.png", "start": 36375534, "end": 36379190}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim32s.png", "start": 36379190, "end": 36382514}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_trim64.png", "start": 36382514, "end": 36385991}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_blu_vtrim.png", "start": 36385991, "end": 36389561}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn2_pat.png", "start": 36389561, "end": 36393410}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_block.png", "start": 36393410, "end": 36397121}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_blockl.png", "start": 36397121, "end": 36400650}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_det1.png", "start": 36400650, "end": 36401813}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_flat.png", "start": 36401813, "end": 36405334}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_grate.png", "start": 36405334, "end": 36408722}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_grate2.png", "start": 36408722, "end": 36412228}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_grate3.png", "start": 36412228, "end": 36414246}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_lit1_fbr.png", "start": 36414246, "end": 36416359}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_lit2_fbr.png", "start": 36416359, "end": 36417547}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_lit3.png", "start": 36417547, "end": 36418718}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_lit4.png", "start": 36418718, "end": 36419928}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_lit5.png", "start": 36419928, "end": 36421981}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_pan1.png", "start": 36421981, "end": 36425444}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_pan2.png", "start": 36425444, "end": 36427720}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_pan3.png", "start": 36427720, "end": 36431197}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_pan4.png", "start": 36431197, "end": 36435060}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_rect.png", "start": 36435060, "end": 36438738}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_rivg.png", "start": 36438738, "end": 36442766}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_rivs.png", "start": 36442766, "end": 36446189}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_signs.png", "start": 36446189, "end": 36451023}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_slat.png", "start": 36451023, "end": 36454666}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_sqr.png", "start": 36454666, "end": 36458357}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_sqrd.png", "start": 36458357, "end": 36462165}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_sqrs.png", "start": 36462165, "end": 36465869}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_stile.png", "start": 36465869, "end": 36469954}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_tile.png", "start": 36469954, "end": 36473335}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_tile2.png", "start": 36473335, "end": 36476668}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim16.png", "start": 36476668, "end": 36480774}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim16g.png", "start": 36480774, "end": 36484978}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim16h.png", "start": 36484978, "end": 36489175}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim16s.png", "start": 36489175, "end": 36492798}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim32.png", "start": 36492798, "end": 36496285}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim32s.png", "start": 36496285, "end": 36499671}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_trim64.png", "start": 36499671, "end": 36503311}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_brn_vtrim.png", "start": 36503311, "end": 36506764}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_cop_flat.png", "start": 36506764, "end": 36509841}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_cop_riv.png", "start": 36509841, "end": 36513442}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_dbrn_flat.png", "start": 36513442, "end": 36516822}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_dbrn_rect.png", "start": 36516822, "end": 36520707}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_dbrn_slat.png", "start": 36520707, "end": 36524522}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grate.png", "start": 36524522, "end": 36527872}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_block.png", "start": 36527872, "end": 36531797}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_blockl.png", "start": 36531797, "end": 36535526}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_det1.png", "start": 36535526, "end": 36536777}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_fac1.png", "start": 36536777, "end": 36539687}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_flat.png", "start": 36539687, "end": 36543572}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_grate.png", "start": 36543572, "end": 36546634}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_grate2.png", "start": 36546634, "end": 36549617}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_grate3.png", "start": 36549617, "end": 36551361}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_lit1_fbr.png", "start": 36551361, "end": 36553495}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_lit2_fbr.png", "start": 36553495, "end": 36554696}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_lit3.png", "start": 36554696, "end": 36555908}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_lit4.png", "start": 36555908, "end": 36557194}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_lit5.png", "start": 36557194, "end": 36559386}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_pan1.png", "start": 36559386, "end": 36563102}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_pan2.png", "start": 36563102, "end": 36565467}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_pan3.png", "start": 36565467, "end": 36569067}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_rect.png", "start": 36569067, "end": 36573104}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_rivg.png", "start": 36573104, "end": 36577516}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_rivs.png", "start": 36577516, "end": 36581283}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_slat.png", "start": 36581283, "end": 36585324}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_sqr.png", "start": 36585324, "end": 36589322}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_sqrd.png", "start": 36589322, "end": 36593432}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_sqrs.png", "start": 36593432, "end": 36597462}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_stile.png", "start": 36597462, "end": 36600746}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_tile.png", "start": 36600746, "end": 36604007}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim16.png", "start": 36604007, "end": 36607619}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim16g.png", "start": 36607619, "end": 36611773}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim16h.png", "start": 36611773, "end": 36615322}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim16s.png", "start": 36615322, "end": 36618833}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim28.png", "start": 36618833, "end": 36623768}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim28r.png", "start": 36623768, "end": 36628794}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim32.png", "start": 36628794, "end": 36632619}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim32r.png", "start": 36632619, "end": 36636489}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim32s.png", "start": 36636489, "end": 36640135}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_trim64.png", "start": 36640135, "end": 36643792}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_grn_vtrim.png", "start": 36643792, "end": 36647042}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_beam.png", "start": 36647042, "end": 36650056}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_block.png", "start": 36650056, "end": 36653079}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_flat.png", "start": 36653079, "end": 36655813}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_lit1_fbr.png", "start": 36655813, "end": 36657624}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_lit2_fbr.png", "start": 36657624, "end": 36658747}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_lit2b.png", "start": 36658747, "end": 36662197}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_pan1.png", "start": 36662197, "end": 36665293}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_pan2.png", "start": 36665293, "end": 36667446}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_pan3.png", "start": 36667446, "end": 36670549}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_rect.png", "start": 36670549, "end": 36673746}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_signs.png", "start": 36673746, "end": 36677576}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_slat.png", "start": 36677576, "end": 36680992}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_sqr.png", "start": 36680992, "end": 36683826}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_sqrd.png", "start": 36683826, "end": 36687074}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_sqrs.png", "start": 36687074, "end": 36690035}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_gry_trim64.png", "start": 36690035, "end": 36693736}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_lbrn_flat.png", "start": 36693736, "end": 36697162}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_lbrn_rect.png", "start": 36697162, "end": 36701050}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_lbrn_slat.png", "start": 36701050, "end": 36705004}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_lift.png", "start": 36705004, "end": 36708049}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_met7_1.png", "start": 36708049, "end": 36710522}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mix_beam.png", "start": 36710522, "end": 36714174}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mix_diam.png", "start": 36714174, "end": 36717363}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mix_diam2.png", "start": 36717363, "end": 36720562}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mix_diamc.png", "start": 36720562, "end": 36724662}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt1_flat.png", "start": 36724662, "end": 36738952}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt1_rect.png", "start": 36738952, "end": 36754662}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt1_slat.png", "start": 36754662, "end": 36770418}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt1_sqr.png", "start": 36770418, "end": 36776958}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt2_flat.png", "start": 36776958, "end": 36793172}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt2_rect.png", "start": 36793172, "end": 36809898}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt2_slat.png", "start": 36809898, "end": 36826536}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt2_sqr.png", "start": 36826536, "end": 36834934}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt3_flat.png", "start": 36834934, "end": 36851534}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt3_rect.png", "start": 36851534, "end": 36868442}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt3_slat.png", "start": 36868442, "end": 36885353}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_mt3_sqr.png", "start": 36885353, "end": 36893888}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_ora_trim64.png", "start": 36893888, "end": 36896667}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_rail_flat.png", "start": 36896667, "end": 36898814}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_rune1_fbr.png", "start": 36898814, "end": 36901618}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_rune_trim32.png", "start": 36901618, "end": 36905959}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_set1.png", "start": 36905959, "end": 36975804}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_shm_flat.png", "start": 36975804, "end": 36979465}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_shm_rect.png", "start": 36979465, "end": 36983337}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_shm_slat.png", "start": 36983337, "end": 36987388}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_shm_sqr.png", "start": 36987388, "end": 36990850}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_teal_block.png", "start": 36990850, "end": 36994228}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_teal_flat.png", "start": 36994228, "end": 36997574}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_teal_trim32.png", "start": 36997574, "end": 37000850}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_teal_trim32r.png", "start": 37000850, "end": 37003825}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_teal_trim64.png", "start": 37003825, "end": 37007309}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_wall3_1.png", "start": 37007309, "end": 37018575}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/met_wall3_1_s.png", "start": 37018575, "end": 37021859}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/metal4_4.png", "start": 37021859, "end": 37026200}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0_sqbut1.png", "start": 37026200, "end": 37027282}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0_sqbut2_fbr.png", "start": 37027282, "end": 37030518}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0_sqshoot1_fbr.png", "start": 37030518, "end": 37031625}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0gig2a_fbr.png", "start": 37031625, "end": 37032098}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0gig_shot_fbr.png", "start": 37032098, "end": 37032574}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0gig_sshot_fbr.png", "start": 37032574, "end": 37033018}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0gig_ye_fbr.png", "start": 37033018, "end": 37033458}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0met_blu_keyg_fbr.png", "start": 37033458, "end": 37034358}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_0met_blu_keys_fbr.png", "start": 37034358, "end": 37035241}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_1_sqbut1.png", "start": 37035241, "end": 37036407}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_1_sqbut2_fbr.png", "start": 37036407, "end": 37039595}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_1_sqshoot1.png", "start": 37039595, "end": 37040709}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_1met_blu_keyg_fbr.png", "start": 37040709, "end": 37041610}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_1met_blu_keys_fbr.png", "start": 37041610, "end": 37042487}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_2met_blu_keyg_fbr.png", "start": 37042487, "end": 37043396}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_2met_blu_keys_fbr.png", "start": 37043396, "end": 37044265}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_3met_blu_keyg_fbr.png", "start": 37044265, "end": 37045173}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_3met_blu_keys_fbr.png", "start": 37045173, "end": 37046029}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_4met_blu_keyg_fbr.png", "start": 37046029, "end": 37046937}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_4met_blu_keys_fbr.png", "start": 37046937, "end": 37047793}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_5met_blu_keyg_fbr.png", "start": 37047793, "end": 37048702}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_5met_blu_keys_fbr.png", "start": 37048702, "end": 37049571}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_6met_blu_keyg_fbr.png", "start": 37049571, "end": 37050472}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_6met_blu_keys_fbr.png", "start": 37050472, "end": 37051349}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_a_sqbut1.png", "start": 37051349, "end": 37052431}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_a_sqbut2_fbr.png", "start": 37052431, "end": 37055667}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_a_sqshoot1_fbr.png", "start": 37055667, "end": 37056774}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_agig2a.png", "start": 37056774, "end": 37057144}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_agig_shot_fbr.png", "start": 37057144, "end": 37057519}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_agig_sshot_fbr.png", "start": 37057519, "end": 37057882}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_agig_ye.png", "start": 37057882, "end": 37058294}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_amet_blu_keyg.png", "start": 37058294, "end": 37059164}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/plus_amet_blu_keys.png", "start": 37059164, "end": 37060034}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/ret_metal1_tile.png", "start": 37060034, "end": 37073966}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/sq_lit1_fbr.png", "start": 37073966, "end": 37074339}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_metal/sq_lit2_fbr.png", "start": 37074339, "end": 37074600}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_a.png", "start": 37074600, "end": 37075148}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_b.png", "start": 37075148, "end": 37075674}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_c.png", "start": 37075674, "end": 37076200}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_d.png", "start": 37076200, "end": 37076726}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_e.png", "start": 37076726, "end": 37077252}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_f.png", "start": 37077252, "end": 37077778}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_g.png", "start": 37077778, "end": 37078304}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_h.png", "start": 37078304, "end": 37078830}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_i.png", "start": 37078830, "end": 37079357}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_j.png", "start": 37079357, "end": 37079884}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_k.png", "start": 37079884, "end": 37080411}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_l.png", "start": 37080411, "end": 37080938}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_m.png", "start": 37080938, "end": 37081465}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_n.png", "start": 37081465, "end": 37081990}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_o.png", "start": 37081990, "end": 37082515}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_01_p.png", "start": 37082515, "end": 37083040}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_a.png", "start": 37083040, "end": 37083566}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_b.png", "start": 37083566, "end": 37084092}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_c.png", "start": 37084092, "end": 37084618}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_d.png", "start": 37084618, "end": 37085144}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_e.png", "start": 37085144, "end": 37085670}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_f.png", "start": 37085670, "end": 37086196}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_g.png", "start": 37086196, "end": 37086722}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_h.png", "start": 37086722, "end": 37087248}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_i.png", "start": 37087248, "end": 37087774}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_j.png", "start": 37087774, "end": 37088300}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_k.png", "start": 37088300, "end": 37088826}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_l.png", "start": 37088826, "end": 37089352}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_m.png", "start": 37089352, "end": 37089878}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_n.png", "start": 37089878, "end": 37090404}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_o.png", "start": 37090404, "end": 37090930}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_02_p.png", "start": 37090930, "end": 37091456}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_a.png", "start": 37091456, "end": 37091982}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_b.png", "start": 37091982, "end": 37092508}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_c.png", "start": 37092508, "end": 37093034}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_d.png", "start": 37093034, "end": 37093560}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_e.png", "start": 37093560, "end": 37094086}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_f.png", "start": 37094086, "end": 37094612}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_g.png", "start": 37094612, "end": 37095138}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_h.png", "start": 37095138, "end": 37095664}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_i.png", "start": 37095664, "end": 37096190}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_j.png", "start": 37096190, "end": 37096716}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_k.png", "start": 37096716, "end": 37097242}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_l.png", "start": 37097242, "end": 37097768}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_m.png", "start": 37097768, "end": 37098295}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_n.png", "start": 37098295, "end": 37098822}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_o.png", "start": 37098822, "end": 37099349}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_03_p.png", "start": 37099349, "end": 37099876}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_a.png", "start": 37099876, "end": 37100424}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_b.png", "start": 37100424, "end": 37100950}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_c.png", "start": 37100950, "end": 37101476}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_d.png", "start": 37101476, "end": 37102002}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_e.png", "start": 37102002, "end": 37102528}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_f.png", "start": 37102528, "end": 37103054}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_g.png", "start": 37103054, "end": 37103580}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_h.png", "start": 37103580, "end": 37104106}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_i.png", "start": 37104106, "end": 37104632}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_j.png", "start": 37104632, "end": 37105158}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_k.png", "start": 37105158, "end": 37105684}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_l.png", "start": 37105684, "end": 37106210}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_m.png", "start": 37106210, "end": 37106736}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_n.png", "start": 37106736, "end": 37107262}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_o.png", "start": 37107262, "end": 37107788}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_04_p.png", "start": 37107788, "end": 37108314}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_a.png", "start": 37108314, "end": 37108840}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_b.png", "start": 37108840, "end": 37109366}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_c.png", "start": 37109366, "end": 37109892}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_d.png", "start": 37109892, "end": 37110418}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_e.png", "start": 37110418, "end": 37110944}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_f.png", "start": 37110944, "end": 37111470}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_g.png", "start": 37111470, "end": 37111996}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_h.png", "start": 37111996, "end": 37112522}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_i.png", "start": 37112522, "end": 37113048}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_j.png", "start": 37113048, "end": 37113574}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_k.png", "start": 37113574, "end": 37114100}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_l.png", "start": 37114100, "end": 37114626}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_m.png", "start": 37114626, "end": 37115152}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_n.png", "start": 37115152, "end": 37115678}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_o.png", "start": 37115678, "end": 37116204}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_05_p.png", "start": 37116204, "end": 37116730}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_a.png", "start": 37116730, "end": 37117256}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_b.png", "start": 37117256, "end": 37117782}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_c.png", "start": 37117782, "end": 37118308}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_d.png", "start": 37118308, "end": 37118834}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_e.png", "start": 37118834, "end": 37119360}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_f.png", "start": 37119360, "end": 37119886}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_g.png", "start": 37119886, "end": 37120412}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_h.png", "start": 37120412, "end": 37120938}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_i.png", "start": 37120938, "end": 37121464}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_j.png", "start": 37121464, "end": 37121990}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_k.png", "start": 37121990, "end": 37122516}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_l.png", "start": 37122516, "end": 37123042}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_m.png", "start": 37123042, "end": 37123568}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_n.png", "start": 37123568, "end": 37124094}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_o.png", "start": 37124094, "end": 37124620}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_06_p.png", "start": 37124620, "end": 37125146}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_a.png", "start": 37125146, "end": 37125672}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_b.png", "start": 37125672, "end": 37126198}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_c.png", "start": 37126198, "end": 37126724}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_d.png", "start": 37126724, "end": 37127250}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_e.png", "start": 37127250, "end": 37127776}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_f.png", "start": 37127776, "end": 37128302}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_g.png", "start": 37128302, "end": 37128828}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_h.png", "start": 37128828, "end": 37129354}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_i.png", "start": 37129354, "end": 37129880}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_j.png", "start": 37129880, "end": 37130406}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_k.png", "start": 37130406, "end": 37130932}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_l.png", "start": 37130932, "end": 37131458}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_m.png", "start": 37131458, "end": 37131984}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_n.png", "start": 37131984, "end": 37132510}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_o.png", "start": 37132510, "end": 37133036}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_07_p.png", "start": 37133036, "end": 37133562}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_a.png", "start": 37133562, "end": 37134088}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_b.png", "start": 37134088, "end": 37134614}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_c.png", "start": 37134614, "end": 37135140}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_d.png", "start": 37135140, "end": 37135666}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_e.png", "start": 37135666, "end": 37136192}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_f.png", "start": 37136192, "end": 37136718}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_g.png", "start": 37136718, "end": 37137244}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_h.png", "start": 37137244, "end": 37137770}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_i.png", "start": 37137770, "end": 37138296}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_j.png", "start": 37138296, "end": 37138822}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_k.png", "start": 37138822, "end": 37139348}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_l.png", "start": 37139348, "end": 37139874}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_m.png", "start": 37139874, "end": 37140400}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_n.png", "start": 37140400, "end": 37140927}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_o.png", "start": 37140927, "end": 37141454}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_08_p.png", "start": 37141454, "end": 37141981}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_a.png", "start": 37141981, "end": 37142508}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_b.png", "start": 37142508, "end": 37143035}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_c.png", "start": 37143035, "end": 37143562}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_d.png", "start": 37143562, "end": 37144088}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_e.png", "start": 37144088, "end": 37144614}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_f.png", "start": 37144614, "end": 37145140}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_g.png", "start": 37145140, "end": 37145666}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_h.png", "start": 37145666, "end": 37146192}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_i.png", "start": 37146192, "end": 37146718}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_j.png", "start": 37146718, "end": 37147244}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_k.png", "start": 37147244, "end": 37147770}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_l.png", "start": 37147770, "end": 37148296}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_m.png", "start": 37148296, "end": 37148822}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_n.png", "start": 37148822, "end": 37149348}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_o.png", "start": 37149348, "end": 37149874}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_09_p.png", "start": 37149874, "end": 37150400}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_a.png", "start": 37150400, "end": 37150927}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_b.png", "start": 37150927, "end": 37151453}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_c.png", "start": 37151453, "end": 37151979}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_d.png", "start": 37151979, "end": 37152505}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_e.png", "start": 37152505, "end": 37153031}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_f.png", "start": 37153031, "end": 37153557}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_g.png", "start": 37153557, "end": 37154083}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_h.png", "start": 37154083, "end": 37154609}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_i.png", "start": 37154609, "end": 37155135}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_j.png", "start": 37155135, "end": 37155661}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_k.png", "start": 37155661, "end": 37156187}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_l.png", "start": 37156187, "end": 37156713}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_m.png", "start": 37156713, "end": 37157239}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_n.png", "start": 37157239, "end": 37157765}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_o.png", "start": 37157765, "end": 37158291}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_10_p.png", "start": 37158291, "end": 37158817}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_a.png", "start": 37158817, "end": 37159344}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_b.png", "start": 37159344, "end": 37159871}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_c.png", "start": 37159871, "end": 37160398}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_d.png", "start": 37160398, "end": 37160925}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_e.png", "start": 37160925, "end": 37161452}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_f.png", "start": 37161452, "end": 37161978}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_g.png", "start": 37161978, "end": 37162504}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_h.png", "start": 37162504, "end": 37163030}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_i.png", "start": 37163030, "end": 37163556}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_j.png", "start": 37163556, "end": 37164082}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_k.png", "start": 37164082, "end": 37164608}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_l.png", "start": 37164608, "end": 37165134}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_m.png", "start": 37165134, "end": 37165660}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_n.png", "start": 37165660, "end": 37166186}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_o.png", "start": 37166186, "end": 37166712}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_11_p.png", "start": 37166712, "end": 37167238}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_a.png", "start": 37167238, "end": 37167764}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_b.png", "start": 37167764, "end": 37168290}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_c.png", "start": 37168290, "end": 37168816}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_d.png", "start": 37168816, "end": 37169342}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_e.png", "start": 37169342, "end": 37169868}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_f.png", "start": 37169868, "end": 37170394}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_g.png", "start": 37170394, "end": 37170920}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_h.png", "start": 37170920, "end": 37171446}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_i.png", "start": 37171446, "end": 37171972}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_j.png", "start": 37171972, "end": 37172498}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_k.png", "start": 37172498, "end": 37173024}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_l.png", "start": 37173024, "end": 37173550}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_m.png", "start": 37173550, "end": 37174076}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_n.png", "start": 37174076, "end": 37174602}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_o.png", "start": 37174602, "end": 37175128}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_12_p.png", "start": 37175128, "end": 37175654}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_a.png", "start": 37175654, "end": 37176180}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_b.png", "start": 37176180, "end": 37176706}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_c.png", "start": 37176706, "end": 37177232}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_d.png", "start": 37177232, "end": 37177758}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_e.png", "start": 37177758, "end": 37178284}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_f.png", "start": 37178284, "end": 37178810}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_g.png", "start": 37178810, "end": 37179336}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_h.png", "start": 37179336, "end": 37179862}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_i.png", "start": 37179862, "end": 37180388}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_j.png", "start": 37180388, "end": 37180914}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_k.png", "start": 37180914, "end": 37181440}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_l.png", "start": 37181440, "end": 37181966}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_m.png", "start": 37181966, "end": 37182492}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_n.png", "start": 37182492, "end": 37183018}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_o.png", "start": 37183018, "end": 37183544}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_13_p.png", "start": 37183544, "end": 37184070}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_a.png", "start": 37184070, "end": 37184618}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_b.png", "start": 37184618, "end": 37185144}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_c.png", "start": 37185144, "end": 37185670}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_d.png", "start": 37185670, "end": 37186196}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_e.png", "start": 37186196, "end": 37186722}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_f.png", "start": 37186722, "end": 37187248}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_g.png", "start": 37187248, "end": 37187774}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_h.png", "start": 37187774, "end": 37188300}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_i.png", "start": 37188300, "end": 37188826}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_j.png", "start": 37188826, "end": 37189352}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_k.png", "start": 37189352, "end": 37189878}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_l.png", "start": 37189878, "end": 37190404}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_m.png", "start": 37190404, "end": 37190930}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_n.png", "start": 37190930, "end": 37191456}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_o.png", "start": 37191456, "end": 37191982}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_14_p.png", "start": 37191982, "end": 37192508}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_a_fbr.png", "start": 37192508, "end": 37193034}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_b_fbr.png", "start": 37193034, "end": 37193560}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_c_fbr.png", "start": 37193560, "end": 37194086}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_d_fbr.png", "start": 37194086, "end": 37194612}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_e_fbr.png", "start": 37194612, "end": 37195138}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_f_fbr.png", "start": 37195138, "end": 37195664}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_g_fbr.png", "start": 37195664, "end": 37196190}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_h_fbr.png", "start": 37196190, "end": 37196716}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_i_fbr.png", "start": 37196716, "end": 37197242}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_j_fbr.png", "start": 37197242, "end": 37197768}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_k_fbr.png", "start": 37197768, "end": 37198294}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_l_fbr.png", "start": 37198294, "end": 37198820}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_m_fbr.png", "start": 37198820, "end": 37199346}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_n_fbr.png", "start": 37199346, "end": 37199872}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_o_fbr.png", "start": 37199872, "end": 37200399}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_15_p_fbr.png", "start": 37200399, "end": 37200926}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_a_fbr.png", "start": 37200926, "end": 37201452}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_b_fbr.png", "start": 37201452, "end": 37201978}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_c_fbr.png", "start": 37201978, "end": 37202504}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_d_fbr.png", "start": 37202504, "end": 37203030}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_e_fbr.png", "start": 37203030, "end": 37203557}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_f_fbr.png", "start": 37203557, "end": 37204084}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_g_fbr.png", "start": 37204084, "end": 37204609}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_h_fbr.png", "start": 37204609, "end": 37205135}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_i_fbr.png", "start": 37205135, "end": 37205661}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_j_fbr.png", "start": 37205661, "end": 37206187}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_k_fbr.png", "start": 37206187, "end": 37206713}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_l_fbr.png", "start": 37206713, "end": 37207239}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_m_fbr.png", "start": 37207239, "end": 37207766}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_n_fbr.png", "start": 37207766, "end": 37208293}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_o_fbr.png", "start": 37208293, "end": 37208818}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_palette/flat_16_p_fbr.png", "start": 37208818, "end": 37209344}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/JarBod1.png", "start": 37209344, "end": 37209691}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/JarBod2.png", "start": 37209691, "end": 37210009}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/JarTop1.png", "start": 37210009, "end": 37210234}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/JarTop2.png", "start": 37210234, "end": 37210374}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/crate-door-brn.png", "start": 37210374, "end": 37219012}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/crate-door-grn.png", "start": 37219012, "end": 37229068}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/crate-door-orn.png", "start": 37229068, "end": 37238167}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/crate-side-brn.png", "start": 37238167, "end": 37258293}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/crate-side-grn.png", "start": 37258293, "end": 37281206}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/crate-side-orn.png", "start": 37281206, "end": 37302535}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/go-ep0_fbr.png", "start": 37302535, "end": 37304310}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_book_blue.png", "start": 37304310, "end": 37304890}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_book_green.png", "start": 37304890, "end": 37305513}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_book_pink.png", "start": 37305513, "end": 37306168}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_book_red.png", "start": 37306168, "end": 37306885}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_book_teal.png", "start": 37306885, "end": 37307472}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_books_wood.png", "start": 37307472, "end": 37320402}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_dbrick4_p1.png", "start": 37320402, "end": 37352239}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_dbrick4_p2.png", "start": 37352239, "end": 37385660}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_ebrick9_p1.png", "start": 37385660, "end": 37417551}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/med_ebrick9_p2.png", "start": 37417551, "end": 37448917}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/note-e0_fbr.png", "start": 37448917, "end": 37468346}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_0blink_fbr.png", "start": 37468346, "end": 37468575}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_0tvnoise.png", "start": 37468575, "end": 37469561}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_1blink_fbr.png", "start": 37469561, "end": 37469790}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_1tvnoise.png", "start": 37469790, "end": 37470768}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_2blink_fbr.png", "start": 37470768, "end": 37470999}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_2tvnoise.png", "start": 37470999, "end": 37471972}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_3blink_fbr.png", "start": 37471972, "end": 37472203}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_3tvnoise.png", "start": 37472203, "end": 37473185}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_4blink_fbr.png", "start": 37473185, "end": 37473415}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_4tvnoise.png", "start": 37473415, "end": 37474399}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_5tvnoise.png", "start": 37474399, "end": 37475363}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_6tvnoise.png", "start": 37475363, "end": 37476353}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_7tvnoise.png", "start": 37476353, "end": 37477327}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_8tvnoise.png", "start": 37477327, "end": 37478328}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_9tvnoise.png", "start": 37478328, "end": 37479305}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_ablink_fbr.png", "start": 37479305, "end": 37479535}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_atvnoise.png", "start": 37479535, "end": 37480095}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/plus_atvnoise64.png", "start": 37480095, "end": 37481365}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/qr.png", "start": 37481365, "end": 37482785}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/radio16.png", "start": 37482785, "end": 37483553}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/radio32.png", "start": 37483553, "end": 37484543}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/radio64.png", "start": 37484543, "end": 37486301}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/radiowood.png", "start": 37486301, "end": 37488444}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/secret_gem_1.png", "start": 37488444, "end": 37491480}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/secret_gem_2.png", "start": 37491480, "end": 37494184}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/secret_gem_3.png", "start": 37494184, "end": 37497244}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/secret_gem_4.png", "start": 37497244, "end": 37501002}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_props/secret_gem_h.png", "start": 37501002, "end": 37501553}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/_t_fence01_fbr.png", "start": 37501553, "end": 37507270}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/_t_flare01_fbr.png", "start": 37507270, "end": 37507656}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqconc03.png", "start": 37507656, "end": 37518216}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqconc04.png", "start": 37518216, "end": 37529272}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqconc05.png", "start": 37529272, "end": 37533863}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqf006b.png", "start": 37533863, "end": 37536490}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqf032.png", "start": 37536490, "end": 37537761}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqf049.png", "start": 37537761, "end": 37540038}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqf074.png", "start": 37540038, "end": 37543460}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqf075.png", "start": 37543460, "end": 37546619}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqmetl01.png", "start": 37546619, "end": 37549347}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqmetl07.png", "start": 37549347, "end": 37554118}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqmetl14.png", "start": 37554118, "end": 37561850}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqmetl28.png", "start": 37561850, "end": 37563502}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqmetl30.png", "start": 37563502, "end": 37565154}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqmetl33.png", "start": 37565154, "end": 37567590}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpanl09.png", "start": 37567590, "end": 37570382}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpanl10.png", "start": 37570382, "end": 37575276}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe01.png", "start": 37575276, "end": 37579464}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe04.png", "start": 37579464, "end": 37581928}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe05.png", "start": 37581928, "end": 37585094}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe08.png", "start": 37585094, "end": 37590726}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe09.png", "start": 37590726, "end": 37598830}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe12.png", "start": 37598830, "end": 37606950}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe13.png", "start": 37606950, "end": 37613261}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqpipe14.png", "start": 37613261, "end": 37620614}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust01.png", "start": 37620614, "end": 37623245}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust02.png", "start": 37623245, "end": 37628546}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust03.png", "start": 37628546, "end": 37633849}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust03b.png", "start": 37633849, "end": 37636579}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust04.png", "start": 37636579, "end": 37637972}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust09.png", "start": 37637972, "end": 37641860}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqrust10.png", "start": 37641860, "end": 37645026}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsect14.png", "start": 37645026, "end": 37647402}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsect15.png", "start": 37647402, "end": 37650349}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsect16.png", "start": 37650349, "end": 37653595}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsect16b.png", "start": 37653595, "end": 37658552}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp01.png", "start": 37658552, "end": 37661198}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp02.png", "start": 37661198, "end": 37666063}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp03.png", "start": 37666063, "end": 37669965}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp04.png", "start": 37669965, "end": 37671508}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp06.png", "start": 37671508, "end": 37672887}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp07.png", "start": 37672887, "end": 37674242}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp08.png", "start": 37674242, "end": 37676444}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqsupp09.png", "start": 37676444, "end": 37679297}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqtrim01.png", "start": 37679297, "end": 37680524}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqtrim02.png", "start": 37680524, "end": 37681448}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqtrim03.png", "start": 37681448, "end": 37682019}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/aqtrim08.png", "start": 37682019, "end": 37682751}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/butmet.png", "start": 37682751, "end": 37684662}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_1.png", "start": 37684662, "end": 37688018}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_2.png", "start": 37688018, "end": 37690697}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_3.png", "start": 37690697, "end": 37693186}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_3b.png", "start": 37693186, "end": 37695814}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_4.png", "start": 37695814, "end": 37697419}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_5.png", "start": 37697419, "end": 37700518}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_6.png", "start": 37700518, "end": 37703960}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_7.png", "start": 37703960, "end": 37706305}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/comp1_8.png", "start": 37706305, "end": 37708634}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/compbase.png", "start": 37708634, "end": 37711053}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate.png", "start": 37711053, "end": 37714170}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_bottom.png", "start": 37714170, "end": 37716286}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_s_bottom.png", "start": 37716286, "end": 37717374}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_s_sside.png", "start": 37717374, "end": 37718319}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_s_top.png", "start": 37718319, "end": 37719758}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_s_tside.png", "start": 37719758, "end": 37721421}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_side.png", "start": 37721421, "end": 37724889}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_top.png", "start": 37724889, "end": 37727635}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_xs_bot.png", "start": 37727635, "end": 37728302}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_xs_sside.png", "start": 37728302, "end": 37729247}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_xs_top.png", "start": 37729247, "end": 37730117}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate0_xs_tside.png", "start": 37730117, "end": 37731057}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_bottom.png", "start": 37731057, "end": 37733021}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_s_bottom.png", "start": 37733021, "end": 37734051}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_s_sside.png", "start": 37734051, "end": 37734929}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_s_top.png", "start": 37734929, "end": 37736062}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_s_tside.png", "start": 37736062, "end": 37737527}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_side.png", "start": 37737527, "end": 37740644}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_top.png", "start": 37740644, "end": 37742765}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_xs_bot.png", "start": 37742765, "end": 37743419}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_xs_sside.png", "start": 37743419, "end": 37744297}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_xs_top.png", "start": 37744297, "end": 37745004}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/crate1_xs_tside.png", "start": 37745004, "end": 37745842}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/dem4_1.png", "start": 37745842, "end": 37755241}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/dem4_4.png", "start": 37755241, "end": 37763287}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/dem5_3_fbr.png", "start": 37763287, "end": 37772104}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/door02_1.png", "start": 37772104, "end": 37776236}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/doorr02_1.png", "start": 37776236, "end": 37777802}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/doortrak1.png", "start": 37777802, "end": 37779413}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/doortrak2-corn.png", "start": 37779413, "end": 37780710}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/doortrak2.png", "start": 37780710, "end": 37782375}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/ecop1_1.png", "start": 37782375, "end": 37785452}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/ecop1_4.png", "start": 37785452, "end": 37789053}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/edoor01_1.png", "start": 37789053, "end": 37801199}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/edoor02.png", "start": 37801199, "end": 37813250}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/edoor02.png.png", "start": 37813250, "end": 37825301}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/fddoor01.png", "start": 37825301, "end": 37834055}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/fddoor01b.png", "start": 37834055, "end": 37844738}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/fdoor02.png", "start": 37844738, "end": 37853292}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/flat4.png", "start": 37853292, "end": 37854465}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/floor5_2.png", "start": 37854465, "end": 37857191}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/floor5_3.png", "start": 37857191, "end": 37859371}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/laserfield1_fbr.png", "start": 37859371, "end": 37870380}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/light2.png", "start": 37870380, "end": 37870978}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/lit8nb.png", "start": 37870978, "end": 37871292}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/lit8sfb_fbr.png", "start": 37871292, "end": 37871589}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/met2.png", "start": 37871589, "end": 37883913}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/metalstrip_1.png", "start": 37883913, "end": 37886196}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_side1.png", "start": 37886196, "end": 37886961}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_stem.png", "start": 37886961, "end": 37887607}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_top1.png", "start": 37887607, "end": 37891007}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_top2.png", "start": 37891007, "end": 37894722}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_top3.png", "start": 37894722, "end": 37898130}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_top4.png", "start": 37898130, "end": 37900826}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plat_top5.png", "start": 37900826, "end": 37902921}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0_gkey.png", "start": 37902921, "end": 37903985}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0_skey.png", "start": 37903985, "end": 37905001}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0_tscrn0.png", "start": 37905001, "end": 37907114}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0_tscrn1.png", "start": 37907114, "end": 37909239}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0arrow2_d_fbr.png", "start": 37909239, "end": 37910493}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0arrow2_h_fbr.png", "start": 37910493, "end": 37911762}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0arrow2_u_fbr.png", "start": 37911762, "end": 37913020}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0arrow_d_fbr.png", "start": 37913020, "end": 37913902}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0arrow_h_fbr.png", "start": 37913902, "end": 37914781}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0arrow_u_fbr.png", "start": 37914781, "end": 37915656}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0basebtn1b_fbr.png", "start": 37915656, "end": 37916545}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0basebtn2_fbr.png", "start": 37916545, "end": 37916891}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0basebtn2b_fbr.png", "start": 37916891, "end": 37917231}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0basebtn_fbr.png", "start": 37917231, "end": 37918512}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0button3_fbr.png", "start": 37918512, "end": 37920388}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0lit8s.png", "start": 37920388, "end": 37920702}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0planet_a_fbr.png", "start": 37920702, "end": 37922293}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0planet_b_fbr.png", "start": 37922293, "end": 37923403}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0planet_c_fbr.png", "start": 37923403, "end": 37924503}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0slipbot.png", "start": 37924503, "end": 37927786}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0sliptop.png", "start": 37927786, "end": 37931429}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0tek_jump1_fbr.png", "start": 37931429, "end": 37933912}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0term128.png", "start": 37933912, "end": 37936716}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0term64.png", "start": 37936716, "end": 37937908}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0tlight1.png", "start": 37937908, "end": 37938506}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0tlight2.png", "start": 37938506, "end": 37939119}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_0tlight3.png", "start": 37939119, "end": 37939690}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1_gkey.png", "start": 37939690, "end": 37940740}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1_skey.png", "start": 37940740, "end": 37941745}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1arrow2_d_fbr.png", "start": 37941745, "end": 37943001}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1arrow2_h_fbr.png", "start": 37943001, "end": 37944268}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1arrow2_u_fbr.png", "start": 37944268, "end": 37945526}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1arrow_d_fbr.png", "start": 37945526, "end": 37946412}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1arrow_h_fbr.png", "start": 37946412, "end": 37947294}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1arrow_u_fbr.png", "start": 37947294, "end": 37948172}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1basebtn.png", "start": 37948172, "end": 37949451}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1basebtn1b.png", "start": 37949451, "end": 37950329}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1basebtn2.png", "start": 37950329, "end": 37950679}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1basebtn2b.png", "start": 37950679, "end": 37951037}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1planet_a_fbr.png", "start": 37951037, "end": 37952658}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1planet_b_fbr.png", "start": 37952658, "end": 37953743}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1planet_c_fbr.png", "start": 37953743, "end": 37954815}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1tek_jump1_fbr.png", "start": 37954815, "end": 37957298}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1term128.png", "start": 37957298, "end": 37960101}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_1term64.png", "start": 37960101, "end": 37961293}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2_gkey.png", "start": 37961293, "end": 37962355}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2_skey.png", "start": 37962355, "end": 37963361}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2arrow2_d_fbr.png", "start": 37963361, "end": 37964604}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2arrow2_h_fbr.png", "start": 37964604, "end": 37965865}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2arrow2_u_fbr.png", "start": 37965865, "end": 37967111}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2arrow_d_fbr.png", "start": 37967111, "end": 37968009}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2arrow_h_fbr.png", "start": 37968009, "end": 37968899}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2arrow_u_fbr.png", "start": 37968899, "end": 37969786}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2planet_a_fbr.png", "start": 37969786, "end": 37971389}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2planet_b_fbr.png", "start": 37971389, "end": 37972491}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_2planet_c_fbr.png", "start": 37972491, "end": 37973609}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_3planet_a_fbr.png", "start": 37973609, "end": 37975199}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_3planet_b_fbr.png", "start": 37975199, "end": 37976293}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_3planet_c_fbr.png", "start": 37976293, "end": 37977416}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_4planet_a_fbr.png", "start": 37977416, "end": 37979025}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_4planet_b_fbr.png", "start": 37979025, "end": 37980140}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_4planet_c_fbr.png", "start": 37980140, "end": 37981287}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_5planet_a_fbr.png", "start": 37981287, "end": 37982919}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_5planet_b_fbr.png", "start": 37982919, "end": 37984046}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_5planet_c_fbr.png", "start": 37984046, "end": 37985153}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_6planet_a_fbr.png", "start": 37985153, "end": 37986774}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_6planet_b_fbr.png", "start": 37986774, "end": 37987898}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_6planet_c_fbr.png", "start": 37987898, "end": 37988992}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_7planet_a_fbr.png", "start": 37988992, "end": 37990580}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_7planet_b_fbr.png", "start": 37990580, "end": 37991693}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_7planet_c_fbr.png", "start": 37991693, "end": 37992785}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_8planet_a_fbr.png", "start": 37992785, "end": 37994407}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_8planet_b_fbr.png", "start": 37994407, "end": 37995534}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_8planet_c_fbr.png", "start": 37995534, "end": 37996623}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_9planet_a_fbr.png", "start": 37996623, "end": 37998207}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_9planet_b_fbr.png", "start": 37998207, "end": 37999295}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_9planet_c_fbr.png", "start": 37999295, "end": 38000371}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_A_tscrn0.png", "start": 38000371, "end": 38001994}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_A_tscrn1.png", "start": 38001994, "end": 38004503}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_A_tscrn2.png", "start": 38004503, "end": 38006103}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_abasebtn.png", "start": 38006103, "end": 38007392}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_abasebtn1b.png", "start": 38007392, "end": 38008681}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_abasebtn2.png", "start": 38008681, "end": 38009052}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_abasebtn2b.png", "start": 38009052, "end": 38009423}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_abasebtnb.png", "start": 38009423, "end": 38009794}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_abutton3_fbr.png", "start": 38009794, "end": 38011682}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_alit8s_fbr.png", "start": 38011682, "end": 38011979}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_atek_jump1_fbr.png", "start": 38011979, "end": 38014472}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_atlight1_fbr.png", "start": 38014472, "end": 38015062}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_atlight2_fbr.png", "start": 38015062, "end": 38015635}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/plus_atlight3_fbr.png", "start": 38015635, "end": 38016237}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_1.png", "start": 38016237, "end": 38021802}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_2.png", "start": 38021802, "end": 38027656}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_3.png", "start": 38027656, "end": 38033402}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_4.png", "start": 38033402, "end": 38035968}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_4b_l.png", "start": 38035968, "end": 38038928}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_5.png", "start": 38038928, "end": 38041800}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_flat.png", "start": 38041800, "end": 38047751}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33_lit.png", "start": 38047751, "end": 38048575}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_1.png", "start": 38048575, "end": 38054038}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_2.png", "start": 38054038, "end": 38059880}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_3.png", "start": 38059880, "end": 38065867}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_4.png", "start": 38065867, "end": 38068629}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_5.png", "start": 38068629, "end": 38071678}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_flat.png", "start": 38071678, "end": 38077876}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw33b_lit.png", "start": 38077876, "end": 38078828}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_1.png", "start": 38078828, "end": 38084440}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_2.png", "start": 38084440, "end": 38090570}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_3.png", "start": 38090570, "end": 38096465}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_4.png", "start": 38096465, "end": 38103476}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_trim1.png", "start": 38103476, "end": 38105984}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_trim2.png", "start": 38105984, "end": 38107767}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw37_trim3.png", "start": 38107767, "end": 38110258}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/rw39_1_fbr.png", "start": 38110258, "end": 38116203}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/spotlight_fbr.png", "start": 38116203, "end": 38118815}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/star_lasergrid.png", "start": 38118815, "end": 38119195}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_band1a.png", "start": 38119195, "end": 38121699}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_band1b.png", "start": 38121699, "end": 38124307}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok01.png", "start": 38124307, "end": 38126985}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok01a.png", "start": 38126985, "end": 38129757}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok02.png", "start": 38129757, "end": 38134884}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok02a.png", "start": 38134884, "end": 38139743}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok03.png", "start": 38139743, "end": 38142185}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok03a.png", "start": 38142185, "end": 38144040}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok04.png", "start": 38144040, "end": 38147106}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok04h.png", "start": 38147106, "end": 38149830}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok05.png", "start": 38149830, "end": 38154118}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok06.png", "start": 38154118, "end": 38156968}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok06h.png", "start": 38156968, "end": 38158945}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok07.png", "start": 38158945, "end": 38161731}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok07a.png", "start": 38161731, "end": 38164517}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok08.png", "start": 38164517, "end": 38169024}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok09.png", "start": 38169024, "end": 38172344}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok10.png", "start": 38172344, "end": 38178355}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok10b.png", "start": 38178355, "end": 38184063}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok10c.png", "start": 38184063, "end": 38187281}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok11.png", "start": 38187281, "end": 38193222}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok11b.png", "start": 38193222, "end": 38198827}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_blok12c.png", "start": 38198827, "end": 38202034}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flat01.png", "start": 38202034, "end": 38204468}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flat02.png", "start": 38204468, "end": 38206833}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flat05.png", "start": 38206833, "end": 38215029}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flor1a.png", "start": 38215029, "end": 38218641}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flor1b.png", "start": 38218641, "end": 38222259}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flor2a.png", "start": 38222259, "end": 38224853}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flor2b.png", "start": 38224853, "end": 38226789}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flor2c.png", "start": 38226789, "end": 38228269}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_flor2d.png", "start": 38228269, "end": 38228773}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit01_fbr.png", "start": 38228773, "end": 38228953}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit02_fbr.png", "start": 38228953, "end": 38229095}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit03_fbr.png", "start": 38229095, "end": 38229230}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit04_fbr.png", "start": 38229230, "end": 38229362}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit05_fbr.png", "start": 38229362, "end": 38229528}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit06_fbr.png", "start": 38229528, "end": 38229684}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit07_fbr.png", "start": 38229684, "end": 38231987}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_lit08_fbr.png", "start": 38231987, "end": 38232204}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_metalsheeta.png", "start": 38232204, "end": 38238803}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_metalsheetb.png", "start": 38238803, "end": 38250316}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_0_fbr.png", "start": 38250316, "end": 38251090}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_1_fbr.png", "start": 38251090, "end": 38251852}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_2_fbr.png", "start": 38251852, "end": 38252647}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_3_fbr.png", "start": 38252647, "end": 38253405}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_4_fbr.png", "start": 38253405, "end": 38254210}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_5_fbr.png", "start": 38254210, "end": 38255008}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_6_fbr.png", "start": 38255008, "end": 38255805}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_7_fbr.png", "start": 38255805, "end": 38256579}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_8_fbr.png", "start": 38256579, "end": 38257359}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_9_fbr.png", "start": 38257359, "end": 38258152}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_num_x.png", "start": 38258152, "end": 38258960}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_rivs01.png", "start": 38258960, "end": 38261388}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_rivs01a.png", "start": 38261388, "end": 38263842}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_sign1.png", "start": 38263842, "end": 38266792}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tech01.png", "start": 38266792, "end": 38270843}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tech02.png", "start": 38270843, "end": 38273946}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tech03.png", "start": 38273946, "end": 38289937}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tech04.png", "start": 38289937, "end": 38292792}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tech05.png", "start": 38292792, "end": 38295571}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tech06.png", "start": 38295571, "end": 38298383}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1a.png", "start": 38298383, "end": 38300871}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1aa.png", "start": 38300871, "end": 38303506}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1b.png", "start": 38303506, "end": 38306237}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1ba.png", "start": 38306237, "end": 38308861}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1c.png", "start": 38308861, "end": 38311646}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1ca.png", "start": 38311646, "end": 38314472}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1d.png", "start": 38314472, "end": 38316772}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim1e.png", "start": 38316772, "end": 38318917}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2a.png", "start": 38318917, "end": 38321990}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2aa.png", "start": 38321990, "end": 38324642}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2b.png", "start": 38324642, "end": 38327270}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2ba.png", "start": 38327270, "end": 38329958}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2c.png", "start": 38329958, "end": 38332749}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2ca.png", "start": 38332749, "end": 38335628}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2d.png", "start": 38335628, "end": 38338090}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_trim2e.png", "start": 38338090, "end": 38340597}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_tris02.png", "start": 38340597, "end": 38343511}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall05.png", "start": 38343511, "end": 38346837}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall1a.png", "start": 38346837, "end": 38357364}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall1aa.png", "start": 38357364, "end": 38367685}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall1b.png", "start": 38367685, "end": 38378785}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall1ba.png", "start": 38378785, "end": 38389624}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall2a.png", "start": 38389624, "end": 38400408}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall2aa.png", "start": 38400408, "end": 38411428}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall2ab.png", "start": 38411428, "end": 38423185}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall2b.png", "start": 38423185, "end": 38437423}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall2ba.png", "start": 38437423, "end": 38450874}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall3a.png", "start": 38450874, "end": 38458841}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall3aa.png", "start": 38458841, "end": 38467108}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall3b.png", "start": 38467108, "end": 38475430}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall3ba.png", "start": 38475430, "end": 38484393}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall6a.png", "start": 38484393, "end": 38487627}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall6b.png", "start": 38487627, "end": 38490641}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall6c.png", "start": 38490641, "end": 38494317}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall6d.png", "start": 38494317, "end": 38498020}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall6e.png", "start": 38498020, "end": 38501816}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall7a.png", "start": 38501816, "end": 38512708}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wall7b.png", "start": 38512708, "end": 38520149}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wire01.png", "start": 38520149, "end": 38523085}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wire02.png", "start": 38523085, "end": 38526620}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/t_wire03.png", "start": 38526620, "end": 38530220}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tech04_1.png", "start": 38530220, "end": 38531043}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tech04_3.png", "start": 38531043, "end": 38532512}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tech08_1.png", "start": 38532512, "end": 38543565}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tech08_2.png", "start": 38543565, "end": 38554618}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tech10_3.png", "start": 38554618, "end": 38558474}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tech14-1.png", "start": 38558474, "end": 38568745}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/techbasetextures.txt", "start": 38568745, "end": 38569255}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/techeye1_fbr.png", "start": 38569255, "end": 38572599}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/techeye2_fbr.png", "start": 38572599, "end": 38576001}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_door1.png", "start": 38576001, "end": 38588147}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_door2.png", "start": 38588147, "end": 38600198}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_flr3.png", "start": 38600198, "end": 38603579}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_grate.png", "start": 38603579, "end": 38606457}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_lit1_fbr.png", "start": 38606457, "end": 38608092}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_lit2_fbr.png", "start": 38608092, "end": 38609078}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_lit3_fbr.png", "start": 38609078, "end": 38610920}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_lit4_fbr.png", "start": 38610920, "end": 38612047}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_pip1_fbr.png", "start": 38612047, "end": 38615129}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_pipe1.png", "start": 38615129, "end": 38617929}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_pipe2.png", "start": 38617929, "end": 38619588}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_trm1.png", "start": 38619588, "end": 38622046}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_trm3.png", "start": 38622046, "end": 38624656}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tek_wall4_1.png", "start": 38624656, "end": 38639101}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tele_frame1.png", "start": 38639101, "end": 38645256}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tele_frame2.png", "start": 38645256, "end": 38647207}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tele_frame3.png", "start": 38647207, "end": 38651194}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/telepad1_fbr.png", "start": 38651194, "end": 38653170}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlight11_fbr.png", "start": 38653170, "end": 38654796}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlight12_fbr.png", "start": 38654796, "end": 38656491}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlight13_fbr.png", "start": 38656491, "end": 38658550}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlightblfb_fbr.png", "start": 38658550, "end": 38659123}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlightfb_fbr.png", "start": 38659123, "end": 38659713}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlightnb.png", "start": 38659713, "end": 38660311}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/tlightrdfb_fbr.png", "start": 38660311, "end": 38660913}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/treadplatemetal.png", "start": 38660913, "end": 38675667}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/twall2_3.png", "start": 38675667, "end": 38679175}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/w17_1.png", "start": 38679175, "end": 38696199}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/w94_1.png", "start": 38696199, "end": 38709147}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_tech/z_exit_fbr.png", "start": 38709147, "end": 38710708}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/afloor1_3.png", "start": 38710708, "end": 38713706}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/asphalt.png", "start": 38713706, "end": 38731166}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/azfloor1_1.png", "start": 38731166, "end": 38734185}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/badlawn.png", "start": 38734185, "end": 38775994}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/cracks1-1.png", "start": 38775994, "end": 38779013}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/darkrock.png", "start": 38779013, "end": 38801582}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/grass1.png", "start": 38801582, "end": 38812153}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/gravel1.png", "start": 38812153, "end": 38825507}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/gravel2.png", "start": 38825507, "end": 38840404}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/grk_leaf1_1.png", "start": 38840404, "end": 38844397}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/grk_leaf1_2.png", "start": 38844397, "end": 38848134}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/marbbrn128.png", "start": 38848134, "end": 38858376}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_drt1_1.png", "start": 38858376, "end": 38861272}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_drt1_2.png", "start": 38861272, "end": 38864126}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_drt2_2.png", "start": 38864126, "end": 38866870}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_rck1_1.png", "start": 38866870, "end": 38880970}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_rck1_2.png", "start": 38880970, "end": 38891648}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_rck1_3.png", "start": 38891648, "end": 38904491}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/may_slat1_1.png", "start": 38904491, "end": 38907339}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_bigdirt.png", "start": 38907339, "end": 39061922}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_bigdirt2.png", "start": 39061922, "end": 39216073}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_bigdirt3.png", "start": 39216073, "end": 39370024}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn1_1.png", "start": 39370024, "end": 39384405}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn1_1a.png", "start": 39384405, "end": 39401379}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn1_2.png", "start": 39401379, "end": 39415818}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn1_2a.png", "start": 39415818, "end": 39432633}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn2_1.png", "start": 39432633, "end": 39446602}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn2_1a.png", "start": 39446602, "end": 39463913}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn2_2.png", "start": 39463913, "end": 39478987}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cobstn2_2a.png", "start": 39478987, "end": 39496091}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_cracks1.png", "start": 39496091, "end": 39509553}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat1.png", "start": 39509553, "end": 39522462}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat12.png", "start": 39522462, "end": 39533850}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat15.png", "start": 39533850, "end": 39546951}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat16.png", "start": 39546951, "end": 39559793}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat2.png", "start": 39559793, "end": 39571070}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat3.png", "start": 39571070, "end": 39584435}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat4.png", "start": 39584435, "end": 39594384}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat5.png", "start": 39594384, "end": 39605469}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat5a.png", "start": 39605469, "end": 39616213}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat6.png", "start": 39616213, "end": 39625198}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_flat7.png", "start": 39625198, "end": 39636656}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_plaster2.png", "start": 39636656, "end": 39646308}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock1.png", "start": 39646308, "end": 39697565}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock10.png", "start": 39697565, "end": 39706860}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock10a.png", "start": 39706860, "end": 39716174}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock10b.png", "start": 39716174, "end": 39729539}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock10c.png", "start": 39729539, "end": 39741874}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock2.png", "start": 39741874, "end": 39783751}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock3.png", "start": 39783751, "end": 39829695}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock3_bump.png", "start": 39829695, "end": 39945789}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock4.png", "start": 39945789, "end": 39957145}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock5.png", "start": 39957145, "end": 39969677}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/med_rock9.png", "start": 39969677, "end": 39980701}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/ret_plaster1.png", "start": 39980701, "end": 39991978}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/rock1_1.png", "start": 39991978, "end": 40042349}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/rock1_1b.png", "start": 40042349, "end": 40053364}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/rock1_2.png", "start": 40053364, "end": 40095407}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/rocks07.png", "start": 40095407, "end": 40106431}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/rocks11d.png", "start": 40106431, "end": 40115745}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/rocks11e.png", "start": 40115745, "end": 40125040}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/sand.png", "start": 40125040, "end": 40151127}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/snow1.png", "start": 40151127, "end": 40152990}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/uwall1_2.png", "start": 40152990, "end": 40180641}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_terra/vines1.png", "start": 40180641, "end": 40185696}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/black.png", "start": 40185696, "end": 40186244}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/clip.png", "start": 40186244, "end": 40186750}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/hint.png", "start": 40186750, "end": 40187616}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/hintskip.png", "start": 40187616, "end": 40188509}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/light_fbr.png", "start": 40188509, "end": 40189425}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/origin.png", "start": 40189425, "end": 40189908}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/skip.png", "start": 40189908, "end": 40190396}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/star_lavaskip.png", "start": 40190396, "end": 40191509}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/star_slimeskip.png", "start": 40191509, "end": 40192582}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/star_waterskip.png", "start": 40192582, "end": 40194322}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_utility/trigger.png", "start": 40194322, "end": 40194819}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/crate4.png", "start": 40194819, "end": 40197961}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/crwdh6.png", "start": 40197961, "end": 40202969}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/crwdl12.png", "start": 40202969, "end": 40205402}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/crwds6.png", "start": 40205402, "end": 40206428}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/may_crate3-small.png", "start": 40206428, "end": 40207419}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/may_crate3.png", "start": 40207419, "end": 40210482}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/may_wood1_1.png", "start": 40210482, "end": 40213322}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/may_wood1_2.png", "start": 40213322, "end": 40216171}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_ret_wood1.png", "start": 40216171, "end": 40223140}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood1.png", "start": 40223140, "end": 40250508}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood2.png", "start": 40250508, "end": 40258021}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood2_plk1.png", "start": 40258021, "end": 40269719}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood2_plk2.png", "start": 40269719, "end": 40282211}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood3.png", "start": 40282211, "end": 40288211}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood3_plk1.png", "start": 40288211, "end": 40301426}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood4.png", "start": 40301426, "end": 40308550}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood5.png", "start": 40308550, "end": 40316348}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood6.png", "start": 40316348, "end": 40323128}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood7.png", "start": 40323128, "end": 40332167}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood8.png", "start": 40332167, "end": 40340131}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood_riv1.png", "start": 40340131, "end": 40346756}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood_riv1b.png", "start": 40346756, "end": 40352835}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood_riv1c.png", "start": 40352835, "end": 40357959}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood_riv2.png", "start": 40357959, "end": 40364369}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood_riv2b.png", "start": 40364369, "end": 40370789}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/med_wood_riv2c.png", "start": 40370789, "end": 40376161}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank1.png", "start": 40376161, "end": 40382575}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank1s.png", "start": 40382575, "end": 40384456}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank2.png", "start": 40384456, "end": 40391036}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank2s.png", "start": 40391036, "end": 40392925}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank3.png", "start": 40392925, "end": 40399785}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank3s.png", "start": 40399785, "end": 40401766}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank4.png", "start": 40401766, "end": 40408212}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank4s.png", "start": 40408212, "end": 40410102}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/plank5.png", "start": 40410102, "end": 40415751}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/sq_wood_1.png", "start": 40415751, "end": 40431792}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/sq_wood_2.png", "start": 40431792, "end": 40434680}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/sq_wood_2a.png", "start": 40434680, "end": 40438049}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/wood_1.png", "start": 40438049, "end": 40442227}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/wood_2.png", "start": 40442227, "end": 40446127}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodbark128.png", "start": 40446127, "end": 40456334}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodbark1m28.png", "start": 40456334, "end": 40465783}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodbark64.png", "start": 40465783, "end": 40468720}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodbarkA128.png", "start": 40468720, "end": 40477418}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodbarkm64.png", "start": 40477418, "end": 40480095}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodend.png", "start": 40480095, "end": 40482598}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodring128.png", "start": 40482598, "end": 40491022}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodring64.png", "start": 40491022, "end": 40493483}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodringm128.png", "start": 40493483, "end": 40503782}, {"filename": "/GameData/Textures/brushes/libreQuake/lq_wood/woodringm64.png", "start": 40503782, "end": 40507041}, {"filename": "/GameData/Textures/brushes/light.png", "start": 40507041, "end": 40507169}, {"filename": "/GameData/Textures/brushes/light_em.png", "start": 40507169, "end": 40507297}, {"filename": "/GameData/Textures/brushes/mask_test_m.png", "start": 40507297, "end": 40511359}, {"filename": "/GameData/Textures/brushes/mirror.png", "start": 40511359, "end": 40511479}, {"filename": "/GameData/Textures/brushes/mirror_orm.png", "start": 40511479, "end": 40511599}, {"filename": "/GameData/Textures/brushes/null_m.png", "start": 40511599, "end": 40513396}, {"filename": "/GameData/Textures/brushes/tormentPack/+0str_bloodfall.png", "start": 40513396, "end": 40515473}, {"filename": "/GameData/Textures/brushes/tormentPack/+1str_bloodfall.png", "start": 40515473, "end": 40517507}, {"filename": "/GameData/Textures/brushes/tormentPack/+2str_bloodfall.png", "start": 40517507, "end": 40519605}, {"filename": "/GameData/Textures/brushes/tormentPack/+3str_bloodfall.png", "start": 40519605, "end": 40521676}, {"filename": "/GameData/Textures/brushes/tormentPack/+4str_bloodfall.png", "start": 40521676, "end": 40523736}, {"filename": "/GameData/Textures/brushes/tormentPack/+5str_bloodfall.png", "start": 40523736, "end": 40525757}, {"filename": "/GameData/Textures/brushes/tormentPack/+6str_bloodfall.png", "start": 40525757, "end": 40527803}, {"filename": "/GameData/Textures/brushes/tormentPack/+7str_bloodfall.png", "start": 40527803, "end": 40529872}, {"filename": "/GameData/Textures/brushes/tormentPack/str_blood.png", "start": 40529872, "end": 40531880}, {"filename": "/GameData/Textures/brushes/tormentPack/str_blood_large.png", "start": 40531880, "end": 40554430}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein1.png", "start": 40554430, "end": 40581105}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein2.png", "start": 40581105, "end": 40613138}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein3.png", "start": 40613138, "end": 40646809}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein4.png", "start": 40646809, "end": 40678451}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein5.png", "start": 40678451, "end": 40711942}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein6.png", "start": 40711942, "end": 40746416}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein7.png", "start": 40746416, "end": 40781307}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein8.png", "start": 40781307, "end": 40809456}, {"filename": "/GameData/Textures/brushes/tormentPack/str_bloodvein9.png", "start": 40809456, "end": 40844909}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr1.png", "start": 40844909, "end": 40881603}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr2.png", "start": 40881603, "end": 40918178}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr3.png", "start": 40918178, "end": 40949171}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr4.png", "start": 40949171, "end": 40979341}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr5.png", "start": 40979341, "end": 41018791}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr6.png", "start": 41018791, "end": 41057951}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr7.png", "start": 41057951, "end": 41099643}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalflr8.png", "start": 41099643, "end": 41140061}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgen1.png", "start": 41140061, "end": 41169331}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgen2.png", "start": 41169331, "end": 41198847}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgen3.png", "start": 41198847, "end": 41228323}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgen4.png", "start": 41228323, "end": 41253146}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgen5.png", "start": 41253146, "end": 41278016}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgen6.png", "start": 41278016, "end": 41302106}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl1.png", "start": 41302106, "end": 41331491}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl2.png", "start": 41331491, "end": 41364221}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl3.png", "start": 41364221, "end": 41390839}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl4.png", "start": 41390839, "end": 41420103}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl5.png", "start": 41420103, "end": 41454478}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl6.png", "start": 41454478, "end": 41492973}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl7.png", "start": 41492973, "end": 41529123}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalgrbl8.png", "start": 41529123, "end": 41567116}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan1.png", "start": 41567116, "end": 41601976}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan2.png", "start": 41601976, "end": 41637517}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan3.png", "start": 41637517, "end": 41667953}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan4.png", "start": 41667953, "end": 41698913}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan5.png", "start": 41698913, "end": 41735285}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan6.png", "start": 41735285, "end": 41773327}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan7.png", "start": 41773327, "end": 41812556}, {"filename": "/GameData/Textures/brushes/tormentPack/str_metalpan8.png", "start": 41812556, "end": 41851650}, {"filename": "/GameData/Textures/brushes/tormentPack/str_rotwoodgen1.png", "start": 41851650, "end": 41870229}, {"filename": "/GameData/Textures/brushes/tormentPack/str_rotwoodgen2.png", "start": 41870229, "end": 41889525}, {"filename": "/GameData/Textures/brushes/tormentPack/str_rotwoodgen3.png", "start": 41889525, "end": 41909579}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk1.png", "start": 41909579, "end": 41934971}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk2.png", "start": 41934971, "end": 41961973}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk3.png", "start": 41961973, "end": 41984603}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk4.png", "start": 41984603, "end": 42008656}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk5.png", "start": 42008656, "end": 42039740}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk6.png", "start": 42039740, "end": 42072412}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk7.png", "start": 42072412, "end": 42100936}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonebrk8.png", "start": 42100936, "end": 42131188}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stoneflr1.png", "start": 42131188, "end": 42155344}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stoneflr2.png", "start": 42155344, "end": 42183250}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stoneflr3.png", "start": 42183250, "end": 42204255}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stoneflr4.png", "start": 42204255, "end": 42225399}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stoneflr5.png", "start": 42225399, "end": 42256931}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stoneflr6.png", "start": 42256931, "end": 42291214}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonegen1.png", "start": 42291214, "end": 42310237}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonegen2.png", "start": 42310237, "end": 42329608}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonegen3.png", "start": 42329608, "end": 42351380}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonegen4.png", "start": 42351380, "end": 42376425}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonegen5.png", "start": 42376425, "end": 42401813}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonegen6.png", "start": 42401813, "end": 42429250}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonerubble.png", "start": 42429250, "end": 42457644}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall1.png", "start": 42457644, "end": 42480901}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall2.png", "start": 42480901, "end": 42505706}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall3.png", "start": 42505706, "end": 42530369}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall4.png", "start": 42530369, "end": 42556839}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall5.png", "start": 42556839, "end": 42584306}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall6.png", "start": 42584306, "end": 42612474}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall7.png", "start": 42612474, "end": 42641863}, {"filename": "/GameData/Textures/brushes/tormentPack/str_stonewall8.png", "start": 42641863, "end": 42672759}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_bloodgunk.png", "start": 42672759, "end": 42699208}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_bloodweb1.png", "start": 42699208, "end": 42716448}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_bloodweb2.png", "start": 42716448, "end": 42725399}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_bloodweb3.png", "start": 42725399, "end": 42731410}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_grating1.png", "start": 42731410, "end": 42739991}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_grating2.png", "start": 42739991, "end": 42755194}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_grating3.png", "start": 42755194, "end": 42774542}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_grating4.png", "start": 42774542, "end": 42786699}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_grating5.png", "start": 42786699, "end": 42808204}, {"filename": "/GameData/Textures/brushes/tormentPack/{str_grating6.png", "start": 42808204, "end": 42836450}, {"filename": "/GameData/Textures/brushes/trigger.png", "start": 42836450, "end": 42848326}, {"filename": "/GameData/Textures/brushes/trigger_t.png", "start": 42848326, "end": 42860202}, {"filename": "/GameData/Textures/brushes/white.png", "start": 42860202, "end": 42860322}, {"filename": "/GameData/Textures/gloves.png", "start": 42860322, "end": 42969997}, {"filename": "/GameData/Textures/jacket.png", "start": 42969997, "end": 43133809}, {"filename": "/GameData/Textures/muzzle_t.png", "start": 43133809, "end": 43143217}, {"filename": "/GameData/Textures/shirt.png", "start": 43143217, "end": 43334968}, {"filename": "/GameData/arms.glb", "start": 43334968, "end": 43514732}, {"filename": "/GameData/bass_beat.ogg", "start": 43514732, "end": 43881608, "audio": 1}, {"filename": "/GameData/bass_beat.wav", "start": 43881608, "end": 49577942, "audio": 1}, {"filename": "/GameData/cat.png", "start": 49577942, "end": 49827380}, {"filename": "/GameData/cube.mtl", "start": 49827380, "end": 49827431}, {"filename": "/GameData/cube.obj", "start": 49827431, "end": 49828350}, {"filename": "/GameData/dog.bin", "start": 49828350, "end": 49894394}, {"filename": "/GameData/dog.dae", "start": 49894394, "end": 50293466}, {"filename": "/GameData/dog.fbx", "start": 50293466, "end": 51207798}, {"filename": "/GameData/dog.glb", "start": 51207798, "end": 51334298}, {"filename": "/GameData/dog.gltf", "start": 51334298, "end": 51425425}, {"filename": "/GameData/happy_hog.png", "start": 51425425, "end": 52342550}, {"filename": "/GameData/hog_sheet.png", "start": 52342550, "end": 54197626}, {"filename": "/GameData/level_1_bg_crop.png", "start": 54197626, "end": 54804805}, {"filename": "/GameData/mini_hog.ico", "start": 54804805, "end": 54809091}, {"filename": "/GameData/monkey.fbx", "start": 54809091, "end": 54840511}, {"filename": "/GameData/sound_test.ogg", "start": 54840511, "end": 54869619, "audio": 1}, {"filename": "/GameData/sprite_sheet_sticher.py", "start": 54869619, "end": 54870145}, {"filename": "/GameData/testViewmodel.glb", "start": 54870145, "end": 55685117}, {"filename": "/GameData/title_bg_crop.png", "start": 55685117, "end": 56600225}, {"filename": "/GameData/yummy.ogg", "start": 56600225, "end": 56635816, "audio": 1}], "remote_package_size": 56635816});

  })();

// end include: C:\Users\Admin\AppData\Local\Temp\tmp6k4tdppf.js
// include: C:\Users\Admin\AppData\Local\Temp\tmpyikqprxh.js

    // All the pre-js content up to here must remain later on, we need to run
    // it.
    if (Module['$ww'] || (typeof ENVIRONMENT_IS_PTHREAD != 'undefined' && ENVIRONMENT_IS_PTHREAD)) Module['preRun'] = [];
    var necessaryPreJSTasks = Module['preRun'].slice();
  // end include: C:\Users\Admin\AppData\Local\Temp\tmpyikqprxh.js
// include: C:\Users\Admin\AppData\Local\Temp\tmpgsdbawbu.js

    if (!Module['preRun']) throw 'Module.preRun should exist because file support used it; did a pre-js delete it?';
    necessaryPreJSTasks.forEach((task) => {
      if (Module['preRun'].indexOf(task) < 0) throw 'All preRun tasks that exist before user pre-js code should remain after; did you replace Module or modify Module.preRun?';
    });
  // end include: C:\Users\Admin\AppData\Local\Temp\tmpgsdbawbu.js


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

  
  
  var stringToNewUTF8 = (str) => {
      var size = lengthBytesUTF8(str) + 1;
      var ret = _malloc(size);
      if (ret) stringToUTF8(str, ret, size);
      return ret;
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
  1491022: ($0) => { var str = UTF8ToString($0) + '\n\n' + 'Abort/Retry/Ignore/AlwaysIgnore? [ariA] :'; var reply = window.prompt(str, "i"); if (reply === null) { reply = "i"; } return allocate(intArrayFromString(reply), 'i8', ALLOC_NORMAL); },  
 1491247: () => { if (typeof(AudioContext) !== 'undefined') { return true; } else if (typeof(webkitAudioContext) !== 'undefined') { return true; } return false; },  
 1491394: () => { if ((typeof(navigator.mediaDevices) !== 'undefined') && (typeof(navigator.mediaDevices.getUserMedia) !== 'undefined')) { return true; } else if (typeof(navigator.webkitGetUserMedia) !== 'undefined') { return true; } return false; },  
 1491628: ($0) => { if(typeof(Module['SDL2']) === 'undefined') { Module['SDL2'] = {}; } var SDL2 = Module['SDL2']; if (!$0) { SDL2.audio = {}; } else { SDL2.capture = {}; } if (!SDL2.audioContext) { if (typeof(AudioContext) !== 'undefined') { SDL2.audioContext = new AudioContext(); } else if (typeof(webkitAudioContext) !== 'undefined') { SDL2.audioContext = new webkitAudioContext(); } if (SDL2.audioContext) { if ((typeof navigator.userActivation) === 'undefined') { autoResumeAudioContext(SDL2.audioContext); } } } return SDL2.audioContext === undefined ? -1 : 0; },  
 1492180: () => { var SDL2 = Module['SDL2']; return SDL2.audioContext.sampleRate; },  
 1492248: ($0, $1, $2, $3) => { var SDL2 = Module['SDL2']; var have_microphone = function(stream) { if (SDL2.capture.silenceTimer !== undefined) { clearInterval(SDL2.capture.silenceTimer); SDL2.capture.silenceTimer = undefined; SDL2.capture.silenceBuffer = undefined } SDL2.capture.mediaStreamNode = SDL2.audioContext.createMediaStreamSource(stream); SDL2.capture.scriptProcessorNode = SDL2.audioContext.createScriptProcessor($1, $0, 1); SDL2.capture.scriptProcessorNode.onaudioprocess = function(audioProcessingEvent) { if ((SDL2 === undefined) || (SDL2.capture === undefined)) { return; } audioProcessingEvent.outputBuffer.getChannelData(0).fill(0.0); SDL2.capture.currentCaptureBuffer = audioProcessingEvent.inputBuffer; dynCall('vi', $2, [$3]); }; SDL2.capture.mediaStreamNode.connect(SDL2.capture.scriptProcessorNode); SDL2.capture.scriptProcessorNode.connect(SDL2.audioContext.destination); SDL2.capture.stream = stream; }; var no_microphone = function(error) { }; SDL2.capture.silenceBuffer = SDL2.audioContext.createBuffer($0, $1, SDL2.audioContext.sampleRate); SDL2.capture.silenceBuffer.getChannelData(0).fill(0.0); var silence_callback = function() { SDL2.capture.currentCaptureBuffer = SDL2.capture.silenceBuffer; dynCall('vi', $2, [$3]); }; SDL2.capture.silenceTimer = setInterval(silence_callback, ($1 / SDL2.audioContext.sampleRate) * 1000); if ((navigator.mediaDevices !== undefined) && (navigator.mediaDevices.getUserMedia !== undefined)) { navigator.mediaDevices.getUserMedia({ audio: true, video: false }).then(have_microphone).catch(no_microphone); } else if (navigator.webkitGetUserMedia !== undefined) { navigator.webkitGetUserMedia({ audio: true, video: false }, have_microphone, no_microphone); } },  
 1493941: ($0, $1, $2, $3) => { var SDL2 = Module['SDL2']; SDL2.audio.scriptProcessorNode = SDL2.audioContext['createScriptProcessor']($1, 0, $0); SDL2.audio.scriptProcessorNode['onaudioprocess'] = function (e) { if ((SDL2 === undefined) || (SDL2.audio === undefined)) { return; } if (SDL2.audio.silenceTimer !== undefined) { clearInterval(SDL2.audio.silenceTimer); SDL2.audio.silenceTimer = undefined; SDL2.audio.silenceBuffer = undefined; } SDL2.audio.currentOutputBuffer = e['outputBuffer']; dynCall('vi', $2, [$3]); }; SDL2.audio.scriptProcessorNode['connect'](SDL2.audioContext['destination']); if (SDL2.audioContext.state === 'suspended') { SDL2.audio.silenceBuffer = SDL2.audioContext.createBuffer($0, $1, SDL2.audioContext.sampleRate); SDL2.audio.silenceBuffer.getChannelData(0).fill(0.0); var silence_callback = function() { if ((typeof navigator.userActivation) !== 'undefined') { if (navigator.userActivation.hasBeenActive) { SDL2.audioContext.resume(); } } SDL2.audio.currentOutputBuffer = SDL2.audio.silenceBuffer; dynCall('vi', $2, [$3]); SDL2.audio.currentOutputBuffer = undefined; }; SDL2.audio.silenceTimer = setInterval(silence_callback, ($1 / SDL2.audioContext.sampleRate) * 1000); } },  
 1495116: ($0, $1) => { var SDL2 = Module['SDL2']; var numChannels = SDL2.capture.currentCaptureBuffer.numberOfChannels; for (var c = 0; c < numChannels; ++c) { var channelData = SDL2.capture.currentCaptureBuffer.getChannelData(c); if (channelData.length != $1) { throw 'Web Audio capture buffer length mismatch! Destination size: ' + channelData.length + ' samples vs expected ' + $1 + ' samples!'; } if (numChannels == 1) { for (var j = 0; j < $1; ++j) { setValue($0 + (j * 4), channelData[j], 'float'); } } else { for (var j = 0; j < $1; ++j) { setValue($0 + (((j * numChannels) + c) * 4), channelData[j], 'float'); } } } },  
 1495721: ($0, $1) => { var SDL2 = Module['SDL2']; var buf = $0 >>> 2; var numChannels = SDL2.audio.currentOutputBuffer['numberOfChannels']; for (var c = 0; c < numChannels; ++c) { var channelData = SDL2.audio.currentOutputBuffer['getChannelData'](c); if (channelData.length != $1) { throw 'Web Audio output buffer length mismatch! Destination size: ' + channelData.length + ' samples vs expected ' + $1 + ' samples!'; } for (var j = 0; j < $1; ++j) { channelData[j] = HEAPF32[buf + (j*numChannels + c)]; } } },  
 1496210: ($0) => { var SDL2 = Module['SDL2']; if ($0) { if (SDL2.capture.silenceTimer !== undefined) { clearInterval(SDL2.capture.silenceTimer); } if (SDL2.capture.stream !== undefined) { var tracks = SDL2.capture.stream.getAudioTracks(); for (var i = 0; i < tracks.length; i++) { SDL2.capture.stream.removeTrack(tracks[i]); } } if (SDL2.capture.scriptProcessorNode !== undefined) { SDL2.capture.scriptProcessorNode.onaudioprocess = function(audioProcessingEvent) {}; SDL2.capture.scriptProcessorNode.disconnect(); } if (SDL2.capture.mediaStreamNode !== undefined) { SDL2.capture.mediaStreamNode.disconnect(); } SDL2.capture = undefined; } else { if (SDL2.audio.scriptProcessorNode != undefined) { SDL2.audio.scriptProcessorNode.disconnect(); } if (SDL2.audio.silenceTimer !== undefined) { clearInterval(SDL2.audio.silenceTimer); } SDL2.audio = undefined; } if ((SDL2.audioContext !== undefined) && (SDL2.audio === undefined) && (SDL2.capture === undefined)) { SDL2.audioContext.close(); SDL2.audioContext = undefined; } },  
 1497216: ($0, $1, $2) => { var w = $0; var h = $1; var pixels = $2; if (!Module['SDL2']) Module['SDL2'] = {}; var SDL2 = Module['SDL2']; if (SDL2.ctxCanvas !== Module['canvas']) { SDL2.ctx = Module['createContext'](Module['canvas'], false, true); SDL2.ctxCanvas = Module['canvas']; } if (SDL2.w !== w || SDL2.h !== h || SDL2.imageCtx !== SDL2.ctx) { SDL2.image = SDL2.ctx.createImageData(w, h); SDL2.w = w; SDL2.h = h; SDL2.imageCtx = SDL2.ctx; } var data = SDL2.image.data; var src = pixels / 4; var dst = 0; var num; if (typeof CanvasPixelArray !== 'undefined' && data instanceof CanvasPixelArray) { num = data.length; while (dst < num) { var val = HEAP32[src]; data[dst ] = val & 0xff; data[dst+1] = (val >> 8) & 0xff; data[dst+2] = (val >> 16) & 0xff; data[dst+3] = 0xff; src++; dst += 4; } } else { if (SDL2.data32Data !== data) { SDL2.data32 = new Int32Array(data.buffer); SDL2.data8 = new Uint8Array(data.buffer); SDL2.data32Data = data; } var data32 = SDL2.data32; num = data32.length; data32.set(HEAP32.subarray(src, src + num)); var data8 = SDL2.data8; var i = 3; var j = i + 4*num; if (num % 8 == 0) { while (i < j) { data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; } } else { while (i < j) { data8[i] = 0xff; i = i + 4 | 0; } } } SDL2.ctx.putImageData(SDL2.image, 0, 0); },  
 1498684: ($0, $1, $2, $3, $4) => { var w = $0; var h = $1; var hot_x = $2; var hot_y = $3; var pixels = $4; var canvas = document.createElement("canvas"); canvas.width = w; canvas.height = h; var ctx = canvas.getContext("2d"); var image = ctx.createImageData(w, h); var data = image.data; var src = pixels / 4; var dst = 0; var num; if (typeof CanvasPixelArray !== 'undefined' && data instanceof CanvasPixelArray) { num = data.length; while (dst < num) { var val = HEAP32[src]; data[dst ] = val & 0xff; data[dst+1] = (val >> 8) & 0xff; data[dst+2] = (val >> 16) & 0xff; data[dst+3] = (val >> 24) & 0xff; src++; dst += 4; } } else { var data32 = new Int32Array(data.buffer); num = data32.length; data32.set(HEAP32.subarray(src, src + num)); } ctx.putImageData(image, 0, 0); var url = hot_x === 0 && hot_y === 0 ? "url(" + canvas.toDataURL() + "), auto" : "url(" + canvas.toDataURL() + ") " + hot_x + " " + hot_y + ", auto"; var urlBuf = _malloc(url.length + 1); stringToUTF8(url, urlBuf, url.length + 1); return urlBuf; },  
 1499672: ($0) => { if (Module['canvas']) { Module['canvas'].style['cursor'] = UTF8ToString($0); } },  
 1499755: () => { if (Module['canvas']) { Module['canvas'].style['cursor'] = 'none'; } },  
 1499824: () => { return window.innerWidth; },  
 1499854: () => { return window.innerHeight; }
};
function canvas_get_width() { return canvas.width; }
function canvas_get_height() { return canvas.height; }
function release_cursor_js() { if (document.pointerLockElement === Module['canvas']) { document.exitPointerLock(); } }
function lock_cursor_js() { if (Module['canvas']) { Module['canvas'].requestPointerLock(); } }
function ImGui_ImplSDL2_EmscriptenOpenURL(url) { url = url ? UTF8ToString(url) : null; if (url) window.open(url, '_blank'); }
var wasmImports = {
  /** @export */
  ImGui_ImplSDL2_EmscriptenOpenURL,
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
  alGenBuffers: _alGenBuffers,
  /** @export */
  alGenSources: _alGenSources,
  /** @export */
  alSourcei: _alSourcei,
  /** @export */
  alcCreateContext: _alcCreateContext,
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
  glBufferData: _glBufferData,
  /** @export */
  glBufferSubData: _glBufferSubData,
  /** @export */
  glClear: _glClear,
  /** @export */
  glClearColor: _glClearColor,
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
  glGenBuffers: _glGenBuffers,
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
  glUniform1i: _glUniform1i,
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

