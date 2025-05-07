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
// include: C:\Users\Admin\AppData\Local\Temp\tmpjnn2jlwu.js

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
Module['FS_createPath']("/GameData", "Models", true, true);
Module['FS_createPath']("/GameData/Models", "Player", true, true);
Module['FS_createPath']("/GameData/Models/Player", "Bike", true, true);
Module['FS_createPath']("/GameData/Models/Player/Bike", "Textures", true, true);
Module['FS_createPath']("/GameData/Models", "Weapons", true, true);
Module['FS_createPath']("/GameData/Models/Weapons", "Bullet", true, true);
Module['FS_createPath']("/GameData", "Shaders", true, true);
Module['FS_createPath']("/GameData", "Sounds", true, true);
Module['FS_createPath']("/GameData/Sounds", "Dog", true, true);
Module['FS_createPath']("/GameData/Sounds", "Weapons", true, true);
Module['FS_createPath']("/GameData/Sounds/Weapons", "Shotgun", true, true);
Module['FS_createPath']("/GameData", "Textures", true, true);
Module['FS_createPath']("/GameData/Textures", "Ground", true, true);
Module['FS_createPath']("/GameData/Textures", "Metal", true, true);
Module['FS_createPath']("/GameData/Textures", "Particles", true, true);
Module['FS_createPath']("/GameData/Textures", "Wall", true, true);
Module['FS_createPath']("/GameData/Textures", "Water", true, true);
Module['FS_createPath']("/GameData/Textures", "Wood", true, true);
Module['FS_createPath']("/GameData/Textures", "delvenPack", true, true);
Module['FS_createPath']("/GameData/Textures", "generic", true, true);
Module['FS_createPath']("/GameData/Textures", "lq_conc", true, true);
Module['FS_createPath']("/GameData/Textures", "lq_dev", true, true);
Module['FS_createPath']("/GameData/Textures", "lq_flesh", true, true);
Module['FS_createPath']("/GameData/Textures", "lq_greek", true, true);
Module['FS_createPath']("/GameData/Textures", "lq_health_ammo", true, true);
Module['FS_createPath']("/GameData/Textures", "lq_legacy", true, true);
Module['FS_createPath']("/GameData/Textures", "lq_liquidsky", true, true);
Module['FS_createPath']("/GameData/Textures", "lq_mayan", true, true);
Module['FS_createPath']("/GameData/Textures", "lq_medieval", true, true);
Module['FS_createPath']("/GameData/Textures", "lq_metal", true, true);
Module['FS_createPath']("/GameData/Textures", "lq_palette", true, true);
Module['FS_createPath']("/GameData/Textures", "lq_props", true, true);
Module['FS_createPath']("/GameData/Textures", "lq_tech", true, true);
Module['FS_createPath']("/GameData/Textures", "lq_terra", true, true);
Module['FS_createPath']("/GameData/Textures", "lq_utility", true, true);
Module['FS_createPath']("/GameData/Textures", "lq_wood", true, true);
Module['FS_createPath']("/GameData/Textures", "tormentPack", true, true);

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
    loadPackage({"files": [{"filename": "/GameData/Fonts/Kingthings_Calligraphica_2.ttf", "start": 0, "end": 29804}, {"filename": "/GameData/Maps/Level.bsp", "start": 29804, "end": 91880}, {"filename": "/GameData/Maps/autosave/test.1.map", "start": 91880, "end": 107831}, {"filename": "/GameData/Maps/autosave/test.2.map", "start": 107831, "end": 123879}, {"filename": "/GameData/Maps/autosave/test.3.map", "start": 123879, "end": 140037}, {"filename": "/GameData/Maps/autosave/test.4.map", "start": 140037, "end": 156181}, {"filename": "/GameData/Maps/autosave/test.5.map", "start": 156181, "end": 172127}, {"filename": "/GameData/Maps/autosave/test.6.map", "start": 172127, "end": 188073}, {"filename": "/GameData/Maps/autosave/test.7.map", "start": 188073, "end": 204023}, {"filename": "/GameData/Maps/autosave/test2.1.map", "start": 204023, "end": 860591}, {"filename": "/GameData/Maps/e1m1.map", "start": 860591, "end": 10872120}, {"filename": "/GameData/Maps/e1m1.obj", "start": 10872120, "end": 20717141}, {"filename": "/GameData/Maps/oa_dm1.bsp", "start": 20717141, "end": 22122117}, {"filename": "/GameData/Maps/test.autosave.map", "start": 22122117, "end": 22143143}, {"filename": "/GameData/Maps/test.bak", "start": 22143143, "end": 22164099}, {"filename": "/GameData/Maps/test.bsp", "start": 22164099, "end": 23336735}, {"filename": "/GameData/Maps/test.map", "start": 23336735, "end": 23357761}, {"filename": "/GameData/Maps/test.mtl", "start": 23357761, "end": 23358038}, {"filename": "/GameData/Maps/test.obj", "start": 23358038, "end": 23376555}, {"filename": "/GameData/Maps/test.srf", "start": 23376555, "end": 23397825}, {"filename": "/GameData/Maps/test2.map", "start": 23397825, "end": 24055700}, {"filename": "/GameData/Maps/test2.mtl", "start": 24055700, "end": 24056395}, {"filename": "/GameData/Maps/test2.obj", "start": 24056395, "end": 24959692}, {"filename": "/GameData/Models/Player/Bike/Textures/body.png", "start": 24959692, "end": 25196373}, {"filename": "/GameData/Models/Player/Bike/Textures/front.png", "start": 25196373, "end": 25668124}, {"filename": "/GameData/Models/Player/Bike/Textures/wheels.png", "start": 25668124, "end": 26794928}, {"filename": "/GameData/Models/Player/Bike/bike.glb", "start": 26794928, "end": 32246804}, {"filename": "/GameData/Models/Weapons/Bullet/bullet.obj", "start": 32246804, "end": 32248062}, {"filename": "/GameData/Models/npc_base.mtl", "start": 32248062, "end": 32248307}, {"filename": "/GameData/Models/npc_base.obj", "start": 32248307, "end": 32251191}, {"filename": "/GameData/Models/test.obj", "start": 32251191, "end": 32284173}, {"filename": "/GameData/Shaders/bsp.frag", "start": 32284173, "end": 32284845}, {"filename": "/GameData/Shaders/bsp.vert", "start": 32284845, "end": 32285502}, {"filename": "/GameData/Shaders/default_pixel.frag", "start": 32285502, "end": 32296237}, {"filename": "/GameData/Shaders/default_pixel_.frag", "start": 32296237, "end": 32303196}, {"filename": "/GameData/Shaders/default_pixel_simple.frag", "start": 32303196, "end": 32314027}, {"filename": "/GameData/Shaders/default_vertex.vert", "start": 32314027, "end": 32316275}, {"filename": "/GameData/Shaders/empty_pixel.frag", "start": 32316275, "end": 32316342}, {"filename": "/GameData/Shaders/fullscreen_vertex.vert", "start": 32316342, "end": 32316546}, {"filename": "/GameData/Shaders/instanced_bilboard_vertex.vert", "start": 32316546, "end": 32318485}, {"filename": "/GameData/Shaders/mask_pixel.frag", "start": 32318485, "end": 32318917}, {"filename": "/GameData/Shaders/solidRed_pixel.frag", "start": 32318917, "end": 32319086}, {"filename": "/GameData/Shaders/texture_pixel.frag", "start": 32319086, "end": 32319274}, {"filename": "/GameData/Shaders/ui.vert", "start": 32319274, "end": 32319577}, {"filename": "/GameData/Shaders/ui_flatcolor.frag", "start": 32319577, "end": 32319713}, {"filename": "/GameData/Shaders/ui_textured.frag", "start": 32319713, "end": 32319970}, {"filename": "/GameData/Sounds/Dog/Death.wav", "start": 32319970, "end": 32393356, "audio": 1}, {"filename": "/GameData/Sounds/Dog/dog_attack.wav", "start": 32393356, "end": 32505880, "audio": 1}, {"filename": "/GameData/Sounds/Dog/dog_attack_start.wav", "start": 32505880, "end": 32702564, "audio": 1}, {"filename": "/GameData/Sounds/Dog/dog_death.wav", "start": 32702564, "end": 32856764, "audio": 1}, {"filename": "/GameData/Sounds/Dog/dog_hit.wav", "start": 32856764, "end": 32930456, "audio": 1}, {"filename": "/GameData/Sounds/Dog/dog_stun.wav", "start": 32930456, "end": 33023906, "audio": 1}, {"filename": "/GameData/Sounds/Weapons/Shotgun/shotgun_fire.wav", "start": 33023906, "end": 33483906, "audio": 1}, {"filename": "/GameData/Sounds/mew.wav", "start": 33483906, "end": 33557292, "audio": 1}, {"filename": "/GameData/Textures/Ground/grass.png", "start": 33557292, "end": 33561076}, {"filename": "/GameData/Textures/M_Shotgun_Base_Color.png", "start": 33561076, "end": 33936218}, {"filename": "/GameData/Textures/Metal/metal1.png", "start": 33936218, "end": 34360274}, {"filename": "/GameData/Textures/Particles/blood.png", "start": 34360274, "end": 34363987}, {"filename": "/GameData/Textures/Particles/smoke.png", "start": 34363987, "end": 34367857}, {"filename": "/GameData/Textures/Particles/trail.png", "start": 34367857, "end": 34386727}, {"filename": "/GameData/Textures/Particles/wood.png", "start": 34386727, "end": 34402210}, {"filename": "/GameData/Textures/Wall/brickWall1.png", "start": 34402210, "end": 34410426}, {"filename": "/GameData/Textures/Wall/brickWall2.png", "start": 34410426, "end": 34418550}, {"filename": "/GameData/Textures/Wall/brickWall3.png", "start": 34418550, "end": 34426433}, {"filename": "/GameData/Textures/Water/Water1_t.png", "start": 34426433, "end": 34739534}, {"filename": "/GameData/Textures/Wood/wood1.png", "start": 34739534, "end": 35087886}, {"filename": "/GameData/Textures/arms.png", "start": 35087886, "end": 35128130}, {"filename": "/GameData/Textures/delvenPack/dlv_door1a.png", "start": 35128130, "end": 35135914}, {"filename": "/GameData/Textures/delvenPack/dlv_door1b.png", "start": 35135914, "end": 35145104}, {"filename": "/GameData/Textures/delvenPack/dlv_door1c.png", "start": 35145104, "end": 35152955}, {"filename": "/GameData/Textures/delvenPack/dlv_door1d.png", "start": 35152955, "end": 35162127}, {"filename": "/GameData/Textures/delvenPack/dlv_door2a.png", "start": 35162127, "end": 35169851}, {"filename": "/GameData/Textures/delvenPack/dlv_door2b.png", "start": 35169851, "end": 35178976}, {"filename": "/GameData/Textures/delvenPack/dlv_door2c.png", "start": 35178976, "end": 35187004}, {"filename": "/GameData/Textures/delvenPack/dlv_door2d.png", "start": 35187004, "end": 35196232}, {"filename": "/GameData/Textures/delvenPack/dlv_door3a.png", "start": 35196232, "end": 35200380}, {"filename": "/GameData/Textures/delvenPack/dlv_door3b.png", "start": 35200380, "end": 35205158}, {"filename": "/GameData/Textures/delvenPack/dlv_door3c.png", "start": 35205158, "end": 35209308}, {"filename": "/GameData/Textures/delvenPack/dlv_door3d.png", "start": 35209308, "end": 35214100}, {"filename": "/GameData/Textures/delvenPack/dlv_door4a.png", "start": 35214100, "end": 35218221}, {"filename": "/GameData/Textures/delvenPack/dlv_door4b.png", "start": 35218221, "end": 35223012}, {"filename": "/GameData/Textures/delvenPack/dlv_door4c.png", "start": 35223012, "end": 35227282}, {"filename": "/GameData/Textures/delvenPack/dlv_door4d.png", "start": 35227282, "end": 35232118}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric1a.png", "start": 35232118, "end": 35237494}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric1b.png", "start": 35237494, "end": 35243622}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric1c.png", "start": 35243622, "end": 35249916}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric1d.png", "start": 35249916, "end": 35253552}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric1e.png", "start": 35253552, "end": 35257238}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric2a.png", "start": 35257238, "end": 35261963}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric2b.png", "start": 35261963, "end": 35267414}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric2c.png", "start": 35267414, "end": 35273092}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric2d.png", "start": 35273092, "end": 35276378}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric2e.png", "start": 35276378, "end": 35279739}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric3a.png", "start": 35279739, "end": 35284693}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric3b.png", "start": 35284693, "end": 35290364}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric3c.png", "start": 35290364, "end": 35296189}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric3d.png", "start": 35296189, "end": 35299569}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric3e.png", "start": 35299569, "end": 35303011}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric4a.png", "start": 35303011, "end": 35312429}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric4b.png", "start": 35312429, "end": 35322394}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric4c.png", "start": 35322394, "end": 35332523}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric4d.png", "start": 35332523, "end": 35338047}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric4e.png", "start": 35338047, "end": 35343617}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric5a.png", "start": 35343617, "end": 35352684}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric5b.png", "start": 35352684, "end": 35362331}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric5c.png", "start": 35362331, "end": 35372131}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric5d.png", "start": 35372131, "end": 35377541}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric5e.png", "start": 35377541, "end": 35382975}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric6a.png", "start": 35382975, "end": 35390109}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric6b.png", "start": 35390109, "end": 35397837}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric6c.png", "start": 35397837, "end": 35405732}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric6d.png", "start": 35405732, "end": 35410132}, {"filename": "/GameData/Textures/delvenPack/dlv_fabric6e.png", "start": 35410132, "end": 35414590}, {"filename": "/GameData/Textures/delvenPack/dlv_ground1a.png", "start": 35414590, "end": 35422512}, {"filename": "/GameData/Textures/delvenPack/dlv_ground1b.png", "start": 35422512, "end": 35431348}, {"filename": "/GameData/Textures/delvenPack/dlv_ground1c.png", "start": 35431348, "end": 35440844}, {"filename": "/GameData/Textures/delvenPack/dlv_ground1d.png", "start": 35440844, "end": 35450887}, {"filename": "/GameData/Textures/delvenPack/dlv_ground2a.png", "start": 35450887, "end": 35460755}, {"filename": "/GameData/Textures/delvenPack/dlv_ground2b.png", "start": 35460755, "end": 35469662}, {"filename": "/GameData/Textures/delvenPack/dlv_ground2c.png", "start": 35469662, "end": 35479387}, {"filename": "/GameData/Textures/delvenPack/dlv_ground2d.png", "start": 35479387, "end": 35489808}, {"filename": "/GameData/Textures/delvenPack/dlv_ground3a.png", "start": 35489808, "end": 35499867}, {"filename": "/GameData/Textures/delvenPack/dlv_ground3b.png", "start": 35499867, "end": 35509579}, {"filename": "/GameData/Textures/delvenPack/dlv_ground3c.png", "start": 35509579, "end": 35519015}, {"filename": "/GameData/Textures/delvenPack/dlv_ground4a.png", "start": 35519015, "end": 35529206}, {"filename": "/GameData/Textures/delvenPack/dlv_ground4b.png", "start": 35529206, "end": 35539656}, {"filename": "/GameData/Textures/delvenPack/dlv_ground4c.png", "start": 35539656, "end": 35549597}, {"filename": "/GameData/Textures/delvenPack/dlv_litmetal1a.png", "start": 35549597, "end": 35553625}, {"filename": "/GameData/Textures/delvenPack/dlv_litmetal1b.png", "start": 35553625, "end": 35558337}, {"filename": "/GameData/Textures/delvenPack/dlv_litmetal1c.png", "start": 35558337, "end": 35562418}, {"filename": "/GameData/Textures/delvenPack/dlv_litmetal2a.png", "start": 35562418, "end": 35566471}, {"filename": "/GameData/Textures/delvenPack/dlv_litmetal2b.png", "start": 35566471, "end": 35571230}, {"filename": "/GameData/Textures/delvenPack/dlv_litmetal2c.png", "start": 35571230, "end": 35575101}, {"filename": "/GameData/Textures/delvenPack/dlv_litstone1a.png", "start": 35575101, "end": 35578479}, {"filename": "/GameData/Textures/delvenPack/dlv_litstone1b.png", "start": 35578479, "end": 35582692}, {"filename": "/GameData/Textures/delvenPack/dlv_litstone1c.png", "start": 35582692, "end": 35586323}, {"filename": "/GameData/Textures/delvenPack/dlv_litstone2a.png", "start": 35586323, "end": 35591120}, {"filename": "/GameData/Textures/delvenPack/dlv_litstone2b.png", "start": 35591120, "end": 35596646}, {"filename": "/GameData/Textures/delvenPack/dlv_litstone2c.png", "start": 35596646, "end": 35600889}, {"filename": "/GameData/Textures/delvenPack/dlv_metalgen1.png", "start": 35600889, "end": 35606714}, {"filename": "/GameData/Textures/delvenPack/dlv_metalgen2.png", "start": 35606714, "end": 35613594}, {"filename": "/GameData/Textures/delvenPack/dlv_metalpan1a.png", "start": 35613594, "end": 35621652}, {"filename": "/GameData/Textures/delvenPack/dlv_metalpan1b.png", "start": 35621652, "end": 35629122}, {"filename": "/GameData/Textures/delvenPack/dlv_metalpan1c.png", "start": 35629122, "end": 35638324}, {"filename": "/GameData/Textures/delvenPack/dlv_metalpan2a.png", "start": 35638324, "end": 35646651}, {"filename": "/GameData/Textures/delvenPack/dlv_metalpan2b.png", "start": 35646651, "end": 35654063}, {"filename": "/GameData/Textures/delvenPack/dlv_metalpan2c.png", "start": 35654063, "end": 35663262}, {"filename": "/GameData/Textures/delvenPack/dlv_metalpan3a.png", "start": 35663262, "end": 35671869}, {"filename": "/GameData/Textures/delvenPack/dlv_metalpan3b.png", "start": 35671869, "end": 35680096}, {"filename": "/GameData/Textures/delvenPack/dlv_metalpan3c.png", "start": 35680096, "end": 35689398}, {"filename": "/GameData/Textures/delvenPack/dlv_metalpan4a.png", "start": 35689398, "end": 35698159}, {"filename": "/GameData/Textures/delvenPack/dlv_metalpan4b.png", "start": 35698159, "end": 35706484}, {"filename": "/GameData/Textures/delvenPack/dlv_metalpan4c.png", "start": 35706484, "end": 35715939}, {"filename": "/GameData/Textures/delvenPack/dlv_metalpip1a.png", "start": 35715939, "end": 35722583}, {"filename": "/GameData/Textures/delvenPack/dlv_metalpip1b.png", "start": 35722583, "end": 35729095}, {"filename": "/GameData/Textures/delvenPack/dlv_metalpip2a.png", "start": 35729095, "end": 35736228}, {"filename": "/GameData/Textures/delvenPack/dlv_metalpip2b.png", "start": 35736228, "end": 35743246}, {"filename": "/GameData/Textures/delvenPack/dlv_metalpip3a.png", "start": 35743246, "end": 35750517}, {"filename": "/GameData/Textures/delvenPack/dlv_metalpip3b.png", "start": 35750517, "end": 35757595}, {"filename": "/GameData/Textures/delvenPack/dlv_metalpip4a.png", "start": 35757595, "end": 35765163}, {"filename": "/GameData/Textures/delvenPack/dlv_metalpip4b.png", "start": 35765163, "end": 35772518}, {"filename": "/GameData/Textures/delvenPack/dlv_metaltrm1a.png", "start": 35772518, "end": 35779516}, {"filename": "/GameData/Textures/delvenPack/dlv_metaltrm1b.png", "start": 35779516, "end": 35786049}, {"filename": "/GameData/Textures/delvenPack/dlv_metaltrm1c.png", "start": 35786049, "end": 35794237}, {"filename": "/GameData/Textures/delvenPack/dlv_metaltrm2a.png", "start": 35794237, "end": 35801734}, {"filename": "/GameData/Textures/delvenPack/dlv_metaltrm2b.png", "start": 35801734, "end": 35808322}, {"filename": "/GameData/Textures/delvenPack/dlv_metaltrm2c.png", "start": 35808322, "end": 35816583}, {"filename": "/GameData/Textures/delvenPack/dlv_metaltrm3a.png", "start": 35816583, "end": 35823859}, {"filename": "/GameData/Textures/delvenPack/dlv_metaltrm3b.png", "start": 35823859, "end": 35830785}, {"filename": "/GameData/Textures/delvenPack/dlv_metaltrm3c.png", "start": 35830785, "end": 35838780}, {"filename": "/GameData/Textures/delvenPack/dlv_metaltrm4a.png", "start": 35838780, "end": 35845660}, {"filename": "/GameData/Textures/delvenPack/dlv_metaltrm4b.png", "start": 35845660, "end": 35852368}, {"filename": "/GameData/Textures/delvenPack/dlv_metaltrm4c.png", "start": 35852368, "end": 35860177}, {"filename": "/GameData/Textures/delvenPack/dlv_metaltrm5.png", "start": 35860177, "end": 35863836}, {"filename": "/GameData/Textures/delvenPack/dlv_metaltrm6.png", "start": 35863836, "end": 35867559}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr1a.png", "start": 35867559, "end": 35874351}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr1b.png", "start": 35874351, "end": 35882722}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr1c.png", "start": 35882722, "end": 35890392}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr1d.png", "start": 35890392, "end": 35898509}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr1e.png", "start": 35898509, "end": 35906401}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr2a.png", "start": 35906401, "end": 35914592}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr2b.png", "start": 35914592, "end": 35924104}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr2c.png", "start": 35924104, "end": 35933006}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr2d.png", "start": 35933006, "end": 35942257}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr2e.png", "start": 35942257, "end": 35951328}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr3a.png", "start": 35951328, "end": 35959459}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr3b.png", "start": 35959459, "end": 35969169}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr3c.png", "start": 35969169, "end": 35978141}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr3d.png", "start": 35978141, "end": 35987615}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr3e.png", "start": 35987615, "end": 35996842}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr4a.png", "start": 35996842, "end": 36005348}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr4b.png", "start": 36005348, "end": 36013808}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr4c.png", "start": 36013808, "end": 36022472}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr5a.png", "start": 36022472, "end": 36029651}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr5b.png", "start": 36029651, "end": 36036779}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr5c.png", "start": 36036779, "end": 36044919}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr6a.png", "start": 36044919, "end": 36053595}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr6b.png", "start": 36053595, "end": 36062010}, {"filename": "/GameData/Textures/delvenPack/dlv_slateflr6c.png", "start": 36062010, "end": 36070743}, {"filename": "/GameData/Textures/delvenPack/dlv_slategen1.png", "start": 36070743, "end": 36076704}, {"filename": "/GameData/Textures/delvenPack/dlv_slategen2.png", "start": 36076704, "end": 36084263}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk1a.png", "start": 36084263, "end": 36090213}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk1b.png", "start": 36090213, "end": 36096029}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk1c.png", "start": 36096029, "end": 36101973}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk1d.png", "start": 36101973, "end": 36107619}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk2a.png", "start": 36107619, "end": 36116491}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk2b.png", "start": 36116491, "end": 36125165}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk2c.png", "start": 36125165, "end": 36133962}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk2d.png", "start": 36133962, "end": 36142258}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk3a.png", "start": 36142258, "end": 36150705}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk3b.png", "start": 36150705, "end": 36159352}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk4a.png", "start": 36159352, "end": 36165971}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk4b.png", "start": 36165971, "end": 36172657}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk4c.png", "start": 36172657, "end": 36179274}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk4d.png", "start": 36179274, "end": 36185888}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk4e.png", "start": 36185888, "end": 36192472}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk4f.png", "start": 36192472, "end": 36198829}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk5a.png", "start": 36198829, "end": 36208306}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk5b.png", "start": 36208306, "end": 36217818}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk5c.png", "start": 36217818, "end": 36227316}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk5d.png", "start": 36227316, "end": 36236789}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk5e.png", "start": 36236789, "end": 36246180}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk5f.png", "start": 36246180, "end": 36255317}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk6a.png", "start": 36255317, "end": 36264242}, {"filename": "/GameData/Textures/delvenPack/dlv_stonebrk6b.png", "start": 36264242, "end": 36273467}, {"filename": "/GameData/Textures/delvenPack/dlv_stoneflr1a.png", "start": 36273467, "end": 36278897}, {"filename": "/GameData/Textures/delvenPack/dlv_stoneflr1b.png", "start": 36278897, "end": 36285319}, {"filename": "/GameData/Textures/delvenPack/dlv_stoneflr1c.png", "start": 36285319, "end": 36292342}, {"filename": "/GameData/Textures/delvenPack/dlv_stoneflr2a.png", "start": 36292342, "end": 36300230}, {"filename": "/GameData/Textures/delvenPack/dlv_stoneflr2b.png", "start": 36300230, "end": 36309045}, {"filename": "/GameData/Textures/delvenPack/dlv_stoneflr2c.png", "start": 36309045, "end": 36318326}, {"filename": "/GameData/Textures/delvenPack/dlv_stoneflr3a.png", "start": 36318326, "end": 36324029}, {"filename": "/GameData/Textures/delvenPack/dlv_stoneflr3b.png", "start": 36324029, "end": 36330507}, {"filename": "/GameData/Textures/delvenPack/dlv_stoneflr3c.png", "start": 36330507, "end": 36337499}, {"filename": "/GameData/Textures/delvenPack/dlv_stoneflr4a.png", "start": 36337499, "end": 36345891}, {"filename": "/GameData/Textures/delvenPack/dlv_stoneflr4b.png", "start": 36345891, "end": 36354960}, {"filename": "/GameData/Textures/delvenPack/dlv_stoneflr4c.png", "start": 36354960, "end": 36364060}, {"filename": "/GameData/Textures/delvenPack/dlv_stoneflr5a.png", "start": 36364060, "end": 36370150}, {"filename": "/GameData/Textures/delvenPack/dlv_stoneflr5b.png", "start": 36370150, "end": 36376563}, {"filename": "/GameData/Textures/delvenPack/dlv_stoneflr5c.png", "start": 36376563, "end": 36383481}, {"filename": "/GameData/Textures/delvenPack/dlv_stoneflr6a.png", "start": 36383481, "end": 36392026}, {"filename": "/GameData/Textures/delvenPack/dlv_stoneflr6b.png", "start": 36392026, "end": 36400846}, {"filename": "/GameData/Textures/delvenPack/dlv_stoneflr6c.png", "start": 36400846, "end": 36410130}, {"filename": "/GameData/Textures/delvenPack/dlv_stonegen1.png", "start": 36410130, "end": 36415224}, {"filename": "/GameData/Textures/delvenPack/dlv_stonegen2.png", "start": 36415224, "end": 36422654}, {"filename": "/GameData/Textures/delvenPack/dlv_stonestep1a.png", "start": 36422654, "end": 36428097}, {"filename": "/GameData/Textures/delvenPack/dlv_stonestep1b.png", "start": 36428097, "end": 36433637}, {"filename": "/GameData/Textures/delvenPack/dlv_stonestep2a.png", "start": 36433637, "end": 36441486}, {"filename": "/GameData/Textures/delvenPack/dlv_stonestep2b.png", "start": 36441486, "end": 36449236}, {"filename": "/GameData/Textures/delvenPack/dlv_stonetrm1a.png", "start": 36449236, "end": 36454526}, {"filename": "/GameData/Textures/delvenPack/dlv_stonetrm1b.png", "start": 36454526, "end": 36460626}, {"filename": "/GameData/Textures/delvenPack/dlv_stonetrm1c.png", "start": 36460626, "end": 36466202}, {"filename": "/GameData/Textures/delvenPack/dlv_stonetrm2a.png", "start": 36466202, "end": 36473948}, {"filename": "/GameData/Textures/delvenPack/dlv_stonetrm2b.png", "start": 36473948, "end": 36482778}, {"filename": "/GameData/Textures/delvenPack/dlv_stonetrm2c.png", "start": 36482778, "end": 36491086}, {"filename": "/GameData/Textures/delvenPack/dlv_stonetrm3a.png", "start": 36491086, "end": 36496386}, {"filename": "/GameData/Textures/delvenPack/dlv_stonetrm3b.png", "start": 36496386, "end": 36502459}, {"filename": "/GameData/Textures/delvenPack/dlv_stonetrm4a.png", "start": 36502459, "end": 36510281}, {"filename": "/GameData/Textures/delvenPack/dlv_stonetrm4b.png", "start": 36510281, "end": 36518812}, {"filename": "/GameData/Textures/delvenPack/dlv_stonetrm5.png", "start": 36518812, "end": 36521969}, {"filename": "/GameData/Textures/delvenPack/dlv_stonetrm6.png", "start": 36521969, "end": 36526324}, {"filename": "/GameData/Textures/delvenPack/dlv_stonewaf1a.png", "start": 36526324, "end": 36531935}, {"filename": "/GameData/Textures/delvenPack/dlv_stonewaf1b.png", "start": 36531935, "end": 36537552}, {"filename": "/GameData/Textures/delvenPack/dlv_stonewaf2a.png", "start": 36537552, "end": 36545425}, {"filename": "/GameData/Textures/delvenPack/dlv_stonewaf2b.png", "start": 36545425, "end": 36553534}, {"filename": "/GameData/Textures/delvenPack/dlv_wood1a.png", "start": 36553534, "end": 36558233}, {"filename": "/GameData/Textures/delvenPack/dlv_wood1b.png", "start": 36558233, "end": 36563192}, {"filename": "/GameData/Textures/delvenPack/dlv_wood2a.png", "start": 36563192, "end": 36569888}, {"filename": "/GameData/Textures/delvenPack/dlv_wood2b.png", "start": 36569888, "end": 36576845}, {"filename": "/GameData/Textures/delvenPack/dlv_wood3a.png", "start": 36576845, "end": 36582049}, {"filename": "/GameData/Textures/delvenPack/dlv_wood3b.png", "start": 36582049, "end": 36587507}, {"filename": "/GameData/Textures/delvenPack/dlv_wood4a.png", "start": 36587507, "end": 36594339}, {"filename": "/GameData/Textures/delvenPack/dlv_wood4b.png", "start": 36594339, "end": 36601384}, {"filename": "/GameData/Textures/delvenPack/dlv_wood5a.png", "start": 36601384, "end": 36607397}, {"filename": "/GameData/Textures/delvenPack/dlv_wood5b.png", "start": 36607397, "end": 36614232}, {"filename": "/GameData/Textures/delvenPack/dlv_wood5c.png", "start": 36614232, "end": 36621242}, {"filename": "/GameData/Textures/delvenPack/dlv_wood5d.png", "start": 36621242, "end": 36628832}, {"filename": "/GameData/Textures/delvenPack/dlv_wood5e.png", "start": 36628832, "end": 36637965}, {"filename": "/GameData/Textures/delvenPack/dlv_wood5f.png", "start": 36637965, "end": 36646994}, {"filename": "/GameData/Textures/delvenPack/dlv_wood5g.png", "start": 36646994, "end": 36655728}, {"filename": "/GameData/Textures/delvenPack/dlv_wood5h.png", "start": 36655728, "end": 36664456}, {"filename": "/GameData/Textures/delvenPack/dlv_wood6a.png", "start": 36664456, "end": 36672086}, {"filename": "/GameData/Textures/delvenPack/dlv_wood6b.png", "start": 36672086, "end": 36680534}, {"filename": "/GameData/Textures/delvenPack/dlv_wood6c.png", "start": 36680534, "end": 36688102}, {"filename": "/GameData/Textures/delvenPack/dlv_wood6d.png", "start": 36688102, "end": 36696499}, {"filename": "/GameData/Textures/delvenPack/dlv_wood6e.png", "start": 36696499, "end": 36706256}, {"filename": "/GameData/Textures/delvenPack/dlv_wood6f.png", "start": 36706256, "end": 36716202}, {"filename": "/GameData/Textures/delvenPack/dlv_wood6g.png", "start": 36716202, "end": 36725405}, {"filename": "/GameData/Textures/delvenPack/dlv_wood6h.png", "start": 36725405, "end": 36734844}, {"filename": "/GameData/Textures/delvenPack/dlv_woodgen1.png", "start": 36734844, "end": 36739391}, {"filename": "/GameData/Textures/delvenPack/dlv_woodgen2.png", "start": 36739391, "end": 36746084}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry1a.png", "start": 36746084, "end": 36753871}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry1b.png", "start": 36753871, "end": 36761018}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry1c.png", "start": 36761018, "end": 36765288}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry1d.png", "start": 36765288, "end": 36769298}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry2a.png", "start": 36769298, "end": 36776437}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry2b.png", "start": 36776437, "end": 36782996}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry2c.png", "start": 36782996, "end": 36786817}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry2d.png", "start": 36786817, "end": 36790455}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry3a.png", "start": 36790455, "end": 36797584}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry3b.png", "start": 36797584, "end": 36804170}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry3c.png", "start": 36804170, "end": 36808143}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry3d.png", "start": 36808143, "end": 36811915}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry4a.png", "start": 36811915, "end": 36822797}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry4b.png", "start": 36822797, "end": 36831635}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry4c.png", "start": 36831635, "end": 36837137}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry4d.png", "start": 36837137, "end": 36841847}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry5a.png", "start": 36841847, "end": 36852431}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry5b.png", "start": 36852431, "end": 36861057}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry5c.png", "start": 36861057, "end": 36866366}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry5d.png", "start": 36866366, "end": 36870916}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry6a.png", "start": 36870916, "end": 36879765}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry6b.png", "start": 36879765, "end": 36887043}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry6c.png", "start": 36887043, "end": 36891661}, {"filename": "/GameData/Textures/delvenPack/{dlv_tapestry6d.png", "start": 36891661, "end": 36895673}, {"filename": "/GameData/Textures/generic/__TB_empty.png", "start": 36895673, "end": 36896431}, {"filename": "/GameData/Textures/generic/brick.png", "start": 36896431, "end": 37409345}, {"filename": "/GameData/Textures/generic/brickPBR.png", "start": 37409345, "end": 38339477}, {"filename": "/GameData/Textures/generic/brickPBR_orm.png", "start": 38339477, "end": 38764328}, {"filename": "/GameData/Textures/generic/bricks.png", "start": 38764328, "end": 38775248}, {"filename": "/GameData/Textures/generic/cat.png", "start": 38775248, "end": 39024686}, {"filename": "/GameData/Textures/generic/foil.png", "start": 39024686, "end": 39281904}, {"filename": "/GameData/Textures/generic/grass.png", "start": 39281904, "end": 39410029}, {"filename": "/GameData/Textures/generic/hole_t.png", "start": 39410029, "end": 39412458}, {"filename": "/GameData/Textures/generic/light.png", "start": 39412458, "end": 39412586}, {"filename": "/GameData/Textures/generic/light_em.png", "start": 39412586, "end": 39412714}, {"filename": "/GameData/Textures/generic/mask_test_m.png", "start": 39412714, "end": 39416776}, {"filename": "/GameData/Textures/generic/mirror.png", "start": 39416776, "end": 39416896}, {"filename": "/GameData/Textures/generic/mirror_orm.png", "start": 39416896, "end": 39417016}, {"filename": "/GameData/Textures/generic/null_m.png", "start": 39417016, "end": 39418813}, {"filename": "/GameData/Textures/generic/trigger.png", "start": 39418813, "end": 39430689}, {"filename": "/GameData/Textures/generic/trigger_t.png", "start": 39430689, "end": 39442565}, {"filename": "/GameData/Textures/generic/white.png", "start": 39442565, "end": 39442685}, {"filename": "/GameData/Textures/gloves.png", "start": 39442685, "end": 39552360}, {"filename": "/GameData/Textures/jacket.png", "start": 39552360, "end": 39716172}, {"filename": "/GameData/Textures/lq_conc/conc1_1.png", "start": 39716172, "end": 39729285}, {"filename": "/GameData/Textures/lq_conc/conc1_10.png", "start": 39729285, "end": 39743399}, {"filename": "/GameData/Textures/lq_conc/conc1_2.png", "start": 39743399, "end": 39757440}, {"filename": "/GameData/Textures/lq_conc/conc1_3.png", "start": 39757440, "end": 39771950}, {"filename": "/GameData/Textures/lq_conc/conc1_4.png", "start": 39771950, "end": 39786336}, {"filename": "/GameData/Textures/lq_conc/conc1_5.png", "start": 39786336, "end": 39800785}, {"filename": "/GameData/Textures/lq_conc/conc1_6.png", "start": 39800785, "end": 39815402}, {"filename": "/GameData/Textures/lq_conc/conc1_7.png", "start": 39815402, "end": 39829999}, {"filename": "/GameData/Textures/lq_conc/conc1_8.png", "start": 39829999, "end": 39844347}, {"filename": "/GameData/Textures/lq_conc/conc1_9.png", "start": 39844347, "end": 39858505}, {"filename": "/GameData/Textures/lq_conc/conc1_a1.png", "start": 39858505, "end": 39872953}, {"filename": "/GameData/Textures/lq_conc/conc2_1.png", "start": 39872953, "end": 39882381}, {"filename": "/GameData/Textures/lq_conc/conc2_10.png", "start": 39882381, "end": 39891830}, {"filename": "/GameData/Textures/lq_conc/conc2_2.png", "start": 39891830, "end": 39901224}, {"filename": "/GameData/Textures/lq_conc/conc2_3.png", "start": 39901224, "end": 39910869}, {"filename": "/GameData/Textures/lq_conc/conc2_4.png", "start": 39910869, "end": 39920376}, {"filename": "/GameData/Textures/lq_conc/conc2_5.png", "start": 39920376, "end": 39929881}, {"filename": "/GameData/Textures/lq_conc/conc2_6.png", "start": 39929881, "end": 39939394}, {"filename": "/GameData/Textures/lq_conc/conc2_7.png", "start": 39939394, "end": 39948939}, {"filename": "/GameData/Textures/lq_conc/conc2_8.png", "start": 39948939, "end": 39958349}, {"filename": "/GameData/Textures/lq_conc/conc2_9.png", "start": 39958349, "end": 39967886}, {"filename": "/GameData/Textures/lq_conc/conc2_a1.png", "start": 39967886, "end": 39976916}, {"filename": "/GameData/Textures/lq_conc/conc3_1.png", "start": 39976916, "end": 39984780}, {"filename": "/GameData/Textures/lq_conc/conc3_10.png", "start": 39984780, "end": 39993716}, {"filename": "/GameData/Textures/lq_conc/conc3_2.png", "start": 39993716, "end": 40002110}, {"filename": "/GameData/Textures/lq_conc/conc3_3.png", "start": 40002110, "end": 40012337}, {"filename": "/GameData/Textures/lq_conc/conc3_4.png", "start": 40012337, "end": 40021616}, {"filename": "/GameData/Textures/lq_conc/conc3_5.png", "start": 40021616, "end": 40031346}, {"filename": "/GameData/Textures/lq_conc/conc3_6.png", "start": 40031346, "end": 40040906}, {"filename": "/GameData/Textures/lq_conc/conc3_7.png", "start": 40040906, "end": 40049731}, {"filename": "/GameData/Textures/lq_conc/conc3_8.png", "start": 40049731, "end": 40058512}, {"filename": "/GameData/Textures/lq_conc/conc3_9.png", "start": 40058512, "end": 40067988}, {"filename": "/GameData/Textures/lq_conc/conc3_a1.png", "start": 40067988, "end": 40076395}, {"filename": "/GameData/Textures/lq_conc/conc4_1.png", "start": 40076395, "end": 40082554}, {"filename": "/GameData/Textures/lq_conc/conc4_10.png", "start": 40082554, "end": 40088914}, {"filename": "/GameData/Textures/lq_conc/conc4_2.png", "start": 40088914, "end": 40095179}, {"filename": "/GameData/Textures/lq_conc/conc4_3.png", "start": 40095179, "end": 40102105}, {"filename": "/GameData/Textures/lq_conc/conc4_4.png", "start": 40102105, "end": 40108595}, {"filename": "/GameData/Textures/lq_conc/conc4_5.png", "start": 40108595, "end": 40115188}, {"filename": "/GameData/Textures/lq_conc/conc4_6.png", "start": 40115188, "end": 40121712}, {"filename": "/GameData/Textures/lq_conc/conc4_7.png", "start": 40121712, "end": 40128120}, {"filename": "/GameData/Textures/lq_conc/conc4_8.png", "start": 40128120, "end": 40134403}, {"filename": "/GameData/Textures/lq_conc/conc4_9.png", "start": 40134403, "end": 40140986}, {"filename": "/GameData/Textures/lq_conc/conc4_a1.png", "start": 40140986, "end": 40147055}, {"filename": "/GameData/Textures/lq_conc/conc5_1.png", "start": 40147055, "end": 40155817}, {"filename": "/GameData/Textures/lq_conc/conc5_10.png", "start": 40155817, "end": 40165432}, {"filename": "/GameData/Textures/lq_conc/conc5_2.png", "start": 40165432, "end": 40174748}, {"filename": "/GameData/Textures/lq_conc/conc5_8.png", "start": 40174748, "end": 40183828}, {"filename": "/GameData/Textures/lq_conc/conc5_9.png", "start": 40183828, "end": 40193542}, {"filename": "/GameData/Textures/lq_conc/conc5_a1.png", "start": 40193542, "end": 40201917}, {"filename": "/GameData/Textures/lq_conc/conc6_1.png", "start": 40201917, "end": 40208330}, {"filename": "/GameData/Textures/lq_conc/conc6_10.png", "start": 40208330, "end": 40214739}, {"filename": "/GameData/Textures/lq_conc/conc6_2.png", "start": 40214739, "end": 40221067}, {"filename": "/GameData/Textures/lq_conc/conc6_3.png", "start": 40221067, "end": 40227928}, {"filename": "/GameData/Textures/lq_conc/conc6_4.png", "start": 40227928, "end": 40234536}, {"filename": "/GameData/Textures/lq_conc/conc6_5.png", "start": 40234536, "end": 40241212}, {"filename": "/GameData/Textures/lq_conc/conc6_6.png", "start": 40241212, "end": 40247986}, {"filename": "/GameData/Textures/lq_conc/conc6_7.png", "start": 40247986, "end": 40254689}, {"filename": "/GameData/Textures/lq_conc/conc6_8.png", "start": 40254689, "end": 40261185}, {"filename": "/GameData/Textures/lq_conc/conc6_9.png", "start": 40261185, "end": 40267807}, {"filename": "/GameData/Textures/lq_conc/conc6_a1.png", "start": 40267807, "end": 40274451}, {"filename": "/GameData/Textures/lq_conc/conc7_1.png", "start": 40274451, "end": 40282241}, {"filename": "/GameData/Textures/lq_conc/conc7_10.png", "start": 40282241, "end": 40289919}, {"filename": "/GameData/Textures/lq_conc/conc7_2.png", "start": 40289919, "end": 40297545}, {"filename": "/GameData/Textures/lq_conc/conc7_3.png", "start": 40297545, "end": 40305489}, {"filename": "/GameData/Textures/lq_conc/conc7_4.png", "start": 40305489, "end": 40313257}, {"filename": "/GameData/Textures/lq_conc/conc7_5.png", "start": 40313257, "end": 40321081}, {"filename": "/GameData/Textures/lq_conc/conc7_6.png", "start": 40321081, "end": 40329080}, {"filename": "/GameData/Textures/lq_conc/conc7_7.png", "start": 40329080, "end": 40337060}, {"filename": "/GameData/Textures/lq_conc/conc7_8.png", "start": 40337060, "end": 40344884}, {"filename": "/GameData/Textures/lq_conc/conc7_9.png", "start": 40344884, "end": 40352684}, {"filename": "/GameData/Textures/lq_conc/conc7_a1.png", "start": 40352684, "end": 40360417}, {"filename": "/GameData/Textures/lq_conc/flr1_1.png", "start": 40360417, "end": 40362749}, {"filename": "/GameData/Textures/lq_conc/flr1_2.png", "start": 40362749, "end": 40366013}, {"filename": "/GameData/Textures/lq_conc/flr1_3.png", "start": 40366013, "end": 40369553}, {"filename": "/GameData/Textures/lq_conc/flr1_4.png", "start": 40369553, "end": 40372330}, {"filename": "/GameData/Textures/lq_conc/flr1_5.png", "start": 40372330, "end": 40375926}, {"filename": "/GameData/Textures/lq_conc/flr1_6.png", "start": 40375926, "end": 40378982}, {"filename": "/GameData/Textures/lq_conc/flr2_1.png", "start": 40378982, "end": 40381092}, {"filename": "/GameData/Textures/lq_conc/flr2_2.png", "start": 40381092, "end": 40384305}, {"filename": "/GameData/Textures/lq_conc/flr2_3.png", "start": 40384305, "end": 40387925}, {"filename": "/GameData/Textures/lq_conc/flr2_4.png", "start": 40387925, "end": 40391323}, {"filename": "/GameData/Textures/lq_conc/flr2_5.png", "start": 40391323, "end": 40394683}, {"filename": "/GameData/Textures/lq_conc/flr2_6.png", "start": 40394683, "end": 40397882}, {"filename": "/GameData/Textures/lq_conc/flr2_7.png", "start": 40397882, "end": 40401017}, {"filename": "/GameData/Textures/lq_conc/flr2_8.png", "start": 40401017, "end": 40403958}, {"filename": "/GameData/Textures/lq_dev/clip.png", "start": 40403958, "end": 40404464}, {"filename": "/GameData/Textures/lq_dev/dot_blue_a.png", "start": 40404464, "end": 40405020}, {"filename": "/GameData/Textures/lq_dev/dot_blue_b.png", "start": 40405020, "end": 40405575}, {"filename": "/GameData/Textures/lq_dev/dot_blue_c.png", "start": 40405575, "end": 40406130}, {"filename": "/GameData/Textures/lq_dev/dot_brown_a.png", "start": 40406130, "end": 40406685}, {"filename": "/GameData/Textures/lq_dev/dot_brown_b.png", "start": 40406685, "end": 40407240}, {"filename": "/GameData/Textures/lq_dev/dot_brown_c.png", "start": 40407240, "end": 40407795}, {"filename": "/GameData/Textures/lq_dev/dot_green_a.png", "start": 40407795, "end": 40408351}, {"filename": "/GameData/Textures/lq_dev/dot_green_b.png", "start": 40408351, "end": 40408906}, {"filename": "/GameData/Textures/lq_dev/dot_green_c.png", "start": 40408906, "end": 40409461}, {"filename": "/GameData/Textures/lq_dev/dot_grey_a.png", "start": 40409461, "end": 40410014}, {"filename": "/GameData/Textures/lq_dev/dot_grey_b.png", "start": 40410014, "end": 40410570}, {"filename": "/GameData/Textures/lq_dev/dot_grey_c.png", "start": 40410570, "end": 40411125}, {"filename": "/GameData/Textures/lq_dev/dot_olive_a.png", "start": 40411125, "end": 40411680}, {"filename": "/GameData/Textures/lq_dev/dot_olive_b.png", "start": 40411680, "end": 40412234}, {"filename": "/GameData/Textures/lq_dev/dot_olive_c.png", "start": 40412234, "end": 40412789}, {"filename": "/GameData/Textures/lq_dev/dot_orange_a.png", "start": 40412789, "end": 40413345}, {"filename": "/GameData/Textures/lq_dev/dot_orange_b.png", "start": 40413345, "end": 40413900}, {"filename": "/GameData/Textures/lq_dev/dot_orange_c.png", "start": 40413900, "end": 40414455}, {"filename": "/GameData/Textures/lq_dev/dot_pink_a.png", "start": 40414455, "end": 40415011}, {"filename": "/GameData/Textures/lq_dev/dot_pink_b.png", "start": 40415011, "end": 40415567}, {"filename": "/GameData/Textures/lq_dev/dot_pink_c.png", "start": 40415567, "end": 40416122}, {"filename": "/GameData/Textures/lq_dev/dot_purple_a.png", "start": 40416122, "end": 40416678}, {"filename": "/GameData/Textures/lq_dev/dot_purple_b.png", "start": 40416678, "end": 40417233}, {"filename": "/GameData/Textures/lq_dev/dot_purple_c.png", "start": 40417233, "end": 40417788}, {"filename": "/GameData/Textures/lq_dev/dot_red_a.png", "start": 40417788, "end": 40418339}, {"filename": "/GameData/Textures/lq_dev/dot_red_b.png", "start": 40418339, "end": 40418890}, {"filename": "/GameData/Textures/lq_dev/dot_red_c.png", "start": 40418890, "end": 40419441}, {"filename": "/GameData/Textures/lq_dev/dot_tan_a.png", "start": 40419441, "end": 40419997}, {"filename": "/GameData/Textures/lq_dev/dot_tan_b.png", "start": 40419997, "end": 40420553}, {"filename": "/GameData/Textures/lq_dev/dot_tan_c.png", "start": 40420553, "end": 40421108}, {"filename": "/GameData/Textures/lq_dev/dot_yellow_a.png", "start": 40421108, "end": 40421664}, {"filename": "/GameData/Textures/lq_dev/dot_yellow_b.png", "start": 40421664, "end": 40422220}, {"filename": "/GameData/Textures/lq_dev/dot_yellow_c.png", "start": 40422220, "end": 40422774}, {"filename": "/GameData/Textures/lq_dev/floor_blue_a.png", "start": 40422774, "end": 40423591}, {"filename": "/GameData/Textures/lq_dev/floor_blue_b.png", "start": 40423591, "end": 40424409}, {"filename": "/GameData/Textures/lq_dev/floor_blue_c.png", "start": 40424409, "end": 40425227}, {"filename": "/GameData/Textures/lq_dev/floor_brown_a.png", "start": 40425227, "end": 40426049}, {"filename": "/GameData/Textures/lq_dev/floor_brown_b.png", "start": 40426049, "end": 40426872}, {"filename": "/GameData/Textures/lq_dev/floor_brown_c.png", "start": 40426872, "end": 40427694}, {"filename": "/GameData/Textures/lq_dev/floor_green_a.png", "start": 40427694, "end": 40428515}, {"filename": "/GameData/Textures/lq_dev/floor_green_b.png", "start": 40428515, "end": 40429337}, {"filename": "/GameData/Textures/lq_dev/floor_green_c.png", "start": 40429337, "end": 40430159}, {"filename": "/GameData/Textures/lq_dev/floor_grey_a.png", "start": 40430159, "end": 40430969}, {"filename": "/GameData/Textures/lq_dev/floor_grey_b.png", "start": 40430969, "end": 40431782}, {"filename": "/GameData/Textures/lq_dev/floor_grey_c.png", "start": 40431782, "end": 40432596}, {"filename": "/GameData/Textures/lq_dev/floor_olive_a.png", "start": 40432596, "end": 40433414}, {"filename": "/GameData/Textures/lq_dev/floor_olive_b.png", "start": 40433414, "end": 40434229}, {"filename": "/GameData/Textures/lq_dev/floor_olive_c.png", "start": 40434229, "end": 40435046}, {"filename": "/GameData/Textures/lq_dev/floor_orange_a.png", "start": 40435046, "end": 40435869}, {"filename": "/GameData/Textures/lq_dev/floor_orange_b.png", "start": 40435869, "end": 40436692}, {"filename": "/GameData/Textures/lq_dev/floor_orange_c.png", "start": 40436692, "end": 40437512}, {"filename": "/GameData/Textures/lq_dev/floor_pink_a.png", "start": 40437512, "end": 40438335}, {"filename": "/GameData/Textures/lq_dev/floor_pink_b.png", "start": 40438335, "end": 40439157}, {"filename": "/GameData/Textures/lq_dev/floor_pink_c.png", "start": 40439157, "end": 40439979}, {"filename": "/GameData/Textures/lq_dev/floor_purple_a.png", "start": 40439979, "end": 40440801}, {"filename": "/GameData/Textures/lq_dev/floor_purple_b.png", "start": 40440801, "end": 40441623}, {"filename": "/GameData/Textures/lq_dev/floor_purple_c.png", "start": 40441623, "end": 40442444}, {"filename": "/GameData/Textures/lq_dev/floor_red_a.png", "start": 40442444, "end": 40443254}, {"filename": "/GameData/Textures/lq_dev/floor_red_b.png", "start": 40443254, "end": 40444064}, {"filename": "/GameData/Textures/lq_dev/floor_red_c.png", "start": 40444064, "end": 40444875}, {"filename": "/GameData/Textures/lq_dev/floor_tan_a.png", "start": 40444875, "end": 40445695}, {"filename": "/GameData/Textures/lq_dev/floor_tan_b.png", "start": 40445695, "end": 40446519}, {"filename": "/GameData/Textures/lq_dev/floor_tan_c.png", "start": 40446519, "end": 40447340}, {"filename": "/GameData/Textures/lq_dev/floor_yellow_a.png", "start": 40447340, "end": 40448161}, {"filename": "/GameData/Textures/lq_dev/floor_yellow_b.png", "start": 40448161, "end": 40448983}, {"filename": "/GameData/Textures/lq_dev/floor_yellow_c.png", "start": 40448983, "end": 40449802}, {"filename": "/GameData/Textures/lq_dev/hint.png", "start": 40449802, "end": 40450668}, {"filename": "/GameData/Textures/lq_dev/hintskip.png", "start": 40450668, "end": 40451561}, {"filename": "/GameData/Textures/lq_dev/key_gold_1.png", "start": 40451561, "end": 40452316}, {"filename": "/GameData/Textures/lq_dev/key_silver_1.png", "start": 40452316, "end": 40453070}, {"filename": "/GameData/Textures/lq_dev/light_fbr.png", "start": 40453070, "end": 40453986}, {"filename": "/GameData/Textures/lq_dev/origin.png", "start": 40453986, "end": 40454469}, {"filename": "/GameData/Textures/lq_dev/plus_0_button_fbr.png", "start": 40454469, "end": 40454668}, {"filename": "/GameData/Textures/lq_dev/plus_0_shoot_fbr.png", "start": 40454668, "end": 40454947}, {"filename": "/GameData/Textures/lq_dev/plus_1_button_fbr.png", "start": 40454947, "end": 40455148}, {"filename": "/GameData/Textures/lq_dev/plus_1_shoot_fbr.png", "start": 40455148, "end": 40455429}, {"filename": "/GameData/Textures/lq_dev/plus_a_button_fbr.png", "start": 40455429, "end": 40455630}, {"filename": "/GameData/Textures/lq_dev/plus_a_shoot_fbr.png", "start": 40455630, "end": 40455917}, {"filename": "/GameData/Textures/lq_dev/skip.png", "start": 40455917, "end": 40456405}, {"filename": "/GameData/Textures/lq_dev/sky_dev.png", "start": 40456405, "end": 40458377}, {"filename": "/GameData/Textures/lq_dev/sky_dev.png.bak", "start": 40458377, "end": 40470731}, {"filename": "/GameData/Textures/lq_dev/sky_dev_day_fbr.png", "start": 40470731, "end": 40474557}, {"filename": "/GameData/Textures/lq_dev/sky_dev_void.png", "start": 40474557, "end": 40475468}, {"filename": "/GameData/Textures/lq_dev/star_blood1.png", "start": 40475468, "end": 40475830}, {"filename": "/GameData/Textures/lq_dev/star_lava1.png", "start": 40475830, "end": 40476201}, {"filename": "/GameData/Textures/lq_dev/star_lavaskip.png", "start": 40476201, "end": 40477314}, {"filename": "/GameData/Textures/lq_dev/star_slime1.png", "start": 40477314, "end": 40477684}, {"filename": "/GameData/Textures/lq_dev/star_slimeskip.png", "start": 40477684, "end": 40478757}, {"filename": "/GameData/Textures/lq_dev/star_smile.png", "start": 40478757, "end": 40479237}, {"filename": "/GameData/Textures/lq_dev/star_teleport.png", "start": 40479237, "end": 40479561}, {"filename": "/GameData/Textures/lq_dev/star_water1.png", "start": 40479561, "end": 40479933}, {"filename": "/GameData/Textures/lq_dev/star_water2.png", "start": 40479933, "end": 40480302}, {"filename": "/GameData/Textures/lq_dev/star_waterskip.png", "start": 40480302, "end": 40482042}, {"filename": "/GameData/Textures/lq_dev/trigger.png", "start": 40482042, "end": 40482539}, {"filename": "/GameData/Textures/lq_dev/wall_blue_a.png", "start": 40482539, "end": 40483219}, {"filename": "/GameData/Textures/lq_dev/wall_blue_b.png", "start": 40483219, "end": 40483899}, {"filename": "/GameData/Textures/lq_dev/wall_blue_c.png", "start": 40483899, "end": 40484579}, {"filename": "/GameData/Textures/lq_dev/wall_brown_a.png", "start": 40484579, "end": 40485261}, {"filename": "/GameData/Textures/lq_dev/wall_brown_b.png", "start": 40485261, "end": 40485943}, {"filename": "/GameData/Textures/lq_dev/wall_brown_c.png", "start": 40485943, "end": 40486625}, {"filename": "/GameData/Textures/lq_dev/wall_green_a.png", "start": 40486625, "end": 40487307}, {"filename": "/GameData/Textures/lq_dev/wall_green_b.png", "start": 40487307, "end": 40487989}, {"filename": "/GameData/Textures/lq_dev/wall_green_c.png", "start": 40487989, "end": 40488671}, {"filename": "/GameData/Textures/lq_dev/wall_grey_a.png", "start": 40488671, "end": 40489345}, {"filename": "/GameData/Textures/lq_dev/wall_grey_b.png", "start": 40489345, "end": 40490021}, {"filename": "/GameData/Textures/lq_dev/wall_grey_c.png", "start": 40490021, "end": 40490698}, {"filename": "/GameData/Textures/lq_dev/wall_olive_a.png", "start": 40490698, "end": 40491378}, {"filename": "/GameData/Textures/lq_dev/wall_olive_b.png", "start": 40491378, "end": 40492055}, {"filename": "/GameData/Textures/lq_dev/wall_olive_c.png", "start": 40492055, "end": 40492734}, {"filename": "/GameData/Textures/lq_dev/wall_orange_a.png", "start": 40492734, "end": 40493416}, {"filename": "/GameData/Textures/lq_dev/wall_orange_b.png", "start": 40493416, "end": 40494098}, {"filename": "/GameData/Textures/lq_dev/wall_orange_c.png", "start": 40494098, "end": 40494779}, {"filename": "/GameData/Textures/lq_dev/wall_pink_a.png", "start": 40494779, "end": 40495461}, {"filename": "/GameData/Textures/lq_dev/wall_pink_b.png", "start": 40495461, "end": 40496143}, {"filename": "/GameData/Textures/lq_dev/wall_pink_c.png", "start": 40496143, "end": 40496825}, {"filename": "/GameData/Textures/lq_dev/wall_purple_a.png", "start": 40496825, "end": 40497508}, {"filename": "/GameData/Textures/lq_dev/wall_purple_b.png", "start": 40497508, "end": 40498190}, {"filename": "/GameData/Textures/lq_dev/wall_purple_c.png", "start": 40498190, "end": 40498872}, {"filename": "/GameData/Textures/lq_dev/wall_red_a.png", "start": 40498872, "end": 40499545}, {"filename": "/GameData/Textures/lq_dev/wall_red_b.png", "start": 40499545, "end": 40500218}, {"filename": "/GameData/Textures/lq_dev/wall_red_c.png", "start": 40500218, "end": 40500892}, {"filename": "/GameData/Textures/lq_dev/wall_tan_a.png", "start": 40500892, "end": 40501574}, {"filename": "/GameData/Textures/lq_dev/wall_tan_b.png", "start": 40501574, "end": 40502256}, {"filename": "/GameData/Textures/lq_dev/wall_tan_c.png", "start": 40502256, "end": 40502938}, {"filename": "/GameData/Textures/lq_dev/wall_yellow_a.png", "start": 40502938, "end": 40503620}, {"filename": "/GameData/Textures/lq_dev/wall_yellow_b.png", "start": 40503620, "end": 40504302}, {"filename": "/GameData/Textures/lq_dev/wall_yellow_c.png", "start": 40504302, "end": 40504983}, {"filename": "/GameData/Textures/lq_dev/{char_0_fbr.png", "start": 40504983, "end": 40505160}, {"filename": "/GameData/Textures/lq_dev/{char_1_fbr.png", "start": 40505160, "end": 40505320}, {"filename": "/GameData/Textures/lq_dev/{char_2_fbr.png", "start": 40505320, "end": 40505492}, {"filename": "/GameData/Textures/lq_dev/{char_3_fbr.png", "start": 40505492, "end": 40505671}, {"filename": "/GameData/Textures/lq_dev/{char_4_fbr.png", "start": 40505671, "end": 40505834}, {"filename": "/GameData/Textures/lq_dev/{char_5_fbr.png", "start": 40505834, "end": 40506007}, {"filename": "/GameData/Textures/lq_dev/{char_6_fbr.png", "start": 40506007, "end": 40506179}, {"filename": "/GameData/Textures/lq_dev/{char_7_fbr.png", "start": 40506179, "end": 40506333}, {"filename": "/GameData/Textures/lq_dev/{char_8_fbr.png", "start": 40506333, "end": 40506501}, {"filename": "/GameData/Textures/lq_dev/{char_9_fbr.png", "start": 40506501, "end": 40506668}, {"filename": "/GameData/Textures/lq_dev/{char_a_fbr.png", "start": 40506668, "end": 40506832}, {"filename": "/GameData/Textures/lq_dev/{char_b_fbr.png", "start": 40506832, "end": 40507002}, {"filename": "/GameData/Textures/lq_dev/{char_c_fbr.png", "start": 40507002, "end": 40507152}, {"filename": "/GameData/Textures/lq_dev/{char_d_fbr.png", "start": 40507152, "end": 40507318}, {"filename": "/GameData/Textures/lq_dev/{char_e_fbr.png", "start": 40507318, "end": 40507486}, {"filename": "/GameData/Textures/lq_dev/{char_f_fbr.png", "start": 40507486, "end": 40507646}, {"filename": "/GameData/Textures/lq_dev/{char_g_fbr.png", "start": 40507646, "end": 40507818}, {"filename": "/GameData/Textures/lq_dev/{char_h_fbr.png", "start": 40507818, "end": 40507977}, {"filename": "/GameData/Textures/lq_dev/{char_i_fbr.png", "start": 40507977, "end": 40508136}, {"filename": "/GameData/Textures/lq_dev/{char_j_fbr.png", "start": 40508136, "end": 40508302}, {"filename": "/GameData/Textures/lq_dev/{char_k_fbr.png", "start": 40508302, "end": 40508496}, {"filename": "/GameData/Textures/lq_dev/{char_l_fbr.png", "start": 40508496, "end": 40508637}, {"filename": "/GameData/Textures/lq_dev/{char_m_fbr.png", "start": 40508637, "end": 40508786}, {"filename": "/GameData/Textures/lq_dev/{char_n_fbr.png", "start": 40508786, "end": 40508969}, {"filename": "/GameData/Textures/lq_dev/{char_o_fbr.png", "start": 40508969, "end": 40509119}, {"filename": "/GameData/Textures/lq_dev/{char_p_fbr.png", "start": 40509119, "end": 40509277}, {"filename": "/GameData/Textures/lq_dev/{char_q_fbr.png", "start": 40509277, "end": 40509443}, {"filename": "/GameData/Textures/lq_dev/{char_r_fbr.png", "start": 40509443, "end": 40509615}, {"filename": "/GameData/Textures/lq_dev/{char_s_fbr.png", "start": 40509615, "end": 40509791}, {"filename": "/GameData/Textures/lq_dev/{char_t_fbr.png", "start": 40509791, "end": 40509939}, {"filename": "/GameData/Textures/lq_dev/{char_trans_fbr.png", "start": 40509939, "end": 40510062}, {"filename": "/GameData/Textures/lq_dev/{char_u_fbr.png", "start": 40510062, "end": 40510208}, {"filename": "/GameData/Textures/lq_dev/{char_v_fbr.png", "start": 40510208, "end": 40510376}, {"filename": "/GameData/Textures/lq_dev/{char_w_fbr.png", "start": 40510376, "end": 40510525}, {"filename": "/GameData/Textures/lq_dev/{char_x_fbr.png", "start": 40510525, "end": 40510707}, {"filename": "/GameData/Textures/lq_dev/{char_y_fbr.png", "start": 40510707, "end": 40510876}, {"filename": "/GameData/Textures/lq_dev/{char_z_fbr.png", "start": 40510876, "end": 40511051}, {"filename": "/GameData/Textures/lq_dev/{charlow_a_fbr.png", "start": 40511051, "end": 40511206}, {"filename": "/GameData/Textures/lq_dev/{charlow_b_fbr.png", "start": 40511206, "end": 40511365}, {"filename": "/GameData/Textures/lq_dev/{charlow_c_fbr.png", "start": 40511365, "end": 40511519}, {"filename": "/GameData/Textures/lq_dev/{charlow_d_fbr.png", "start": 40511519, "end": 40511680}, {"filename": "/GameData/Textures/lq_dev/{charlow_e_fbr.png", "start": 40511680, "end": 40511834}, {"filename": "/GameData/Textures/lq_dev/{charlow_f_fbr.png", "start": 40511834, "end": 40511994}, {"filename": "/GameData/Textures/lq_dev/{charlow_g_fbr.png", "start": 40511994, "end": 40512152}, {"filename": "/GameData/Textures/lq_dev/{charlow_h_fbr.png", "start": 40512152, "end": 40512306}, {"filename": "/GameData/Textures/lq_dev/{charlow_i_fbr.png", "start": 40512306, "end": 40512452}, {"filename": "/GameData/Textures/lq_dev/{charlow_j_fbr.png", "start": 40512452, "end": 40512612}, {"filename": "/GameData/Textures/lq_dev/{charlow_k_fbr.png", "start": 40512612, "end": 40512791}, {"filename": "/GameData/Textures/lq_dev/{charlow_l_fbr.png", "start": 40512791, "end": 40512929}, {"filename": "/GameData/Textures/lq_dev/{charlow_m_fbr.png", "start": 40512929, "end": 40513082}, {"filename": "/GameData/Textures/lq_dev/{charlow_n_fbr.png", "start": 40513082, "end": 40513232}, {"filename": "/GameData/Textures/lq_dev/{charlow_o_fbr.png", "start": 40513232, "end": 40513386}, {"filename": "/GameData/Textures/lq_dev/{charlow_p_fbr.png", "start": 40513386, "end": 40513541}, {"filename": "/GameData/Textures/lq_dev/{charlow_q_fbr.png", "start": 40513541, "end": 40513691}, {"filename": "/GameData/Textures/lq_dev/{charlow_r_fbr.png", "start": 40513691, "end": 40513844}, {"filename": "/GameData/Textures/lq_dev/{charlow_s_fbr.png", "start": 40513844, "end": 40513996}, {"filename": "/GameData/Textures/lq_dev/{charlow_t_fbr.png", "start": 40513996, "end": 40514162}, {"filename": "/GameData/Textures/lq_dev/{charlow_u_fbr.png", "start": 40514162, "end": 40514313}, {"filename": "/GameData/Textures/lq_dev/{charlow_v_fbr.png", "start": 40514313, "end": 40514487}, {"filename": "/GameData/Textures/lq_dev/{charlow_w_fbr.png", "start": 40514487, "end": 40514641}, {"filename": "/GameData/Textures/lq_dev/{charlow_x_fbr.png", "start": 40514641, "end": 40514828}, {"filename": "/GameData/Textures/lq_dev/{charlow_y_fbr.png", "start": 40514828, "end": 40515002}, {"filename": "/GameData/Textures/lq_dev/{charlow_z_fbr.png", "start": 40515002, "end": 40515173}, {"filename": "/GameData/Textures/lq_dev/{chars_add_fbr.png", "start": 40515173, "end": 40515336}, {"filename": "/GameData/Textures/lq_dev/{chars_and_fbr.png", "start": 40515336, "end": 40515528}, {"filename": "/GameData/Textures/lq_dev/{chars_ardown_fbr.png", "start": 40515528, "end": 40515713}, {"filename": "/GameData/Textures/lq_dev/{chars_arleft_fbr.png", "start": 40515713, "end": 40515897}, {"filename": "/GameData/Textures/lq_dev/{chars_arright_fbr.png", "start": 40515897, "end": 40516082}, {"filename": "/GameData/Textures/lq_dev/{chars_arup_fbr.png", "start": 40516082, "end": 40516260}, {"filename": "/GameData/Textures/lq_dev/{chars_at_fbr.png", "start": 40516260, "end": 40516417}, {"filename": "/GameData/Textures/lq_dev/{chars_brackc1_fbr.png", "start": 40516417, "end": 40516596}, {"filename": "/GameData/Textures/lq_dev/{chars_brackc2_fbr.png", "start": 40516596, "end": 40516780}, {"filename": "/GameData/Textures/lq_dev/{chars_brackr1_fbr.png", "start": 40516780, "end": 40516947}, {"filename": "/GameData/Textures/lq_dev/{chars_brackr2_fbr.png", "start": 40516947, "end": 40517117}, {"filename": "/GameData/Textures/lq_dev/{chars_bracks1_fbr.png", "start": 40517117, "end": 40517271}, {"filename": "/GameData/Textures/lq_dev/{chars_bracks2_fbr.png", "start": 40517271, "end": 40517424}, {"filename": "/GameData/Textures/lq_dev/{chars_caret_fbr.png", "start": 40517424, "end": 40517604}, {"filename": "/GameData/Textures/lq_dev/{chars_colon_fbr.png", "start": 40517604, "end": 40517756}, {"filename": "/GameData/Textures/lq_dev/{chars_colonsemi_fbr.png", "start": 40517756, "end": 40517923}, {"filename": "/GameData/Textures/lq_dev/{chars_comma_fbr.png", "start": 40517923, "end": 40518079}, {"filename": "/GameData/Textures/lq_dev/{chars_div_fbr.png", "start": 40518079, "end": 40518248}, {"filename": "/GameData/Textures/lq_dev/{chars_dollar_fbr.png", "start": 40518248, "end": 40518424}, {"filename": "/GameData/Textures/lq_dev/{chars_equ_fbr.png", "start": 40518424, "end": 40518583}, {"filename": "/GameData/Textures/lq_dev/{chars_exclam_fbr.png", "start": 40518583, "end": 40518727}, {"filename": "/GameData/Textures/lq_dev/{chars_grave_fbr.png", "start": 40518727, "end": 40518877}, {"filename": "/GameData/Textures/lq_dev/{chars_hash_fbr.png", "start": 40518877, "end": 40519062}, {"filename": "/GameData/Textures/lq_dev/{chars_heart_fbr.png", "start": 40519062, "end": 40519247}, {"filename": "/GameData/Textures/lq_dev/{chars_multi_fbr.png", "start": 40519247, "end": 40519416}, {"filename": "/GameData/Textures/lq_dev/{chars_percent_fbr.png", "start": 40519416, "end": 40519622}, {"filename": "/GameData/Textures/lq_dev/{chars_perio_fbr.png", "start": 40519622, "end": 40519759}, {"filename": "/GameData/Textures/lq_dev/{chars_pipe_fbr.png", "start": 40519759, "end": 40519906}, {"filename": "/GameData/Textures/lq_dev/{chars_quest_fbr.png", "start": 40519906, "end": 40520079}, {"filename": "/GameData/Textures/lq_dev/{chars_slaback_fbr.png", "start": 40520079, "end": 40520266}, {"filename": "/GameData/Textures/lq_dev/{chars_slafoward_fbr.png", "start": 40520266, "end": 40520445}, {"filename": "/GameData/Textures/lq_dev/{chars_smile_fbr.png", "start": 40520445, "end": 40520605}, {"filename": "/GameData/Textures/lq_dev/{chars_sub_fbr.png", "start": 40520605, "end": 40520752}, {"filename": "/GameData/Textures/lq_dev/{chars_sun_fbr.png", "start": 40520752, "end": 40520951}, {"filename": "/GameData/Textures/lq_dev/{chars_thngreater_fbr.png", "start": 40520951, "end": 40521144}, {"filename": "/GameData/Textures/lq_dev/{chars_thnless_fbr.png", "start": 40521144, "end": 40521331}, {"filename": "/GameData/Textures/lq_dev/{chars_tilde_fbr.png", "start": 40521331, "end": 40521500}, {"filename": "/GameData/Textures/lq_dev/{chars_unders_fbr.png", "start": 40521500, "end": 40521637}, {"filename": "/GameData/Textures/lq_flesh/bone1_1.png", "start": 40521637, "end": 40529044}, {"filename": "/GameData/Textures/lq_flesh/bone1_2.png", "start": 40529044, "end": 40538236}, {"filename": "/GameData/Textures/lq_flesh/bone2_1.png", "start": 40538236, "end": 40548668}, {"filename": "/GameData/Textures/lq_flesh/dopefish_fbr.png", "start": 40548668, "end": 40556898}, {"filename": "/GameData/Textures/lq_flesh/flesh_gut1.png", "start": 40556898, "end": 40567213}, {"filename": "/GameData/Textures/lq_flesh/flesh_gut2.png", "start": 40567213, "end": 40579013}, {"filename": "/GameData/Textures/lq_flesh/flesh_rot1_1.png", "start": 40579013, "end": 40592763}, {"filename": "/GameData/Textures/lq_flesh/flesh_rot1_2.png", "start": 40592763, "end": 40605743}, {"filename": "/GameData/Textures/lq_flesh/flesh_rot1_3.png", "start": 40605743, "end": 40617776}, {"filename": "/GameData/Textures/lq_flesh/flesh_rot1_4a.png", "start": 40617776, "end": 40630989}, {"filename": "/GameData/Textures/lq_flesh/flesh_rot1_4b.png", "start": 40630989, "end": 40644129}, {"filename": "/GameData/Textures/lq_flesh/flesh_rot1_5a.png", "start": 40644129, "end": 40657773}, {"filename": "/GameData/Textures/lq_flesh/flesh_rot1_5b.png", "start": 40657773, "end": 40671346}, {"filename": "/GameData/Textures/lq_flesh/flesh_rot2_1.png", "start": 40671346, "end": 40684572}, {"filename": "/GameData/Textures/lq_flesh/flesh_rot3_1.png", "start": 40684572, "end": 40699518}, {"filename": "/GameData/Textures/lq_flesh/flesh_rot3_2.png", "start": 40699518, "end": 40714557}, {"filename": "/GameData/Textures/lq_flesh/flesh_rot4_1.png", "start": 40714557, "end": 40729194}, {"filename": "/GameData/Textures/lq_flesh/flesh_rot5_1.png", "start": 40729194, "end": 40741462}, {"filename": "/GameData/Textures/lq_flesh/flesh_rot6_1.png", "start": 40741462, "end": 40757869}, {"filename": "/GameData/Textures/lq_flesh/flesh_wod1_1.png", "start": 40757869, "end": 40769387}, {"filename": "/GameData/Textures/lq_flesh/flesh_wod1_2.png", "start": 40769387, "end": 40782449}, {"filename": "/GameData/Textures/lq_flesh/flesh_wod1_3.png", "start": 40782449, "end": 40793452}, {"filename": "/GameData/Textures/lq_flesh/flesh_wod1_4.png", "start": 40793452, "end": 40804864}, {"filename": "/GameData/Textures/lq_flesh/flesh_wod1_5.png", "start": 40804864, "end": 40815463}, {"filename": "/GameData/Textures/lq_flesh/flesh_wod2_1.png", "start": 40815463, "end": 40819119}, {"filename": "/GameData/Textures/lq_flesh/flesh_wod2_2.png", "start": 40819119, "end": 40823216}, {"filename": "/GameData/Textures/lq_flesh/flesh_wod2_3.png", "start": 40823216, "end": 40826592}, {"filename": "/GameData/Textures/lq_flesh/flesh_wod2_4.png", "start": 40826592, "end": 40830190}, {"filename": "/GameData/Textures/lq_flesh/flesh_wod2_5.png", "start": 40830190, "end": 40833580}, {"filename": "/GameData/Textures/lq_flesh/fleshtile.png", "start": 40833580, "end": 40844549}, {"filename": "/GameData/Textures/lq_flesh/marbred128.png", "start": 40844549, "end": 40856778}, {"filename": "/GameData/Textures/lq_flesh/may_eat_eye2_fbr.png", "start": 40856778, "end": 40860651}, {"filename": "/GameData/Textures/lq_flesh/may_eat_eye3_fbr.png", "start": 40860651, "end": 40864528}, {"filename": "/GameData/Textures/lq_flesh/may_eat_eye_fbr.png", "start": 40864528, "end": 40868403}, {"filename": "/GameData/Textures/lq_flesh/may_flesh1_b.png", "start": 40868403, "end": 40881066}, {"filename": "/GameData/Textures/lq_flesh/may_flesh1_c.png", "start": 40881066, "end": 40893790}, {"filename": "/GameData/Textures/lq_flesh/may_flesh1_hol1.png", "start": 40893790, "end": 40898462}, {"filename": "/GameData/Textures/lq_flesh/may_flesh1_lit1_fbr.png", "start": 40898462, "end": 40902532}, {"filename": "/GameData/Textures/lq_flesh/may_flesh1_shut1.png", "start": 40902532, "end": 40917498}, {"filename": "/GameData/Textures/lq_flesh/may_flesh1_sp.png", "start": 40917498, "end": 40937640}, {"filename": "/GameData/Textures/lq_flesh/may_flesh1_sp2.png", "start": 40937640, "end": 40963263}, {"filename": "/GameData/Textures/lq_flesh/may_flesh1_te.png", "start": 40963263, "end": 40978369}, {"filename": "/GameData/Textures/lq_flesh/may_flesh1_tet.png", "start": 40978369, "end": 40991589}, {"filename": "/GameData/Textures/lq_flesh/may_flesh2_b.png", "start": 40991589, "end": 41005112}, {"filename": "/GameData/Textures/lq_flesh/may_flesh3_1a.png", "start": 41005112, "end": 41018838}, {"filename": "/GameData/Textures/lq_flesh/may_flesh3_1b.png", "start": 41018838, "end": 41055400}, {"filename": "/GameData/Textures/lq_flesh/may_flesh3_b.png", "start": 41055400, "end": 41069286}, {"filename": "/GameData/Textures/lq_flesh/may_flesh4_det.png", "start": 41069286, "end": 41079597}, {"filename": "/GameData/Textures/lq_flesh/may_flesh4a_det.png", "start": 41079597, "end": 41094458}, {"filename": "/GameData/Textures/lq_flesh/may_flesh5.png", "start": 41094458, "end": 41104794}, {"filename": "/GameData/Textures/lq_flesh/may_flesh5_1a.png", "start": 41104794, "end": 41159529}, {"filename": "/GameData/Textures/lq_flesh/may_flesh5_1b.png", "start": 41159529, "end": 41212247}, {"filename": "/GameData/Textures/lq_flesh/may_flesh5_1c.png", "start": 41212247, "end": 41255889}, {"filename": "/GameData/Textures/lq_flesh/may_flesh5_1lit_fbr.png", "start": 41255889, "end": 41272823}, {"filename": "/GameData/Textures/lq_flesh/may_flesh_dr1a.png", "start": 41272823, "end": 41329716}, {"filename": "/GameData/Textures/lq_flesh/may_skin_eye.png", "start": 41329716, "end": 41340933}, {"filename": "/GameData/Textures/lq_flesh/meat-teeth0.png", "start": 41340933, "end": 41384304}, {"filename": "/GameData/Textures/lq_flesh/meat-teeth1.png", "start": 41384304, "end": 41429496}, {"filename": "/GameData/Textures/lq_flesh/meat_det1.png", "start": 41429496, "end": 41448227}, {"filename": "/GameData/Textures/lq_flesh/meat_det2.png", "start": 41448227, "end": 41470798}, {"filename": "/GameData/Textures/lq_flesh/meat_pipe1.png", "start": 41470798, "end": 41495222}, {"filename": "/GameData/Textures/lq_flesh/plus_0eye.png", "start": 41495222, "end": 41499367}, {"filename": "/GameData/Textures/lq_flesh/plus_0flesh2_gl.png", "start": 41499367, "end": 41514547}, {"filename": "/GameData/Textures/lq_flesh/plus_0flesh_but1_fbr.png", "start": 41514547, "end": 41518842}, {"filename": "/GameData/Textures/lq_flesh/plus_0flesh_but2_fbr.png", "start": 41518842, "end": 41523031}, {"filename": "/GameData/Textures/lq_flesh/plus_0flsh_vent.png", "start": 41523031, "end": 41537282}, {"filename": "/GameData/Textures/lq_flesh/plus_1eye.png", "start": 41537282, "end": 41541426}, {"filename": "/GameData/Textures/lq_flesh/plus_1flesh2_gl.png", "start": 41541426, "end": 41556585}, {"filename": "/GameData/Textures/lq_flesh/plus_1flesh_but2_fbr.png", "start": 41556585, "end": 41560780}, {"filename": "/GameData/Textures/lq_flesh/plus_2eye.png", "start": 41560780, "end": 41564820}, {"filename": "/GameData/Textures/lq_flesh/plus_2flesh2_gl.png", "start": 41564820, "end": 41579996}, {"filename": "/GameData/Textures/lq_flesh/plus_2flesh_but2_fbr.png", "start": 41579996, "end": 41584179}, {"filename": "/GameData/Textures/lq_flesh/plus_3eye.png", "start": 41584179, "end": 41588227}, {"filename": "/GameData/Textures/lq_flesh/plus_3flesh2_gl.png", "start": 41588227, "end": 41603401}, {"filename": "/GameData/Textures/lq_flesh/plus_3flesh_but2_fbr.png", "start": 41603401, "end": 41607572}, {"filename": "/GameData/Textures/lq_flesh/plus_4eye.png", "start": 41607572, "end": 41611691}, {"filename": "/GameData/Textures/lq_flesh/plus_4flesh2_gl.png", "start": 41611691, "end": 41626871}, {"filename": "/GameData/Textures/lq_flesh/plus_5eye.png", "start": 41626871, "end": 41630924}, {"filename": "/GameData/Textures/lq_flesh/plus_6eye.png", "start": 41630924, "end": 41634972}, {"filename": "/GameData/Textures/lq_flesh/plus_7eye.png", "start": 41634972, "end": 41639043}, {"filename": "/GameData/Textures/lq_flesh/plus_8eye.png", "start": 41639043, "end": 41643151}, {"filename": "/GameData/Textures/lq_flesh/plus_9eye.png", "start": 41643151, "end": 41647299}, {"filename": "/GameData/Textures/lq_flesh/plus_aeye.png", "start": 41647299, "end": 41651291}, {"filename": "/GameData/Textures/lq_flesh/plus_aflesh_but1.png", "start": 41651291, "end": 41655857}, {"filename": "/GameData/Textures/lq_flesh/plus_aflesh_but2.png", "start": 41655857, "end": 41660386}, {"filename": "/GameData/Textures/lq_flesh/plus_aflsh_vent.png", "start": 41660386, "end": 41674786}, {"filename": "/GameData/Textures/lq_greek/black.png", "start": 41674786, "end": 41674931}, {"filename": "/GameData/Textures/lq_greek/grk_arch1.png", "start": 41674931, "end": 41679948}, {"filename": "/GameData/Textures/lq_greek/grk_arch1_2.png", "start": 41679948, "end": 41685574}, {"filename": "/GameData/Textures/lq_greek/grk_arch1_a.png", "start": 41685574, "end": 41698349}, {"filename": "/GameData/Textures/lq_greek/grk_arch1_b.png", "start": 41698349, "end": 41711809}, {"filename": "/GameData/Textures/lq_greek/grk_arch1_c.png", "start": 41711809, "end": 41724982}, {"filename": "/GameData/Textures/lq_greek/grk_arch1_d.png", "start": 41724982, "end": 41738499}, {"filename": "/GameData/Textures/lq_greek/grk_arch2.png", "start": 41738499, "end": 41741781}, {"filename": "/GameData/Textures/lq_greek/grk_arch2_2.png", "start": 41741781, "end": 41745103}, {"filename": "/GameData/Textures/lq_greek/grk_arch2_a.png", "start": 41745103, "end": 41755506}, {"filename": "/GameData/Textures/lq_greek/grk_arch2_b.png", "start": 41755506, "end": 41766023}, {"filename": "/GameData/Textures/lq_greek/grk_arch2_c.png", "start": 41766023, "end": 41776152}, {"filename": "/GameData/Textures/lq_greek/grk_arch_end.png", "start": 41776152, "end": 41776908}, {"filename": "/GameData/Textures/lq_greek/grk_arch_tcap.png", "start": 41776908, "end": 41777622}, {"filename": "/GameData/Textures/lq_greek/grk_arch_trim.png", "start": 41777622, "end": 41779094}, {"filename": "/GameData/Textures/lq_greek/grk_bl_arch1a.png", "start": 41779094, "end": 41786729}, {"filename": "/GameData/Textures/lq_greek/grk_bl_arch1b.png", "start": 41786729, "end": 41795264}, {"filename": "/GameData/Textures/lq_greek/grk_bl_brk1a.png", "start": 41795264, "end": 41797736}, {"filename": "/GameData/Textures/lq_greek/grk_bl_brk1b.png", "start": 41797736, "end": 41799874}, {"filename": "/GameData/Textures/lq_greek/grk_bl_brk2a.png", "start": 41799874, "end": 41802295}, {"filename": "/GameData/Textures/lq_greek/grk_bl_brk2b.png", "start": 41802295, "end": 41804755}, {"filename": "/GameData/Textures/lq_greek/grk_bl_flt1.png", "start": 41804755, "end": 41806979}, {"filename": "/GameData/Textures/lq_greek/grk_bl_fsh1.png", "start": 41806979, "end": 41809905}, {"filename": "/GameData/Textures/lq_greek/grk_bl_fsh2.png", "start": 41809905, "end": 41817997}, {"filename": "/GameData/Textures/lq_greek/grk_bl_pil1.png", "start": 41817997, "end": 41820784}, {"filename": "/GameData/Textures/lq_greek/grk_bl_trim1.png", "start": 41820784, "end": 41823196}, {"filename": "/GameData/Textures/lq_greek/grk_bl_wll1.png", "start": 41823196, "end": 41825916}, {"filename": "/GameData/Textures/lq_greek/grk_bl_wll2.png", "start": 41825916, "end": 41828642}, {"filename": "/GameData/Textures/lq_greek/grk_bl_wll3a.png", "start": 41828642, "end": 41831000}, {"filename": "/GameData/Textures/lq_greek/grk_bl_wll3b.png", "start": 41831000, "end": 41833468}, {"filename": "/GameData/Textures/lq_greek/grk_bl_wll4b.png", "start": 41833468, "end": 41836034}, {"filename": "/GameData/Textures/lq_greek/grk_bl_wll5a.png", "start": 41836034, "end": 41838251}, {"filename": "/GameData/Textures/lq_greek/grk_bl_wll5b.png", "start": 41838251, "end": 41840550}, {"filename": "/GameData/Textures/lq_greek/grk_bl_wll5c.png", "start": 41840550, "end": 41842873}, {"filename": "/GameData/Textures/lq_greek/grk_bl_wll5d.png", "start": 41842873, "end": 41845280}, {"filename": "/GameData/Textures/lq_greek/grk_brk15.png", "start": 41845280, "end": 41854786}, {"filename": "/GameData/Textures/lq_greek/grk_brk15_b.png", "start": 41854786, "end": 41864034}, {"filename": "/GameData/Textures/lq_greek/grk_brk15_c.png", "start": 41864034, "end": 41880902}, {"filename": "/GameData/Textures/lq_greek/grk_brk15_f.png", "start": 41880902, "end": 41889195}, {"filename": "/GameData/Textures/lq_greek/grk_brk15_g.png", "start": 41889195, "end": 41896844}, {"filename": "/GameData/Textures/lq_greek/grk_brk16.png", "start": 41896844, "end": 41906120}, {"filename": "/GameData/Textures/lq_greek/grk_brk16_a.png", "start": 41906120, "end": 41915399}, {"filename": "/GameData/Textures/lq_greek/grk_brk16_f.png", "start": 41915399, "end": 41925104}, {"filename": "/GameData/Textures/lq_greek/grk_brk17.png", "start": 41925104, "end": 41960248}, {"filename": "/GameData/Textures/lq_greek/grk_brk17_f.png", "start": 41960248, "end": 41972850}, {"filename": "/GameData/Textures/lq_greek/grk_but1.png", "start": 41972850, "end": 41973799}, {"filename": "/GameData/Textures/lq_greek/grk_det1.png", "start": 41973799, "end": 41976872}, {"filename": "/GameData/Textures/lq_greek/grk_door1.png", "start": 41976872, "end": 41991979}, {"filename": "/GameData/Textures/lq_greek/grk_door1_f.png", "start": 41991979, "end": 42002014}, {"filename": "/GameData/Textures/lq_greek/grk_door2.png", "start": 42002014, "end": 42012874}, {"filename": "/GameData/Textures/lq_greek/grk_door3.png", "start": 42012874, "end": 42025531}, {"filename": "/GameData/Textures/lq_greek/grk_ebrick10.png", "start": 42025531, "end": 42036158}, {"filename": "/GameData/Textures/lq_greek/grk_ebrick10_bl.png", "start": 42036158, "end": 42050692}, {"filename": "/GameData/Textures/lq_greek/grk_ebrick22.png", "start": 42050692, "end": 42064558}, {"filename": "/GameData/Textures/lq_greek/grk_ebrick23.png", "start": 42064558, "end": 42073924}, {"filename": "/GameData/Textures/lq_greek/grk_ebrick24.png", "start": 42073924, "end": 42082117}, {"filename": "/GameData/Textures/lq_greek/grk_flr1.png", "start": 42082117, "end": 42082448}, {"filename": "/GameData/Textures/lq_greek/grk_flr2.png", "start": 42082448, "end": 42083302}, {"filename": "/GameData/Textures/lq_greek/grk_flr3.png", "start": 42083302, "end": 42085898}, {"filename": "/GameData/Textures/lq_greek/grk_flr4_1.png", "start": 42085898, "end": 42088028}, {"filename": "/GameData/Textures/lq_greek/grk_flr4_2.png", "start": 42088028, "end": 42090290}, {"filename": "/GameData/Textures/lq_greek/grk_flr4_3.png", "start": 42090290, "end": 42092653}, {"filename": "/GameData/Textures/lq_greek/grk_flr4_4.png", "start": 42092653, "end": 42094948}, {"filename": "/GameData/Textures/lq_greek/grk_flr4_5.png", "start": 42094948, "end": 42098580}, {"filename": "/GameData/Textures/lq_greek/grk_flr4_6.png", "start": 42098580, "end": 42101586}, {"filename": "/GameData/Textures/lq_greek/grk_flr4_8.png", "start": 42101586, "end": 42104921}, {"filename": "/GameData/Textures/lq_greek/grk_flr5_1.png", "start": 42104921, "end": 42107508}, {"filename": "/GameData/Textures/lq_greek/grk_flr5_2.png", "start": 42107508, "end": 42110002}, {"filename": "/GameData/Textures/lq_greek/grk_flr5_3.png", "start": 42110002, "end": 42112744}, {"filename": "/GameData/Textures/lq_greek/grk_flr5_4.png", "start": 42112744, "end": 42115047}, {"filename": "/GameData/Textures/lq_greek/grk_key_gl2.png", "start": 42115047, "end": 42116071}, {"filename": "/GameData/Textures/lq_greek/grk_key_sl2.png", "start": 42116071, "end": 42117072}, {"filename": "/GameData/Textures/lq_greek/grk_lion1.png", "start": 42117072, "end": 42127546}, {"filename": "/GameData/Textures/lq_greek/grk_lion2.png", "start": 42127546, "end": 42137213}, {"filename": "/GameData/Textures/lq_greek/grk_lion3.png", "start": 42137213, "end": 42147756}, {"filename": "/GameData/Textures/lq_greek/grk_lion4.png", "start": 42147756, "end": 42158908}, {"filename": "/GameData/Textures/lq_greek/grk_met1.png", "start": 42158908, "end": 42169289}, {"filename": "/GameData/Textures/lq_greek/grk_met1_trim.png", "start": 42169289, "end": 42180564}, {"filename": "/GameData/Textures/lq_greek/grk_met1a_trim.png", "start": 42180564, "end": 42191532}, {"filename": "/GameData/Textures/lq_greek/grk_met1b_trim.png", "start": 42191532, "end": 42205645}, {"filename": "/GameData/Textures/lq_greek/grk_met2_trim.png", "start": 42205645, "end": 42218914}, {"filename": "/GameData/Textures/lq_greek/grk_met_plt.png", "start": 42218914, "end": 42234305}, {"filename": "/GameData/Textures/lq_greek/grk_mural1.png", "start": 42234305, "end": 42254163}, {"filename": "/GameData/Textures/lq_greek/grk_mural2.png", "start": 42254163, "end": 42276880}, {"filename": "/GameData/Textures/lq_greek/grk_mural3.png", "start": 42276880, "end": 42366160}, {"filename": "/GameData/Textures/lq_greek/grk_pl1_a.png", "start": 42366160, "end": 42367976}, {"filename": "/GameData/Textures/lq_greek/grk_pl1_b.png", "start": 42367976, "end": 42370059}, {"filename": "/GameData/Textures/lq_greek/grk_pl2_a.png", "start": 42370059, "end": 42372291}, {"filename": "/GameData/Textures/lq_greek/grk_pl2_b.png", "start": 42372291, "end": 42374503}, {"filename": "/GameData/Textures/lq_greek/grk_plat1_side.png", "start": 42374503, "end": 42375720}, {"filename": "/GameData/Textures/lq_greek/grk_plat1_top.png", "start": 42375720, "end": 42379617}, {"filename": "/GameData/Textures/lq_greek/grk_tile2_1.png", "start": 42379617, "end": 42382051}, {"filename": "/GameData/Textures/lq_greek/grk_tile2_2.png", "start": 42382051, "end": 42384404}, {"filename": "/GameData/Textures/lq_greek/grk_trim1.png", "start": 42384404, "end": 42391161}, {"filename": "/GameData/Textures/lq_greek/grk_trim1_3.png", "start": 42391161, "end": 42397775}, {"filename": "/GameData/Textures/lq_greek/grk_trim1_3_s.png", "start": 42397775, "end": 42399815}, {"filename": "/GameData/Textures/lq_greek/grk_trim1_4_s.png", "start": 42399815, "end": 42401891}, {"filename": "/GameData/Textures/lq_greek/grk_trim1_5.png", "start": 42401891, "end": 42408827}, {"filename": "/GameData/Textures/lq_greek/grk_trim1_5_s.png", "start": 42408827, "end": 42411047}, {"filename": "/GameData/Textures/lq_greek/grk_trim1_6_s.png", "start": 42411047, "end": 42413324}, {"filename": "/GameData/Textures/lq_greek/grk_trim1_7_s.png", "start": 42413324, "end": 42416666}, {"filename": "/GameData/Textures/lq_greek/grk_trim2.png", "start": 42416666, "end": 42419027}, {"filename": "/GameData/Textures/lq_greek/grk_wall1.png", "start": 42419027, "end": 42422059}, {"filename": "/GameData/Textures/lq_greek/grk_wall2.png", "start": 42422059, "end": 42424730}, {"filename": "/GameData/Textures/lq_greek/grk_wall3.png", "start": 42424730, "end": 42430516}, {"filename": "/GameData/Textures/lq_greek/grk_wall3b.png", "start": 42430516, "end": 42457550}, {"filename": "/GameData/Textures/lq_greek/grk_win1_a.png", "start": 42457550, "end": 42466689}, {"filename": "/GameData/Textures/lq_greek/grk_win1_b.png", "start": 42466689, "end": 42476032}, {"filename": "/GameData/Textures/lq_greek/plus_0grk_but1_fbr.png", "start": 42476032, "end": 42477223}, {"filename": "/GameData/Textures/lq_greek/plus_0grk_hbut_fbr.png", "start": 42477223, "end": 42478246}, {"filename": "/GameData/Textures/lq_greek/plus_1grk_but1_fbr.png", "start": 42478246, "end": 42479421}, {"filename": "/GameData/Textures/lq_greek/plus_1grk_hbut_fbr.png", "start": 42479421, "end": 42480502}, {"filename": "/GameData/Textures/lq_greek/plus_2grk_but1_fbr.png", "start": 42480502, "end": 42481653}, {"filename": "/GameData/Textures/lq_greek/plus_2grk_hbut_fbr.png", "start": 42481653, "end": 42482693}, {"filename": "/GameData/Textures/lq_greek/plus_3grk_but1_fbr.png", "start": 42482693, "end": 42483846}, {"filename": "/GameData/Textures/lq_greek/plus_3grk_hbut_fbr.png", "start": 42483846, "end": 42484888}, {"filename": "/GameData/Textures/lq_greek/plus_agrk_but1.png", "start": 42484888, "end": 42485879}, {"filename": "/GameData/Textures/lq_greek/plus_agrk_hbut.png", "start": 42485879, "end": 42486857}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_bottom.png", "start": 42486857, "end": 42487787}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_c_b1_fbr.png", "start": 42487787, "end": 42488395}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_c_b2_fbr.png", "start": 42488395, "end": 42489007}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_c_b3_fbr.png", "start": 42489007, "end": 42489814}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_c_s1_fbr.png", "start": 42489814, "end": 42490407}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_c_s2_fbr.png", "start": 42490407, "end": 42491397}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_c_s3_fbr.png", "start": 42491397, "end": 42492030}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_fl.png", "start": 42492030, "end": 42492589}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_fl2.png", "start": 42492589, "end": 42493144}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_n_b1_fbr.png", "start": 42493144, "end": 42493898}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_n_b2_fbr.png", "start": 42493898, "end": 42494583}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_n_b3.png", "start": 42494583, "end": 42495154}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_n_s1.png", "start": 42495154, "end": 42495725}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_n_s2_fbr.png", "start": 42495725, "end": 42496380}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_n_s3_fbr.png", "start": 42496380, "end": 42496917}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_r_b1_fbr.png", "start": 42496917, "end": 42497632}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_r_b2_fbr.png", "start": 42497632, "end": 42498258}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_r_b3_fbr.png", "start": 42498258, "end": 42498716}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_r_s1_fbr.png", "start": 42498716, "end": 42499330}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_r_s2_fbr.png", "start": 42499330, "end": 42499679}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_s_b1_fbr.png", "start": 42499679, "end": 42500338}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_s_b2_fbr.png", "start": 42500338, "end": 42500966}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_s_b3.png", "start": 42500966, "end": 42501743}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_s_s1_fbr.png", "start": 42501743, "end": 42502249}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_s_s2_fbr.png", "start": 42502249, "end": 42502827}, {"filename": "/GameData/Textures/lq_health_ammo/ammo_s_s3_fbr.png", "start": 42502827, "end": 42503378}, {"filename": "/GameData/Textures/lq_health_ammo/ammobotsmall.png", "start": 42503378, "end": 42504046}, {"filename": "/GameData/Textures/lq_health_ammo/ammotop.png", "start": 42504046, "end": 42504791}, {"filename": "/GameData/Textures/lq_health_ammo/ammotopsmall.png", "start": 42504791, "end": 42505342}, {"filename": "/GameData/Textures/lq_health_ammo/boom.png", "start": 42505342, "end": 42506360}, {"filename": "/GameData/Textures/lq_health_ammo/boomammo_bottom.png", "start": 42506360, "end": 42506726}, {"filename": "/GameData/Textures/lq_health_ammo/boomammotop.png", "start": 42506726, "end": 42507068}, {"filename": "/GameData/Textures/lq_health_ammo/boomsmall.png", "start": 42507068, "end": 42507743}, {"filename": "/GameData/Textures/lq_health_ammo/epboxlarge_fbr.png", "start": 42507743, "end": 42508927}, {"filename": "/GameData/Textures/lq_health_ammo/epboxsmall_fbr.png", "start": 42508927, "end": 42509824}, {"filename": "/GameData/Textures/lq_health_ammo/explob_s2.png", "start": 42509824, "end": 42510320}, {"filename": "/GameData/Textures/lq_health_ammo/hp15_side.png", "start": 42510320, "end": 42510905}, {"filename": "/GameData/Textures/lq_health_ammo/hp25_top2.png", "start": 42510905, "end": 42511334}, {"filename": "/GameData/Textures/lq_health_ammo/hp_bottom.png", "start": 42511334, "end": 42512241}, {"filename": "/GameData/Textures/lq_health_ammo/hp_details.png", "start": 42512241, "end": 42512745}, {"filename": "/GameData/Textures/lq_health_ammo/nails.png", "start": 42512745, "end": 42513792}, {"filename": "/GameData/Textures/lq_health_ammo/nailssmall.png", "start": 42513792, "end": 42514533}, {"filename": "/GameData/Textures/lq_health_ammo/plus_0_hp100-winq_fbr.png", "start": 42514533, "end": 42517133}, {"filename": "/GameData/Textures/lq_health_ammo/plus_0_hp100_side_fbr.png", "start": 42517133, "end": 42518025}, {"filename": "/GameData/Textures/lq_health_ammo/plus_0_hp100_top_fbr.png", "start": 42518025, "end": 42518756}, {"filename": "/GameData/Textures/lq_health_ammo/plus_0_hp15_top_fbr.png", "start": 42518756, "end": 42519486}, {"filename": "/GameData/Textures/lq_health_ammo/plus_0_hp15winq_fbr.png", "start": 42519486, "end": 42522273}, {"filename": "/GameData/Textures/lq_health_ammo/plus_0_hp25-winq_fbr.png", "start": 42522273, "end": 42524827}, {"filename": "/GameData/Textures/lq_health_ammo/plus_0_hp25_side_fbr.png", "start": 42524827, "end": 42525649}, {"filename": "/GameData/Textures/lq_health_ammo/plus_0_hp25_top_fbr.png", "start": 42525649, "end": 42526383}, {"filename": "/GameData/Textures/lq_health_ammo/plus_0explob2_s1_fbr.png", "start": 42526383, "end": 42527486}, {"filename": "/GameData/Textures/lq_health_ammo/plus_0explob_s1_fbr.png", "start": 42527486, "end": 42528119}, {"filename": "/GameData/Textures/lq_health_ammo/plus_1_hp100-winq_fbr.png", "start": 42528119, "end": 42530724}, {"filename": "/GameData/Textures/lq_health_ammo/plus_1_hp100_side_fbr.png", "start": 42530724, "end": 42531618}, {"filename": "/GameData/Textures/lq_health_ammo/plus_1_hp100_top_fbr.png", "start": 42531618, "end": 42532352}, {"filename": "/GameData/Textures/lq_health_ammo/plus_1_hp15_top_fbr.png", "start": 42532352, "end": 42533086}, {"filename": "/GameData/Textures/lq_health_ammo/plus_1_hp15winq_fbr.png", "start": 42533086, "end": 42535872}, {"filename": "/GameData/Textures/lq_health_ammo/plus_1_hp25-winq_fbr.png", "start": 42535872, "end": 42538436}, {"filename": "/GameData/Textures/lq_health_ammo/plus_1_hp25_side_fbr.png", "start": 42538436, "end": 42539262}, {"filename": "/GameData/Textures/lq_health_ammo/plus_1_hp25_top_fbr.png", "start": 42539262, "end": 42540001}, {"filename": "/GameData/Textures/lq_health_ammo/plus_1explob2_s1_fbr.png", "start": 42540001, "end": 42541108}, {"filename": "/GameData/Textures/lq_health_ammo/plus_1explob_s1_fbr.png", "start": 42541108, "end": 42541741}, {"filename": "/GameData/Textures/lq_health_ammo/plus_2_hp100-winq_fbr.png", "start": 42541741, "end": 42544345}, {"filename": "/GameData/Textures/lq_health_ammo/plus_2_hp100_side_fbr.png", "start": 42544345, "end": 42545239}, {"filename": "/GameData/Textures/lq_health_ammo/plus_2_hp25-winq_fbr.png", "start": 42545239, "end": 42547797}, {"filename": "/GameData/Textures/lq_health_ammo/plus_2_hp25_side_fbr.png", "start": 42547797, "end": 42548622}, {"filename": "/GameData/Textures/lq_health_ammo/plus_2_hp25_top_fbr.png", "start": 42548622, "end": 42549360}, {"filename": "/GameData/Textures/lq_health_ammo/plus_2explob2_s1_fbr.png", "start": 42549360, "end": 42550456}, {"filename": "/GameData/Textures/lq_health_ammo/plus_2explob_s1_fbr.png", "start": 42550456, "end": 42551098}, {"filename": "/GameData/Textures/lq_health_ammo/plus_3_hp100-winq_fbr.png", "start": 42551098, "end": 42553708}, {"filename": "/GameData/Textures/lq_health_ammo/plus_3_hp100_side_fbr.png", "start": 42553708, "end": 42554605}, {"filename": "/GameData/Textures/lq_health_ammo/plus_3_hp25-winq_fbr.png", "start": 42554605, "end": 42557174}, {"filename": "/GameData/Textures/lq_health_ammo/plus_3_hp25_side_fbr.png", "start": 42557174, "end": 42558005}, {"filename": "/GameData/Textures/lq_health_ammo/plus_3_hp25_top_fbr.png", "start": 42558005, "end": 42558747}, {"filename": "/GameData/Textures/lq_health_ammo/plus_3explob2_s1_fbr.png", "start": 42558747, "end": 42559843}, {"filename": "/GameData/Textures/lq_health_ammo/plus_3explob_s1_fbr.png", "start": 42559843, "end": 42560485}, {"filename": "/GameData/Textures/lq_health_ammo/shells.png", "start": 42560485, "end": 42561431}, {"filename": "/GameData/Textures/lq_health_ammo/shellssmall.png", "start": 42561431, "end": 42562157}, {"filename": "/GameData/Textures/lq_health_ammo/zap.png", "start": 42562157, "end": 42563150}, {"filename": "/GameData/Textures/lq_health_ammo/zapsmall.png", "start": 42563150, "end": 42563914}, {"filename": "/GameData/Textures/lq_legacy/brick7.png", "start": 42563914, "end": 42566847}, {"filename": "/GameData/Textures/lq_legacy/brick8.png", "start": 42566847, "end": 42569934}, {"filename": "/GameData/Textures/lq_legacy/button_0.png", "start": 42569934, "end": 42573406}, {"filename": "/GameData/Textures/lq_legacy/button_0_grey.png", "start": 42573406, "end": 42576616}, {"filename": "/GameData/Textures/lq_legacy/button_0_grn.png", "start": 42576616, "end": 42579951}, {"filename": "/GameData/Textures/lq_legacy/button_1.png", "start": 42579951, "end": 42583406}, {"filename": "/GameData/Textures/lq_legacy/button_1_grey.png", "start": 42583406, "end": 42586601}, {"filename": "/GameData/Textures/lq_legacy/button_1_grn.png", "start": 42586601, "end": 42589902}, {"filename": "/GameData/Textures/lq_legacy/floor_temp.png", "start": 42589902, "end": 42591960}, {"filename": "/GameData/Textures/lq_legacy/flr.png", "start": 42591960, "end": 42594717}, {"filename": "/GameData/Textures/lq_legacy/gardgrass_1.png", "start": 42594717, "end": 42614887}, {"filename": "/GameData/Textures/lq_legacy/go-savgx.png", "start": 42614887, "end": 42616636}, {"filename": "/GameData/Textures/lq_legacy/grass.png", "start": 42616636, "end": 42620037}, {"filename": "/GameData/Textures/lq_legacy/grk_brk15_c_old.png", "start": 42620037, "end": 42630564}, {"filename": "/GameData/Textures/lq_legacy/grk_brk17_f_old.png", "start": 42630564, "end": 42639273}, {"filename": "/GameData/Textures/lq_legacy/grk_door1_old.png", "start": 42639273, "end": 42650290}, {"filename": "/GameData/Textures/lq_legacy/grk_door2_old.png", "start": 42650290, "end": 42659463}, {"filename": "/GameData/Textures/lq_legacy/grk_door3_old.png", "start": 42659463, "end": 42668269}, {"filename": "/GameData/Textures/lq_legacy/grk_ebrick22_old.png", "start": 42668269, "end": 42677586}, {"filename": "/GameData/Textures/lq_legacy/grk_trim1_7_s_old.png", "start": 42677586, "end": 42679791}, {"filename": "/GameData/Textures/lq_legacy/ground_1.png", "start": 42679791, "end": 42681972}, {"filename": "/GameData/Textures/lq_legacy/leaves.png", "start": 42681972, "end": 42685034}, {"filename": "/GameData/Textures/lq_legacy/marble1_4.png", "start": 42685034, "end": 42687937}, {"filename": "/GameData/Textures/lq_legacy/marble1_5.png", "start": 42687937, "end": 42691027}, {"filename": "/GameData/Textures/lq_legacy/med_cflat1_3.png", "start": 42691027, "end": 42693508}, {"filename": "/GameData/Textures/lq_legacy/med_csl_trm3.png", "start": 42693508, "end": 42694833}, {"filename": "/GameData/Textures/lq_legacy/med_csl_trm3b.png", "start": 42694833, "end": 42695823}, {"filename": "/GameData/Textures/lq_legacy/med_csl_trm3c.png", "start": 42695823, "end": 42697351}, {"filename": "/GameData/Textures/lq_legacy/med_flat10.png", "start": 42697351, "end": 42701244}, {"filename": "/GameData/Textures/lq_legacy/med_flat11.png", "start": 42701244, "end": 42704449}, {"filename": "/GameData/Textures/lq_legacy/med_flat13.png", "start": 42704449, "end": 42715226}, {"filename": "/GameData/Textures/lq_legacy/med_flat14.png", "start": 42715226, "end": 42725627}, {"filename": "/GameData/Textures/lq_legacy/med_flat9a.png", "start": 42725627, "end": 42769207}, {"filename": "/GameData/Textures/lq_legacy/med_flat9b.png", "start": 42769207, "end": 42811106}, {"filename": "/GameData/Textures/lq_legacy/med_plaster1.png", "start": 42811106, "end": 42819148}, {"filename": "/GameData/Textures/lq_legacy/med_ret_rock1.png", "start": 42819148, "end": 42822967}, {"filename": "/GameData/Textures/lq_legacy/med_ret_wood1_old.png", "start": 42822967, "end": 42830048}, {"filename": "/GameData/Textures/lq_legacy/med_rmet_key.png", "start": 42830048, "end": 42834009}, {"filename": "/GameData/Textures/lq_legacy/med_rock6.png", "start": 42834009, "end": 42879612}, {"filename": "/GameData/Textures/lq_legacy/med_rock7.png", "start": 42879612, "end": 42882496}, {"filename": "/GameData/Textures/lq_legacy/med_rock8.png", "start": 42882496, "end": 42884381}, {"filename": "/GameData/Textures/lq_legacy/med_rough_block.png", "start": 42884381, "end": 42933224}, {"filename": "/GameData/Textures/lq_legacy/med_rough_block_f.png", "start": 42933224, "end": 42987769}, {"filename": "/GameData/Textures/lq_legacy/med_tile.png", "start": 42987769, "end": 42989905}, {"filename": "/GameData/Textures/lq_legacy/med_wall1.png", "start": 42989905, "end": 42999770}, {"filename": "/GameData/Textures/lq_legacy/metground_1.png", "start": 42999770, "end": 43002701}, {"filename": "/GameData/Textures/lq_legacy/note-savgx.png", "start": 43002701, "end": 43021721}, {"filename": "/GameData/Textures/lq_legacy/plus_0button1.png", "start": 43021721, "end": 43025166}, {"filename": "/GameData/Textures/lq_legacy/plus_0button2_fbr.png", "start": 43025166, "end": 43026653}, {"filename": "/GameData/Textures/lq_legacy/plus_1button2_fbr.png", "start": 43026653, "end": 43028141}, {"filename": "/GameData/Textures/lq_legacy/plus_1button3.png", "start": 43028141, "end": 43030021}, {"filename": "/GameData/Textures/lq_legacy/plus_abutton1_fbr.png", "start": 43030021, "end": 43033623}, {"filename": "/GameData/Textures/lq_legacy/plus_abutton2_fbr.png", "start": 43033623, "end": 43035121}, {"filename": "/GameData/Textures/lq_legacy/readme.txt", "start": 43035121, "end": 43035197}, {"filename": "/GameData/Textures/lq_legacy/riktoiflat.png", "start": 43035197, "end": 43049316}, {"filename": "/GameData/Textures/lq_legacy/riktoiflat_blu.png", "start": 43049316, "end": 43062251}, {"filename": "/GameData/Textures/lq_legacy/riktoiflat_grn.png", "start": 43062251, "end": 43073905}, {"filename": "/GameData/Textures/lq_legacy/riktoilava.png", "start": 43073905, "end": 43078409}, {"filename": "/GameData/Textures/lq_legacy/riktoislime.png", "start": 43078409, "end": 43082114}, {"filename": "/GameData/Textures/lq_legacy/riktoitrim.png", "start": 43082114, "end": 43083601}, {"filename": "/GameData/Textures/lq_legacy/riktoitrim__purp.png", "start": 43083601, "end": 43085317}, {"filename": "/GameData/Textures/lq_legacy/riktoitrim_blu.png", "start": 43085317, "end": 43087047}, {"filename": "/GameData/Textures/lq_legacy/riktoiwall.png", "start": 43087047, "end": 43126844}, {"filename": "/GameData/Textures/lq_legacy/riktoiwall__purp.png", "start": 43126844, "end": 43162917}, {"filename": "/GameData/Textures/lq_legacy/riktoiwall_blu.png", "start": 43162917, "end": 43198978}, {"filename": "/GameData/Textures/lq_legacy/riktoiwater.png", "start": 43198978, "end": 43201869}, {"filename": "/GameData/Textures/lq_legacy/rune1_fbr.png", "start": 43201869, "end": 43204882}, {"filename": "/GameData/Textures/lq_legacy/rune2_fbr.png", "start": 43204882, "end": 43207948}, {"filename": "/GameData/Textures/lq_legacy/rune3_fbr.png", "start": 43207948, "end": 43211344}, {"filename": "/GameData/Textures/lq_legacy/rune4_fbr.png", "start": 43211344, "end": 43213896}, {"filename": "/GameData/Textures/lq_legacy/sign_easy.png", "start": 43213896, "end": 43218848}, {"filename": "/GameData/Textures/lq_legacy/sign_empty.png", "start": 43218848, "end": 43223605}, {"filename": "/GameData/Textures/lq_legacy/sign_hard.png", "start": 43223605, "end": 43228478}, {"filename": "/GameData/Textures/lq_legacy/sign_medium.png", "start": 43228478, "end": 43233488}, {"filename": "/GameData/Textures/lq_legacy/sign_metal_1.png", "start": 43233488, "end": 43234700}, {"filename": "/GameData/Textures/lq_legacy/sign_metal_2.png", "start": 43234700, "end": 43236295}, {"filename": "/GameData/Textures/lq_legacy/sign_nmare.png", "start": 43236295, "end": 43241316}, {"filename": "/GameData/Textures/lq_legacy/sky2.png", "start": 43241316, "end": 43256806}, {"filename": "/GameData/Textures/lq_legacy/sky4.png", "start": 43256806, "end": 43266318}, {"filename": "/GameData/Textures/lq_legacy/sky5_fbr.png", "start": 43266318, "end": 43283471}, {"filename": "/GameData/Textures/lq_legacy/sky5a.png", "start": 43283471, "end": 43292482}, {"filename": "/GameData/Textures/lq_legacy/sky7.png", "start": 43292482, "end": 43306397}, {"filename": "/GameData/Textures/lq_legacy/sky8.png", "start": 43306397, "end": 43323070}, {"filename": "/GameData/Textures/lq_legacy/sky8a_fbr.png", "start": 43323070, "end": 43331866}, {"filename": "/GameData/Textures/lq_legacy/tile.png", "start": 43331866, "end": 43340682}, {"filename": "/GameData/Textures/lq_legacy/tile_blu.png", "start": 43340682, "end": 43349688}, {"filename": "/GameData/Textures/lq_legacy/tile_grn.png", "start": 43349688, "end": 43357951}, {"filename": "/GameData/Textures/lq_legacy/trim2_blu.png", "start": 43357951, "end": 43359064}, {"filename": "/GameData/Textures/lq_legacy/trim2_grn.png", "start": 43359064, "end": 43360128}, {"filename": "/GameData/Textures/lq_legacy/ultrasteel1.png", "start": 43360128, "end": 43373018}, {"filename": "/GameData/Textures/lq_legacy/ultrasteel2.png", "start": 43373018, "end": 43384812}, {"filename": "/GameData/Textures/lq_legacy/vines1_old.png", "start": 43384812, "end": 43389666}, {"filename": "/GameData/Textures/lq_legacy/wiz1_4.png", "start": 43389666, "end": 43393487}, {"filename": "/GameData/Textures/lq_liquidsky/+0water_f3.png", "start": 43393487, "end": 43396428}, {"filename": "/GameData/Textures/lq_liquidsky/+1water_f3.png", "start": 43396428, "end": 43399352}, {"filename": "/GameData/Textures/lq_liquidsky/+2water_f3.png", "start": 43399352, "end": 43402270}, {"filename": "/GameData/Textures/lq_liquidsky/+3water_f3.png", "start": 43402270, "end": 43405122}, {"filename": "/GameData/Textures/lq_liquidsky/plus_0blood_f1.png", "start": 43405122, "end": 43406911}, {"filename": "/GameData/Textures/lq_liquidsky/plus_0fslime.png", "start": 43406911, "end": 43420867}, {"filename": "/GameData/Textures/lq_liquidsky/plus_0lava_fall3_fbr.png", "start": 43420867, "end": 43429860}, {"filename": "/GameData/Textures/lq_liquidsky/plus_0water_f1.png", "start": 43429860, "end": 43431709}, {"filename": "/GameData/Textures/lq_liquidsky/plus_0water_f2.png", "start": 43431709, "end": 43433527}, {"filename": "/GameData/Textures/lq_liquidsky/plus_0wfall0.png", "start": 43433527, "end": 43443136}, {"filename": "/GameData/Textures/lq_liquidsky/plus_1blood_f1.png", "start": 43443136, "end": 43444941}, {"filename": "/GameData/Textures/lq_liquidsky/plus_1fslime.png", "start": 43444941, "end": 43458684}, {"filename": "/GameData/Textures/lq_liquidsky/plus_1lava_fall3_fbr.png", "start": 43458684, "end": 43468251}, {"filename": "/GameData/Textures/lq_liquidsky/plus_1water_f1.png", "start": 43468251, "end": 43469899}, {"filename": "/GameData/Textures/lq_liquidsky/plus_1water_f2.png", "start": 43469899, "end": 43471728}, {"filename": "/GameData/Textures/lq_liquidsky/plus_1wfall0.png", "start": 43471728, "end": 43481379}, {"filename": "/GameData/Textures/lq_liquidsky/plus_2blood_f1.png", "start": 43481379, "end": 43483237}, {"filename": "/GameData/Textures/lq_liquidsky/plus_2fslime.png", "start": 43483237, "end": 43497131}, {"filename": "/GameData/Textures/lq_liquidsky/plus_2lava_fall3_fbr.png", "start": 43497131, "end": 43506680}, {"filename": "/GameData/Textures/lq_liquidsky/plus_2water_f1.png", "start": 43506680, "end": 43508451}, {"filename": "/GameData/Textures/lq_liquidsky/plus_2water_f2.png", "start": 43508451, "end": 43510337}, {"filename": "/GameData/Textures/lq_liquidsky/plus_2wfall0.png", "start": 43510337, "end": 43519999}, {"filename": "/GameData/Textures/lq_liquidsky/plus_3blood_f1.png", "start": 43519999, "end": 43521799}, {"filename": "/GameData/Textures/lq_liquidsky/plus_3fslime.png", "start": 43521799, "end": 43535482}, {"filename": "/GameData/Textures/lq_liquidsky/plus_3lava_fall3_fbr.png", "start": 43535482, "end": 43544819}, {"filename": "/GameData/Textures/lq_liquidsky/plus_3water_f1.png", "start": 43544819, "end": 43546470}, {"filename": "/GameData/Textures/lq_liquidsky/plus_3water_f2.png", "start": 43546470, "end": 43548297}, {"filename": "/GameData/Textures/lq_liquidsky/plus_3wfall0.png", "start": 43548297, "end": 43558000}, {"filename": "/GameData/Textures/lq_liquidsky/plus_4fslime.png", "start": 43558000, "end": 43571664}, {"filename": "/GameData/Textures/lq_liquidsky/plus_4lava_fall3_fbr.png", "start": 43571664, "end": 43581037}, {"filename": "/GameData/Textures/lq_liquidsky/plus_4wfall0.png", "start": 43581037, "end": 43590616}, {"filename": "/GameData/Textures/lq_liquidsky/plus_5fslime.png", "start": 43590616, "end": 43604384}, {"filename": "/GameData/Textures/lq_liquidsky/plus_5lava_fall3_fbr.png", "start": 43604384, "end": 43613747}, {"filename": "/GameData/Textures/lq_liquidsky/plus_5wfall0.png", "start": 43613747, "end": 43623346}, {"filename": "/GameData/Textures/lq_liquidsky/plus_6fslime.png", "start": 43623346, "end": 43637198}, {"filename": "/GameData/Textures/lq_liquidsky/plus_6lava_fall3_fbr.png", "start": 43637198, "end": 43646583}, {"filename": "/GameData/Textures/lq_liquidsky/plus_6wfall0.png", "start": 43646583, "end": 43656168}, {"filename": "/GameData/Textures/lq_liquidsky/plus_7fslime.png", "start": 43656168, "end": 43670019}, {"filename": "/GameData/Textures/lq_liquidsky/plus_7lava_fall3_fbr.png", "start": 43670019, "end": 43679534}, {"filename": "/GameData/Textures/lq_liquidsky/plus_7wfall0.png", "start": 43679534, "end": 43689203}, {"filename": "/GameData/Textures/lq_liquidsky/plus_8wfall0.png", "start": 43689203, "end": 43698865}, {"filename": "/GameData/Textures/lq_liquidsky/plus_9wfall0.png", "start": 43698865, "end": 43708487}, {"filename": "/GameData/Textures/lq_liquidsky/sky-test.png", "start": 43708487, "end": 43726083}, {"filename": "/GameData/Textures/lq_liquidsky/sky-test.xcf", "start": 43726083, "end": 43859348}, {"filename": "/GameData/Textures/lq_liquidsky/sky5_blu.png", "start": 43859348, "end": 43872661}, {"filename": "/GameData/Textures/lq_liquidsky/sky5_dismal.png", "start": 43872661, "end": 43885844}, {"filename": "/GameData/Textures/lq_liquidsky/sky_galx_fbr.png", "start": 43885844, "end": 43908599}, {"filename": "/GameData/Textures/lq_liquidsky/sky_galx_spark_fbr.png", "start": 43908599, "end": 43928157}, {"filename": "/GameData/Textures/lq_liquidsky/sky_orng.png", "start": 43928157, "end": 43942796}, {"filename": "/GameData/Textures/lq_liquidsky/sky_pando.png", "start": 43942796, "end": 43958904}, {"filename": "/GameData/Textures/lq_liquidsky/sky_pando2.png", "start": 43958904, "end": 43975439}, {"filename": "/GameData/Textures/lq_liquidsky/sky_star.png", "start": 43975439, "end": 43977099}, {"filename": "/GameData/Textures/lq_liquidsky/sky_void.png", "start": 43977099, "end": 43977794}, {"filename": "/GameData/Textures/lq_liquidsky/sky_wfog_fbr.png", "start": 43977794, "end": 43978700}, {"filename": "/GameData/Textures/lq_liquidsky/star_acid.png", "start": 43978700, "end": 43980879}, {"filename": "/GameData/Textures/lq_liquidsky/star_blood1.png", "start": 43980879, "end": 43982952}, {"filename": "/GameData/Textures/lq_liquidsky/star_lava1_fbr.png", "start": 43982952, "end": 43986157}, {"filename": "/GameData/Textures/lq_liquidsky/star_lava2_fbr.png", "start": 43986157, "end": 43990239}, {"filename": "/GameData/Textures/lq_liquidsky/star_lava3_fbr.png", "start": 43990239, "end": 43994208}, {"filename": "/GameData/Textures/lq_liquidsky/star_lava_void_fbr.png", "start": 43994208, "end": 43997898}, {"filename": "/GameData/Textures/lq_liquidsky/star_lavaskip.png", "start": 43997898, "end": 43999011}, {"filename": "/GameData/Textures/lq_liquidsky/star_meatgoo2_fbr.png", "start": 43999011, "end": 44002199}, {"filename": "/GameData/Textures/lq_liquidsky/star_meatgoo_fbr.png", "start": 44002199, "end": 44004996}, {"filename": "/GameData/Textures/lq_liquidsky/star_slime1.png", "start": 44004996, "end": 44011287}, {"filename": "/GameData/Textures/lq_liquidsky/star_slime2.png", "start": 44011287, "end": 44014476}, {"filename": "/GameData/Textures/lq_liquidsky/star_slime3.png", "start": 44014476, "end": 44016508}, {"filename": "/GameData/Textures/lq_liquidsky/star_slime_soul.png", "start": 44016508, "end": 44019298}, {"filename": "/GameData/Textures/lq_liquidsky/star_slimeskip.png", "start": 44019298, "end": 44020371}, {"filename": "/GameData/Textures/lq_liquidsky/star_soul_drain.png", "start": 44020371, "end": 44023339}, {"filename": "/GameData/Textures/lq_liquidsky/star_tele1_fbr.png", "start": 44023339, "end": 44025219}, {"filename": "/GameData/Textures/lq_liquidsky/star_tele2_fbr.png", "start": 44025219, "end": 44028020}, {"filename": "/GameData/Textures/lq_liquidsky/star_tele3_fbr.png", "start": 44028020, "end": 44031056}, {"filename": "/GameData/Textures/lq_liquidsky/star_tele4_fbr.png", "start": 44031056, "end": 44033748}, {"filename": "/GameData/Textures/lq_liquidsky/star_water0.png", "start": 44033748, "end": 44036596}, {"filename": "/GameData/Textures/lq_liquidsky/star_water1.png", "start": 44036596, "end": 44039705}, {"filename": "/GameData/Textures/lq_liquidsky/star_water2.png", "start": 44039705, "end": 44041965}, {"filename": "/GameData/Textures/lq_liquidsky/star_water3.png", "start": 44041965, "end": 44044346}, {"filename": "/GameData/Textures/lq_liquidsky/star_water4.png", "start": 44044346, "end": 44049133}, {"filename": "/GameData/Textures/lq_liquidsky/star_waterskip.png", "start": 44049133, "end": 44050873}, {"filename": "/GameData/Textures/lq_liquidsky/star_wstill0.png", "start": 44050873, "end": 44053712}, {"filename": "/GameData/Textures/lq_mayan/btn1.png", "start": 44053712, "end": 44056630}, {"filename": "/GameData/Textures/lq_mayan/may_arrow.png", "start": 44056630, "end": 44057511}, {"filename": "/GameData/Textures/lq_mayan/may_blok1_1.png", "start": 44057511, "end": 44060032}, {"filename": "/GameData/Textures/lq_mayan/may_blok1_2.png", "start": 44060032, "end": 44062223}, {"filename": "/GameData/Textures/lq_mayan/may_blok1_2_m.png", "start": 44062223, "end": 44065319}, {"filename": "/GameData/Textures/lq_mayan/may_blok1_m.png", "start": 44065319, "end": 44074593}, {"filename": "/GameData/Textures/lq_mayan/may_blok2_1.png", "start": 44074593, "end": 44077157}, {"filename": "/GameData/Textures/lq_mayan/may_blok2_1_m.png", "start": 44077157, "end": 44080378}, {"filename": "/GameData/Textures/lq_mayan/may_blok2_2.png", "start": 44080378, "end": 44090641}, {"filename": "/GameData/Textures/lq_mayan/may_blok2_2_m.png", "start": 44090641, "end": 44103069}, {"filename": "/GameData/Textures/lq_mayan/may_blud1_1.png", "start": 44103069, "end": 44105661}, {"filename": "/GameData/Textures/lq_mayan/may_blud1_1m.png", "start": 44105661, "end": 44108881}, {"filename": "/GameData/Textures/lq_mayan/may_bnd1_1.png", "start": 44108881, "end": 44111316}, {"filename": "/GameData/Textures/lq_mayan/may_bnd1_1m.png", "start": 44111316, "end": 44114386}, {"filename": "/GameData/Textures/lq_mayan/may_bnd1_2.png", "start": 44114386, "end": 44116813}, {"filename": "/GameData/Textures/lq_mayan/may_bnd1_2m.png", "start": 44116813, "end": 44119841}, {"filename": "/GameData/Textures/lq_mayan/may_bnd1_3.png", "start": 44119841, "end": 44122100}, {"filename": "/GameData/Textures/lq_mayan/may_bnd1_3m.png", "start": 44122100, "end": 44125509}, {"filename": "/GameData/Textures/lq_mayan/may_bnd1_5.png", "start": 44125509, "end": 44128362}, {"filename": "/GameData/Textures/lq_mayan/may_bnd1_5m.png", "start": 44128362, "end": 44132011}, {"filename": "/GameData/Textures/lq_mayan/may_bnd_skull.png", "start": 44132011, "end": 44134643}, {"filename": "/GameData/Textures/lq_mayan/may_brk1_0.png", "start": 44134643, "end": 44137747}, {"filename": "/GameData/Textures/lq_mayan/may_brk1_0m.png", "start": 44137747, "end": 44140804}, {"filename": "/GameData/Textures/lq_mayan/may_brk1_1.png", "start": 44140804, "end": 44143486}, {"filename": "/GameData/Textures/lq_mayan/may_brk1_1m.png", "start": 44143486, "end": 44146685}, {"filename": "/GameData/Textures/lq_mayan/may_brk1_2.png", "start": 44146685, "end": 44149636}, {"filename": "/GameData/Textures/lq_mayan/may_brk1_2m.png", "start": 44149636, "end": 44153041}, {"filename": "/GameData/Textures/lq_mayan/may_brk1_3.png", "start": 44153041, "end": 44155978}, {"filename": "/GameData/Textures/lq_mayan/may_brk1_3m.png", "start": 44155978, "end": 44159154}, {"filename": "/GameData/Textures/lq_mayan/may_brk2_0.png", "start": 44159154, "end": 44169009}, {"filename": "/GameData/Textures/lq_mayan/may_brk2_0_m.png", "start": 44169009, "end": 44181342}, {"filename": "/GameData/Textures/lq_mayan/may_brk_old.png", "start": 44181342, "end": 44183941}, {"filename": "/GameData/Textures/lq_mayan/may_brk_oldm.png", "start": 44183941, "end": 44187280}, {"filename": "/GameData/Textures/lq_mayan/may_deco1_1.png", "start": 44187280, "end": 44198509}, {"filename": "/GameData/Textures/lq_mayan/may_deco1_1m.png", "start": 44198509, "end": 44212544}, {"filename": "/GameData/Textures/lq_mayan/may_deco1_2.png", "start": 44212544, "end": 44215064}, {"filename": "/GameData/Textures/lq_mayan/may_deco1_2my.png", "start": 44215064, "end": 44218452}, {"filename": "/GameData/Textures/lq_mayan/may_deco1_3.png", "start": 44218452, "end": 44237025}, {"filename": "/GameData/Textures/lq_mayan/may_deco1_3m.png", "start": 44237025, "end": 44257376}, {"filename": "/GameData/Textures/lq_mayan/may_door1_1.png", "start": 44257376, "end": 44266843}, {"filename": "/GameData/Textures/lq_mayan/may_door1_1m.png", "start": 44266843, "end": 44277934}, {"filename": "/GameData/Textures/lq_mayan/may_door2_1.png", "start": 44277934, "end": 44289902}, {"filename": "/GameData/Textures/lq_mayan/may_door2_2.png", "start": 44289902, "end": 44301680}, {"filename": "/GameData/Textures/lq_mayan/may_drt2_1.png", "start": 44301680, "end": 44304517}, {"filename": "/GameData/Textures/lq_mayan/may_flr1_1.png", "start": 44304517, "end": 44307422}, {"filename": "/GameData/Textures/lq_mayan/may_flr1_2.png", "start": 44307422, "end": 44310645}, {"filename": "/GameData/Textures/lq_mayan/may_flt1_1.png", "start": 44310645, "end": 44312934}, {"filename": "/GameData/Textures/lq_mayan/may_flt1_1m.png", "start": 44312934, "end": 44316382}, {"filename": "/GameData/Textures/lq_mayan/may_key1_1.png", "start": 44316382, "end": 44317969}, {"filename": "/GameData/Textures/lq_mayan/may_key1_2.png", "start": 44317969, "end": 44319966}, {"filename": "/GameData/Textures/lq_mayan/may_lite1_1_fbr.png", "start": 44319966, "end": 44320896}, {"filename": "/GameData/Textures/lq_mayan/may_lite1_2.png", "start": 44320896, "end": 44321259}, {"filename": "/GameData/Textures/lq_mayan/may_lite2_1.png", "start": 44321259, "end": 44324196}, {"filename": "/GameData/Textures/lq_mayan/may_lite2_2.png", "start": 44324196, "end": 44324682}, {"filename": "/GameData/Textures/lq_mayan/may_lite3_1_fbr.png", "start": 44324682, "end": 44325639}, {"filename": "/GameData/Textures/lq_mayan/may_lite_f1.png", "start": 44325639, "end": 44326125}, {"filename": "/GameData/Textures/lq_mayan/may_oldmtomb1_1_fbr.png", "start": 44326125, "end": 44339977}, {"filename": "/GameData/Textures/lq_mayan/may_oldtomb1_2_fbr.png", "start": 44339977, "end": 44353869}, {"filename": "/GameData/Textures/lq_mayan/may_plat_stem_m.png", "start": 44353869, "end": 44354735}, {"filename": "/GameData/Textures/lq_mayan/may_plats.png", "start": 44354735, "end": 44357397}, {"filename": "/GameData/Textures/lq_mayan/may_platst.png", "start": 44357397, "end": 44359824}, {"filename": "/GameData/Textures/lq_mayan/may_platt.png", "start": 44359824, "end": 44362447}, {"filename": "/GameData/Textures/lq_mayan/may_plr1_1.png", "start": 44362447, "end": 44365641}, {"filename": "/GameData/Textures/lq_mayan/may_tomb1_1_fbr.png", "start": 44365641, "end": 44379493}, {"filename": "/GameData/Textures/lq_mayan/may_tomb1_2_fbr.png", "start": 44379493, "end": 44393385}, {"filename": "/GameData/Textures/lq_mayan/may_trm1_1.png", "start": 44393385, "end": 44402313}, {"filename": "/GameData/Textures/lq_mayan/may_trm1_2.png", "start": 44402313, "end": 44405018}, {"filename": "/GameData/Textures/lq_mayan/may_trm1_a.png", "start": 44405018, "end": 44407360}, {"filename": "/GameData/Textures/lq_mayan/may_tskull.png", "start": 44407360, "end": 44418366}, {"filename": "/GameData/Textures/lq_mayan/may_wall1_1.png", "start": 44418366, "end": 44421348}, {"filename": "/GameData/Textures/lq_mayan/may_wall1_2.png", "start": 44421348, "end": 44424314}, {"filename": "/GameData/Textures/lq_mayan/may_wall1_3.png", "start": 44424314, "end": 44427508}, {"filename": "/GameData/Textures/lq_mayan/may_wall1_3a.png", "start": 44427508, "end": 44430089}, {"filename": "/GameData/Textures/lq_mayan/may_wall1_4.png", "start": 44430089, "end": 44433077}, {"filename": "/GameData/Textures/lq_mayan/may_wall1_4a.png", "start": 44433077, "end": 44435476}, {"filename": "/GameData/Textures/lq_mayan/maya_end_dr1.png", "start": 44435476, "end": 44439862}, {"filename": "/GameData/Textures/lq_mayan/maya_end_dr2.png", "start": 44439862, "end": 44444247}, {"filename": "/GameData/Textures/lq_mayan/maya_end_trim1.png", "start": 44444247, "end": 44448637}, {"filename": "/GameData/Textures/lq_mayan/plus_0_may_btn1.png", "start": 44448637, "end": 44451554}, {"filename": "/GameData/Textures/lq_mayan/plus_0_may_mpiloilon_fbr.png", "start": 44451554, "end": 44453112}, {"filename": "/GameData/Textures/lq_mayan/plus_0_may_mpilon_fbr.png", "start": 44453112, "end": 44454687}, {"filename": "/GameData/Textures/lq_mayan/plus_0_may_mshoohoot_fbr.png", "start": 44454687, "end": 44455632}, {"filename": "/GameData/Textures/lq_mayan/plus_0_may_mshoot_fbr.png", "start": 44455632, "end": 44456575}, {"filename": "/GameData/Textures/lq_mayan/plus_1_may_btn1.png", "start": 44456575, "end": 44459483}, {"filename": "/GameData/Textures/lq_mayan/plus_1_may_mpiloilon_fbr.png", "start": 44459483, "end": 44461056}, {"filename": "/GameData/Textures/lq_mayan/plus_1_may_mpilon_fbr.png", "start": 44461056, "end": 44462649}, {"filename": "/GameData/Textures/lq_mayan/plus_1_may_mshoohoot_fbr.png", "start": 44462649, "end": 44463634}, {"filename": "/GameData/Textures/lq_mayan/plus_1_may_mshoot_fbr.png", "start": 44463634, "end": 44464618}, {"filename": "/GameData/Textures/lq_mayan/plus_2_may_btn1.png", "start": 44464618, "end": 44467509}, {"filename": "/GameData/Textures/lq_mayan/plus_2_may_mpilon.png", "start": 44467509, "end": 44469086}, {"filename": "/GameData/Textures/lq_mayan/plus_2_may_mshoohoot_fbr.png", "start": 44469086, "end": 44470111}, {"filename": "/GameData/Textures/lq_mayan/plus_2_may_mshoot_fbr.png", "start": 44470111, "end": 44471138}, {"filename": "/GameData/Textures/lq_mayan/plus_3_may_btn1.png", "start": 44471138, "end": 44474200}, {"filename": "/GameData/Textures/lq_mayan/plus_3_may_mpiloilon_fbr.png", "start": 44474200, "end": 44475773}, {"filename": "/GameData/Textures/lq_mayan/plus_3_may_mpilon_fbr.png", "start": 44475773, "end": 44477366}, {"filename": "/GameData/Textures/lq_mayan/plus_3_may_mshoohoot_fbr.png", "start": 44477366, "end": 44478351}, {"filename": "/GameData/Textures/lq_mayan/plus_3_may_mshoot_fbr.png", "start": 44478351, "end": 44479335}, {"filename": "/GameData/Textures/lq_mayan/plus_4_may_btn1.png", "start": 44479335, "end": 44482398}, {"filename": "/GameData/Textures/lq_mayan/plus_5_may_btn1.png", "start": 44482398, "end": 44485139}, {"filename": "/GameData/Textures/lq_mayan/plus_a_may_btn1.png", "start": 44485139, "end": 44487862}, {"filename": "/GameData/Textures/lq_mayan/plus_a_may_mpiloilon_fbr.png", "start": 44487862, "end": 44489439}, {"filename": "/GameData/Textures/lq_mayan/plus_a_may_mpilon_fbr.png", "start": 44489439, "end": 44491023}, {"filename": "/GameData/Textures/lq_mayan/plus_a_may_mshoot.png", "start": 44491023, "end": 44491972}, {"filename": "/GameData/Textures/lq_medieval/+0med_but1.png", "start": 44491972, "end": 44496227}, {"filename": "/GameData/Textures/lq_medieval/+0med_but2.png", "start": 44496227, "end": 44497939}, {"filename": "/GameData/Textures/lq_medieval/+0med_but3.png", "start": 44497939, "end": 44499642}, {"filename": "/GameData/Textures/lq_medieval/+0med_but_s1.png", "start": 44499642, "end": 44503888}, {"filename": "/GameData/Textures/lq_medieval/+0med_sht_but1.png", "start": 44503888, "end": 44505181}, {"filename": "/GameData/Textures/lq_medieval/+1med_but3.png", "start": 44505181, "end": 44506883}, {"filename": "/GameData/Textures/lq_medieval/+1med_but_s1.png", "start": 44506883, "end": 44511102}, {"filename": "/GameData/Textures/lq_medieval/+1med_sht_but1.png", "start": 44511102, "end": 44512415}, {"filename": "/GameData/Textures/lq_medieval/+2med_but_s1.png", "start": 44512415, "end": 44516689}, {"filename": "/GameData/Textures/lq_medieval/+3med_but_s1.png", "start": 44516689, "end": 44520908}, {"filename": "/GameData/Textures/lq_medieval/+amed_but1.png", "start": 44520908, "end": 44524991}, {"filename": "/GameData/Textures/lq_medieval/+amed_but2.png", "start": 44524991, "end": 44526709}, {"filename": "/GameData/Textures/lq_medieval/+amed_but3.png", "start": 44526709, "end": 44528319}, {"filename": "/GameData/Textures/lq_medieval/+amed_but_s1.png", "start": 44528319, "end": 44532532}, {"filename": "/GameData/Textures/lq_medieval/+amed_sht_but1.png", "start": 44532532, "end": 44533688}, {"filename": "/GameData/Textures/lq_medieval/Art1.png", "start": 44533688, "end": 44666681}, {"filename": "/GameData/Textures/lq_medieval/afloor1_4.png", "start": 44666681, "end": 44669682}, {"filename": "/GameData/Textures/lq_medieval/afloor1_8.png", "start": 44669682, "end": 44672522}, {"filename": "/GameData/Textures/lq_medieval/afloor3_1.png", "start": 44672522, "end": 44675332}, {"filename": "/GameData/Textures/lq_medieval/altar1_1.png", "start": 44675332, "end": 44679220}, {"filename": "/GameData/Textures/lq_medieval/altar1_3.png", "start": 44679220, "end": 44683701}, {"filename": "/GameData/Textures/lq_medieval/altar1_4.png", "start": 44683701, "end": 44687754}, {"filename": "/GameData/Textures/lq_medieval/brick0.png", "start": 44687754, "end": 44695976}, {"filename": "/GameData/Textures/lq_medieval/brick1.png", "start": 44695976, "end": 44706730}, {"filename": "/GameData/Textures/lq_medieval/brick4_s.png", "start": 44706730, "end": 44708663}, {"filename": "/GameData/Textures/lq_medieval/brown1.png", "start": 44708663, "end": 44717839}, {"filename": "/GameData/Textures/lq_medieval/med_block_1a.png", "start": 44717839, "end": 44764194}, {"filename": "/GameData/Textures/lq_medieval/med_block_1b.png", "start": 44764194, "end": 44847558}, {"filename": "/GameData/Textures/lq_medieval/med_block_1c.png", "start": 44847558, "end": 44945729}, {"filename": "/GameData/Textures/lq_medieval/med_block_1d.png", "start": 44945729, "end": 45054398}, {"filename": "/GameData/Textures/lq_medieval/med_block_1e.png", "start": 45054398, "end": 45162891}, {"filename": "/GameData/Textures/lq_medieval/med_block_1f.png", "start": 45162891, "end": 45230750}, {"filename": "/GameData/Textures/lq_medieval/med_block_1s.png", "start": 45230750, "end": 45348201}, {"filename": "/GameData/Textures/lq_medieval/med_block_2a.png", "start": 45348201, "end": 45425585}, {"filename": "/GameData/Textures/lq_medieval/med_block_2b.png", "start": 45425585, "end": 45503024}, {"filename": "/GameData/Textures/lq_medieval/med_block_2c.png", "start": 45503024, "end": 45591899}, {"filename": "/GameData/Textures/lq_medieval/med_block_2d.png", "start": 45591899, "end": 45690093}, {"filename": "/GameData/Textures/lq_medieval/med_block_2e.png", "start": 45690093, "end": 45788330}, {"filename": "/GameData/Textures/lq_medieval/med_block_2f.png", "start": 45788330, "end": 45852874}, {"filename": "/GameData/Textures/lq_medieval/med_block_2s.png", "start": 45852874, "end": 45958626}, {"filename": "/GameData/Textures/lq_medieval/med_brk9_ceil1a.png", "start": 45958626, "end": 45970434}, {"filename": "/GameData/Textures/lq_medieval/med_brk9_ceil1b.png", "start": 45970434, "end": 45982969}, {"filename": "/GameData/Textures/lq_medieval/med_brk9_wal2a.png", "start": 45982969, "end": 45995866}, {"filename": "/GameData/Textures/lq_medieval/med_brk9_wal2b.png", "start": 45995866, "end": 46009122}, {"filename": "/GameData/Textures/lq_medieval/med_brk9_wal2c.png", "start": 46009122, "end": 46021564}, {"filename": "/GameData/Textures/lq_medieval/med_brk9_win1.png", "start": 46021564, "end": 46064562}, {"filename": "/GameData/Textures/lq_medieval/med_brk9_win1b.png", "start": 46064562, "end": 46109888}, {"filename": "/GameData/Textures/lq_medieval/med_but_side.png", "start": 46109888, "end": 46110886}, {"filename": "/GameData/Textures/lq_medieval/med_cmet1.png", "start": 46110886, "end": 46114010}, {"filename": "/GameData/Textures/lq_medieval/med_cmet2a.png", "start": 46114010, "end": 46117161}, {"filename": "/GameData/Textures/lq_medieval/med_cmet2b.png", "start": 46117161, "end": 46120223}, {"filename": "/GameData/Textures/lq_medieval/med_cmet2c.png", "start": 46120223, "end": 46123360}, {"filename": "/GameData/Textures/lq_medieval/med_cmet3a.png", "start": 46123360, "end": 46126632}, {"filename": "/GameData/Textures/lq_medieval/med_cmet3b.png", "start": 46126632, "end": 46129814}, {"filename": "/GameData/Textures/lq_medieval/med_cmet4.png", "start": 46129814, "end": 46132655}, {"filename": "/GameData/Textures/lq_medieval/med_cmet5a.png", "start": 46132655, "end": 46135793}, {"filename": "/GameData/Textures/lq_medieval/med_cmet5c.png", "start": 46135793, "end": 46138936}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk10.png", "start": 46138936, "end": 46153662}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk10_f.png", "start": 46153662, "end": 46165950}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk10b.png", "start": 46165950, "end": 46180513}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk11.png", "start": 46180513, "end": 46191761}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk12.png", "start": 46191761, "end": 46208710}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk12_f.png", "start": 46208710, "end": 46226043}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk13.png", "start": 46226043, "end": 46282160}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk14.png", "start": 46282160, "end": 46298014}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk14_f.png", "start": 46298014, "end": 46310983}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk14b.png", "start": 46310983, "end": 46324473}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk15.png", "start": 46324473, "end": 46338299}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk15b.png", "start": 46338299, "end": 46352229}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk15f.png", "start": 46352229, "end": 46365427}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk16.png", "start": 46365427, "end": 46381648}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk16b.png", "start": 46381648, "end": 46399240}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk16f.png", "start": 46399240, "end": 46416511}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk17.png", "start": 46416511, "end": 46426618}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk17_f.png", "start": 46426618, "end": 46435643}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk17b.png", "start": 46435643, "end": 46445780}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk18_f.png", "start": 46445780, "end": 46454413}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk18_gb.png", "start": 46454413, "end": 46457161}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk18_gt.png", "start": 46457161, "end": 46459898}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk18_t.png", "start": 46459898, "end": 46468084}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk18_tb.png", "start": 46468084, "end": 46478710}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk18_tc.png", "start": 46478710, "end": 46483239}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk18b.png", "start": 46483239, "end": 46491412}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk19_f.png", "start": 46491412, "end": 46503113}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk19_t.png", "start": 46503113, "end": 46514504}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk19b.png", "start": 46514504, "end": 46525824}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk1_1.png", "start": 46525824, "end": 46528309}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk1_2.png", "start": 46528309, "end": 46531480}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk1_3.png", "start": 46531480, "end": 46533723}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk2_1.png", "start": 46533723, "end": 46536272}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk2_2.png", "start": 46536272, "end": 46539645}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk5.png", "start": 46539645, "end": 46542812}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk6_1.png", "start": 46542812, "end": 46545455}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk6_2.png", "start": 46545455, "end": 46550517}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk7_1.png", "start": 46550517, "end": 46553157}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk7_1b.png", "start": 46553157, "end": 46555701}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk7_2.png", "start": 46555701, "end": 46558867}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk8_1c.png", "start": 46558867, "end": 46571472}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk8_1d.png", "start": 46571472, "end": 46584075}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk9_1.png", "start": 46584075, "end": 46597285}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk9_1b.png", "start": 46597285, "end": 46610942}, {"filename": "/GameData/Textures/lq_medieval/med_csl_brk9_f.png", "start": 46610942, "end": 46623729}, {"filename": "/GameData/Textures/lq_medieval/med_csl_flr1_1.png", "start": 46623729, "end": 46633676}, {"filename": "/GameData/Textures/lq_medieval/med_csl_flr2_1.png", "start": 46633676, "end": 46635962}, {"filename": "/GameData/Textures/lq_medieval/med_csl_flr2_2.png", "start": 46635962, "end": 46638135}, {"filename": "/GameData/Textures/lq_medieval/med_csl_flr4_1.png", "start": 46638135, "end": 46641875}, {"filename": "/GameData/Textures/lq_medieval/med_csl_flr4_3.png", "start": 46641875, "end": 46645309}, {"filename": "/GameData/Textures/lq_medieval/med_csl_flr4_4.png", "start": 46645309, "end": 46658365}, {"filename": "/GameData/Textures/lq_medieval/med_csl_flr4_5.png", "start": 46658365, "end": 46672883}, {"filename": "/GameData/Textures/lq_medieval/med_csl_flr5_1.png", "start": 46672883, "end": 46687813}, {"filename": "/GameData/Textures/lq_medieval/med_csl_flr5_2.png", "start": 46687813, "end": 46703341}, {"filename": "/GameData/Textures/lq_medieval/med_csl_stp1.png", "start": 46703341, "end": 46705938}, {"filename": "/GameData/Textures/lq_medieval/med_csl_stp2.png", "start": 46705938, "end": 46708481}, {"filename": "/GameData/Textures/lq_medieval/med_csl_trm1.png", "start": 46708481, "end": 46709806}, {"filename": "/GameData/Textures/lq_medieval/med_dbrick1.png", "start": 46709806, "end": 46731279}, {"filename": "/GameData/Textures/lq_medieval/med_dbrick1_t.png", "start": 46731279, "end": 46742666}, {"filename": "/GameData/Textures/lq_medieval/med_dbrick1_t2.png", "start": 46742666, "end": 46755828}, {"filename": "/GameData/Textures/lq_medieval/med_dbrick1_t2b_fbr.png", "start": 46755828, "end": 46769778}, {"filename": "/GameData/Textures/lq_medieval/med_dbrick1_t3.png", "start": 46769778, "end": 46781518}, {"filename": "/GameData/Textures/lq_medieval/med_dbrick1_t4.png", "start": 46781518, "end": 46794294}, {"filename": "/GameData/Textures/lq_medieval/med_dbrick2.png", "start": 46794294, "end": 46799828}, {"filename": "/GameData/Textures/lq_medieval/med_dbrick3.png", "start": 46799828, "end": 46805202}, {"filename": "/GameData/Textures/lq_medieval/med_dbrick4.png", "start": 46805202, "end": 46810510}, {"filename": "/GameData/Textures/lq_medieval/med_dbrick5.png", "start": 46810510, "end": 46816332}, {"filename": "/GameData/Textures/lq_medieval/med_dbrick6.png", "start": 46816332, "end": 46825493}, {"filename": "/GameData/Textures/lq_medieval/med_dbrick6b.png", "start": 46825493, "end": 46834501}, {"filename": "/GameData/Textures/lq_medieval/med_dbrick6f.png", "start": 46834501, "end": 46843791}, {"filename": "/GameData/Textures/lq_medieval/med_door1.png", "start": 46843791, "end": 46848474}, {"filename": "/GameData/Textures/lq_medieval/med_door2.png", "start": 46848474, "end": 46852973}, {"filename": "/GameData/Textures/lq_medieval/med_door3.png", "start": 46852973, "end": 46857622}, {"filename": "/GameData/Textures/lq_medieval/med_door3b.png", "start": 46857622, "end": 46860163}, {"filename": "/GameData/Textures/lq_medieval/med_door4.png", "start": 46860163, "end": 46864727}, {"filename": "/GameData/Textures/lq_medieval/med_door4b.png", "start": 46864727, "end": 46867323}, {"filename": "/GameData/Textures/lq_medieval/med_dr1a.png", "start": 46867323, "end": 46886107}, {"filename": "/GameData/Textures/lq_medieval/med_dr1a_blu.png", "start": 46886107, "end": 46904471}, {"filename": "/GameData/Textures/lq_medieval/med_dr1b.png", "start": 46904471, "end": 46928620}, {"filename": "/GameData/Textures/lq_medieval/med_dr1b_blu.png", "start": 46928620, "end": 46948239}, {"filename": "/GameData/Textures/lq_medieval/med_dr2a.png", "start": 46948239, "end": 46972616}, {"filename": "/GameData/Textures/lq_medieval/med_dr2a_blu.png", "start": 46972616, "end": 46992510}, {"filename": "/GameData/Textures/lq_medieval/med_dr3a.png", "start": 46992510, "end": 47005993}, {"filename": "/GameData/Textures/lq_medieval/med_dr3a_blu.png", "start": 47005993, "end": 47019283}, {"filename": "/GameData/Textures/lq_medieval/med_dr3b.png", "start": 47019283, "end": 47033591}, {"filename": "/GameData/Textures/lq_medieval/med_dr3b_blu.png", "start": 47033591, "end": 47047797}, {"filename": "/GameData/Textures/lq_medieval/med_dr3c.png", "start": 47047797, "end": 47060640}, {"filename": "/GameData/Textures/lq_medieval/med_dr3c_blu.png", "start": 47060640, "end": 47073764}, {"filename": "/GameData/Textures/lq_medieval/med_dwall1.png", "start": 47073764, "end": 47076240}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick1.png", "start": 47076240, "end": 47088822}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick10.png", "start": 47088822, "end": 47099707}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick10b.png", "start": 47099707, "end": 47110193}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick11.png", "start": 47110193, "end": 47125452}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick12.png", "start": 47125452, "end": 47142869}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick12b.png", "start": 47142869, "end": 47156108}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick13.png", "start": 47156108, "end": 47166006}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick14.png", "start": 47166006, "end": 47175021}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick15.png", "start": 47175021, "end": 47189822}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick16.png", "start": 47189822, "end": 47201186}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick16b.png", "start": 47201186, "end": 47209504}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick17.png", "start": 47209504, "end": 47222638}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick17b.png", "start": 47222638, "end": 47234904}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick17c.png", "start": 47234904, "end": 47248163}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick18.png", "start": 47248163, "end": 47264163}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick18b.png", "start": 47264163, "end": 47278551}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick2.png", "start": 47278551, "end": 47291043}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick20.png", "start": 47291043, "end": 47304443}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick21.png", "start": 47304443, "end": 47316585}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick22.png", "start": 47316585, "end": 47328746}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick3.png", "start": 47328746, "end": 47341206}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick4.png", "start": 47341206, "end": 47356756}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick5.png", "start": 47356756, "end": 47370572}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick6.png", "start": 47370572, "end": 47385283}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick7.png", "start": 47385283, "end": 47398928}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick8.png", "start": 47398928, "end": 47411667}, {"filename": "/GameData/Textures/lq_medieval/med_ebrick9.png", "start": 47411667, "end": 47426222}, {"filename": "/GameData/Textures/lq_medieval/med_etrim1.png", "start": 47426222, "end": 47429551}, {"filename": "/GameData/Textures/lq_medieval/med_glass1.png", "start": 47429551, "end": 47441930}, {"filename": "/GameData/Textures/lq_medieval/med_glass2.png", "start": 47441930, "end": 47453587}, {"filename": "/GameData/Textures/lq_medieval/med_glass3.png", "start": 47453587, "end": 47464602}, {"filename": "/GameData/Textures/lq_medieval/med_glass4.png", "start": 47464602, "end": 47496421}, {"filename": "/GameData/Textures/lq_medieval/med_glass5.png", "start": 47496421, "end": 47506203}, {"filename": "/GameData/Textures/lq_medieval/med_met_dec1.png", "start": 47506203, "end": 47510343}, {"filename": "/GameData/Textures/lq_medieval/med_met_key1a.png", "start": 47510343, "end": 47512103}, {"filename": "/GameData/Textures/lq_medieval/med_met_key1b.png", "start": 47512103, "end": 47513437}, {"filename": "/GameData/Textures/lq_medieval/med_met_key2a.png", "start": 47513437, "end": 47515197}, {"filename": "/GameData/Textures/lq_medieval/med_met_key2b.png", "start": 47515197, "end": 47516903}, {"filename": "/GameData/Textures/lq_medieval/med_met_trim1.png", "start": 47516903, "end": 47517930}, {"filename": "/GameData/Textures/lq_medieval/med_met_trim2.png", "start": 47517930, "end": 47518877}, {"filename": "/GameData/Textures/lq_medieval/med_met_trim3.png", "start": 47518877, "end": 47519751}, {"filename": "/GameData/Textures/lq_medieval/med_metw1a.png", "start": 47519751, "end": 47532059}, {"filename": "/GameData/Textures/lq_medieval/med_metw1b.png", "start": 47532059, "end": 47540766}, {"filename": "/GameData/Textures/lq_medieval/med_metw2a.png", "start": 47540766, "end": 47550673}, {"filename": "/GameData/Textures/lq_medieval/med_metw2b.png", "start": 47550673, "end": 47563051}, {"filename": "/GameData/Textures/lq_medieval/med_rmet.png", "start": 47563051, "end": 47576152}, {"filename": "/GameData/Textures/lq_medieval/med_rmet_slat.png", "start": 47576152, "end": 47590463}, {"filename": "/GameData/Textures/lq_medieval/med_rmet_tile.png", "start": 47590463, "end": 47604395}, {"filename": "/GameData/Textures/lq_medieval/med_rmet_trim32.png", "start": 47604395, "end": 47617604}, {"filename": "/GameData/Textures/lq_medieval/med_roof1.png", "start": 47617604, "end": 47629519}, {"filename": "/GameData/Textures/lq_medieval/med_roof2.png", "start": 47629519, "end": 47644592}, {"filename": "/GameData/Textures/lq_medieval/med_roof3.png", "start": 47644592, "end": 47659521}, {"filename": "/GameData/Textures/lq_medieval/med_roof4.png", "start": 47659521, "end": 47677335}, {"filename": "/GameData/Textures/lq_medieval/med_roof5.png", "start": 47677335, "end": 47687841}, {"filename": "/GameData/Textures/lq_medieval/med_tanwall1.png", "start": 47687841, "end": 47729519}, {"filename": "/GameData/Textures/lq_medieval/med_tanwall2.png", "start": 47729519, "end": 47789296}, {"filename": "/GameData/Textures/lq_medieval/med_tanwall3.png", "start": 47789296, "end": 47839731}, {"filename": "/GameData/Textures/lq_medieval/med_tanwall4.png", "start": 47839731, "end": 47888872}, {"filename": "/GameData/Textures/lq_medieval/med_tanwall4_f.png", "start": 47888872, "end": 47932825}, {"filename": "/GameData/Textures/lq_medieval/med_tanwall6.png", "start": 47932825, "end": 47977370}, {"filename": "/GameData/Textures/lq_medieval/med_tanwall7.png", "start": 47977370, "end": 48022963}, {"filename": "/GameData/Textures/lq_medieval/med_tanwall8.png", "start": 48022963, "end": 48060561}, {"filename": "/GameData/Textures/lq_medieval/med_tanwall9.png", "start": 48060561, "end": 48109641}, {"filename": "/GameData/Textures/lq_medieval/med_tanwall9_f.png", "start": 48109641, "end": 48153620}, {"filename": "/GameData/Textures/lq_medieval/med_telepad.png", "start": 48153620, "end": 48158247}, {"filename": "/GameData/Textures/lq_medieval/med_tile1.png", "start": 48158247, "end": 48230858}, {"filename": "/GameData/Textures/lq_medieval/med_tmpl_lit1_fbr.png", "start": 48230858, "end": 48232461}, {"filename": "/GameData/Textures/lq_medieval/med_tmpl_lit3_fbr.png", "start": 48232461, "end": 48233766}, {"filename": "/GameData/Textures/lq_medieval/med_tmpl_tele.png", "start": 48233766, "end": 48236948}, {"filename": "/GameData/Textures/lq_medieval/med_tmpl_trim1.png", "start": 48236948, "end": 48240114}, {"filename": "/GameData/Textures/lq_medieval/med_tmpl_trim1b.png", "start": 48240114, "end": 48242169}, {"filename": "/GameData/Textures/lq_medieval/med_tmpl_trim2.png", "start": 48242169, "end": 48245709}, {"filename": "/GameData/Textures/lq_medieval/med_tmpl_trim3.png", "start": 48245709, "end": 48249186}, {"filename": "/GameData/Textures/lq_medieval/med_tmpl_trim4.png", "start": 48249186, "end": 48252299}, {"filename": "/GameData/Textures/lq_medieval/med_trim1_1.png", "start": 48252299, "end": 48261003}, {"filename": "/GameData/Textures/lq_medieval/med_trim2_1.png", "start": 48261003, "end": 48262117}, {"filename": "/GameData/Textures/lq_medieval/med_trim3_1.png", "start": 48262117, "end": 48265712}, {"filename": "/GameData/Textures/lq_medieval/med_trim3_2.png", "start": 48265712, "end": 48269271}, {"filename": "/GameData/Textures/lq_medieval/med_trim3_3.png", "start": 48269271, "end": 48272968}, {"filename": "/GameData/Textures/lq_medieval/med_trim3_4.png", "start": 48272968, "end": 48276339}, {"filename": "/GameData/Textures/lq_medieval/med_trim4_1.png", "start": 48276339, "end": 48279763}, {"filename": "/GameData/Textures/lq_medieval/med_trim4_2.png", "start": 48279763, "end": 48283287}, {"filename": "/GameData/Textures/lq_medieval/med_trim4_3.png", "start": 48283287, "end": 48286647}, {"filename": "/GameData/Textures/lq_medieval/med_trim4_4.png", "start": 48286647, "end": 48289870}, {"filename": "/GameData/Textures/lq_medieval/plus_0_csl_brk14.png", "start": 48289870, "end": 48306183}, {"filename": "/GameData/Textures/lq_medieval/plus_1_csl_brk14.png", "start": 48306183, "end": 48322476}, {"filename": "/GameData/Textures/lq_medieval/plus_2_csl_brk14.png", "start": 48322476, "end": 48338775}, {"filename": "/GameData/Textures/lq_medieval/plus_3_csl_brk14.png", "start": 48338775, "end": 48355017}, {"filename": "/GameData/Textures/lq_medieval/plus_4_csl_brk14.png", "start": 48355017, "end": 48371358}, {"filename": "/GameData/Textures/lq_medieval/sidewalk.png", "start": 48371358, "end": 48380114}, {"filename": "/GameData/Textures/lq_medieval/sq_trim1_2.png", "start": 48380114, "end": 48388396}, {"filename": "/GameData/Textures/lq_medieval/sq_trim1_2_s.png", "start": 48388396, "end": 48390971}, {"filename": "/GameData/Textures/lq_medieval/tile.png", "start": 48390971, "end": 48394096}, {"filename": "/GameData/Textures/lq_medieval/tile1.png", "start": 48394096, "end": 48466709}, {"filename": "/GameData/Textures/lq_medieval/wall14_5.png", "start": 48466709, "end": 48470789}, {"filename": "/GameData/Textures/lq_medieval/wbrick1_5.png", "start": 48470789, "end": 48474555}, {"filename": "/GameData/Textures/lq_medieval/wswamp2_1.png", "start": 48474555, "end": 48477681}, {"filename": "/GameData/Textures/lq_medieval/wswamp2_2.png", "start": 48477681, "end": 48481458}, {"filename": "/GameData/Textures/lq_metal/gig1_bone.png", "start": 48481458, "end": 48491139}, {"filename": "/GameData/Textures/lq_metal/gig1_bone_l.png", "start": 48491139, "end": 48521800}, {"filename": "/GameData/Textures/lq_metal/gig1_skull.png", "start": 48521800, "end": 48524464}, {"filename": "/GameData/Textures/lq_metal/gig1_skull_l.png", "start": 48524464, "end": 48533249}, {"filename": "/GameData/Textures/lq_metal/gig1_spine.png", "start": 48533249, "end": 48581950}, {"filename": "/GameData/Textures/lq_metal/gig2_bone.png", "start": 48581950, "end": 48590373}, {"filename": "/GameData/Textures/lq_metal/gig2_bone_l.png", "start": 48590373, "end": 48616121}, {"filename": "/GameData/Textures/lq_metal/gig2_bone_s.png", "start": 48616121, "end": 48618734}, {"filename": "/GameData/Textures/lq_metal/gig2_mouth_s.png", "start": 48618734, "end": 48621149}, {"filename": "/GameData/Textures/lq_metal/med_flat8.png", "start": 48621149, "end": 48624007}, {"filename": "/GameData/Textures/lq_metal/med_flat9.png", "start": 48624007, "end": 48626995}, {"filename": "/GameData/Textures/lq_metal/met_blc_block.png", "start": 48626995, "end": 48630334}, {"filename": "/GameData/Textures/lq_metal/met_blc_diam.png", "start": 48630334, "end": 48633224}, {"filename": "/GameData/Textures/lq_metal/met_blc_trim28.png", "start": 48633224, "end": 48638267}, {"filename": "/GameData/Textures/lq_metal/met_blc_trim32.png", "start": 48638267, "end": 48641413}, {"filename": "/GameData/Textures/lq_metal/met_blc_trim32r.png", "start": 48641413, "end": 48644668}, {"filename": "/GameData/Textures/lq_metal/met_blc_trim32s.png", "start": 48644668, "end": 48647768}, {"filename": "/GameData/Textures/lq_metal/met_blc_trim64.png", "start": 48647768, "end": 48650911}, {"filename": "/GameData/Textures/lq_metal/met_blu_block.png", "start": 48650911, "end": 48655931}, {"filename": "/GameData/Textures/lq_metal/met_blu_det1.png", "start": 48655931, "end": 48657087}, {"filename": "/GameData/Textures/lq_metal/met_blu_diam.png", "start": 48657087, "end": 48659984}, {"filename": "/GameData/Textures/lq_metal/met_blu_diam2.png", "start": 48659984, "end": 48662779}, {"filename": "/GameData/Textures/lq_metal/met_blu_diamc.png", "start": 48662779, "end": 48666310}, {"filename": "/GameData/Textures/lq_metal/met_blu_door1.png", "start": 48666310, "end": 48670993}, {"filename": "/GameData/Textures/lq_metal/met_blu_door2.png", "start": 48670993, "end": 48675492}, {"filename": "/GameData/Textures/lq_metal/met_blu_door3.png", "start": 48675492, "end": 48680141}, {"filename": "/GameData/Textures/lq_metal/met_blu_door4.png", "start": 48680141, "end": 48684705}, {"filename": "/GameData/Textures/lq_metal/met_blu_door5.png", "start": 48684705, "end": 48687648}, {"filename": "/GameData/Textures/lq_metal/met_blu_door6.png", "start": 48687648, "end": 48690244}, {"filename": "/GameData/Textures/lq_metal/met_blu_fac1.png", "start": 48690244, "end": 48692646}, {"filename": "/GameData/Textures/lq_metal/met_blu_flat.png", "start": 48692646, "end": 48696080}, {"filename": "/GameData/Textures/lq_metal/met_blu_flatst.png", "start": 48696080, "end": 48698309}, {"filename": "/GameData/Textures/lq_metal/met_blu_gig1.png", "start": 48698309, "end": 48702428}, {"filename": "/GameData/Textures/lq_metal/met_blu_gig2.png", "start": 48702428, "end": 48706041}, {"filename": "/GameData/Textures/lq_metal/met_blu_gig2b.png", "start": 48706041, "end": 48712235}, {"filename": "/GameData/Textures/lq_metal/met_blu_grate.png", "start": 48712235, "end": 48715290}, {"filename": "/GameData/Textures/lq_metal/met_blu_grate2.png", "start": 48715290, "end": 48718121}, {"filename": "/GameData/Textures/lq_metal/met_blu_grate3.png", "start": 48718121, "end": 48719781}, {"filename": "/GameData/Textures/lq_metal/met_blu_lit1_fbr.png", "start": 48719781, "end": 48721852}, {"filename": "/GameData/Textures/lq_metal/met_blu_lit2_fbr.png", "start": 48721852, "end": 48723041}, {"filename": "/GameData/Textures/lq_metal/met_blu_lit3.png", "start": 48723041, "end": 48724227}, {"filename": "/GameData/Textures/lq_metal/met_blu_lit4.png", "start": 48724227, "end": 48725488}, {"filename": "/GameData/Textures/lq_metal/met_blu_lit5.png", "start": 48725488, "end": 48727581}, {"filename": "/GameData/Textures/lq_metal/met_blu_pan1.png", "start": 48727581, "end": 48731063}, {"filename": "/GameData/Textures/lq_metal/met_blu_pan2.png", "start": 48731063, "end": 48733349}, {"filename": "/GameData/Textures/lq_metal/met_blu_pan3.png", "start": 48733349, "end": 48736776}, {"filename": "/GameData/Textures/lq_metal/met_blu_rect.png", "start": 48736776, "end": 48740555}, {"filename": "/GameData/Textures/lq_metal/met_blu_rivg.png", "start": 48740555, "end": 48744476}, {"filename": "/GameData/Textures/lq_metal/met_blu_rivs.png", "start": 48744476, "end": 48747834}, {"filename": "/GameData/Textures/lq_metal/met_blu_slat.png", "start": 48747834, "end": 48751620}, {"filename": "/GameData/Textures/lq_metal/met_blu_sqr.png", "start": 48751620, "end": 48755391}, {"filename": "/GameData/Textures/lq_metal/met_blu_sqrd.png", "start": 48755391, "end": 48759229}, {"filename": "/GameData/Textures/lq_metal/met_blu_sqrs.png", "start": 48759229, "end": 48762983}, {"filename": "/GameData/Textures/lq_metal/met_blu_stile.png", "start": 48762983, "end": 48766210}, {"filename": "/GameData/Textures/lq_metal/met_blu_tile.png", "start": 48766210, "end": 48769659}, {"filename": "/GameData/Textures/lq_metal/met_blu_trim16.png", "start": 48769659, "end": 48773363}, {"filename": "/GameData/Textures/lq_metal/met_blu_trim16g.png", "start": 48773363, "end": 48777113}, {"filename": "/GameData/Textures/lq_metal/met_blu_trim16h.png", "start": 48777113, "end": 48780730}, {"filename": "/GameData/Textures/lq_metal/met_blu_trim16s.png", "start": 48780730, "end": 48783898}, {"filename": "/GameData/Textures/lq_metal/met_blu_trim28.png", "start": 48783898, "end": 48789195}, {"filename": "/GameData/Textures/lq_metal/met_blu_trim32.png", "start": 48789195, "end": 48792645}, {"filename": "/GameData/Textures/lq_metal/met_blu_trim32r.png", "start": 48792645, "end": 48796301}, {"filename": "/GameData/Textures/lq_metal/met_blu_trim32s.png", "start": 48796301, "end": 48799625}, {"filename": "/GameData/Textures/lq_metal/met_blu_trim64.png", "start": 48799625, "end": 48803102}, {"filename": "/GameData/Textures/lq_metal/met_blu_vtrim.png", "start": 48803102, "end": 48806672}, {"filename": "/GameData/Textures/lq_metal/met_brn2_pat.png", "start": 48806672, "end": 48810521}, {"filename": "/GameData/Textures/lq_metal/met_brn_block.png", "start": 48810521, "end": 48814232}, {"filename": "/GameData/Textures/lq_metal/met_brn_blockl.png", "start": 48814232, "end": 48817761}, {"filename": "/GameData/Textures/lq_metal/met_brn_det1.png", "start": 48817761, "end": 48818924}, {"filename": "/GameData/Textures/lq_metal/met_brn_flat.png", "start": 48818924, "end": 48822445}, {"filename": "/GameData/Textures/lq_metal/met_brn_grate.png", "start": 48822445, "end": 48825833}, {"filename": "/GameData/Textures/lq_metal/met_brn_grate2.png", "start": 48825833, "end": 48829339}, {"filename": "/GameData/Textures/lq_metal/met_brn_grate3.png", "start": 48829339, "end": 48831357}, {"filename": "/GameData/Textures/lq_metal/met_brn_lit1_fbr.png", "start": 48831357, "end": 48833470}, {"filename": "/GameData/Textures/lq_metal/met_brn_lit2_fbr.png", "start": 48833470, "end": 48834658}, {"filename": "/GameData/Textures/lq_metal/met_brn_lit3.png", "start": 48834658, "end": 48835829}, {"filename": "/GameData/Textures/lq_metal/met_brn_lit4.png", "start": 48835829, "end": 48837039}, {"filename": "/GameData/Textures/lq_metal/met_brn_lit5.png", "start": 48837039, "end": 48839092}, {"filename": "/GameData/Textures/lq_metal/met_brn_pan1.png", "start": 48839092, "end": 48842555}, {"filename": "/GameData/Textures/lq_metal/met_brn_pan2.png", "start": 48842555, "end": 48844831}, {"filename": "/GameData/Textures/lq_metal/met_brn_pan3.png", "start": 48844831, "end": 48848308}, {"filename": "/GameData/Textures/lq_metal/met_brn_pan4.png", "start": 48848308, "end": 48852171}, {"filename": "/GameData/Textures/lq_metal/met_brn_rect.png", "start": 48852171, "end": 48855849}, {"filename": "/GameData/Textures/lq_metal/met_brn_rivg.png", "start": 48855849, "end": 48859877}, {"filename": "/GameData/Textures/lq_metal/met_brn_rivs.png", "start": 48859877, "end": 48863300}, {"filename": "/GameData/Textures/lq_metal/met_brn_signs.png", "start": 48863300, "end": 48868134}, {"filename": "/GameData/Textures/lq_metal/met_brn_slat.png", "start": 48868134, "end": 48871777}, {"filename": "/GameData/Textures/lq_metal/met_brn_sqr.png", "start": 48871777, "end": 48875468}, {"filename": "/GameData/Textures/lq_metal/met_brn_sqrd.png", "start": 48875468, "end": 48879276}, {"filename": "/GameData/Textures/lq_metal/met_brn_sqrs.png", "start": 48879276, "end": 48882980}, {"filename": "/GameData/Textures/lq_metal/met_brn_stile.png", "start": 48882980, "end": 48887065}, {"filename": "/GameData/Textures/lq_metal/met_brn_tile.png", "start": 48887065, "end": 48890446}, {"filename": "/GameData/Textures/lq_metal/met_brn_tile2.png", "start": 48890446, "end": 48893779}, {"filename": "/GameData/Textures/lq_metal/met_brn_trim16.png", "start": 48893779, "end": 48897885}, {"filename": "/GameData/Textures/lq_metal/met_brn_trim16g.png", "start": 48897885, "end": 48902089}, {"filename": "/GameData/Textures/lq_metal/met_brn_trim16h.png", "start": 48902089, "end": 48906286}, {"filename": "/GameData/Textures/lq_metal/met_brn_trim16s.png", "start": 48906286, "end": 48909909}, {"filename": "/GameData/Textures/lq_metal/met_brn_trim32.png", "start": 48909909, "end": 48913396}, {"filename": "/GameData/Textures/lq_metal/met_brn_trim32s.png", "start": 48913396, "end": 48916782}, {"filename": "/GameData/Textures/lq_metal/met_brn_trim64.png", "start": 48916782, "end": 48920422}, {"filename": "/GameData/Textures/lq_metal/met_brn_vtrim.png", "start": 48920422, "end": 48923875}, {"filename": "/GameData/Textures/lq_metal/met_cop_flat.png", "start": 48923875, "end": 48926952}, {"filename": "/GameData/Textures/lq_metal/met_cop_riv.png", "start": 48926952, "end": 48930553}, {"filename": "/GameData/Textures/lq_metal/met_dbrn_flat.png", "start": 48930553, "end": 48933933}, {"filename": "/GameData/Textures/lq_metal/met_dbrn_rect.png", "start": 48933933, "end": 48937818}, {"filename": "/GameData/Textures/lq_metal/met_dbrn_slat.png", "start": 48937818, "end": 48941633}, {"filename": "/GameData/Textures/lq_metal/met_grate.png", "start": 48941633, "end": 48944983}, {"filename": "/GameData/Textures/lq_metal/met_grn_block.png", "start": 48944983, "end": 48948908}, {"filename": "/GameData/Textures/lq_metal/met_grn_blockl.png", "start": 48948908, "end": 48952637}, {"filename": "/GameData/Textures/lq_metal/met_grn_det1.png", "start": 48952637, "end": 48953888}, {"filename": "/GameData/Textures/lq_metal/met_grn_fac1.png", "start": 48953888, "end": 48956798}, {"filename": "/GameData/Textures/lq_metal/met_grn_flat.png", "start": 48956798, "end": 48960683}, {"filename": "/GameData/Textures/lq_metal/met_grn_grate.png", "start": 48960683, "end": 48963745}, {"filename": "/GameData/Textures/lq_metal/met_grn_grate2.png", "start": 48963745, "end": 48966728}, {"filename": "/GameData/Textures/lq_metal/met_grn_grate3.png", "start": 48966728, "end": 48968472}, {"filename": "/GameData/Textures/lq_metal/met_grn_lit1_fbr.png", "start": 48968472, "end": 48970606}, {"filename": "/GameData/Textures/lq_metal/met_grn_lit2_fbr.png", "start": 48970606, "end": 48971807}, {"filename": "/GameData/Textures/lq_metal/met_grn_lit3.png", "start": 48971807, "end": 48973019}, {"filename": "/GameData/Textures/lq_metal/met_grn_lit4.png", "start": 48973019, "end": 48974305}, {"filename": "/GameData/Textures/lq_metal/met_grn_lit5.png", "start": 48974305, "end": 48976497}, {"filename": "/GameData/Textures/lq_metal/met_grn_pan1.png", "start": 48976497, "end": 48980213}, {"filename": "/GameData/Textures/lq_metal/met_grn_pan2.png", "start": 48980213, "end": 48982578}, {"filename": "/GameData/Textures/lq_metal/met_grn_pan3.png", "start": 48982578, "end": 48986178}, {"filename": "/GameData/Textures/lq_metal/met_grn_rect.png", "start": 48986178, "end": 48990215}, {"filename": "/GameData/Textures/lq_metal/met_grn_rivg.png", "start": 48990215, "end": 48994627}, {"filename": "/GameData/Textures/lq_metal/met_grn_rivs.png", "start": 48994627, "end": 48998394}, {"filename": "/GameData/Textures/lq_metal/met_grn_slat.png", "start": 48998394, "end": 49002435}, {"filename": "/GameData/Textures/lq_metal/met_grn_sqr.png", "start": 49002435, "end": 49006433}, {"filename": "/GameData/Textures/lq_metal/met_grn_sqrd.png", "start": 49006433, "end": 49010543}, {"filename": "/GameData/Textures/lq_metal/met_grn_sqrs.png", "start": 49010543, "end": 49014573}, {"filename": "/GameData/Textures/lq_metal/met_grn_stile.png", "start": 49014573, "end": 49017857}, {"filename": "/GameData/Textures/lq_metal/met_grn_tile.png", "start": 49017857, "end": 49021118}, {"filename": "/GameData/Textures/lq_metal/met_grn_trim16.png", "start": 49021118, "end": 49024730}, {"filename": "/GameData/Textures/lq_metal/met_grn_trim16g.png", "start": 49024730, "end": 49028884}, {"filename": "/GameData/Textures/lq_metal/met_grn_trim16h.png", "start": 49028884, "end": 49032433}, {"filename": "/GameData/Textures/lq_metal/met_grn_trim16s.png", "start": 49032433, "end": 49035944}, {"filename": "/GameData/Textures/lq_metal/met_grn_trim28.png", "start": 49035944, "end": 49040879}, {"filename": "/GameData/Textures/lq_metal/met_grn_trim28r.png", "start": 49040879, "end": 49045905}, {"filename": "/GameData/Textures/lq_metal/met_grn_trim32.png", "start": 49045905, "end": 49049730}, {"filename": "/GameData/Textures/lq_metal/met_grn_trim32r.png", "start": 49049730, "end": 49053600}, {"filename": "/GameData/Textures/lq_metal/met_grn_trim32s.png", "start": 49053600, "end": 49057246}, {"filename": "/GameData/Textures/lq_metal/met_grn_trim64.png", "start": 49057246, "end": 49060903}, {"filename": "/GameData/Textures/lq_metal/met_grn_vtrim.png", "start": 49060903, "end": 49064153}, {"filename": "/GameData/Textures/lq_metal/met_gry_beam.png", "start": 49064153, "end": 49067167}, {"filename": "/GameData/Textures/lq_metal/met_gry_block.png", "start": 49067167, "end": 49070190}, {"filename": "/GameData/Textures/lq_metal/met_gry_flat.png", "start": 49070190, "end": 49072924}, {"filename": "/GameData/Textures/lq_metal/met_gry_lit1_fbr.png", "start": 49072924, "end": 49074735}, {"filename": "/GameData/Textures/lq_metal/met_gry_lit2_fbr.png", "start": 49074735, "end": 49075858}, {"filename": "/GameData/Textures/lq_metal/met_gry_lit2b.png", "start": 49075858, "end": 49079308}, {"filename": "/GameData/Textures/lq_metal/met_gry_pan1.png", "start": 49079308, "end": 49082404}, {"filename": "/GameData/Textures/lq_metal/met_gry_pan2.png", "start": 49082404, "end": 49084557}, {"filename": "/GameData/Textures/lq_metal/met_gry_pan3.png", "start": 49084557, "end": 49087660}, {"filename": "/GameData/Textures/lq_metal/met_gry_rect.png", "start": 49087660, "end": 49090857}, {"filename": "/GameData/Textures/lq_metal/met_gry_signs.png", "start": 49090857, "end": 49094687}, {"filename": "/GameData/Textures/lq_metal/met_gry_slat.png", "start": 49094687, "end": 49098103}, {"filename": "/GameData/Textures/lq_metal/met_gry_sqr.png", "start": 49098103, "end": 49100937}, {"filename": "/GameData/Textures/lq_metal/met_gry_sqrd.png", "start": 49100937, "end": 49104185}, {"filename": "/GameData/Textures/lq_metal/met_gry_sqrs.png", "start": 49104185, "end": 49107146}, {"filename": "/GameData/Textures/lq_metal/met_gry_trim64.png", "start": 49107146, "end": 49110847}, {"filename": "/GameData/Textures/lq_metal/met_lbrn_flat.png", "start": 49110847, "end": 49114273}, {"filename": "/GameData/Textures/lq_metal/met_lbrn_rect.png", "start": 49114273, "end": 49118161}, {"filename": "/GameData/Textures/lq_metal/met_lbrn_slat.png", "start": 49118161, "end": 49122115}, {"filename": "/GameData/Textures/lq_metal/met_lift.png", "start": 49122115, "end": 49125160}, {"filename": "/GameData/Textures/lq_metal/met_met7_1.png", "start": 49125160, "end": 49127633}, {"filename": "/GameData/Textures/lq_metal/met_mix_beam.png", "start": 49127633, "end": 49131285}, {"filename": "/GameData/Textures/lq_metal/met_mix_diam.png", "start": 49131285, "end": 49134474}, {"filename": "/GameData/Textures/lq_metal/met_mix_diam2.png", "start": 49134474, "end": 49137673}, {"filename": "/GameData/Textures/lq_metal/met_mix_diamc.png", "start": 49137673, "end": 49141773}, {"filename": "/GameData/Textures/lq_metal/met_mt1_flat.png", "start": 49141773, "end": 49156063}, {"filename": "/GameData/Textures/lq_metal/met_mt1_rect.png", "start": 49156063, "end": 49171773}, {"filename": "/GameData/Textures/lq_metal/met_mt1_slat.png", "start": 49171773, "end": 49187529}, {"filename": "/GameData/Textures/lq_metal/met_mt1_sqr.png", "start": 49187529, "end": 49194069}, {"filename": "/GameData/Textures/lq_metal/met_mt2_flat.png", "start": 49194069, "end": 49210283}, {"filename": "/GameData/Textures/lq_metal/met_mt2_rect.png", "start": 49210283, "end": 49227009}, {"filename": "/GameData/Textures/lq_metal/met_mt2_slat.png", "start": 49227009, "end": 49243647}, {"filename": "/GameData/Textures/lq_metal/met_mt2_sqr.png", "start": 49243647, "end": 49252045}, {"filename": "/GameData/Textures/lq_metal/met_mt3_flat.png", "start": 49252045, "end": 49268645}, {"filename": "/GameData/Textures/lq_metal/met_mt3_rect.png", "start": 49268645, "end": 49285553}, {"filename": "/GameData/Textures/lq_metal/met_mt3_slat.png", "start": 49285553, "end": 49302464}, {"filename": "/GameData/Textures/lq_metal/met_mt3_sqr.png", "start": 49302464, "end": 49310999}, {"filename": "/GameData/Textures/lq_metal/met_ora_trim64.png", "start": 49310999, "end": 49313778}, {"filename": "/GameData/Textures/lq_metal/met_rail_flat.png", "start": 49313778, "end": 49315925}, {"filename": "/GameData/Textures/lq_metal/met_rune1_fbr.png", "start": 49315925, "end": 49318729}, {"filename": "/GameData/Textures/lq_metal/met_rune_trim32.png", "start": 49318729, "end": 49323070}, {"filename": "/GameData/Textures/lq_metal/met_set1.png", "start": 49323070, "end": 49392915}, {"filename": "/GameData/Textures/lq_metal/met_shm_flat.png", "start": 49392915, "end": 49396576}, {"filename": "/GameData/Textures/lq_metal/met_shm_rect.png", "start": 49396576, "end": 49400448}, {"filename": "/GameData/Textures/lq_metal/met_shm_slat.png", "start": 49400448, "end": 49404499}, {"filename": "/GameData/Textures/lq_metal/met_shm_sqr.png", "start": 49404499, "end": 49407961}, {"filename": "/GameData/Textures/lq_metal/met_teal_block.png", "start": 49407961, "end": 49411339}, {"filename": "/GameData/Textures/lq_metal/met_teal_flat.png", "start": 49411339, "end": 49414685}, {"filename": "/GameData/Textures/lq_metal/met_teal_trim32.png", "start": 49414685, "end": 49417961}, {"filename": "/GameData/Textures/lq_metal/met_teal_trim32r.png", "start": 49417961, "end": 49420936}, {"filename": "/GameData/Textures/lq_metal/met_teal_trim64.png", "start": 49420936, "end": 49424420}, {"filename": "/GameData/Textures/lq_metal/met_wall3_1.png", "start": 49424420, "end": 49435686}, {"filename": "/GameData/Textures/lq_metal/met_wall3_1_s.png", "start": 49435686, "end": 49438970}, {"filename": "/GameData/Textures/lq_metal/metal4_4.png", "start": 49438970, "end": 49443311}, {"filename": "/GameData/Textures/lq_metal/plus_0_sqbut1.png", "start": 49443311, "end": 49444393}, {"filename": "/GameData/Textures/lq_metal/plus_0_sqbut2_fbr.png", "start": 49444393, "end": 49447629}, {"filename": "/GameData/Textures/lq_metal/plus_0_sqshoot1_fbr.png", "start": 49447629, "end": 49448736}, {"filename": "/GameData/Textures/lq_metal/plus_0gig2a_fbr.png", "start": 49448736, "end": 49449209}, {"filename": "/GameData/Textures/lq_metal/plus_0gig_shot_fbr.png", "start": 49449209, "end": 49449685}, {"filename": "/GameData/Textures/lq_metal/plus_0gig_sshot_fbr.png", "start": 49449685, "end": 49450129}, {"filename": "/GameData/Textures/lq_metal/plus_0gig_ye_fbr.png", "start": 49450129, "end": 49450569}, {"filename": "/GameData/Textures/lq_metal/plus_0met_blu_keyg_fbr.png", "start": 49450569, "end": 49451469}, {"filename": "/GameData/Textures/lq_metal/plus_0met_blu_keys_fbr.png", "start": 49451469, "end": 49452352}, {"filename": "/GameData/Textures/lq_metal/plus_1_sqbut1.png", "start": 49452352, "end": 49453518}, {"filename": "/GameData/Textures/lq_metal/plus_1_sqbut2_fbr.png", "start": 49453518, "end": 49456706}, {"filename": "/GameData/Textures/lq_metal/plus_1_sqshoot1.png", "start": 49456706, "end": 49457820}, {"filename": "/GameData/Textures/lq_metal/plus_1met_blu_keyg_fbr.png", "start": 49457820, "end": 49458721}, {"filename": "/GameData/Textures/lq_metal/plus_1met_blu_keys_fbr.png", "start": 49458721, "end": 49459598}, {"filename": "/GameData/Textures/lq_metal/plus_2met_blu_keyg_fbr.png", "start": 49459598, "end": 49460507}, {"filename": "/GameData/Textures/lq_metal/plus_2met_blu_keys_fbr.png", "start": 49460507, "end": 49461376}, {"filename": "/GameData/Textures/lq_metal/plus_3met_blu_keyg_fbr.png", "start": 49461376, "end": 49462284}, {"filename": "/GameData/Textures/lq_metal/plus_3met_blu_keys_fbr.png", "start": 49462284, "end": 49463140}, {"filename": "/GameData/Textures/lq_metal/plus_4met_blu_keyg_fbr.png", "start": 49463140, "end": 49464048}, {"filename": "/GameData/Textures/lq_metal/plus_4met_blu_keys_fbr.png", "start": 49464048, "end": 49464904}, {"filename": "/GameData/Textures/lq_metal/plus_5met_blu_keyg_fbr.png", "start": 49464904, "end": 49465813}, {"filename": "/GameData/Textures/lq_metal/plus_5met_blu_keys_fbr.png", "start": 49465813, "end": 49466682}, {"filename": "/GameData/Textures/lq_metal/plus_6met_blu_keyg_fbr.png", "start": 49466682, "end": 49467583}, {"filename": "/GameData/Textures/lq_metal/plus_6met_blu_keys_fbr.png", "start": 49467583, "end": 49468460}, {"filename": "/GameData/Textures/lq_metal/plus_a_sqbut1.png", "start": 49468460, "end": 49469542}, {"filename": "/GameData/Textures/lq_metal/plus_a_sqbut2_fbr.png", "start": 49469542, "end": 49472778}, {"filename": "/GameData/Textures/lq_metal/plus_a_sqshoot1_fbr.png", "start": 49472778, "end": 49473885}, {"filename": "/GameData/Textures/lq_metal/plus_agig2a.png", "start": 49473885, "end": 49474255}, {"filename": "/GameData/Textures/lq_metal/plus_agig_shot_fbr.png", "start": 49474255, "end": 49474630}, {"filename": "/GameData/Textures/lq_metal/plus_agig_sshot_fbr.png", "start": 49474630, "end": 49474993}, {"filename": "/GameData/Textures/lq_metal/plus_agig_ye.png", "start": 49474993, "end": 49475405}, {"filename": "/GameData/Textures/lq_metal/plus_amet_blu_keyg.png", "start": 49475405, "end": 49476275}, {"filename": "/GameData/Textures/lq_metal/plus_amet_blu_keys.png", "start": 49476275, "end": 49477145}, {"filename": "/GameData/Textures/lq_metal/ret_metal1_tile.png", "start": 49477145, "end": 49491077}, {"filename": "/GameData/Textures/lq_metal/sq_lit1_fbr.png", "start": 49491077, "end": 49491450}, {"filename": "/GameData/Textures/lq_metal/sq_lit2_fbr.png", "start": 49491450, "end": 49491711}, {"filename": "/GameData/Textures/lq_palette/flat_01_a.png", "start": 49491711, "end": 49492259}, {"filename": "/GameData/Textures/lq_palette/flat_01_b.png", "start": 49492259, "end": 49492785}, {"filename": "/GameData/Textures/lq_palette/flat_01_c.png", "start": 49492785, "end": 49493311}, {"filename": "/GameData/Textures/lq_palette/flat_01_d.png", "start": 49493311, "end": 49493837}, {"filename": "/GameData/Textures/lq_palette/flat_01_e.png", "start": 49493837, "end": 49494363}, {"filename": "/GameData/Textures/lq_palette/flat_01_f.png", "start": 49494363, "end": 49494889}, {"filename": "/GameData/Textures/lq_palette/flat_01_g.png", "start": 49494889, "end": 49495415}, {"filename": "/GameData/Textures/lq_palette/flat_01_h.png", "start": 49495415, "end": 49495941}, {"filename": "/GameData/Textures/lq_palette/flat_01_i.png", "start": 49495941, "end": 49496468}, {"filename": "/GameData/Textures/lq_palette/flat_01_j.png", "start": 49496468, "end": 49496995}, {"filename": "/GameData/Textures/lq_palette/flat_01_k.png", "start": 49496995, "end": 49497522}, {"filename": "/GameData/Textures/lq_palette/flat_01_l.png", "start": 49497522, "end": 49498049}, {"filename": "/GameData/Textures/lq_palette/flat_01_m.png", "start": 49498049, "end": 49498576}, {"filename": "/GameData/Textures/lq_palette/flat_01_n.png", "start": 49498576, "end": 49499101}, {"filename": "/GameData/Textures/lq_palette/flat_01_o.png", "start": 49499101, "end": 49499626}, {"filename": "/GameData/Textures/lq_palette/flat_01_p.png", "start": 49499626, "end": 49500151}, {"filename": "/GameData/Textures/lq_palette/flat_02_a.png", "start": 49500151, "end": 49500677}, {"filename": "/GameData/Textures/lq_palette/flat_02_b.png", "start": 49500677, "end": 49501203}, {"filename": "/GameData/Textures/lq_palette/flat_02_c.png", "start": 49501203, "end": 49501729}, {"filename": "/GameData/Textures/lq_palette/flat_02_d.png", "start": 49501729, "end": 49502255}, {"filename": "/GameData/Textures/lq_palette/flat_02_e.png", "start": 49502255, "end": 49502781}, {"filename": "/GameData/Textures/lq_palette/flat_02_f.png", "start": 49502781, "end": 49503307}, {"filename": "/GameData/Textures/lq_palette/flat_02_g.png", "start": 49503307, "end": 49503833}, {"filename": "/GameData/Textures/lq_palette/flat_02_h.png", "start": 49503833, "end": 49504359}, {"filename": "/GameData/Textures/lq_palette/flat_02_i.png", "start": 49504359, "end": 49504885}, {"filename": "/GameData/Textures/lq_palette/flat_02_j.png", "start": 49504885, "end": 49505411}, {"filename": "/GameData/Textures/lq_palette/flat_02_k.png", "start": 49505411, "end": 49505937}, {"filename": "/GameData/Textures/lq_palette/flat_02_l.png", "start": 49505937, "end": 49506463}, {"filename": "/GameData/Textures/lq_palette/flat_02_m.png", "start": 49506463, "end": 49506989}, {"filename": "/GameData/Textures/lq_palette/flat_02_n.png", "start": 49506989, "end": 49507515}, {"filename": "/GameData/Textures/lq_palette/flat_02_o.png", "start": 49507515, "end": 49508041}, {"filename": "/GameData/Textures/lq_palette/flat_02_p.png", "start": 49508041, "end": 49508567}, {"filename": "/GameData/Textures/lq_palette/flat_03_a.png", "start": 49508567, "end": 49509093}, {"filename": "/GameData/Textures/lq_palette/flat_03_b.png", "start": 49509093, "end": 49509619}, {"filename": "/GameData/Textures/lq_palette/flat_03_c.png", "start": 49509619, "end": 49510145}, {"filename": "/GameData/Textures/lq_palette/flat_03_d.png", "start": 49510145, "end": 49510671}, {"filename": "/GameData/Textures/lq_palette/flat_03_e.png", "start": 49510671, "end": 49511197}, {"filename": "/GameData/Textures/lq_palette/flat_03_f.png", "start": 49511197, "end": 49511723}, {"filename": "/GameData/Textures/lq_palette/flat_03_g.png", "start": 49511723, "end": 49512249}, {"filename": "/GameData/Textures/lq_palette/flat_03_h.png", "start": 49512249, "end": 49512775}, {"filename": "/GameData/Textures/lq_palette/flat_03_i.png", "start": 49512775, "end": 49513301}, {"filename": "/GameData/Textures/lq_palette/flat_03_j.png", "start": 49513301, "end": 49513827}, {"filename": "/GameData/Textures/lq_palette/flat_03_k.png", "start": 49513827, "end": 49514353}, {"filename": "/GameData/Textures/lq_palette/flat_03_l.png", "start": 49514353, "end": 49514879}, {"filename": "/GameData/Textures/lq_palette/flat_03_m.png", "start": 49514879, "end": 49515406}, {"filename": "/GameData/Textures/lq_palette/flat_03_n.png", "start": 49515406, "end": 49515933}, {"filename": "/GameData/Textures/lq_palette/flat_03_o.png", "start": 49515933, "end": 49516460}, {"filename": "/GameData/Textures/lq_palette/flat_03_p.png", "start": 49516460, "end": 49516987}, {"filename": "/GameData/Textures/lq_palette/flat_04_a.png", "start": 49516987, "end": 49517535}, {"filename": "/GameData/Textures/lq_palette/flat_04_b.png", "start": 49517535, "end": 49518061}, {"filename": "/GameData/Textures/lq_palette/flat_04_c.png", "start": 49518061, "end": 49518587}, {"filename": "/GameData/Textures/lq_palette/flat_04_d.png", "start": 49518587, "end": 49519113}, {"filename": "/GameData/Textures/lq_palette/flat_04_e.png", "start": 49519113, "end": 49519639}, {"filename": "/GameData/Textures/lq_palette/flat_04_f.png", "start": 49519639, "end": 49520165}, {"filename": "/GameData/Textures/lq_palette/flat_04_g.png", "start": 49520165, "end": 49520691}, {"filename": "/GameData/Textures/lq_palette/flat_04_h.png", "start": 49520691, "end": 49521217}, {"filename": "/GameData/Textures/lq_palette/flat_04_i.png", "start": 49521217, "end": 49521743}, {"filename": "/GameData/Textures/lq_palette/flat_04_j.png", "start": 49521743, "end": 49522269}, {"filename": "/GameData/Textures/lq_palette/flat_04_k.png", "start": 49522269, "end": 49522795}, {"filename": "/GameData/Textures/lq_palette/flat_04_l.png", "start": 49522795, "end": 49523321}, {"filename": "/GameData/Textures/lq_palette/flat_04_m.png", "start": 49523321, "end": 49523847}, {"filename": "/GameData/Textures/lq_palette/flat_04_n.png", "start": 49523847, "end": 49524373}, {"filename": "/GameData/Textures/lq_palette/flat_04_o.png", "start": 49524373, "end": 49524899}, {"filename": "/GameData/Textures/lq_palette/flat_04_p.png", "start": 49524899, "end": 49525425}, {"filename": "/GameData/Textures/lq_palette/flat_05_a.png", "start": 49525425, "end": 49525951}, {"filename": "/GameData/Textures/lq_palette/flat_05_b.png", "start": 49525951, "end": 49526477}, {"filename": "/GameData/Textures/lq_palette/flat_05_c.png", "start": 49526477, "end": 49527003}, {"filename": "/GameData/Textures/lq_palette/flat_05_d.png", "start": 49527003, "end": 49527529}, {"filename": "/GameData/Textures/lq_palette/flat_05_e.png", "start": 49527529, "end": 49528055}, {"filename": "/GameData/Textures/lq_palette/flat_05_f.png", "start": 49528055, "end": 49528581}, {"filename": "/GameData/Textures/lq_palette/flat_05_g.png", "start": 49528581, "end": 49529107}, {"filename": "/GameData/Textures/lq_palette/flat_05_h.png", "start": 49529107, "end": 49529633}, {"filename": "/GameData/Textures/lq_palette/flat_05_i.png", "start": 49529633, "end": 49530159}, {"filename": "/GameData/Textures/lq_palette/flat_05_j.png", "start": 49530159, "end": 49530685}, {"filename": "/GameData/Textures/lq_palette/flat_05_k.png", "start": 49530685, "end": 49531211}, {"filename": "/GameData/Textures/lq_palette/flat_05_l.png", "start": 49531211, "end": 49531737}, {"filename": "/GameData/Textures/lq_palette/flat_05_m.png", "start": 49531737, "end": 49532263}, {"filename": "/GameData/Textures/lq_palette/flat_05_n.png", "start": 49532263, "end": 49532789}, {"filename": "/GameData/Textures/lq_palette/flat_05_o.png", "start": 49532789, "end": 49533315}, {"filename": "/GameData/Textures/lq_palette/flat_05_p.png", "start": 49533315, "end": 49533841}, {"filename": "/GameData/Textures/lq_palette/flat_06_a.png", "start": 49533841, "end": 49534367}, {"filename": "/GameData/Textures/lq_palette/flat_06_b.png", "start": 49534367, "end": 49534893}, {"filename": "/GameData/Textures/lq_palette/flat_06_c.png", "start": 49534893, "end": 49535419}, {"filename": "/GameData/Textures/lq_palette/flat_06_d.png", "start": 49535419, "end": 49535945}, {"filename": "/GameData/Textures/lq_palette/flat_06_e.png", "start": 49535945, "end": 49536471}, {"filename": "/GameData/Textures/lq_palette/flat_06_f.png", "start": 49536471, "end": 49536997}, {"filename": "/GameData/Textures/lq_palette/flat_06_g.png", "start": 49536997, "end": 49537523}, {"filename": "/GameData/Textures/lq_palette/flat_06_h.png", "start": 49537523, "end": 49538049}, {"filename": "/GameData/Textures/lq_palette/flat_06_i.png", "start": 49538049, "end": 49538575}, {"filename": "/GameData/Textures/lq_palette/flat_06_j.png", "start": 49538575, "end": 49539101}, {"filename": "/GameData/Textures/lq_palette/flat_06_k.png", "start": 49539101, "end": 49539627}, {"filename": "/GameData/Textures/lq_palette/flat_06_l.png", "start": 49539627, "end": 49540153}, {"filename": "/GameData/Textures/lq_palette/flat_06_m.png", "start": 49540153, "end": 49540679}, {"filename": "/GameData/Textures/lq_palette/flat_06_n.png", "start": 49540679, "end": 49541205}, {"filename": "/GameData/Textures/lq_palette/flat_06_o.png", "start": 49541205, "end": 49541731}, {"filename": "/GameData/Textures/lq_palette/flat_06_p.png", "start": 49541731, "end": 49542257}, {"filename": "/GameData/Textures/lq_palette/flat_07_a.png", "start": 49542257, "end": 49542783}, {"filename": "/GameData/Textures/lq_palette/flat_07_b.png", "start": 49542783, "end": 49543309}, {"filename": "/GameData/Textures/lq_palette/flat_07_c.png", "start": 49543309, "end": 49543835}, {"filename": "/GameData/Textures/lq_palette/flat_07_d.png", "start": 49543835, "end": 49544361}, {"filename": "/GameData/Textures/lq_palette/flat_07_e.png", "start": 49544361, "end": 49544887}, {"filename": "/GameData/Textures/lq_palette/flat_07_f.png", "start": 49544887, "end": 49545413}, {"filename": "/GameData/Textures/lq_palette/flat_07_g.png", "start": 49545413, "end": 49545939}, {"filename": "/GameData/Textures/lq_palette/flat_07_h.png", "start": 49545939, "end": 49546465}, {"filename": "/GameData/Textures/lq_palette/flat_07_i.png", "start": 49546465, "end": 49546991}, {"filename": "/GameData/Textures/lq_palette/flat_07_j.png", "start": 49546991, "end": 49547517}, {"filename": "/GameData/Textures/lq_palette/flat_07_k.png", "start": 49547517, "end": 49548043}, {"filename": "/GameData/Textures/lq_palette/flat_07_l.png", "start": 49548043, "end": 49548569}, {"filename": "/GameData/Textures/lq_palette/flat_07_m.png", "start": 49548569, "end": 49549095}, {"filename": "/GameData/Textures/lq_palette/flat_07_n.png", "start": 49549095, "end": 49549621}, {"filename": "/GameData/Textures/lq_palette/flat_07_o.png", "start": 49549621, "end": 49550147}, {"filename": "/GameData/Textures/lq_palette/flat_07_p.png", "start": 49550147, "end": 49550673}, {"filename": "/GameData/Textures/lq_palette/flat_08_a.png", "start": 49550673, "end": 49551199}, {"filename": "/GameData/Textures/lq_palette/flat_08_b.png", "start": 49551199, "end": 49551725}, {"filename": "/GameData/Textures/lq_palette/flat_08_c.png", "start": 49551725, "end": 49552251}, {"filename": "/GameData/Textures/lq_palette/flat_08_d.png", "start": 49552251, "end": 49552777}, {"filename": "/GameData/Textures/lq_palette/flat_08_e.png", "start": 49552777, "end": 49553303}, {"filename": "/GameData/Textures/lq_palette/flat_08_f.png", "start": 49553303, "end": 49553829}, {"filename": "/GameData/Textures/lq_palette/flat_08_g.png", "start": 49553829, "end": 49554355}, {"filename": "/GameData/Textures/lq_palette/flat_08_h.png", "start": 49554355, "end": 49554881}, {"filename": "/GameData/Textures/lq_palette/flat_08_i.png", "start": 49554881, "end": 49555407}, {"filename": "/GameData/Textures/lq_palette/flat_08_j.png", "start": 49555407, "end": 49555933}, {"filename": "/GameData/Textures/lq_palette/flat_08_k.png", "start": 49555933, "end": 49556459}, {"filename": "/GameData/Textures/lq_palette/flat_08_l.png", "start": 49556459, "end": 49556985}, {"filename": "/GameData/Textures/lq_palette/flat_08_m.png", "start": 49556985, "end": 49557511}, {"filename": "/GameData/Textures/lq_palette/flat_08_n.png", "start": 49557511, "end": 49558038}, {"filename": "/GameData/Textures/lq_palette/flat_08_o.png", "start": 49558038, "end": 49558565}, {"filename": "/GameData/Textures/lq_palette/flat_08_p.png", "start": 49558565, "end": 49559092}, {"filename": "/GameData/Textures/lq_palette/flat_09_a.png", "start": 49559092, "end": 49559619}, {"filename": "/GameData/Textures/lq_palette/flat_09_b.png", "start": 49559619, "end": 49560146}, {"filename": "/GameData/Textures/lq_palette/flat_09_c.png", "start": 49560146, "end": 49560673}, {"filename": "/GameData/Textures/lq_palette/flat_09_d.png", "start": 49560673, "end": 49561199}, {"filename": "/GameData/Textures/lq_palette/flat_09_e.png", "start": 49561199, "end": 49561725}, {"filename": "/GameData/Textures/lq_palette/flat_09_f.png", "start": 49561725, "end": 49562251}, {"filename": "/GameData/Textures/lq_palette/flat_09_g.png", "start": 49562251, "end": 49562777}, {"filename": "/GameData/Textures/lq_palette/flat_09_h.png", "start": 49562777, "end": 49563303}, {"filename": "/GameData/Textures/lq_palette/flat_09_i.png", "start": 49563303, "end": 49563829}, {"filename": "/GameData/Textures/lq_palette/flat_09_j.png", "start": 49563829, "end": 49564355}, {"filename": "/GameData/Textures/lq_palette/flat_09_k.png", "start": 49564355, "end": 49564881}, {"filename": "/GameData/Textures/lq_palette/flat_09_l.png", "start": 49564881, "end": 49565407}, {"filename": "/GameData/Textures/lq_palette/flat_09_m.png", "start": 49565407, "end": 49565933}, {"filename": "/GameData/Textures/lq_palette/flat_09_n.png", "start": 49565933, "end": 49566459}, {"filename": "/GameData/Textures/lq_palette/flat_09_o.png", "start": 49566459, "end": 49566985}, {"filename": "/GameData/Textures/lq_palette/flat_09_p.png", "start": 49566985, "end": 49567511}, {"filename": "/GameData/Textures/lq_palette/flat_10_a.png", "start": 49567511, "end": 49568038}, {"filename": "/GameData/Textures/lq_palette/flat_10_b.png", "start": 49568038, "end": 49568564}, {"filename": "/GameData/Textures/lq_palette/flat_10_c.png", "start": 49568564, "end": 49569090}, {"filename": "/GameData/Textures/lq_palette/flat_10_d.png", "start": 49569090, "end": 49569616}, {"filename": "/GameData/Textures/lq_palette/flat_10_e.png", "start": 49569616, "end": 49570142}, {"filename": "/GameData/Textures/lq_palette/flat_10_f.png", "start": 49570142, "end": 49570668}, {"filename": "/GameData/Textures/lq_palette/flat_10_g.png", "start": 49570668, "end": 49571194}, {"filename": "/GameData/Textures/lq_palette/flat_10_h.png", "start": 49571194, "end": 49571720}, {"filename": "/GameData/Textures/lq_palette/flat_10_i.png", "start": 49571720, "end": 49572246}, {"filename": "/GameData/Textures/lq_palette/flat_10_j.png", "start": 49572246, "end": 49572772}, {"filename": "/GameData/Textures/lq_palette/flat_10_k.png", "start": 49572772, "end": 49573298}, {"filename": "/GameData/Textures/lq_palette/flat_10_l.png", "start": 49573298, "end": 49573824}, {"filename": "/GameData/Textures/lq_palette/flat_10_m.png", "start": 49573824, "end": 49574350}, {"filename": "/GameData/Textures/lq_palette/flat_10_n.png", "start": 49574350, "end": 49574876}, {"filename": "/GameData/Textures/lq_palette/flat_10_o.png", "start": 49574876, "end": 49575402}, {"filename": "/GameData/Textures/lq_palette/flat_10_p.png", "start": 49575402, "end": 49575928}, {"filename": "/GameData/Textures/lq_palette/flat_11_a.png", "start": 49575928, "end": 49576455}, {"filename": "/GameData/Textures/lq_palette/flat_11_b.png", "start": 49576455, "end": 49576982}, {"filename": "/GameData/Textures/lq_palette/flat_11_c.png", "start": 49576982, "end": 49577509}, {"filename": "/GameData/Textures/lq_palette/flat_11_d.png", "start": 49577509, "end": 49578036}, {"filename": "/GameData/Textures/lq_palette/flat_11_e.png", "start": 49578036, "end": 49578563}, {"filename": "/GameData/Textures/lq_palette/flat_11_f.png", "start": 49578563, "end": 49579089}, {"filename": "/GameData/Textures/lq_palette/flat_11_g.png", "start": 49579089, "end": 49579615}, {"filename": "/GameData/Textures/lq_palette/flat_11_h.png", "start": 49579615, "end": 49580141}, {"filename": "/GameData/Textures/lq_palette/flat_11_i.png", "start": 49580141, "end": 49580667}, {"filename": "/GameData/Textures/lq_palette/flat_11_j.png", "start": 49580667, "end": 49581193}, {"filename": "/GameData/Textures/lq_palette/flat_11_k.png", "start": 49581193, "end": 49581719}, {"filename": "/GameData/Textures/lq_palette/flat_11_l.png", "start": 49581719, "end": 49582245}, {"filename": "/GameData/Textures/lq_palette/flat_11_m.png", "start": 49582245, "end": 49582771}, {"filename": "/GameData/Textures/lq_palette/flat_11_n.png", "start": 49582771, "end": 49583297}, {"filename": "/GameData/Textures/lq_palette/flat_11_o.png", "start": 49583297, "end": 49583823}, {"filename": "/GameData/Textures/lq_palette/flat_11_p.png", "start": 49583823, "end": 49584349}, {"filename": "/GameData/Textures/lq_palette/flat_12_a.png", "start": 49584349, "end": 49584875}, {"filename": "/GameData/Textures/lq_palette/flat_12_b.png", "start": 49584875, "end": 49585401}, {"filename": "/GameData/Textures/lq_palette/flat_12_c.png", "start": 49585401, "end": 49585927}, {"filename": "/GameData/Textures/lq_palette/flat_12_d.png", "start": 49585927, "end": 49586453}, {"filename": "/GameData/Textures/lq_palette/flat_12_e.png", "start": 49586453, "end": 49586979}, {"filename": "/GameData/Textures/lq_palette/flat_12_f.png", "start": 49586979, "end": 49587505}, {"filename": "/GameData/Textures/lq_palette/flat_12_g.png", "start": 49587505, "end": 49588031}, {"filename": "/GameData/Textures/lq_palette/flat_12_h.png", "start": 49588031, "end": 49588557}, {"filename": "/GameData/Textures/lq_palette/flat_12_i.png", "start": 49588557, "end": 49589083}, {"filename": "/GameData/Textures/lq_palette/flat_12_j.png", "start": 49589083, "end": 49589609}, {"filename": "/GameData/Textures/lq_palette/flat_12_k.png", "start": 49589609, "end": 49590135}, {"filename": "/GameData/Textures/lq_palette/flat_12_l.png", "start": 49590135, "end": 49590661}, {"filename": "/GameData/Textures/lq_palette/flat_12_m.png", "start": 49590661, "end": 49591187}, {"filename": "/GameData/Textures/lq_palette/flat_12_n.png", "start": 49591187, "end": 49591713}, {"filename": "/GameData/Textures/lq_palette/flat_12_o.png", "start": 49591713, "end": 49592239}, {"filename": "/GameData/Textures/lq_palette/flat_12_p.png", "start": 49592239, "end": 49592765}, {"filename": "/GameData/Textures/lq_palette/flat_13_a.png", "start": 49592765, "end": 49593291}, {"filename": "/GameData/Textures/lq_palette/flat_13_b.png", "start": 49593291, "end": 49593817}, {"filename": "/GameData/Textures/lq_palette/flat_13_c.png", "start": 49593817, "end": 49594343}, {"filename": "/GameData/Textures/lq_palette/flat_13_d.png", "start": 49594343, "end": 49594869}, {"filename": "/GameData/Textures/lq_palette/flat_13_e.png", "start": 49594869, "end": 49595395}, {"filename": "/GameData/Textures/lq_palette/flat_13_f.png", "start": 49595395, "end": 49595921}, {"filename": "/GameData/Textures/lq_palette/flat_13_g.png", "start": 49595921, "end": 49596447}, {"filename": "/GameData/Textures/lq_palette/flat_13_h.png", "start": 49596447, "end": 49596973}, {"filename": "/GameData/Textures/lq_palette/flat_13_i.png", "start": 49596973, "end": 49597499}, {"filename": "/GameData/Textures/lq_palette/flat_13_j.png", "start": 49597499, "end": 49598025}, {"filename": "/GameData/Textures/lq_palette/flat_13_k.png", "start": 49598025, "end": 49598551}, {"filename": "/GameData/Textures/lq_palette/flat_13_l.png", "start": 49598551, "end": 49599077}, {"filename": "/GameData/Textures/lq_palette/flat_13_m.png", "start": 49599077, "end": 49599603}, {"filename": "/GameData/Textures/lq_palette/flat_13_n.png", "start": 49599603, "end": 49600129}, {"filename": "/GameData/Textures/lq_palette/flat_13_o.png", "start": 49600129, "end": 49600655}, {"filename": "/GameData/Textures/lq_palette/flat_13_p.png", "start": 49600655, "end": 49601181}, {"filename": "/GameData/Textures/lq_palette/flat_14_a.png", "start": 49601181, "end": 49601729}, {"filename": "/GameData/Textures/lq_palette/flat_14_b.png", "start": 49601729, "end": 49602255}, {"filename": "/GameData/Textures/lq_palette/flat_14_c.png", "start": 49602255, "end": 49602781}, {"filename": "/GameData/Textures/lq_palette/flat_14_d.png", "start": 49602781, "end": 49603307}, {"filename": "/GameData/Textures/lq_palette/flat_14_e.png", "start": 49603307, "end": 49603833}, {"filename": "/GameData/Textures/lq_palette/flat_14_f.png", "start": 49603833, "end": 49604359}, {"filename": "/GameData/Textures/lq_palette/flat_14_g.png", "start": 49604359, "end": 49604885}, {"filename": "/GameData/Textures/lq_palette/flat_14_h.png", "start": 49604885, "end": 49605411}, {"filename": "/GameData/Textures/lq_palette/flat_14_i.png", "start": 49605411, "end": 49605937}, {"filename": "/GameData/Textures/lq_palette/flat_14_j.png", "start": 49605937, "end": 49606463}, {"filename": "/GameData/Textures/lq_palette/flat_14_k.png", "start": 49606463, "end": 49606989}, {"filename": "/GameData/Textures/lq_palette/flat_14_l.png", "start": 49606989, "end": 49607515}, {"filename": "/GameData/Textures/lq_palette/flat_14_m.png", "start": 49607515, "end": 49608041}, {"filename": "/GameData/Textures/lq_palette/flat_14_n.png", "start": 49608041, "end": 49608567}, {"filename": "/GameData/Textures/lq_palette/flat_14_o.png", "start": 49608567, "end": 49609093}, {"filename": "/GameData/Textures/lq_palette/flat_14_p.png", "start": 49609093, "end": 49609619}, {"filename": "/GameData/Textures/lq_palette/flat_15_a_fbr.png", "start": 49609619, "end": 49610145}, {"filename": "/GameData/Textures/lq_palette/flat_15_b_fbr.png", "start": 49610145, "end": 49610671}, {"filename": "/GameData/Textures/lq_palette/flat_15_c_fbr.png", "start": 49610671, "end": 49611197}, {"filename": "/GameData/Textures/lq_palette/flat_15_d_fbr.png", "start": 49611197, "end": 49611723}, {"filename": "/GameData/Textures/lq_palette/flat_15_e_fbr.png", "start": 49611723, "end": 49612249}, {"filename": "/GameData/Textures/lq_palette/flat_15_f_fbr.png", "start": 49612249, "end": 49612775}, {"filename": "/GameData/Textures/lq_palette/flat_15_g_fbr.png", "start": 49612775, "end": 49613301}, {"filename": "/GameData/Textures/lq_palette/flat_15_h_fbr.png", "start": 49613301, "end": 49613827}, {"filename": "/GameData/Textures/lq_palette/flat_15_i_fbr.png", "start": 49613827, "end": 49614353}, {"filename": "/GameData/Textures/lq_palette/flat_15_j_fbr.png", "start": 49614353, "end": 49614879}, {"filename": "/GameData/Textures/lq_palette/flat_15_k_fbr.png", "start": 49614879, "end": 49615405}, {"filename": "/GameData/Textures/lq_palette/flat_15_l_fbr.png", "start": 49615405, "end": 49615931}, {"filename": "/GameData/Textures/lq_palette/flat_15_m_fbr.png", "start": 49615931, "end": 49616457}, {"filename": "/GameData/Textures/lq_palette/flat_15_n_fbr.png", "start": 49616457, "end": 49616983}, {"filename": "/GameData/Textures/lq_palette/flat_15_o_fbr.png", "start": 49616983, "end": 49617510}, {"filename": "/GameData/Textures/lq_palette/flat_15_p_fbr.png", "start": 49617510, "end": 49618037}, {"filename": "/GameData/Textures/lq_palette/flat_16_a_fbr.png", "start": 49618037, "end": 49618563}, {"filename": "/GameData/Textures/lq_palette/flat_16_b_fbr.png", "start": 49618563, "end": 49619089}, {"filename": "/GameData/Textures/lq_palette/flat_16_c_fbr.png", "start": 49619089, "end": 49619615}, {"filename": "/GameData/Textures/lq_palette/flat_16_d_fbr.png", "start": 49619615, "end": 49620141}, {"filename": "/GameData/Textures/lq_palette/flat_16_e_fbr.png", "start": 49620141, "end": 49620668}, {"filename": "/GameData/Textures/lq_palette/flat_16_f_fbr.png", "start": 49620668, "end": 49621195}, {"filename": "/GameData/Textures/lq_palette/flat_16_g_fbr.png", "start": 49621195, "end": 49621720}, {"filename": "/GameData/Textures/lq_palette/flat_16_h_fbr.png", "start": 49621720, "end": 49622246}, {"filename": "/GameData/Textures/lq_palette/flat_16_i_fbr.png", "start": 49622246, "end": 49622772}, {"filename": "/GameData/Textures/lq_palette/flat_16_j_fbr.png", "start": 49622772, "end": 49623298}, {"filename": "/GameData/Textures/lq_palette/flat_16_k_fbr.png", "start": 49623298, "end": 49623824}, {"filename": "/GameData/Textures/lq_palette/flat_16_l_fbr.png", "start": 49623824, "end": 49624350}, {"filename": "/GameData/Textures/lq_palette/flat_16_m_fbr.png", "start": 49624350, "end": 49624877}, {"filename": "/GameData/Textures/lq_palette/flat_16_n_fbr.png", "start": 49624877, "end": 49625404}, {"filename": "/GameData/Textures/lq_palette/flat_16_o_fbr.png", "start": 49625404, "end": 49625929}, {"filename": "/GameData/Textures/lq_palette/flat_16_p_fbr.png", "start": 49625929, "end": 49626455}, {"filename": "/GameData/Textures/lq_props/JarBod1.png", "start": 49626455, "end": 49626802}, {"filename": "/GameData/Textures/lq_props/JarBod2.png", "start": 49626802, "end": 49627120}, {"filename": "/GameData/Textures/lq_props/JarTop1.png", "start": 49627120, "end": 49627345}, {"filename": "/GameData/Textures/lq_props/JarTop2.png", "start": 49627345, "end": 49627485}, {"filename": "/GameData/Textures/lq_props/crate-door-brn.png", "start": 49627485, "end": 49636123}, {"filename": "/GameData/Textures/lq_props/crate-door-grn.png", "start": 49636123, "end": 49646179}, {"filename": "/GameData/Textures/lq_props/crate-door-orn.png", "start": 49646179, "end": 49655278}, {"filename": "/GameData/Textures/lq_props/crate-side-brn.png", "start": 49655278, "end": 49675404}, {"filename": "/GameData/Textures/lq_props/crate-side-grn.png", "start": 49675404, "end": 49698317}, {"filename": "/GameData/Textures/lq_props/crate-side-orn.png", "start": 49698317, "end": 49719646}, {"filename": "/GameData/Textures/lq_props/go-ep0_fbr.png", "start": 49719646, "end": 49721421}, {"filename": "/GameData/Textures/lq_props/med_book_blue.png", "start": 49721421, "end": 49722001}, {"filename": "/GameData/Textures/lq_props/med_book_green.png", "start": 49722001, "end": 49722624}, {"filename": "/GameData/Textures/lq_props/med_book_pink.png", "start": 49722624, "end": 49723279}, {"filename": "/GameData/Textures/lq_props/med_book_red.png", "start": 49723279, "end": 49723996}, {"filename": "/GameData/Textures/lq_props/med_book_teal.png", "start": 49723996, "end": 49724583}, {"filename": "/GameData/Textures/lq_props/med_books_wood.png", "start": 49724583, "end": 49737513}, {"filename": "/GameData/Textures/lq_props/med_dbrick4_p1.png", "start": 49737513, "end": 49769350}, {"filename": "/GameData/Textures/lq_props/med_dbrick4_p2.png", "start": 49769350, "end": 49802771}, {"filename": "/GameData/Textures/lq_props/med_ebrick9_p1.png", "start": 49802771, "end": 49834662}, {"filename": "/GameData/Textures/lq_props/med_ebrick9_p2.png", "start": 49834662, "end": 49866028}, {"filename": "/GameData/Textures/lq_props/note-e0_fbr.png", "start": 49866028, "end": 49885457}, {"filename": "/GameData/Textures/lq_props/plus_0blink_fbr.png", "start": 49885457, "end": 49885686}, {"filename": "/GameData/Textures/lq_props/plus_0tvnoise.png", "start": 49885686, "end": 49886672}, {"filename": "/GameData/Textures/lq_props/plus_1blink_fbr.png", "start": 49886672, "end": 49886901}, {"filename": "/GameData/Textures/lq_props/plus_1tvnoise.png", "start": 49886901, "end": 49887879}, {"filename": "/GameData/Textures/lq_props/plus_2blink_fbr.png", "start": 49887879, "end": 49888110}, {"filename": "/GameData/Textures/lq_props/plus_2tvnoise.png", "start": 49888110, "end": 49889083}, {"filename": "/GameData/Textures/lq_props/plus_3blink_fbr.png", "start": 49889083, "end": 49889314}, {"filename": "/GameData/Textures/lq_props/plus_3tvnoise.png", "start": 49889314, "end": 49890296}, {"filename": "/GameData/Textures/lq_props/plus_4blink_fbr.png", "start": 49890296, "end": 49890526}, {"filename": "/GameData/Textures/lq_props/plus_4tvnoise.png", "start": 49890526, "end": 49891510}, {"filename": "/GameData/Textures/lq_props/plus_5tvnoise.png", "start": 49891510, "end": 49892474}, {"filename": "/GameData/Textures/lq_props/plus_6tvnoise.png", "start": 49892474, "end": 49893464}, {"filename": "/GameData/Textures/lq_props/plus_7tvnoise.png", "start": 49893464, "end": 49894438}, {"filename": "/GameData/Textures/lq_props/plus_8tvnoise.png", "start": 49894438, "end": 49895439}, {"filename": "/GameData/Textures/lq_props/plus_9tvnoise.png", "start": 49895439, "end": 49896416}, {"filename": "/GameData/Textures/lq_props/plus_ablink_fbr.png", "start": 49896416, "end": 49896646}, {"filename": "/GameData/Textures/lq_props/plus_atvnoise.png", "start": 49896646, "end": 49897206}, {"filename": "/GameData/Textures/lq_props/plus_atvnoise64.png", "start": 49897206, "end": 49898476}, {"filename": "/GameData/Textures/lq_props/qr.png", "start": 49898476, "end": 49899896}, {"filename": "/GameData/Textures/lq_props/radio16.png", "start": 49899896, "end": 49900664}, {"filename": "/GameData/Textures/lq_props/radio32.png", "start": 49900664, "end": 49901654}, {"filename": "/GameData/Textures/lq_props/radio64.png", "start": 49901654, "end": 49903412}, {"filename": "/GameData/Textures/lq_props/radiowood.png", "start": 49903412, "end": 49905555}, {"filename": "/GameData/Textures/lq_props/secret_gem_1.png", "start": 49905555, "end": 49908591}, {"filename": "/GameData/Textures/lq_props/secret_gem_2.png", "start": 49908591, "end": 49911295}, {"filename": "/GameData/Textures/lq_props/secret_gem_3.png", "start": 49911295, "end": 49914355}, {"filename": "/GameData/Textures/lq_props/secret_gem_4.png", "start": 49914355, "end": 49918113}, {"filename": "/GameData/Textures/lq_props/secret_gem_h.png", "start": 49918113, "end": 49918664}, {"filename": "/GameData/Textures/lq_tech/_t_fence01_fbr.png", "start": 49918664, "end": 49924381}, {"filename": "/GameData/Textures/lq_tech/_t_flare01_fbr.png", "start": 49924381, "end": 49924767}, {"filename": "/GameData/Textures/lq_tech/aqconc03.png", "start": 49924767, "end": 49935327}, {"filename": "/GameData/Textures/lq_tech/aqconc04.png", "start": 49935327, "end": 49946383}, {"filename": "/GameData/Textures/lq_tech/aqconc05.png", "start": 49946383, "end": 49950974}, {"filename": "/GameData/Textures/lq_tech/aqf006b.png", "start": 49950974, "end": 49953601}, {"filename": "/GameData/Textures/lq_tech/aqf032.png", "start": 49953601, "end": 49954872}, {"filename": "/GameData/Textures/lq_tech/aqf049.png", "start": 49954872, "end": 49957149}, {"filename": "/GameData/Textures/lq_tech/aqf074.png", "start": 49957149, "end": 49960571}, {"filename": "/GameData/Textures/lq_tech/aqf075.png", "start": 49960571, "end": 49963730}, {"filename": "/GameData/Textures/lq_tech/aqmetl01.png", "start": 49963730, "end": 49966458}, {"filename": "/GameData/Textures/lq_tech/aqmetl07.png", "start": 49966458, "end": 49971229}, {"filename": "/GameData/Textures/lq_tech/aqmetl14.png", "start": 49971229, "end": 49978961}, {"filename": "/GameData/Textures/lq_tech/aqmetl28.png", "start": 49978961, "end": 49980613}, {"filename": "/GameData/Textures/lq_tech/aqmetl30.png", "start": 49980613, "end": 49982265}, {"filename": "/GameData/Textures/lq_tech/aqmetl33.png", "start": 49982265, "end": 49984701}, {"filename": "/GameData/Textures/lq_tech/aqpanl09.png", "start": 49984701, "end": 49987493}, {"filename": "/GameData/Textures/lq_tech/aqpanl10.png", "start": 49987493, "end": 49992387}, {"filename": "/GameData/Textures/lq_tech/aqpipe01.png", "start": 49992387, "end": 49996575}, {"filename": "/GameData/Textures/lq_tech/aqpipe04.png", "start": 49996575, "end": 49999039}, {"filename": "/GameData/Textures/lq_tech/aqpipe05.png", "start": 49999039, "end": 50002205}, {"filename": "/GameData/Textures/lq_tech/aqpipe08.png", "start": 50002205, "end": 50007837}, {"filename": "/GameData/Textures/lq_tech/aqpipe09.png", "start": 50007837, "end": 50015941}, {"filename": "/GameData/Textures/lq_tech/aqpipe12.png", "start": 50015941, "end": 50024061}, {"filename": "/GameData/Textures/lq_tech/aqpipe13.png", "start": 50024061, "end": 50030372}, {"filename": "/GameData/Textures/lq_tech/aqpipe14.png", "start": 50030372, "end": 50037725}, {"filename": "/GameData/Textures/lq_tech/aqrust01.png", "start": 50037725, "end": 50040356}, {"filename": "/GameData/Textures/lq_tech/aqrust02.png", "start": 50040356, "end": 50045657}, {"filename": "/GameData/Textures/lq_tech/aqrust03.png", "start": 50045657, "end": 50050960}, {"filename": "/GameData/Textures/lq_tech/aqrust03b.png", "start": 50050960, "end": 50053690}, {"filename": "/GameData/Textures/lq_tech/aqrust04.png", "start": 50053690, "end": 50055083}, {"filename": "/GameData/Textures/lq_tech/aqrust09.png", "start": 50055083, "end": 50058971}, {"filename": "/GameData/Textures/lq_tech/aqrust10.png", "start": 50058971, "end": 50062137}, {"filename": "/GameData/Textures/lq_tech/aqsect14.png", "start": 50062137, "end": 50064513}, {"filename": "/GameData/Textures/lq_tech/aqsect15.png", "start": 50064513, "end": 50067460}, {"filename": "/GameData/Textures/lq_tech/aqsect16.png", "start": 50067460, "end": 50070706}, {"filename": "/GameData/Textures/lq_tech/aqsect16b.png", "start": 50070706, "end": 50075663}, {"filename": "/GameData/Textures/lq_tech/aqsupp01.png", "start": 50075663, "end": 50078309}, {"filename": "/GameData/Textures/lq_tech/aqsupp02.png", "start": 50078309, "end": 50083174}, {"filename": "/GameData/Textures/lq_tech/aqsupp03.png", "start": 50083174, "end": 50087076}, {"filename": "/GameData/Textures/lq_tech/aqsupp04.png", "start": 50087076, "end": 50088619}, {"filename": "/GameData/Textures/lq_tech/aqsupp06.png", "start": 50088619, "end": 50089998}, {"filename": "/GameData/Textures/lq_tech/aqsupp07.png", "start": 50089998, "end": 50091353}, {"filename": "/GameData/Textures/lq_tech/aqsupp08.png", "start": 50091353, "end": 50093555}, {"filename": "/GameData/Textures/lq_tech/aqsupp09.png", "start": 50093555, "end": 50096408}, {"filename": "/GameData/Textures/lq_tech/aqtrim01.png", "start": 50096408, "end": 50097635}, {"filename": "/GameData/Textures/lq_tech/aqtrim02.png", "start": 50097635, "end": 50098559}, {"filename": "/GameData/Textures/lq_tech/aqtrim03.png", "start": 50098559, "end": 50099130}, {"filename": "/GameData/Textures/lq_tech/aqtrim08.png", "start": 50099130, "end": 50099862}, {"filename": "/GameData/Textures/lq_tech/butmet.png", "start": 50099862, "end": 50101773}, {"filename": "/GameData/Textures/lq_tech/comp1_1.png", "start": 50101773, "end": 50105129}, {"filename": "/GameData/Textures/lq_tech/comp1_2.png", "start": 50105129, "end": 50107808}, {"filename": "/GameData/Textures/lq_tech/comp1_3.png", "start": 50107808, "end": 50110297}, {"filename": "/GameData/Textures/lq_tech/comp1_3b.png", "start": 50110297, "end": 50112925}, {"filename": "/GameData/Textures/lq_tech/comp1_4.png", "start": 50112925, "end": 50114530}, {"filename": "/GameData/Textures/lq_tech/comp1_5.png", "start": 50114530, "end": 50117629}, {"filename": "/GameData/Textures/lq_tech/comp1_6.png", "start": 50117629, "end": 50121071}, {"filename": "/GameData/Textures/lq_tech/comp1_7.png", "start": 50121071, "end": 50123416}, {"filename": "/GameData/Textures/lq_tech/comp1_8.png", "start": 50123416, "end": 50125745}, {"filename": "/GameData/Textures/lq_tech/compbase.png", "start": 50125745, "end": 50128164}, {"filename": "/GameData/Textures/lq_tech/crate.png", "start": 50128164, "end": 50131281}, {"filename": "/GameData/Textures/lq_tech/crate0_bottom.png", "start": 50131281, "end": 50133397}, {"filename": "/GameData/Textures/lq_tech/crate0_s_bottom.png", "start": 50133397, "end": 50134485}, {"filename": "/GameData/Textures/lq_tech/crate0_s_sside.png", "start": 50134485, "end": 50135430}, {"filename": "/GameData/Textures/lq_tech/crate0_s_top.png", "start": 50135430, "end": 50136869}, {"filename": "/GameData/Textures/lq_tech/crate0_s_tside.png", "start": 50136869, "end": 50138532}, {"filename": "/GameData/Textures/lq_tech/crate0_side.png", "start": 50138532, "end": 50142000}, {"filename": "/GameData/Textures/lq_tech/crate0_top.png", "start": 50142000, "end": 50144746}, {"filename": "/GameData/Textures/lq_tech/crate0_xs_bot.png", "start": 50144746, "end": 50145413}, {"filename": "/GameData/Textures/lq_tech/crate0_xs_sside.png", "start": 50145413, "end": 50146358}, {"filename": "/GameData/Textures/lq_tech/crate0_xs_top.png", "start": 50146358, "end": 50147228}, {"filename": "/GameData/Textures/lq_tech/crate0_xs_tside.png", "start": 50147228, "end": 50148168}, {"filename": "/GameData/Textures/lq_tech/crate1_bottom.png", "start": 50148168, "end": 50150132}, {"filename": "/GameData/Textures/lq_tech/crate1_s_bottom.png", "start": 50150132, "end": 50151162}, {"filename": "/GameData/Textures/lq_tech/crate1_s_sside.png", "start": 50151162, "end": 50152040}, {"filename": "/GameData/Textures/lq_tech/crate1_s_top.png", "start": 50152040, "end": 50153173}, {"filename": "/GameData/Textures/lq_tech/crate1_s_tside.png", "start": 50153173, "end": 50154638}, {"filename": "/GameData/Textures/lq_tech/crate1_side.png", "start": 50154638, "end": 50157755}, {"filename": "/GameData/Textures/lq_tech/crate1_top.png", "start": 50157755, "end": 50159876}, {"filename": "/GameData/Textures/lq_tech/crate1_xs_bot.png", "start": 50159876, "end": 50160530}, {"filename": "/GameData/Textures/lq_tech/crate1_xs_sside.png", "start": 50160530, "end": 50161408}, {"filename": "/GameData/Textures/lq_tech/crate1_xs_top.png", "start": 50161408, "end": 50162115}, {"filename": "/GameData/Textures/lq_tech/crate1_xs_tside.png", "start": 50162115, "end": 50162953}, {"filename": "/GameData/Textures/lq_tech/dem4_1.png", "start": 50162953, "end": 50172352}, {"filename": "/GameData/Textures/lq_tech/dem4_4.png", "start": 50172352, "end": 50180398}, {"filename": "/GameData/Textures/lq_tech/dem5_3_fbr.png", "start": 50180398, "end": 50189215}, {"filename": "/GameData/Textures/lq_tech/door02_1.png", "start": 50189215, "end": 50193347}, {"filename": "/GameData/Textures/lq_tech/doorr02_1.png", "start": 50193347, "end": 50194913}, {"filename": "/GameData/Textures/lq_tech/doortrak1.png", "start": 50194913, "end": 50196524}, {"filename": "/GameData/Textures/lq_tech/doortrak2-corn.png", "start": 50196524, "end": 50197821}, {"filename": "/GameData/Textures/lq_tech/doortrak2.png", "start": 50197821, "end": 50199486}, {"filename": "/GameData/Textures/lq_tech/ecop1_1.png", "start": 50199486, "end": 50202563}, {"filename": "/GameData/Textures/lq_tech/ecop1_4.png", "start": 50202563, "end": 50206164}, {"filename": "/GameData/Textures/lq_tech/edoor01_1.png", "start": 50206164, "end": 50218310}, {"filename": "/GameData/Textures/lq_tech/edoor02.png", "start": 50218310, "end": 50230361}, {"filename": "/GameData/Textures/lq_tech/edoor02.png.png", "start": 50230361, "end": 50242412}, {"filename": "/GameData/Textures/lq_tech/fddoor01.png", "start": 50242412, "end": 50251166}, {"filename": "/GameData/Textures/lq_tech/fddoor01b.png", "start": 50251166, "end": 50261849}, {"filename": "/GameData/Textures/lq_tech/fdoor02.png", "start": 50261849, "end": 50270403}, {"filename": "/GameData/Textures/lq_tech/flat4.png", "start": 50270403, "end": 50271576}, {"filename": "/GameData/Textures/lq_tech/floor5_2.png", "start": 50271576, "end": 50274302}, {"filename": "/GameData/Textures/lq_tech/floor5_3.png", "start": 50274302, "end": 50276482}, {"filename": "/GameData/Textures/lq_tech/laserfield1_fbr.png", "start": 50276482, "end": 50287491}, {"filename": "/GameData/Textures/lq_tech/light2.png", "start": 50287491, "end": 50288089}, {"filename": "/GameData/Textures/lq_tech/lit8nb.png", "start": 50288089, "end": 50288403}, {"filename": "/GameData/Textures/lq_tech/lit8sfb_fbr.png", "start": 50288403, "end": 50288700}, {"filename": "/GameData/Textures/lq_tech/met2.png", "start": 50288700, "end": 50301024}, {"filename": "/GameData/Textures/lq_tech/metalstrip_1.png", "start": 50301024, "end": 50303307}, {"filename": "/GameData/Textures/lq_tech/plat_side1.png", "start": 50303307, "end": 50304072}, {"filename": "/GameData/Textures/lq_tech/plat_stem.png", "start": 50304072, "end": 50304718}, {"filename": "/GameData/Textures/lq_tech/plat_top1.png", "start": 50304718, "end": 50308118}, {"filename": "/GameData/Textures/lq_tech/plat_top2.png", "start": 50308118, "end": 50311833}, {"filename": "/GameData/Textures/lq_tech/plat_top3.png", "start": 50311833, "end": 50315241}, {"filename": "/GameData/Textures/lq_tech/plat_top4.png", "start": 50315241, "end": 50317937}, {"filename": "/GameData/Textures/lq_tech/plat_top5.png", "start": 50317937, "end": 50320032}, {"filename": "/GameData/Textures/lq_tech/plus_0_gkey.png", "start": 50320032, "end": 50321096}, {"filename": "/GameData/Textures/lq_tech/plus_0_skey.png", "start": 50321096, "end": 50322112}, {"filename": "/GameData/Textures/lq_tech/plus_0_tscrn0.png", "start": 50322112, "end": 50324225}, {"filename": "/GameData/Textures/lq_tech/plus_0_tscrn1.png", "start": 50324225, "end": 50326350}, {"filename": "/GameData/Textures/lq_tech/plus_0arrow2_d_fbr.png", "start": 50326350, "end": 50327604}, {"filename": "/GameData/Textures/lq_tech/plus_0arrow2_h_fbr.png", "start": 50327604, "end": 50328873}, {"filename": "/GameData/Textures/lq_tech/plus_0arrow2_u_fbr.png", "start": 50328873, "end": 50330131}, {"filename": "/GameData/Textures/lq_tech/plus_0arrow_d_fbr.png", "start": 50330131, "end": 50331013}, {"filename": "/GameData/Textures/lq_tech/plus_0arrow_h_fbr.png", "start": 50331013, "end": 50331892}, {"filename": "/GameData/Textures/lq_tech/plus_0arrow_u_fbr.png", "start": 50331892, "end": 50332767}, {"filename": "/GameData/Textures/lq_tech/plus_0basebtn1b_fbr.png", "start": 50332767, "end": 50333656}, {"filename": "/GameData/Textures/lq_tech/plus_0basebtn2_fbr.png", "start": 50333656, "end": 50334002}, {"filename": "/GameData/Textures/lq_tech/plus_0basebtn2b_fbr.png", "start": 50334002, "end": 50334342}, {"filename": "/GameData/Textures/lq_tech/plus_0basebtn_fbr.png", "start": 50334342, "end": 50335623}, {"filename": "/GameData/Textures/lq_tech/plus_0button3_fbr.png", "start": 50335623, "end": 50337499}, {"filename": "/GameData/Textures/lq_tech/plus_0lit8s.png", "start": 50337499, "end": 50337813}, {"filename": "/GameData/Textures/lq_tech/plus_0planet_a_fbr.png", "start": 50337813, "end": 50339404}, {"filename": "/GameData/Textures/lq_tech/plus_0planet_b_fbr.png", "start": 50339404, "end": 50340514}, {"filename": "/GameData/Textures/lq_tech/plus_0planet_c_fbr.png", "start": 50340514, "end": 50341614}, {"filename": "/GameData/Textures/lq_tech/plus_0slipbot.png", "start": 50341614, "end": 50344897}, {"filename": "/GameData/Textures/lq_tech/plus_0sliptop.png", "start": 50344897, "end": 50348540}, {"filename": "/GameData/Textures/lq_tech/plus_0tek_jump1_fbr.png", "start": 50348540, "end": 50351023}, {"filename": "/GameData/Textures/lq_tech/plus_0term128.png", "start": 50351023, "end": 50353827}, {"filename": "/GameData/Textures/lq_tech/plus_0term64.png", "start": 50353827, "end": 50355019}, {"filename": "/GameData/Textures/lq_tech/plus_0tlight1.png", "start": 50355019, "end": 50355617}, {"filename": "/GameData/Textures/lq_tech/plus_0tlight2.png", "start": 50355617, "end": 50356230}, {"filename": "/GameData/Textures/lq_tech/plus_0tlight3.png", "start": 50356230, "end": 50356801}, {"filename": "/GameData/Textures/lq_tech/plus_1_gkey.png", "start": 50356801, "end": 50357851}, {"filename": "/GameData/Textures/lq_tech/plus_1_skey.png", "start": 50357851, "end": 50358856}, {"filename": "/GameData/Textures/lq_tech/plus_1arrow2_d_fbr.png", "start": 50358856, "end": 50360112}, {"filename": "/GameData/Textures/lq_tech/plus_1arrow2_h_fbr.png", "start": 50360112, "end": 50361379}, {"filename": "/GameData/Textures/lq_tech/plus_1arrow2_u_fbr.png", "start": 50361379, "end": 50362637}, {"filename": "/GameData/Textures/lq_tech/plus_1arrow_d_fbr.png", "start": 50362637, "end": 50363523}, {"filename": "/GameData/Textures/lq_tech/plus_1arrow_h_fbr.png", "start": 50363523, "end": 50364405}, {"filename": "/GameData/Textures/lq_tech/plus_1arrow_u_fbr.png", "start": 50364405, "end": 50365283}, {"filename": "/GameData/Textures/lq_tech/plus_1basebtn.png", "start": 50365283, "end": 50366562}, {"filename": "/GameData/Textures/lq_tech/plus_1basebtn1b.png", "start": 50366562, "end": 50367440}, {"filename": "/GameData/Textures/lq_tech/plus_1basebtn2.png", "start": 50367440, "end": 50367790}, {"filename": "/GameData/Textures/lq_tech/plus_1basebtn2b.png", "start": 50367790, "end": 50368148}, {"filename": "/GameData/Textures/lq_tech/plus_1planet_a_fbr.png", "start": 50368148, "end": 50369769}, {"filename": "/GameData/Textures/lq_tech/plus_1planet_b_fbr.png", "start": 50369769, "end": 50370854}, {"filename": "/GameData/Textures/lq_tech/plus_1planet_c_fbr.png", "start": 50370854, "end": 50371926}, {"filename": "/GameData/Textures/lq_tech/plus_1tek_jump1_fbr.png", "start": 50371926, "end": 50374409}, {"filename": "/GameData/Textures/lq_tech/plus_1term128.png", "start": 50374409, "end": 50377212}, {"filename": "/GameData/Textures/lq_tech/plus_1term64.png", "start": 50377212, "end": 50378404}, {"filename": "/GameData/Textures/lq_tech/plus_2_gkey.png", "start": 50378404, "end": 50379466}, {"filename": "/GameData/Textures/lq_tech/plus_2_skey.png", "start": 50379466, "end": 50380472}, {"filename": "/GameData/Textures/lq_tech/plus_2arrow2_d_fbr.png", "start": 50380472, "end": 50381715}, {"filename": "/GameData/Textures/lq_tech/plus_2arrow2_h_fbr.png", "start": 50381715, "end": 50382976}, {"filename": "/GameData/Textures/lq_tech/plus_2arrow2_u_fbr.png", "start": 50382976, "end": 50384222}, {"filename": "/GameData/Textures/lq_tech/plus_2arrow_d_fbr.png", "start": 50384222, "end": 50385120}, {"filename": "/GameData/Textures/lq_tech/plus_2arrow_h_fbr.png", "start": 50385120, "end": 50386010}, {"filename": "/GameData/Textures/lq_tech/plus_2arrow_u_fbr.png", "start": 50386010, "end": 50386897}, {"filename": "/GameData/Textures/lq_tech/plus_2planet_a_fbr.png", "start": 50386897, "end": 50388500}, {"filename": "/GameData/Textures/lq_tech/plus_2planet_b_fbr.png", "start": 50388500, "end": 50389602}, {"filename": "/GameData/Textures/lq_tech/plus_2planet_c_fbr.png", "start": 50389602, "end": 50390720}, {"filename": "/GameData/Textures/lq_tech/plus_3planet_a_fbr.png", "start": 50390720, "end": 50392310}, {"filename": "/GameData/Textures/lq_tech/plus_3planet_b_fbr.png", "start": 50392310, "end": 50393404}, {"filename": "/GameData/Textures/lq_tech/plus_3planet_c_fbr.png", "start": 50393404, "end": 50394527}, {"filename": "/GameData/Textures/lq_tech/plus_4planet_a_fbr.png", "start": 50394527, "end": 50396136}, {"filename": "/GameData/Textures/lq_tech/plus_4planet_b_fbr.png", "start": 50396136, "end": 50397251}, {"filename": "/GameData/Textures/lq_tech/plus_4planet_c_fbr.png", "start": 50397251, "end": 50398398}, {"filename": "/GameData/Textures/lq_tech/plus_5planet_a_fbr.png", "start": 50398398, "end": 50400030}, {"filename": "/GameData/Textures/lq_tech/plus_5planet_b_fbr.png", "start": 50400030, "end": 50401157}, {"filename": "/GameData/Textures/lq_tech/plus_5planet_c_fbr.png", "start": 50401157, "end": 50402264}, {"filename": "/GameData/Textures/lq_tech/plus_6planet_a_fbr.png", "start": 50402264, "end": 50403885}, {"filename": "/GameData/Textures/lq_tech/plus_6planet_b_fbr.png", "start": 50403885, "end": 50405009}, {"filename": "/GameData/Textures/lq_tech/plus_6planet_c_fbr.png", "start": 50405009, "end": 50406103}, {"filename": "/GameData/Textures/lq_tech/plus_7planet_a_fbr.png", "start": 50406103, "end": 50407691}, {"filename": "/GameData/Textures/lq_tech/plus_7planet_b_fbr.png", "start": 50407691, "end": 50408804}, {"filename": "/GameData/Textures/lq_tech/plus_7planet_c_fbr.png", "start": 50408804, "end": 50409896}, {"filename": "/GameData/Textures/lq_tech/plus_8planet_a_fbr.png", "start": 50409896, "end": 50411518}, {"filename": "/GameData/Textures/lq_tech/plus_8planet_b_fbr.png", "start": 50411518, "end": 50412645}, {"filename": "/GameData/Textures/lq_tech/plus_8planet_c_fbr.png", "start": 50412645, "end": 50413734}, {"filename": "/GameData/Textures/lq_tech/plus_9planet_a_fbr.png", "start": 50413734, "end": 50415318}, {"filename": "/GameData/Textures/lq_tech/plus_9planet_b_fbr.png", "start": 50415318, "end": 50416406}, {"filename": "/GameData/Textures/lq_tech/plus_9planet_c_fbr.png", "start": 50416406, "end": 50417482}, {"filename": "/GameData/Textures/lq_tech/plus_A_tscrn0.png", "start": 50417482, "end": 50419105}, {"filename": "/GameData/Textures/lq_tech/plus_A_tscrn1.png", "start": 50419105, "end": 50421614}, {"filename": "/GameData/Textures/lq_tech/plus_A_tscrn2.png", "start": 50421614, "end": 50423214}, {"filename": "/GameData/Textures/lq_tech/plus_abasebtn.png", "start": 50423214, "end": 50424503}, {"filename": "/GameData/Textures/lq_tech/plus_abasebtn1b.png", "start": 50424503, "end": 50425792}, {"filename": "/GameData/Textures/lq_tech/plus_abasebtn2.png", "start": 50425792, "end": 50426163}, {"filename": "/GameData/Textures/lq_tech/plus_abasebtn2b.png", "start": 50426163, "end": 50426534}, {"filename": "/GameData/Textures/lq_tech/plus_abasebtnb.png", "start": 50426534, "end": 50426905}, {"filename": "/GameData/Textures/lq_tech/plus_abutton3_fbr.png", "start": 50426905, "end": 50428793}, {"filename": "/GameData/Textures/lq_tech/plus_alit8s_fbr.png", "start": 50428793, "end": 50429090}, {"filename": "/GameData/Textures/lq_tech/plus_atek_jump1_fbr.png", "start": 50429090, "end": 50431583}, {"filename": "/GameData/Textures/lq_tech/plus_atlight1_fbr.png", "start": 50431583, "end": 50432173}, {"filename": "/GameData/Textures/lq_tech/plus_atlight2_fbr.png", "start": 50432173, "end": 50432746}, {"filename": "/GameData/Textures/lq_tech/plus_atlight3_fbr.png", "start": 50432746, "end": 50433348}, {"filename": "/GameData/Textures/lq_tech/rw33_1.png", "start": 50433348, "end": 50438913}, {"filename": "/GameData/Textures/lq_tech/rw33_2.png", "start": 50438913, "end": 50444767}, {"filename": "/GameData/Textures/lq_tech/rw33_3.png", "start": 50444767, "end": 50450513}, {"filename": "/GameData/Textures/lq_tech/rw33_4.png", "start": 50450513, "end": 50453079}, {"filename": "/GameData/Textures/lq_tech/rw33_4b_l.png", "start": 50453079, "end": 50456039}, {"filename": "/GameData/Textures/lq_tech/rw33_5.png", "start": 50456039, "end": 50458911}, {"filename": "/GameData/Textures/lq_tech/rw33_flat.png", "start": 50458911, "end": 50464862}, {"filename": "/GameData/Textures/lq_tech/rw33_lit.png", "start": 50464862, "end": 50465686}, {"filename": "/GameData/Textures/lq_tech/rw33b_1.png", "start": 50465686, "end": 50471149}, {"filename": "/GameData/Textures/lq_tech/rw33b_2.png", "start": 50471149, "end": 50476991}, {"filename": "/GameData/Textures/lq_tech/rw33b_3.png", "start": 50476991, "end": 50482978}, {"filename": "/GameData/Textures/lq_tech/rw33b_4.png", "start": 50482978, "end": 50485740}, {"filename": "/GameData/Textures/lq_tech/rw33b_5.png", "start": 50485740, "end": 50488789}, {"filename": "/GameData/Textures/lq_tech/rw33b_flat.png", "start": 50488789, "end": 50494987}, {"filename": "/GameData/Textures/lq_tech/rw33b_lit.png", "start": 50494987, "end": 50495939}, {"filename": "/GameData/Textures/lq_tech/rw37_1.png", "start": 50495939, "end": 50501551}, {"filename": "/GameData/Textures/lq_tech/rw37_2.png", "start": 50501551, "end": 50507681}, {"filename": "/GameData/Textures/lq_tech/rw37_3.png", "start": 50507681, "end": 50513576}, {"filename": "/GameData/Textures/lq_tech/rw37_4.png", "start": 50513576, "end": 50520587}, {"filename": "/GameData/Textures/lq_tech/rw37_trim1.png", "start": 50520587, "end": 50523095}, {"filename": "/GameData/Textures/lq_tech/rw37_trim2.png", "start": 50523095, "end": 50524878}, {"filename": "/GameData/Textures/lq_tech/rw37_trim3.png", "start": 50524878, "end": 50527369}, {"filename": "/GameData/Textures/lq_tech/rw39_1_fbr.png", "start": 50527369, "end": 50533314}, {"filename": "/GameData/Textures/lq_tech/spotlight_fbr.png", "start": 50533314, "end": 50535926}, {"filename": "/GameData/Textures/lq_tech/star_lasergrid.png", "start": 50535926, "end": 50536306}, {"filename": "/GameData/Textures/lq_tech/t_band1a.png", "start": 50536306, "end": 50538810}, {"filename": "/GameData/Textures/lq_tech/t_band1b.png", "start": 50538810, "end": 50541418}, {"filename": "/GameData/Textures/lq_tech/t_blok01.png", "start": 50541418, "end": 50544096}, {"filename": "/GameData/Textures/lq_tech/t_blok01a.png", "start": 50544096, "end": 50546868}, {"filename": "/GameData/Textures/lq_tech/t_blok02.png", "start": 50546868, "end": 50551995}, {"filename": "/GameData/Textures/lq_tech/t_blok02a.png", "start": 50551995, "end": 50556854}, {"filename": "/GameData/Textures/lq_tech/t_blok03.png", "start": 50556854, "end": 50559296}, {"filename": "/GameData/Textures/lq_tech/t_blok03a.png", "start": 50559296, "end": 50561151}, {"filename": "/GameData/Textures/lq_tech/t_blok04.png", "start": 50561151, "end": 50564217}, {"filename": "/GameData/Textures/lq_tech/t_blok04h.png", "start": 50564217, "end": 50566941}, {"filename": "/GameData/Textures/lq_tech/t_blok05.png", "start": 50566941, "end": 50571229}, {"filename": "/GameData/Textures/lq_tech/t_blok06.png", "start": 50571229, "end": 50574079}, {"filename": "/GameData/Textures/lq_tech/t_blok06h.png", "start": 50574079, "end": 50576056}, {"filename": "/GameData/Textures/lq_tech/t_blok07.png", "start": 50576056, "end": 50578842}, {"filename": "/GameData/Textures/lq_tech/t_blok07a.png", "start": 50578842, "end": 50581628}, {"filename": "/GameData/Textures/lq_tech/t_blok08.png", "start": 50581628, "end": 50586135}, {"filename": "/GameData/Textures/lq_tech/t_blok09.png", "start": 50586135, "end": 50589455}, {"filename": "/GameData/Textures/lq_tech/t_blok10.png", "start": 50589455, "end": 50595466}, {"filename": "/GameData/Textures/lq_tech/t_blok10b.png", "start": 50595466, "end": 50601174}, {"filename": "/GameData/Textures/lq_tech/t_blok10c.png", "start": 50601174, "end": 50604392}, {"filename": "/GameData/Textures/lq_tech/t_blok11.png", "start": 50604392, "end": 50610333}, {"filename": "/GameData/Textures/lq_tech/t_blok11b.png", "start": 50610333, "end": 50615938}, {"filename": "/GameData/Textures/lq_tech/t_blok12c.png", "start": 50615938, "end": 50619145}, {"filename": "/GameData/Textures/lq_tech/t_flat01.png", "start": 50619145, "end": 50621579}, {"filename": "/GameData/Textures/lq_tech/t_flat02.png", "start": 50621579, "end": 50623944}, {"filename": "/GameData/Textures/lq_tech/t_flat05.png", "start": 50623944, "end": 50632140}, {"filename": "/GameData/Textures/lq_tech/t_flor1a.png", "start": 50632140, "end": 50635752}, {"filename": "/GameData/Textures/lq_tech/t_flor1b.png", "start": 50635752, "end": 50639370}, {"filename": "/GameData/Textures/lq_tech/t_flor2a.png", "start": 50639370, "end": 50641964}, {"filename": "/GameData/Textures/lq_tech/t_flor2b.png", "start": 50641964, "end": 50643900}, {"filename": "/GameData/Textures/lq_tech/t_flor2c.png", "start": 50643900, "end": 50645380}, {"filename": "/GameData/Textures/lq_tech/t_flor2d.png", "start": 50645380, "end": 50645884}, {"filename": "/GameData/Textures/lq_tech/t_lit01_fbr.png", "start": 50645884, "end": 50646064}, {"filename": "/GameData/Textures/lq_tech/t_lit02_fbr.png", "start": 50646064, "end": 50646206}, {"filename": "/GameData/Textures/lq_tech/t_lit03_fbr.png", "start": 50646206, "end": 50646341}, {"filename": "/GameData/Textures/lq_tech/t_lit04_fbr.png", "start": 50646341, "end": 50646473}, {"filename": "/GameData/Textures/lq_tech/t_lit05_fbr.png", "start": 50646473, "end": 50646639}, {"filename": "/GameData/Textures/lq_tech/t_lit06_fbr.png", "start": 50646639, "end": 50646795}, {"filename": "/GameData/Textures/lq_tech/t_lit07_fbr.png", "start": 50646795, "end": 50649098}, {"filename": "/GameData/Textures/lq_tech/t_lit08_fbr.png", "start": 50649098, "end": 50649315}, {"filename": "/GameData/Textures/lq_tech/t_metalsheeta.png", "start": 50649315, "end": 50655914}, {"filename": "/GameData/Textures/lq_tech/t_metalsheetb.png", "start": 50655914, "end": 50667427}, {"filename": "/GameData/Textures/lq_tech/t_num_0_fbr.png", "start": 50667427, "end": 50668201}, {"filename": "/GameData/Textures/lq_tech/t_num_1_fbr.png", "start": 50668201, "end": 50668963}, {"filename": "/GameData/Textures/lq_tech/t_num_2_fbr.png", "start": 50668963, "end": 50669758}, {"filename": "/GameData/Textures/lq_tech/t_num_3_fbr.png", "start": 50669758, "end": 50670516}, {"filename": "/GameData/Textures/lq_tech/t_num_4_fbr.png", "start": 50670516, "end": 50671321}, {"filename": "/GameData/Textures/lq_tech/t_num_5_fbr.png", "start": 50671321, "end": 50672119}, {"filename": "/GameData/Textures/lq_tech/t_num_6_fbr.png", "start": 50672119, "end": 50672916}, {"filename": "/GameData/Textures/lq_tech/t_num_7_fbr.png", "start": 50672916, "end": 50673690}, {"filename": "/GameData/Textures/lq_tech/t_num_8_fbr.png", "start": 50673690, "end": 50674470}, {"filename": "/GameData/Textures/lq_tech/t_num_9_fbr.png", "start": 50674470, "end": 50675263}, {"filename": "/GameData/Textures/lq_tech/t_num_x.png", "start": 50675263, "end": 50676071}, {"filename": "/GameData/Textures/lq_tech/t_rivs01.png", "start": 50676071, "end": 50678499}, {"filename": "/GameData/Textures/lq_tech/t_rivs01a.png", "start": 50678499, "end": 50680953}, {"filename": "/GameData/Textures/lq_tech/t_sign1.png", "start": 50680953, "end": 50683903}, {"filename": "/GameData/Textures/lq_tech/t_tech01.png", "start": 50683903, "end": 50687954}, {"filename": "/GameData/Textures/lq_tech/t_tech02.png", "start": 50687954, "end": 50691057}, {"filename": "/GameData/Textures/lq_tech/t_tech03.png", "start": 50691057, "end": 50707048}, {"filename": "/GameData/Textures/lq_tech/t_tech04.png", "start": 50707048, "end": 50709903}, {"filename": "/GameData/Textures/lq_tech/t_tech05.png", "start": 50709903, "end": 50712682}, {"filename": "/GameData/Textures/lq_tech/t_tech06.png", "start": 50712682, "end": 50715494}, {"filename": "/GameData/Textures/lq_tech/t_trim1a.png", "start": 50715494, "end": 50717982}, {"filename": "/GameData/Textures/lq_tech/t_trim1aa.png", "start": 50717982, "end": 50720617}, {"filename": "/GameData/Textures/lq_tech/t_trim1b.png", "start": 50720617, "end": 50723348}, {"filename": "/GameData/Textures/lq_tech/t_trim1ba.png", "start": 50723348, "end": 50725972}, {"filename": "/GameData/Textures/lq_tech/t_trim1c.png", "start": 50725972, "end": 50728757}, {"filename": "/GameData/Textures/lq_tech/t_trim1ca.png", "start": 50728757, "end": 50731583}, {"filename": "/GameData/Textures/lq_tech/t_trim1d.png", "start": 50731583, "end": 50733883}, {"filename": "/GameData/Textures/lq_tech/t_trim1e.png", "start": 50733883, "end": 50736028}, {"filename": "/GameData/Textures/lq_tech/t_trim2a.png", "start": 50736028, "end": 50739101}, {"filename": "/GameData/Textures/lq_tech/t_trim2aa.png", "start": 50739101, "end": 50741753}, {"filename": "/GameData/Textures/lq_tech/t_trim2b.png", "start": 50741753, "end": 50744381}, {"filename": "/GameData/Textures/lq_tech/t_trim2ba.png", "start": 50744381, "end": 50747069}, {"filename": "/GameData/Textures/lq_tech/t_trim2c.png", "start": 50747069, "end": 50749860}, {"filename": "/GameData/Textures/lq_tech/t_trim2ca.png", "start": 50749860, "end": 50752739}, {"filename": "/GameData/Textures/lq_tech/t_trim2d.png", "start": 50752739, "end": 50755201}, {"filename": "/GameData/Textures/lq_tech/t_trim2e.png", "start": 50755201, "end": 50757708}, {"filename": "/GameData/Textures/lq_tech/t_tris02.png", "start": 50757708, "end": 50760622}, {"filename": "/GameData/Textures/lq_tech/t_wall05.png", "start": 50760622, "end": 50763948}, {"filename": "/GameData/Textures/lq_tech/t_wall1a.png", "start": 50763948, "end": 50774475}, {"filename": "/GameData/Textures/lq_tech/t_wall1aa.png", "start": 50774475, "end": 50784796}, {"filename": "/GameData/Textures/lq_tech/t_wall1b.png", "start": 50784796, "end": 50795896}, {"filename": "/GameData/Textures/lq_tech/t_wall1ba.png", "start": 50795896, "end": 50806735}, {"filename": "/GameData/Textures/lq_tech/t_wall2a.png", "start": 50806735, "end": 50817519}, {"filename": "/GameData/Textures/lq_tech/t_wall2aa.png", "start": 50817519, "end": 50828539}, {"filename": "/GameData/Textures/lq_tech/t_wall2ab.png", "start": 50828539, "end": 50840296}, {"filename": "/GameData/Textures/lq_tech/t_wall2b.png", "start": 50840296, "end": 50854534}, {"filename": "/GameData/Textures/lq_tech/t_wall2ba.png", "start": 50854534, "end": 50867985}, {"filename": "/GameData/Textures/lq_tech/t_wall3a.png", "start": 50867985, "end": 50875952}, {"filename": "/GameData/Textures/lq_tech/t_wall3aa.png", "start": 50875952, "end": 50884219}, {"filename": "/GameData/Textures/lq_tech/t_wall3b.png", "start": 50884219, "end": 50892541}, {"filename": "/GameData/Textures/lq_tech/t_wall3ba.png", "start": 50892541, "end": 50901504}, {"filename": "/GameData/Textures/lq_tech/t_wall6a.png", "start": 50901504, "end": 50904738}, {"filename": "/GameData/Textures/lq_tech/t_wall6b.png", "start": 50904738, "end": 50907752}, {"filename": "/GameData/Textures/lq_tech/t_wall6c.png", "start": 50907752, "end": 50911428}, {"filename": "/GameData/Textures/lq_tech/t_wall6d.png", "start": 50911428, "end": 50915131}, {"filename": "/GameData/Textures/lq_tech/t_wall6e.png", "start": 50915131, "end": 50918927}, {"filename": "/GameData/Textures/lq_tech/t_wall7a.png", "start": 50918927, "end": 50929819}, {"filename": "/GameData/Textures/lq_tech/t_wall7b.png", "start": 50929819, "end": 50937260}, {"filename": "/GameData/Textures/lq_tech/t_wire01.png", "start": 50937260, "end": 50940196}, {"filename": "/GameData/Textures/lq_tech/t_wire02.png", "start": 50940196, "end": 50943731}, {"filename": "/GameData/Textures/lq_tech/t_wire03.png", "start": 50943731, "end": 50947331}, {"filename": "/GameData/Textures/lq_tech/tech04_1.png", "start": 50947331, "end": 50948154}, {"filename": "/GameData/Textures/lq_tech/tech04_3.png", "start": 50948154, "end": 50949623}, {"filename": "/GameData/Textures/lq_tech/tech08_1.png", "start": 50949623, "end": 50960676}, {"filename": "/GameData/Textures/lq_tech/tech08_2.png", "start": 50960676, "end": 50971729}, {"filename": "/GameData/Textures/lq_tech/tech10_3.png", "start": 50971729, "end": 50975585}, {"filename": "/GameData/Textures/lq_tech/tech14-1.png", "start": 50975585, "end": 50985856}, {"filename": "/GameData/Textures/lq_tech/techbasetextures.txt", "start": 50985856, "end": 50986366}, {"filename": "/GameData/Textures/lq_tech/techeye1_fbr.png", "start": 50986366, "end": 50989710}, {"filename": "/GameData/Textures/lq_tech/techeye2_fbr.png", "start": 50989710, "end": 50993112}, {"filename": "/GameData/Textures/lq_tech/tek_door1.png", "start": 50993112, "end": 51005258}, {"filename": "/GameData/Textures/lq_tech/tek_door2.png", "start": 51005258, "end": 51017309}, {"filename": "/GameData/Textures/lq_tech/tek_flr3.png", "start": 51017309, "end": 51020690}, {"filename": "/GameData/Textures/lq_tech/tek_grate.png", "start": 51020690, "end": 51023568}, {"filename": "/GameData/Textures/lq_tech/tek_lit1_fbr.png", "start": 51023568, "end": 51025203}, {"filename": "/GameData/Textures/lq_tech/tek_lit2_fbr.png", "start": 51025203, "end": 51026189}, {"filename": "/GameData/Textures/lq_tech/tek_lit3_fbr.png", "start": 51026189, "end": 51028031}, {"filename": "/GameData/Textures/lq_tech/tek_lit4_fbr.png", "start": 51028031, "end": 51029158}, {"filename": "/GameData/Textures/lq_tech/tek_pip1_fbr.png", "start": 51029158, "end": 51032240}, {"filename": "/GameData/Textures/lq_tech/tek_pipe1.png", "start": 51032240, "end": 51035040}, {"filename": "/GameData/Textures/lq_tech/tek_pipe2.png", "start": 51035040, "end": 51036699}, {"filename": "/GameData/Textures/lq_tech/tek_trm1.png", "start": 51036699, "end": 51039157}, {"filename": "/GameData/Textures/lq_tech/tek_trm3.png", "start": 51039157, "end": 51041767}, {"filename": "/GameData/Textures/lq_tech/tek_wall4_1.png", "start": 51041767, "end": 51056212}, {"filename": "/GameData/Textures/lq_tech/tele_frame1.png", "start": 51056212, "end": 51062367}, {"filename": "/GameData/Textures/lq_tech/tele_frame2.png", "start": 51062367, "end": 51064318}, {"filename": "/GameData/Textures/lq_tech/tele_frame3.png", "start": 51064318, "end": 51068305}, {"filename": "/GameData/Textures/lq_tech/telepad1_fbr.png", "start": 51068305, "end": 51070281}, {"filename": "/GameData/Textures/lq_tech/tlight11_fbr.png", "start": 51070281, "end": 51071907}, {"filename": "/GameData/Textures/lq_tech/tlight12_fbr.png", "start": 51071907, "end": 51073602}, {"filename": "/GameData/Textures/lq_tech/tlight13_fbr.png", "start": 51073602, "end": 51075661}, {"filename": "/GameData/Textures/lq_tech/tlightblfb_fbr.png", "start": 51075661, "end": 51076234}, {"filename": "/GameData/Textures/lq_tech/tlightfb_fbr.png", "start": 51076234, "end": 51076824}, {"filename": "/GameData/Textures/lq_tech/tlightnb.png", "start": 51076824, "end": 51077422}, {"filename": "/GameData/Textures/lq_tech/tlightrdfb_fbr.png", "start": 51077422, "end": 51078024}, {"filename": "/GameData/Textures/lq_tech/treadplatemetal.png", "start": 51078024, "end": 51092778}, {"filename": "/GameData/Textures/lq_tech/twall2_3.png", "start": 51092778, "end": 51096286}, {"filename": "/GameData/Textures/lq_tech/w17_1.png", "start": 51096286, "end": 51113310}, {"filename": "/GameData/Textures/lq_tech/w94_1.png", "start": 51113310, "end": 51126258}, {"filename": "/GameData/Textures/lq_tech/z_exit_fbr.png", "start": 51126258, "end": 51127819}, {"filename": "/GameData/Textures/lq_terra/afloor1_3.png", "start": 51127819, "end": 51130817}, {"filename": "/GameData/Textures/lq_terra/asphalt.png", "start": 51130817, "end": 51148277}, {"filename": "/GameData/Textures/lq_terra/azfloor1_1.png", "start": 51148277, "end": 51151296}, {"filename": "/GameData/Textures/lq_terra/badlawn.png", "start": 51151296, "end": 51193105}, {"filename": "/GameData/Textures/lq_terra/cracks1-1.png", "start": 51193105, "end": 51196124}, {"filename": "/GameData/Textures/lq_terra/darkrock.png", "start": 51196124, "end": 51218693}, {"filename": "/GameData/Textures/lq_terra/grass1.png", "start": 51218693, "end": 51229264}, {"filename": "/GameData/Textures/lq_terra/gravel1.png", "start": 51229264, "end": 51242618}, {"filename": "/GameData/Textures/lq_terra/gravel2.png", "start": 51242618, "end": 51257515}, {"filename": "/GameData/Textures/lq_terra/grk_leaf1_1.png", "start": 51257515, "end": 51261508}, {"filename": "/GameData/Textures/lq_terra/grk_leaf1_2.png", "start": 51261508, "end": 51265245}, {"filename": "/GameData/Textures/lq_terra/marbbrn128.png", "start": 51265245, "end": 51275487}, {"filename": "/GameData/Textures/lq_terra/may_drt1_1.png", "start": 51275487, "end": 51278383}, {"filename": "/GameData/Textures/lq_terra/may_drt1_2.png", "start": 51278383, "end": 51281237}, {"filename": "/GameData/Textures/lq_terra/may_drt2_2.png", "start": 51281237, "end": 51283981}, {"filename": "/GameData/Textures/lq_terra/may_rck1_1.png", "start": 51283981, "end": 51298081}, {"filename": "/GameData/Textures/lq_terra/may_rck1_2.png", "start": 51298081, "end": 51308759}, {"filename": "/GameData/Textures/lq_terra/may_rck1_3.png", "start": 51308759, "end": 51321602}, {"filename": "/GameData/Textures/lq_terra/may_slat1_1.png", "start": 51321602, "end": 51324450}, {"filename": "/GameData/Textures/lq_terra/med_bigdirt.png", "start": 51324450, "end": 51479033}, {"filename": "/GameData/Textures/lq_terra/med_bigdirt2.png", "start": 51479033, "end": 51633184}, {"filename": "/GameData/Textures/lq_terra/med_bigdirt3.png", "start": 51633184, "end": 51787135}, {"filename": "/GameData/Textures/lq_terra/med_cobstn1_1.png", "start": 51787135, "end": 51801516}, {"filename": "/GameData/Textures/lq_terra/med_cobstn1_1a.png", "start": 51801516, "end": 51818490}, {"filename": "/GameData/Textures/lq_terra/med_cobstn1_2.png", "start": 51818490, "end": 51832929}, {"filename": "/GameData/Textures/lq_terra/med_cobstn1_2a.png", "start": 51832929, "end": 51849744}, {"filename": "/GameData/Textures/lq_terra/med_cobstn2_1.png", "start": 51849744, "end": 51863713}, {"filename": "/GameData/Textures/lq_terra/med_cobstn2_1a.png", "start": 51863713, "end": 51881024}, {"filename": "/GameData/Textures/lq_terra/med_cobstn2_2.png", "start": 51881024, "end": 51896098}, {"filename": "/GameData/Textures/lq_terra/med_cobstn2_2a.png", "start": 51896098, "end": 51913202}, {"filename": "/GameData/Textures/lq_terra/med_cracks1.png", "start": 51913202, "end": 51926664}, {"filename": "/GameData/Textures/lq_terra/med_flat1.png", "start": 51926664, "end": 51939573}, {"filename": "/GameData/Textures/lq_terra/med_flat12.png", "start": 51939573, "end": 51950961}, {"filename": "/GameData/Textures/lq_terra/med_flat15.png", "start": 51950961, "end": 51964062}, {"filename": "/GameData/Textures/lq_terra/med_flat16.png", "start": 51964062, "end": 51976904}, {"filename": "/GameData/Textures/lq_terra/med_flat2.png", "start": 51976904, "end": 51988181}, {"filename": "/GameData/Textures/lq_terra/med_flat3.png", "start": 51988181, "end": 52001546}, {"filename": "/GameData/Textures/lq_terra/med_flat4.png", "start": 52001546, "end": 52011495}, {"filename": "/GameData/Textures/lq_terra/med_flat5.png", "start": 52011495, "end": 52022580}, {"filename": "/GameData/Textures/lq_terra/med_flat5a.png", "start": 52022580, "end": 52033324}, {"filename": "/GameData/Textures/lq_terra/med_flat6.png", "start": 52033324, "end": 52042309}, {"filename": "/GameData/Textures/lq_terra/med_flat7.png", "start": 52042309, "end": 52053767}, {"filename": "/GameData/Textures/lq_terra/med_plaster2.png", "start": 52053767, "end": 52063419}, {"filename": "/GameData/Textures/lq_terra/med_rock1.png", "start": 52063419, "end": 52114676}, {"filename": "/GameData/Textures/lq_terra/med_rock10.png", "start": 52114676, "end": 52123971}, {"filename": "/GameData/Textures/lq_terra/med_rock10a.png", "start": 52123971, "end": 52133285}, {"filename": "/GameData/Textures/lq_terra/med_rock10b.png", "start": 52133285, "end": 52146650}, {"filename": "/GameData/Textures/lq_terra/med_rock10c.png", "start": 52146650, "end": 52158985}, {"filename": "/GameData/Textures/lq_terra/med_rock2.png", "start": 52158985, "end": 52200862}, {"filename": "/GameData/Textures/lq_terra/med_rock3.png", "start": 52200862, "end": 52246806}, {"filename": "/GameData/Textures/lq_terra/med_rock3_bump.png", "start": 52246806, "end": 52362900}, {"filename": "/GameData/Textures/lq_terra/med_rock4.png", "start": 52362900, "end": 52374256}, {"filename": "/GameData/Textures/lq_terra/med_rock5.png", "start": 52374256, "end": 52386788}, {"filename": "/GameData/Textures/lq_terra/med_rock9.png", "start": 52386788, "end": 52397812}, {"filename": "/GameData/Textures/lq_terra/ret_plaster1.png", "start": 52397812, "end": 52409089}, {"filename": "/GameData/Textures/lq_terra/rock1_1.png", "start": 52409089, "end": 52459460}, {"filename": "/GameData/Textures/lq_terra/rock1_1b.png", "start": 52459460, "end": 52470475}, {"filename": "/GameData/Textures/lq_terra/rock1_2.png", "start": 52470475, "end": 52512518}, {"filename": "/GameData/Textures/lq_terra/rocks07.png", "start": 52512518, "end": 52523542}, {"filename": "/GameData/Textures/lq_terra/rocks11d.png", "start": 52523542, "end": 52532856}, {"filename": "/GameData/Textures/lq_terra/rocks11e.png", "start": 52532856, "end": 52542151}, {"filename": "/GameData/Textures/lq_terra/sand.png", "start": 52542151, "end": 52568238}, {"filename": "/GameData/Textures/lq_terra/snow1.png", "start": 52568238, "end": 52570101}, {"filename": "/GameData/Textures/lq_terra/uwall1_2.png", "start": 52570101, "end": 52597752}, {"filename": "/GameData/Textures/lq_terra/vines1.png", "start": 52597752, "end": 52602807}, {"filename": "/GameData/Textures/lq_utility/black.png", "start": 52602807, "end": 52603355}, {"filename": "/GameData/Textures/lq_utility/clip.png", "start": 52603355, "end": 52603861}, {"filename": "/GameData/Textures/lq_utility/hint.png", "start": 52603861, "end": 52604727}, {"filename": "/GameData/Textures/lq_utility/hintskip.png", "start": 52604727, "end": 52605620}, {"filename": "/GameData/Textures/lq_utility/light_fbr.png", "start": 52605620, "end": 52606536}, {"filename": "/GameData/Textures/lq_utility/origin.png", "start": 52606536, "end": 52607019}, {"filename": "/GameData/Textures/lq_utility/skip.png", "start": 52607019, "end": 52607507}, {"filename": "/GameData/Textures/lq_utility/star_lavaskip.png", "start": 52607507, "end": 52608620}, {"filename": "/GameData/Textures/lq_utility/star_slimeskip.png", "start": 52608620, "end": 52609693}, {"filename": "/GameData/Textures/lq_utility/star_waterskip.png", "start": 52609693, "end": 52611433}, {"filename": "/GameData/Textures/lq_utility/trigger.png", "start": 52611433, "end": 52611930}, {"filename": "/GameData/Textures/lq_wood/crate4.png", "start": 52611930, "end": 52615072}, {"filename": "/GameData/Textures/lq_wood/crwdh6.png", "start": 52615072, "end": 52620080}, {"filename": "/GameData/Textures/lq_wood/crwdl12.png", "start": 52620080, "end": 52622513}, {"filename": "/GameData/Textures/lq_wood/crwds6.png", "start": 52622513, "end": 52623539}, {"filename": "/GameData/Textures/lq_wood/may_crate3-small.png", "start": 52623539, "end": 52624530}, {"filename": "/GameData/Textures/lq_wood/may_crate3.png", "start": 52624530, "end": 52627593}, {"filename": "/GameData/Textures/lq_wood/may_wood1_1.png", "start": 52627593, "end": 52630433}, {"filename": "/GameData/Textures/lq_wood/may_wood1_2.png", "start": 52630433, "end": 52633282}, {"filename": "/GameData/Textures/lq_wood/med_ret_wood1.png", "start": 52633282, "end": 52640251}, {"filename": "/GameData/Textures/lq_wood/med_wood1.png", "start": 52640251, "end": 52667619}, {"filename": "/GameData/Textures/lq_wood/med_wood2.png", "start": 52667619, "end": 52675132}, {"filename": "/GameData/Textures/lq_wood/med_wood2_plk1.png", "start": 52675132, "end": 52686830}, {"filename": "/GameData/Textures/lq_wood/med_wood2_plk2.png", "start": 52686830, "end": 52699322}, {"filename": "/GameData/Textures/lq_wood/med_wood3.png", "start": 52699322, "end": 52705322}, {"filename": "/GameData/Textures/lq_wood/med_wood3_plk1.png", "start": 52705322, "end": 52718537}, {"filename": "/GameData/Textures/lq_wood/med_wood4.png", "start": 52718537, "end": 52725661}, {"filename": "/GameData/Textures/lq_wood/med_wood5.png", "start": 52725661, "end": 52733459}, {"filename": "/GameData/Textures/lq_wood/med_wood6.png", "start": 52733459, "end": 52740239}, {"filename": "/GameData/Textures/lq_wood/med_wood7.png", "start": 52740239, "end": 52749278}, {"filename": "/GameData/Textures/lq_wood/med_wood8.png", "start": 52749278, "end": 52757242}, {"filename": "/GameData/Textures/lq_wood/med_wood_riv1.png", "start": 52757242, "end": 52763867}, {"filename": "/GameData/Textures/lq_wood/med_wood_riv1b.png", "start": 52763867, "end": 52769946}, {"filename": "/GameData/Textures/lq_wood/med_wood_riv1c.png", "start": 52769946, "end": 52775070}, {"filename": "/GameData/Textures/lq_wood/med_wood_riv2.png", "start": 52775070, "end": 52781480}, {"filename": "/GameData/Textures/lq_wood/med_wood_riv2b.png", "start": 52781480, "end": 52787900}, {"filename": "/GameData/Textures/lq_wood/med_wood_riv2c.png", "start": 52787900, "end": 52793272}, {"filename": "/GameData/Textures/lq_wood/plank1.png", "start": 52793272, "end": 52799686}, {"filename": "/GameData/Textures/lq_wood/plank1s.png", "start": 52799686, "end": 52801567}, {"filename": "/GameData/Textures/lq_wood/plank2.png", "start": 52801567, "end": 52808147}, {"filename": "/GameData/Textures/lq_wood/plank2s.png", "start": 52808147, "end": 52810036}, {"filename": "/GameData/Textures/lq_wood/plank3.png", "start": 52810036, "end": 52816896}, {"filename": "/GameData/Textures/lq_wood/plank3s.png", "start": 52816896, "end": 52818877}, {"filename": "/GameData/Textures/lq_wood/plank4.png", "start": 52818877, "end": 52825323}, {"filename": "/GameData/Textures/lq_wood/plank4s.png", "start": 52825323, "end": 52827213}, {"filename": "/GameData/Textures/lq_wood/plank5.png", "start": 52827213, "end": 52832862}, {"filename": "/GameData/Textures/lq_wood/sq_wood_1.png", "start": 52832862, "end": 52848903}, {"filename": "/GameData/Textures/lq_wood/sq_wood_2.png", "start": 52848903, "end": 52851791}, {"filename": "/GameData/Textures/lq_wood/sq_wood_2a.png", "start": 52851791, "end": 52855160}, {"filename": "/GameData/Textures/lq_wood/wood_1.png", "start": 52855160, "end": 52859338}, {"filename": "/GameData/Textures/lq_wood/wood_2.png", "start": 52859338, "end": 52863238}, {"filename": "/GameData/Textures/lq_wood/woodbark128.png", "start": 52863238, "end": 52873445}, {"filename": "/GameData/Textures/lq_wood/woodbark1m28.png", "start": 52873445, "end": 52882894}, {"filename": "/GameData/Textures/lq_wood/woodbark64.png", "start": 52882894, "end": 52885831}, {"filename": "/GameData/Textures/lq_wood/woodbarkA128.png", "start": 52885831, "end": 52894529}, {"filename": "/GameData/Textures/lq_wood/woodbarkm64.png", "start": 52894529, "end": 52897206}, {"filename": "/GameData/Textures/lq_wood/woodend.png", "start": 52897206, "end": 52899709}, {"filename": "/GameData/Textures/lq_wood/woodring128.png", "start": 52899709, "end": 52908133}, {"filename": "/GameData/Textures/lq_wood/woodring64.png", "start": 52908133, "end": 52910594}, {"filename": "/GameData/Textures/lq_wood/woodringm128.png", "start": 52910594, "end": 52920893}, {"filename": "/GameData/Textures/lq_wood/woodringm64.png", "start": 52920893, "end": 52924152}, {"filename": "/GameData/Textures/muzzle_t.png", "start": 52924152, "end": 52933560}, {"filename": "/GameData/Textures/shirt.png", "start": 52933560, "end": 53125311}, {"filename": "/GameData/Textures/tormentPack/+0str_bloodfall.png", "start": 53125311, "end": 53127388}, {"filename": "/GameData/Textures/tormentPack/+1str_bloodfall.png", "start": 53127388, "end": 53129422}, {"filename": "/GameData/Textures/tormentPack/+2str_bloodfall.png", "start": 53129422, "end": 53131520}, {"filename": "/GameData/Textures/tormentPack/+3str_bloodfall.png", "start": 53131520, "end": 53133591}, {"filename": "/GameData/Textures/tormentPack/+4str_bloodfall.png", "start": 53133591, "end": 53135651}, {"filename": "/GameData/Textures/tormentPack/+5str_bloodfall.png", "start": 53135651, "end": 53137672}, {"filename": "/GameData/Textures/tormentPack/+6str_bloodfall.png", "start": 53137672, "end": 53139718}, {"filename": "/GameData/Textures/tormentPack/+7str_bloodfall.png", "start": 53139718, "end": 53141787}, {"filename": "/GameData/Textures/tormentPack/str_blood.png", "start": 53141787, "end": 53143795}, {"filename": "/GameData/Textures/tormentPack/str_blood_large.png", "start": 53143795, "end": 53166345}, {"filename": "/GameData/Textures/tormentPack/str_bloodvein1.png", "start": 53166345, "end": 53193020}, {"filename": "/GameData/Textures/tormentPack/str_bloodvein2.png", "start": 53193020, "end": 53225053}, {"filename": "/GameData/Textures/tormentPack/str_bloodvein3.png", "start": 53225053, "end": 53258724}, {"filename": "/GameData/Textures/tormentPack/str_bloodvein4.png", "start": 53258724, "end": 53290366}, {"filename": "/GameData/Textures/tormentPack/str_bloodvein5.png", "start": 53290366, "end": 53323857}, {"filename": "/GameData/Textures/tormentPack/str_bloodvein6.png", "start": 53323857, "end": 53358331}, {"filename": "/GameData/Textures/tormentPack/str_bloodvein7.png", "start": 53358331, "end": 53393222}, {"filename": "/GameData/Textures/tormentPack/str_bloodvein8.png", "start": 53393222, "end": 53421371}, {"filename": "/GameData/Textures/tormentPack/str_bloodvein9.png", "start": 53421371, "end": 53456824}, {"filename": "/GameData/Textures/tormentPack/str_metalflr1.png", "start": 53456824, "end": 53493518}, {"filename": "/GameData/Textures/tormentPack/str_metalflr2.png", "start": 53493518, "end": 53530093}, {"filename": "/GameData/Textures/tormentPack/str_metalflr3.png", "start": 53530093, "end": 53561086}, {"filename": "/GameData/Textures/tormentPack/str_metalflr4.png", "start": 53561086, "end": 53591256}, {"filename": "/GameData/Textures/tormentPack/str_metalflr5.png", "start": 53591256, "end": 53630706}, {"filename": "/GameData/Textures/tormentPack/str_metalflr6.png", "start": 53630706, "end": 53669866}, {"filename": "/GameData/Textures/tormentPack/str_metalflr7.png", "start": 53669866, "end": 53711558}, {"filename": "/GameData/Textures/tormentPack/str_metalflr8.png", "start": 53711558, "end": 53751976}, {"filename": "/GameData/Textures/tormentPack/str_metalgen1.png", "start": 53751976, "end": 53781246}, {"filename": "/GameData/Textures/tormentPack/str_metalgen2.png", "start": 53781246, "end": 53810762}, {"filename": "/GameData/Textures/tormentPack/str_metalgen3.png", "start": 53810762, "end": 53840238}, {"filename": "/GameData/Textures/tormentPack/str_metalgen4.png", "start": 53840238, "end": 53865061}, {"filename": "/GameData/Textures/tormentPack/str_metalgen5.png", "start": 53865061, "end": 53889931}, {"filename": "/GameData/Textures/tormentPack/str_metalgen6.png", "start": 53889931, "end": 53914021}, {"filename": "/GameData/Textures/tormentPack/str_metalgrbl1.png", "start": 53914021, "end": 53943406}, {"filename": "/GameData/Textures/tormentPack/str_metalgrbl2.png", "start": 53943406, "end": 53976136}, {"filename": "/GameData/Textures/tormentPack/str_metalgrbl3.png", "start": 53976136, "end": 54002754}, {"filename": "/GameData/Textures/tormentPack/str_metalgrbl4.png", "start": 54002754, "end": 54032018}, {"filename": "/GameData/Textures/tormentPack/str_metalgrbl5.png", "start": 54032018, "end": 54066393}, {"filename": "/GameData/Textures/tormentPack/str_metalgrbl6.png", "start": 54066393, "end": 54104888}, {"filename": "/GameData/Textures/tormentPack/str_metalgrbl7.png", "start": 54104888, "end": 54141038}, {"filename": "/GameData/Textures/tormentPack/str_metalgrbl8.png", "start": 54141038, "end": 54179031}, {"filename": "/GameData/Textures/tormentPack/str_metalpan1.png", "start": 54179031, "end": 54213891}, {"filename": "/GameData/Textures/tormentPack/str_metalpan2.png", "start": 54213891, "end": 54249432}, {"filename": "/GameData/Textures/tormentPack/str_metalpan3.png", "start": 54249432, "end": 54279868}, {"filename": "/GameData/Textures/tormentPack/str_metalpan4.png", "start": 54279868, "end": 54310828}, {"filename": "/GameData/Textures/tormentPack/str_metalpan5.png", "start": 54310828, "end": 54347200}, {"filename": "/GameData/Textures/tormentPack/str_metalpan6.png", "start": 54347200, "end": 54385242}, {"filename": "/GameData/Textures/tormentPack/str_metalpan7.png", "start": 54385242, "end": 54424471}, {"filename": "/GameData/Textures/tormentPack/str_metalpan8.png", "start": 54424471, "end": 54463565}, {"filename": "/GameData/Textures/tormentPack/str_rotwoodgen1.png", "start": 54463565, "end": 54482144}, {"filename": "/GameData/Textures/tormentPack/str_rotwoodgen2.png", "start": 54482144, "end": 54501440}, {"filename": "/GameData/Textures/tormentPack/str_rotwoodgen3.png", "start": 54501440, "end": 54521494}, {"filename": "/GameData/Textures/tormentPack/str_stonebrk1.png", "start": 54521494, "end": 54546886}, {"filename": "/GameData/Textures/tormentPack/str_stonebrk2.png", "start": 54546886, "end": 54573888}, {"filename": "/GameData/Textures/tormentPack/str_stonebrk3.png", "start": 54573888, "end": 54596518}, {"filename": "/GameData/Textures/tormentPack/str_stonebrk4.png", "start": 54596518, "end": 54620571}, {"filename": "/GameData/Textures/tormentPack/str_stonebrk5.png", "start": 54620571, "end": 54651655}, {"filename": "/GameData/Textures/tormentPack/str_stonebrk6.png", "start": 54651655, "end": 54684327}, {"filename": "/GameData/Textures/tormentPack/str_stonebrk7.png", "start": 54684327, "end": 54712851}, {"filename": "/GameData/Textures/tormentPack/str_stonebrk8.png", "start": 54712851, "end": 54743103}, {"filename": "/GameData/Textures/tormentPack/str_stoneflr1.png", "start": 54743103, "end": 54767259}, {"filename": "/GameData/Textures/tormentPack/str_stoneflr2.png", "start": 54767259, "end": 54795165}, {"filename": "/GameData/Textures/tormentPack/str_stoneflr3.png", "start": 54795165, "end": 54816170}, {"filename": "/GameData/Textures/tormentPack/str_stoneflr4.png", "start": 54816170, "end": 54837314}, {"filename": "/GameData/Textures/tormentPack/str_stoneflr5.png", "start": 54837314, "end": 54868846}, {"filename": "/GameData/Textures/tormentPack/str_stoneflr6.png", "start": 54868846, "end": 54903129}, {"filename": "/GameData/Textures/tormentPack/str_stonegen1.png", "start": 54903129, "end": 54922152}, {"filename": "/GameData/Textures/tormentPack/str_stonegen2.png", "start": 54922152, "end": 54941523}, {"filename": "/GameData/Textures/tormentPack/str_stonegen3.png", "start": 54941523, "end": 54963295}, {"filename": "/GameData/Textures/tormentPack/str_stonegen4.png", "start": 54963295, "end": 54988340}, {"filename": "/GameData/Textures/tormentPack/str_stonegen5.png", "start": 54988340, "end": 55013728}, {"filename": "/GameData/Textures/tormentPack/str_stonegen6.png", "start": 55013728, "end": 55041165}, {"filename": "/GameData/Textures/tormentPack/str_stonerubble.png", "start": 55041165, "end": 55069559}, {"filename": "/GameData/Textures/tormentPack/str_stonewall1.png", "start": 55069559, "end": 55092816}, {"filename": "/GameData/Textures/tormentPack/str_stonewall2.png", "start": 55092816, "end": 55117621}, {"filename": "/GameData/Textures/tormentPack/str_stonewall3.png", "start": 55117621, "end": 55142284}, {"filename": "/GameData/Textures/tormentPack/str_stonewall4.png", "start": 55142284, "end": 55168754}, {"filename": "/GameData/Textures/tormentPack/str_stonewall5.png", "start": 55168754, "end": 55196221}, {"filename": "/GameData/Textures/tormentPack/str_stonewall6.png", "start": 55196221, "end": 55224389}, {"filename": "/GameData/Textures/tormentPack/str_stonewall7.png", "start": 55224389, "end": 55253778}, {"filename": "/GameData/Textures/tormentPack/str_stonewall8.png", "start": 55253778, "end": 55284674}, {"filename": "/GameData/Textures/tormentPack/{str_bloodgunk.png", "start": 55284674, "end": 55311123}, {"filename": "/GameData/Textures/tormentPack/{str_bloodweb1.png", "start": 55311123, "end": 55328363}, {"filename": "/GameData/Textures/tormentPack/{str_bloodweb2.png", "start": 55328363, "end": 55337314}, {"filename": "/GameData/Textures/tormentPack/{str_bloodweb3.png", "start": 55337314, "end": 55343325}, {"filename": "/GameData/Textures/tormentPack/{str_grating1.png", "start": 55343325, "end": 55351906}, {"filename": "/GameData/Textures/tormentPack/{str_grating2.png", "start": 55351906, "end": 55367109}, {"filename": "/GameData/Textures/tormentPack/{str_grating3.png", "start": 55367109, "end": 55386457}, {"filename": "/GameData/Textures/tormentPack/{str_grating4.png", "start": 55386457, "end": 55398614}, {"filename": "/GameData/Textures/tormentPack/{str_grating5.png", "start": 55398614, "end": 55420119}, {"filename": "/GameData/Textures/tormentPack/{str_grating6.png", "start": 55420119, "end": 55448365}, {"filename": "/GameData/arms.glb", "start": 55448365, "end": 55628129}, {"filename": "/GameData/bass_beat.ogg", "start": 55628129, "end": 55995005, "audio": 1}, {"filename": "/GameData/bass_beat.wav", "start": 55995005, "end": 61691339, "audio": 1}, {"filename": "/GameData/cat.png", "start": 61691339, "end": 61940777}, {"filename": "/GameData/cube.mtl", "start": 61940777, "end": 61940828}, {"filename": "/GameData/cube.obj", "start": 61940828, "end": 61941747}, {"filename": "/GameData/dog.bin", "start": 61941747, "end": 62007791}, {"filename": "/GameData/dog.dae", "start": 62007791, "end": 62406863}, {"filename": "/GameData/dog.fbx", "start": 62406863, "end": 63321195}, {"filename": "/GameData/dog.glb", "start": 63321195, "end": 63468683}, {"filename": "/GameData/dog.glb.skmm", "start": 63468683, "end": 63471083}, {"filename": "/GameData/dog.gltf", "start": 63471083, "end": 63562210}, {"filename": "/GameData/happy_hog.png", "start": 63562210, "end": 64479335}, {"filename": "/GameData/hog_sheet.png", "start": 64479335, "end": 66334411}, {"filename": "/GameData/level_1_bg_crop.png", "start": 66334411, "end": 66941590}, {"filename": "/GameData/mini_hog.ico", "start": 66941590, "end": 66945876}, {"filename": "/GameData/monkey.fbx", "start": 66945876, "end": 66977296}, {"filename": "/GameData/sound_test.ogg", "start": 66977296, "end": 67006404, "audio": 1}, {"filename": "/GameData/sprite_sheet_sticher.py", "start": 67006404, "end": 67006930}, {"filename": "/GameData/testViewmodel.glb", "start": 67006930, "end": 67821902}, {"filename": "/GameData/title_bg_crop.png", "start": 67821902, "end": 68737010}, {"filename": "/GameData/yummy.ogg", "start": 68737010, "end": 68772601, "audio": 1}], "remote_package_size": 68772601});

  })();

// end include: C:\Users\Admin\AppData\Local\Temp\tmpjnn2jlwu.js
// include: C:\Users\Admin\AppData\Local\Temp\tmpplntf79j.js

    // All the pre-js content up to here must remain later on, we need to run
    // it.
    if (Module['$ww'] || (typeof ENVIRONMENT_IS_PTHREAD != 'undefined' && ENVIRONMENT_IS_PTHREAD)) Module['preRun'] = [];
    var necessaryPreJSTasks = Module['preRun'].slice();
  // end include: C:\Users\Admin\AppData\Local\Temp\tmpplntf79j.js
// include: C:\Users\Admin\AppData\Local\Temp\tmp991n43i3.js

    if (!Module['preRun']) throw 'Module.preRun should exist because file support used it; did a pre-js delete it?';
    necessaryPreJSTasks.forEach((task) => {
      if (Module['preRun'].indexOf(task) < 0) throw 'All preRun tasks that exist before user pre-js code should remain after; did you replace Module or modify Module.preRun?';
    });
  // end include: C:\Users\Admin\AppData\Local\Temp\tmp991n43i3.js


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

  var _alDeleteBuffers = (count, pBufferIds) => {
      if (!AL.currentCtx) {
        return;
      }
  
      for (var i = 0; i < count; ++i) {
        var bufId = HEAP32[(((pBufferIds)+(i*4))>>2)];
        /// Deleting the zero buffer is a legal NOP, so ignore it
        if (bufId === 0) {
          continue;
        }
  
        // Make sure the buffer index is valid.
        if (!AL.buffers[bufId]) {
          AL.currentCtx.err = 40961;
          return;
        }
  
        // Make sure the buffer is no longer in use.
        if (AL.buffers[bufId].refCount) {
          AL.currentCtx.err = 40964;
          return;
        }
      }
  
      for (var i = 0; i < count; ++i) {
        var bufId = HEAP32[(((pBufferIds)+(i*4))>>2)];
        if (bufId === 0) {
          continue;
        }
  
        AL.deviceRefCounts[AL.buffers[bufId].deviceId]--;
        delete AL.buffers[bufId];
        AL.freeIds.push(bufId);
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
  
  var _alDeleteSources = (count, pSourceIds) => {
      if (!AL.currentCtx) {
        return;
      }
  
      for (var i = 0; i < count; ++i) {
        var srcId = HEAP32[(((pSourceIds)+(i*4))>>2)];
        if (!AL.currentCtx.sources[srcId]) {
          AL.currentCtx.err = 40961;
          return;
        }
      }
  
      for (var i = 0; i < count; ++i) {
        var srcId = HEAP32[(((pSourceIds)+(i*4))>>2)];
        AL.setSourceState(AL.currentCtx.sources[srcId], 4116);
        _alSourcei(srcId, 0x1009 /* AL_BUFFER */, 0);
        delete AL.currentCtx.sources[srcId];
        AL.freeIds.push(srcId);
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

  var _alGetError = () => {
      if (!AL.currentCtx) {
        return 40964;
      }
      // Reset error on get.
      var err = AL.currentCtx.err;
      AL.currentCtx.err = 0;
      return err;
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

  var _alGetSourcei = (sourceId, param, pValue) => {
      var val = AL.getSourceParam('alGetSourcei', sourceId, param);
      if (val === null) {
        return;
      }
      if (!pValue) {
        AL.currentCtx.err = 40963;
        return;
      }
  
      switch (param) {
      case 0x202 /* AL_SOURCE_RELATIVE */:
      case 0x1001 /* AL_CONE_INNER_ANGLE */:
      case 0x1002 /* AL_CONE_OUTER_ANGLE */:
      case 0x1007 /* AL_LOOPING */:
      case 0x1009 /* AL_BUFFER */:
      case 0x1010 /* AL_SOURCE_STATE */:
      case 0x1015 /* AL_BUFFERS_QUEUED */:
      case 0x1016 /* AL_BUFFERS_PROCESSED */:
      case 0x1020 /* AL_REFERENCE_DISTANCE */:
      case 0x1021 /* AL_ROLLOFF_FACTOR */:
      case 0x1023 /* AL_MAX_DISTANCE */:
      case 0x1024 /* AL_SEC_OFFSET */:
      case 0x1025 /* AL_SAMPLE_OFFSET */:
      case 0x1026 /* AL_BYTE_OFFSET */:
      case 0x1027 /* AL_SOURCE_TYPE */:
      case 0x1214 /* AL_SOURCE_SPATIALIZE_SOFT */:
      case 0x2009 /* AL_BYTE_LENGTH_SOFT */:
      case 0x200A /* AL_SAMPLE_LENGTH_SOFT */:
      case 53248:
        HEAP32[((pValue)>>2)] = val;
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

  
  var _alIsExtensionPresent = (pExtName) => {
      var name = UTF8ToString(pExtName);
  
      return AL.AL_EXTENSIONS[name] ? 1 : 0;
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

  var _alSourceStop = (sourceId) => {
      if (!AL.currentCtx) {
        return;
      }
      var src = AL.currentCtx.sources[sourceId];
      if (!src) {
        AL.currentCtx.err = 40961;
        return;
      }
      AL.setSourceState(src, 4116);
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

  var _alcGetContextsDevice = (contextId) => {
      if (contextId in AL.contexts) {
        return AL.contexts[contextId].deviceId;
      }
      return 0;
    };

  var _alcGetCurrentContext = () => {
      if (AL.currentCtx !== null) {
        return AL.currentCtx.id;
      }
      return 0;
    };

  var _alcGetIntegerv = (deviceId, param, size, pValues) => {
      if (size === 0 || !pValues) {
        // Ignore the query, per the spec
        return;
      }
  
      switch (param) {
      case 0x1000 /* ALC_MAJOR_VERSION */:
        HEAP32[((pValues)>>2)] = 1;
        break;
      case 0x1001 /* ALC_MINOR_VERSION */:
        HEAP32[((pValues)>>2)] = 1;
        break;
      case 0x1002 /* ALC_ATTRIBUTES_SIZE */:
        if (!(deviceId in AL.deviceRefCounts)) {
          AL.alcErr = 40961;
          return;
        }
        if (!AL.currentCtx) {
          AL.alcErr = 0xA002 /* ALC_INVALID_CONTEXT */;
          return;
        }
  
        HEAP32[((pValues)>>2)] = AL.currentCtx.attrs.length;
        break;
      case 0x1003 /* ALC_ALL_ATTRIBUTES */:
        if (!(deviceId in AL.deviceRefCounts)) {
          AL.alcErr = 40961;
          return;
        }
        if (!AL.currentCtx) {
          AL.alcErr = 0xA002 /* ALC_INVALID_CONTEXT */;
          return;
        }
  
        for (var i = 0; i < AL.currentCtx.attrs.length; i++) {
          HEAP32[(((pValues)+(i*4))>>2)] = AL.currentCtx.attrs[i];
        }
        break;
      case 0x1007 /* ALC_FREQUENCY */:
        if (!(deviceId in AL.deviceRefCounts)) {
          AL.alcErr = 40961;
          return;
        }
        if (!AL.currentCtx) {
          AL.alcErr = 0xA002 /* ALC_INVALID_CONTEXT */;
          return;
        }
  
        HEAP32[((pValues)>>2)] = AL.currentCtx.audioCtx.sampleRate;
        break;
      case 0x1010 /* ALC_MONO_SOURCES */:
      case 0x1011 /* ALC_STEREO_SOURCES */:
        if (!(deviceId in AL.deviceRefCounts)) {
          AL.alcErr = 40961;
          return;
        }
        if (!AL.currentCtx) {
          AL.alcErr = 0xA002 /* ALC_INVALID_CONTEXT */;
          return;
        }
  
        HEAP32[((pValues)>>2)] = 0x7FFFFFFF;
        break;
      case 0x1992 /* ALC_HRTF_SOFT */:
      case 0x1993 /* ALC_HRTF_STATUS_SOFT */:
        if (!(deviceId in AL.deviceRefCounts)) {
          AL.alcErr = 40961;
          return;
        }
  
        var hrtfStatus = 0 /* ALC_HRTF_DISABLED_SOFT */;
        for (var ctxId in AL.contexts) {
          var ctx = AL.contexts[ctxId];
          if (ctx.deviceId === deviceId) {
            hrtfStatus = ctx.hrtf ? 1 /* ALC_HRTF_ENABLED_SOFT */ : 0 /* ALC_HRTF_DISABLED_SOFT */;
          }
        }
        HEAP32[((pValues)>>2)] = hrtfStatus;
        break;
      case 0x1994 /* ALC_NUM_HRTF_SPECIFIERS_SOFT */:
        if (!(deviceId in AL.deviceRefCounts)) {
          AL.alcErr = 40961;
          return;
        }
        HEAP32[((pValues)>>2)] = 1;
        break;
      case 0x20003 /* ALC_MAX_AUXILIARY_SENDS */:
        if (!(deviceId in AL.deviceRefCounts)) {
          AL.alcErr = 40961;
          return;
        }
        if (!AL.currentCtx) {
          AL.alcErr = 0xA002 /* ALC_INVALID_CONTEXT */;
          return;
        }
  
        HEAP32[((pValues)>>2)] = 1;
      case 0x312 /* ALC_CAPTURE_SAMPLES */:
        var c = AL.requireValidCaptureDevice(deviceId, 'alcGetIntegerv');
        if (!c) {
          return;
        }
        var n = c.capturedFrameCount;
        var dstfreq = c.requestedSampleRate;
        var srcfreq = c.audioCtx.sampleRate;
        var nsamples = Math.floor(n * (dstfreq/srcfreq));
        HEAP32[((pValues)>>2)] = nsamples;
        break;
      default:
        AL.alcErr = 40963;
        return;
      }
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
  1510068: () => { if (typeof(AudioContext) !== 'undefined') { return true; } else if (typeof(webkitAudioContext) !== 'undefined') { return true; } return false; },  
 1510215: () => { if ((typeof(navigator.mediaDevices) !== 'undefined') && (typeof(navigator.mediaDevices.getUserMedia) !== 'undefined')) { return true; } else if (typeof(navigator.webkitGetUserMedia) !== 'undefined') { return true; } return false; },  
 1510449: ($0) => { if(typeof(Module['SDL2']) === 'undefined') { Module['SDL2'] = {}; } var SDL2 = Module['SDL2']; if (!$0) { SDL2.audio = {}; } else { SDL2.capture = {}; } if (!SDL2.audioContext) { if (typeof(AudioContext) !== 'undefined') { SDL2.audioContext = new AudioContext(); } else if (typeof(webkitAudioContext) !== 'undefined') { SDL2.audioContext = new webkitAudioContext(); } if (SDL2.audioContext) { if ((typeof navigator.userActivation) === 'undefined') { autoResumeAudioContext(SDL2.audioContext); } } } return SDL2.audioContext === undefined ? -1 : 0; },  
 1511001: () => { var SDL2 = Module['SDL2']; return SDL2.audioContext.sampleRate; },  
 1511069: ($0, $1, $2, $3) => { var SDL2 = Module['SDL2']; var have_microphone = function(stream) { if (SDL2.capture.silenceTimer !== undefined) { clearInterval(SDL2.capture.silenceTimer); SDL2.capture.silenceTimer = undefined; SDL2.capture.silenceBuffer = undefined } SDL2.capture.mediaStreamNode = SDL2.audioContext.createMediaStreamSource(stream); SDL2.capture.scriptProcessorNode = SDL2.audioContext.createScriptProcessor($1, $0, 1); SDL2.capture.scriptProcessorNode.onaudioprocess = function(audioProcessingEvent) { if ((SDL2 === undefined) || (SDL2.capture === undefined)) { return; } audioProcessingEvent.outputBuffer.getChannelData(0).fill(0.0); SDL2.capture.currentCaptureBuffer = audioProcessingEvent.inputBuffer; dynCall('vi', $2, [$3]); }; SDL2.capture.mediaStreamNode.connect(SDL2.capture.scriptProcessorNode); SDL2.capture.scriptProcessorNode.connect(SDL2.audioContext.destination); SDL2.capture.stream = stream; }; var no_microphone = function(error) { }; SDL2.capture.silenceBuffer = SDL2.audioContext.createBuffer($0, $1, SDL2.audioContext.sampleRate); SDL2.capture.silenceBuffer.getChannelData(0).fill(0.0); var silence_callback = function() { SDL2.capture.currentCaptureBuffer = SDL2.capture.silenceBuffer; dynCall('vi', $2, [$3]); }; SDL2.capture.silenceTimer = setInterval(silence_callback, ($1 / SDL2.audioContext.sampleRate) * 1000); if ((navigator.mediaDevices !== undefined) && (navigator.mediaDevices.getUserMedia !== undefined)) { navigator.mediaDevices.getUserMedia({ audio: true, video: false }).then(have_microphone).catch(no_microphone); } else if (navigator.webkitGetUserMedia !== undefined) { navigator.webkitGetUserMedia({ audio: true, video: false }, have_microphone, no_microphone); } },  
 1512762: ($0, $1, $2, $3) => { var SDL2 = Module['SDL2']; SDL2.audio.scriptProcessorNode = SDL2.audioContext['createScriptProcessor']($1, 0, $0); SDL2.audio.scriptProcessorNode['onaudioprocess'] = function (e) { if ((SDL2 === undefined) || (SDL2.audio === undefined)) { return; } if (SDL2.audio.silenceTimer !== undefined) { clearInterval(SDL2.audio.silenceTimer); SDL2.audio.silenceTimer = undefined; SDL2.audio.silenceBuffer = undefined; } SDL2.audio.currentOutputBuffer = e['outputBuffer']; dynCall('vi', $2, [$3]); }; SDL2.audio.scriptProcessorNode['connect'](SDL2.audioContext['destination']); if (SDL2.audioContext.state === 'suspended') { SDL2.audio.silenceBuffer = SDL2.audioContext.createBuffer($0, $1, SDL2.audioContext.sampleRate); SDL2.audio.silenceBuffer.getChannelData(0).fill(0.0); var silence_callback = function() { if ((typeof navigator.userActivation) !== 'undefined') { if (navigator.userActivation.hasBeenActive) { SDL2.audioContext.resume(); } } SDL2.audio.currentOutputBuffer = SDL2.audio.silenceBuffer; dynCall('vi', $2, [$3]); SDL2.audio.currentOutputBuffer = undefined; }; SDL2.audio.silenceTimer = setInterval(silence_callback, ($1 / SDL2.audioContext.sampleRate) * 1000); } },  
 1513937: ($0, $1) => { var SDL2 = Module['SDL2']; var numChannels = SDL2.capture.currentCaptureBuffer.numberOfChannels; for (var c = 0; c < numChannels; ++c) { var channelData = SDL2.capture.currentCaptureBuffer.getChannelData(c); if (channelData.length != $1) { throw 'Web Audio capture buffer length mismatch! Destination size: ' + channelData.length + ' samples vs expected ' + $1 + ' samples!'; } if (numChannels == 1) { for (var j = 0; j < $1; ++j) { setValue($0 + (j * 4), channelData[j], 'float'); } } else { for (var j = 0; j < $1; ++j) { setValue($0 + (((j * numChannels) + c) * 4), channelData[j], 'float'); } } } },  
 1514542: ($0, $1) => { var SDL2 = Module['SDL2']; var buf = $0 >>> 2; var numChannels = SDL2.audio.currentOutputBuffer['numberOfChannels']; for (var c = 0; c < numChannels; ++c) { var channelData = SDL2.audio.currentOutputBuffer['getChannelData'](c); if (channelData.length != $1) { throw 'Web Audio output buffer length mismatch! Destination size: ' + channelData.length + ' samples vs expected ' + $1 + ' samples!'; } for (var j = 0; j < $1; ++j) { channelData[j] = HEAPF32[buf + (j*numChannels + c)]; } } },  
 1515031: ($0) => { var SDL2 = Module['SDL2']; if ($0) { if (SDL2.capture.silenceTimer !== undefined) { clearInterval(SDL2.capture.silenceTimer); } if (SDL2.capture.stream !== undefined) { var tracks = SDL2.capture.stream.getAudioTracks(); for (var i = 0; i < tracks.length; i++) { SDL2.capture.stream.removeTrack(tracks[i]); } } if (SDL2.capture.scriptProcessorNode !== undefined) { SDL2.capture.scriptProcessorNode.onaudioprocess = function(audioProcessingEvent) {}; SDL2.capture.scriptProcessorNode.disconnect(); } if (SDL2.capture.mediaStreamNode !== undefined) { SDL2.capture.mediaStreamNode.disconnect(); } SDL2.capture = undefined; } else { if (SDL2.audio.scriptProcessorNode != undefined) { SDL2.audio.scriptProcessorNode.disconnect(); } if (SDL2.audio.silenceTimer !== undefined) { clearInterval(SDL2.audio.silenceTimer); } SDL2.audio = undefined; } if ((SDL2.audioContext !== undefined) && (SDL2.audio === undefined) && (SDL2.capture === undefined)) { SDL2.audioContext.close(); SDL2.audioContext = undefined; } },  
 1516037: ($0) => { var str = UTF8ToString($0) + '\n\n' + 'Abort/Retry/Ignore/AlwaysIgnore? [ariA] :'; var reply = window.prompt(str, "i"); if (reply === null) { reply = "i"; } return allocate(intArrayFromString(reply), 'i8', ALLOC_NORMAL); },  
 1516262: ($0, $1, $2) => { var w = $0; var h = $1; var pixels = $2; if (!Module['SDL2']) Module['SDL2'] = {}; var SDL2 = Module['SDL2']; if (SDL2.ctxCanvas !== Module['canvas']) { SDL2.ctx = Module['createContext'](Module['canvas'], false, true); SDL2.ctxCanvas = Module['canvas']; } if (SDL2.w !== w || SDL2.h !== h || SDL2.imageCtx !== SDL2.ctx) { SDL2.image = SDL2.ctx.createImageData(w, h); SDL2.w = w; SDL2.h = h; SDL2.imageCtx = SDL2.ctx; } var data = SDL2.image.data; var src = pixels / 4; var dst = 0; var num; if (typeof CanvasPixelArray !== 'undefined' && data instanceof CanvasPixelArray) { num = data.length; while (dst < num) { var val = HEAP32[src]; data[dst ] = val & 0xff; data[dst+1] = (val >> 8) & 0xff; data[dst+2] = (val >> 16) & 0xff; data[dst+3] = 0xff; src++; dst += 4; } } else { if (SDL2.data32Data !== data) { SDL2.data32 = new Int32Array(data.buffer); SDL2.data8 = new Uint8Array(data.buffer); SDL2.data32Data = data; } var data32 = SDL2.data32; num = data32.length; data32.set(HEAP32.subarray(src, src + num)); var data8 = SDL2.data8; var i = 3; var j = i + 4*num; if (num % 8 == 0) { while (i < j) { data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; } } else { while (i < j) { data8[i] = 0xff; i = i + 4 | 0; } } } SDL2.ctx.putImageData(SDL2.image, 0, 0); },  
 1517730: ($0, $1, $2, $3, $4) => { var w = $0; var h = $1; var hot_x = $2; var hot_y = $3; var pixels = $4; var canvas = document.createElement("canvas"); canvas.width = w; canvas.height = h; var ctx = canvas.getContext("2d"); var image = ctx.createImageData(w, h); var data = image.data; var src = pixels / 4; var dst = 0; var num; if (typeof CanvasPixelArray !== 'undefined' && data instanceof CanvasPixelArray) { num = data.length; while (dst < num) { var val = HEAP32[src]; data[dst ] = val & 0xff; data[dst+1] = (val >> 8) & 0xff; data[dst+2] = (val >> 16) & 0xff; data[dst+3] = (val >> 24) & 0xff; src++; dst += 4; } } else { var data32 = new Int32Array(data.buffer); num = data32.length; data32.set(HEAP32.subarray(src, src + num)); } ctx.putImageData(image, 0, 0); var url = hot_x === 0 && hot_y === 0 ? "url(" + canvas.toDataURL() + "), auto" : "url(" + canvas.toDataURL() + ") " + hot_x + " " + hot_y + ", auto"; var urlBuf = _malloc(url.length + 1); stringToUTF8(url, urlBuf, url.length + 1); return urlBuf; },  
 1518718: ($0) => { if (Module['canvas']) { Module['canvas'].style['cursor'] = UTF8ToString($0); } },  
 1518801: () => { if (Module['canvas']) { Module['canvas'].style['cursor'] = 'none'; } },  
 1518870: () => { return window.innerWidth; },  
 1518900: () => { return window.innerHeight; }
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
  alDeleteBuffers: _alDeleteBuffers,
  /** @export */
  alDeleteSources: _alDeleteSources,
  /** @export */
  alDistanceModel: _alDistanceModel,
  /** @export */
  alGenBuffers: _alGenBuffers,
  /** @export */
  alGenSources: _alGenSources,
  /** @export */
  alGetError: _alGetError,
  /** @export */
  alGetListenerfv: _alGetListenerfv,
  /** @export */
  alGetSourcei: _alGetSourcei,
  /** @export */
  alGetString: _alGetString,
  /** @export */
  alIsExtensionPresent: _alIsExtensionPresent,
  /** @export */
  alListener3f: _alListener3f,
  /** @export */
  alListenerfv: _alListenerfv,
  /** @export */
  alSource3f: _alSource3f,
  /** @export */
  alSourcePlay: _alSourcePlay,
  /** @export */
  alSourceStop: _alSourceStop,
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
  alcGetContextsDevice: _alcGetContextsDevice,
  /** @export */
  alcGetCurrentContext: _alcGetCurrentContext,
  /** @export */
  alcGetIntegerv: _alcGetIntegerv,
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

