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
// include: C:\Users\Admin\AppData\Local\Temp\tmp21qreegl.js

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
Module['FS_createPath']("/GameData", "fonts", true, true);
Module['FS_createPath']("/GameData", "maps", true, true);
Module['FS_createPath']("/GameData/maps", "autosave", true, true);
Module['FS_createPath']("/GameData/maps", "test", true, true);
Module['FS_createPath']("/GameData", "models", true, true);
Module['FS_createPath']("/GameData/models", "player", true, true);
Module['FS_createPath']("/GameData/models/player", "bike", true, true);
Module['FS_createPath']("/GameData/models/player/bike", "textures", true, true);
Module['FS_createPath']("/GameData/models", "weapons", true, true);
Module['FS_createPath']("/GameData/models/weapons", "bullet", true, true);
Module['FS_createPath']("/GameData", "scripts", true, true);
Module['FS_createPath']("/GameData", "shaders", true, true);
Module['FS_createPath']("/GameData", "sounds", true, true);
Module['FS_createPath']("/GameData/sounds", "dog", true, true);
Module['FS_createPath']("/GameData/sounds", "weapons", true, true);
Module['FS_createPath']("/GameData/sounds/weapons", "shotgun", true, true);
Module['FS_createPath']("/GameData", "textures", true, true);
Module['FS_createPath']("/GameData/textures", "Ground", true, true);
Module['FS_createPath']("/GameData/textures", "common", true, true);
Module['FS_createPath']("/GameData/textures", "delvenPack", true, true);
Module['FS_createPath']("/GameData/textures", "generic", true, true);
Module['FS_createPath']("/GameData/textures", "lq_conc", true, true);
Module['FS_createPath']("/GameData/textures", "lq_dev", true, true);
Module['FS_createPath']("/GameData/textures", "lq_flesh", true, true);
Module['FS_createPath']("/GameData/textures", "lq_greek", true, true);
Module['FS_createPath']("/GameData/textures", "lq_health_ammo", true, true);
Module['FS_createPath']("/GameData/textures", "lq_legacy", true, true);
Module['FS_createPath']("/GameData/textures", "lq_liquidsky", true, true);
Module['FS_createPath']("/GameData/textures", "lq_mayan", true, true);
Module['FS_createPath']("/GameData/textures", "lq_medieval", true, true);
Module['FS_createPath']("/GameData/textures", "lq_metal", true, true);
Module['FS_createPath']("/GameData/textures", "lq_palette", true, true);
Module['FS_createPath']("/GameData/textures", "lq_props", true, true);
Module['FS_createPath']("/GameData/textures", "lq_tech", true, true);
Module['FS_createPath']("/GameData/textures", "lq_terra", true, true);
Module['FS_createPath']("/GameData/textures", "lq_utility", true, true);
Module['FS_createPath']("/GameData/textures", "lq_wood", true, true);
Module['FS_createPath']("/GameData/textures", "metal", true, true);
Module['FS_createPath']("/GameData/textures", "particles", true, true);
Module['FS_createPath']("/GameData/textures", "tormentPack", true, true);
Module['FS_createPath']("/GameData/textures", "wall", true, true);
Module['FS_createPath']("/GameData/textures", "water", true, true);
Module['FS_createPath']("/GameData/textures", "wood", true, true);

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
    loadPackage({"files": [{"filename": "/GameData/arms.glb", "start": 0, "end": 179764}, {"filename": "/GameData/bass_beat.ogg", "start": 179764, "end": 546640, "audio": 1}, {"filename": "/GameData/bass_beat.wav", "start": 546640, "end": 6242974, "audio": 1}, {"filename": "/GameData/cat.png", "start": 6242974, "end": 6492412}, {"filename": "/GameData/cube.mtl", "start": 6492412, "end": 6492463}, {"filename": "/GameData/cube.obj", "start": 6492463, "end": 6493382}, {"filename": "/GameData/dog.bin", "start": 6493382, "end": 6559426}, {"filename": "/GameData/dog.dae", "start": 6559426, "end": 6958498}, {"filename": "/GameData/dog.fbx", "start": 6958498, "end": 7872830}, {"filename": "/GameData/dog.glb", "start": 7872830, "end": 8020318}, {"filename": "/GameData/dog.glb.skmm", "start": 8020318, "end": 8022718}, {"filename": "/GameData/dog.gltf", "start": 8022718, "end": 8113845}, {"filename": "/GameData/fonts/Kingthings_Calligraphica_2.ttf", "start": 8113845, "end": 8143649}, {"filename": "/GameData/happy_hog.png", "start": 8143649, "end": 9060774}, {"filename": "/GameData/hog_sheet.png", "start": 9060774, "end": 10915850}, {"filename": "/GameData/level_1_bg_crop.png", "start": 10915850, "end": 11523029}, {"filename": "/GameData/maps/Level.bsp", "start": 11523029, "end": 11585105}, {"filename": "/GameData/maps/autosave/test.1.map", "start": 11585105, "end": 11601056}, {"filename": "/GameData/maps/autosave/test.2.map", "start": 11601056, "end": 11617104}, {"filename": "/GameData/maps/autosave/test.3.map", "start": 11617104, "end": 11633262}, {"filename": "/GameData/maps/autosave/test.4.map", "start": 11633262, "end": 11649406}, {"filename": "/GameData/maps/autosave/test.5.map", "start": 11649406, "end": 11665352}, {"filename": "/GameData/maps/autosave/test.6.map", "start": 11665352, "end": 11681298}, {"filename": "/GameData/maps/autosave/test.7.map", "start": 11681298, "end": 11697248}, {"filename": "/GameData/maps/autosave/test2.1.map", "start": 11697248, "end": 12353816}, {"filename": "/GameData/maps/e1m1.map", "start": 12353816, "end": 22365345}, {"filename": "/GameData/maps/e1m1.obj", "start": 22365345, "end": 32210366}, {"filename": "/GameData/maps/oa_dm1.bsp", "start": 32210366, "end": 33615342}, {"filename": "/GameData/maps/test.autosave.map", "start": 33615342, "end": 33645365}, {"filename": "/GameData/maps/test.bak", "start": 33645365, "end": 33675430}, {"filename": "/GameData/maps/test.bsp", "start": 33675430, "end": 33924238}, {"filename": "/GameData/maps/test.map", "start": 33924238, "end": 33954261}, {"filename": "/GameData/maps/test.mtl", "start": 33954261, "end": 33954538}, {"filename": "/GameData/maps/test.obj", "start": 33954538, "end": 33973055}, {"filename": "/GameData/maps/test.srf", "start": 33973055, "end": 33998078}, {"filename": "/GameData/maps/test/lm_0000.tga", "start": 33998078, "end": 34194704}, {"filename": "/GameData/maps/test/lm_0001.tga", "start": 34194704, "end": 34391330}, {"filename": "/GameData/maps/test/lm_0002.tga", "start": 34391330, "end": 34587956}, {"filename": "/GameData/maps/test/lm_0003.tga", "start": 34587956, "end": 34784582}, {"filename": "/GameData/maps/test/lm_0004.tga", "start": 34784582, "end": 34981208}, {"filename": "/GameData/maps/test/lm_0005.tga", "start": 34981208, "end": 35177834}, {"filename": "/GameData/maps/test/lm_0006.tga", "start": 35177834, "end": 35374460}, {"filename": "/GameData/maps/test/lm_0007.tga", "start": 35374460, "end": 35571086}, {"filename": "/GameData/maps/test/lm_0008.tga", "start": 35571086, "end": 35767712}, {"filename": "/GameData/maps/test/lm_0009.tga", "start": 35767712, "end": 35964338}, {"filename": "/GameData/maps/test/lm_0010.tga", "start": 35964338, "end": 36160964}, {"filename": "/GameData/maps/test/lm_0011.tga", "start": 36160964, "end": 36357590}, {"filename": "/GameData/maps/test/lm_0012.tga", "start": 36357590, "end": 36554216}, {"filename": "/GameData/maps/test/lm_0013.tga", "start": 36554216, "end": 36750842}, {"filename": "/GameData/maps/test/lm_0014.tga", "start": 36750842, "end": 36947468}, {"filename": "/GameData/maps/test/lm_0015.tga", "start": 36947468, "end": 37144094}, {"filename": "/GameData/maps/test/lm_0016.tga", "start": 37144094, "end": 37340720}, {"filename": "/GameData/maps/test/lm_0017.tga", "start": 37340720, "end": 37537346}, {"filename": "/GameData/maps/test/lm_0018.tga", "start": 37537346, "end": 37733972}, {"filename": "/GameData/maps/test/lm_0019.tga", "start": 37733972, "end": 37930598}, {"filename": "/GameData/maps/test/lm_0020.tga", "start": 37930598, "end": 38127224}, {"filename": "/GameData/maps/test/lm_0021.tga", "start": 38127224, "end": 38323850}, {"filename": "/GameData/maps/test/lm_0022.tga", "start": 38323850, "end": 38520476}, {"filename": "/GameData/maps/test/lm_0023.tga", "start": 38520476, "end": 38717102}, {"filename": "/GameData/maps/test2.map", "start": 38717102, "end": 39374977}, {"filename": "/GameData/maps/test2.mtl", "start": 39374977, "end": 39375672}, {"filename": "/GameData/maps/test2.obj", "start": 39375672, "end": 40278969}, {"filename": "/GameData/mini_hog.ico", "start": 40278969, "end": 40283255}, {"filename": "/GameData/models/npc_base.mtl", "start": 40283255, "end": 40283500}, {"filename": "/GameData/models/npc_base.obj", "start": 40283500, "end": 40286384}, {"filename": "/GameData/models/player/bike/bike.glb", "start": 40286384, "end": 45738260}, {"filename": "/GameData/models/player/bike/textures/body.png", "start": 45738260, "end": 45974941}, {"filename": "/GameData/models/player/bike/textures/front.png", "start": 45974941, "end": 46446692}, {"filename": "/GameData/models/player/bike/textures/wheels.png", "start": 46446692, "end": 47573496}, {"filename": "/GameData/models/test.obj", "start": 47573496, "end": 47606478}, {"filename": "/GameData/models/weapons/bullet/bullet.obj", "start": 47606478, "end": 47607736}, {"filename": "/GameData/monkey.fbx", "start": 47607736, "end": 47639156}, {"filename": "/GameData/scripts/common.shader", "start": 47639156, "end": 47639601}, {"filename": "/GameData/shaders/bsp.frag", "start": 47639601, "end": 47640398}, {"filename": "/GameData/shaders/bsp.vert", "start": 47640398, "end": 47641149}, {"filename": "/GameData/shaders/default_pixel.frag", "start": 47641149, "end": 47651884}, {"filename": "/GameData/shaders/default_pixel_.frag", "start": 47651884, "end": 47658843}, {"filename": "/GameData/shaders/default_pixel_simple.frag", "start": 47658843, "end": 47669674}, {"filename": "/GameData/shaders/default_vertex.vert", "start": 47669674, "end": 47671922}, {"filename": "/GameData/shaders/empty_pixel.frag", "start": 47671922, "end": 47671989}, {"filename": "/GameData/shaders/fullscreen_vertex.vert", "start": 47671989, "end": 47672193}, {"filename": "/GameData/shaders/instanced_bilboard_vertex.vert", "start": 47672193, "end": 47674132}, {"filename": "/GameData/shaders/mask_pixel.frag", "start": 47674132, "end": 47674564}, {"filename": "/GameData/shaders/solidRed_pixel.frag", "start": 47674564, "end": 47674733}, {"filename": "/GameData/shaders/texture_pixel.frag", "start": 47674733, "end": 47674921}, {"filename": "/GameData/shaders/ui.vert", "start": 47674921, "end": 47675224}, {"filename": "/GameData/shaders/ui_flatcolor.frag", "start": 47675224, "end": 47675360}, {"filename": "/GameData/shaders/ui_textured.frag", "start": 47675360, "end": 47675617}, {"filename": "/GameData/sound_test.ogg", "start": 47675617, "end": 47704725, "audio": 1}, {"filename": "/GameData/sounds/dog/death.wav", "start": 47704725, "end": 47778111, "audio": 1}, {"filename": "/GameData/sounds/dog/dog_attack.wav", "start": 47778111, "end": 47890635, "audio": 1}, {"filename": "/GameData/sounds/dog/dog_attack_start.wav", "start": 47890635, "end": 48087319, "audio": 1}, {"filename": "/GameData/sounds/dog/dog_death.wav", "start": 48087319, "end": 48241519, "audio": 1}, {"filename": "/GameData/sounds/dog/dog_hit.wav", "start": 48241519, "end": 48315211, "audio": 1}, {"filename": "/GameData/sounds/dog/dog_stun.wav", "start": 48315211, "end": 48408661, "audio": 1}, {"filename": "/GameData/sounds/mew.wav", "start": 48408661, "end": 48482047, "audio": 1}, {"filename": "/GameData/sounds/weapons/shotgun/shotgun_fire.wav", "start": 48482047, "end": 48942047, "audio": 1}, {"filename": "/GameData/sprite_sheet_sticher.py", "start": 48942047, "end": 48942573}, {"filename": "/GameData/testViewmodel.glb", "start": 48942573, "end": 49757545}, {"filename": "/GameData/textures/Ground/grass.png", "start": 49757545, "end": 49761329}, {"filename": "/GameData/textures/M_Shotgun_Base_Color.png", "start": 49761329, "end": 50136471}, {"filename": "/GameData/textures/NULL.png", "start": 50136471, "end": 50138268}, {"filename": "/GameData/textures/arms.png", "start": 50138268, "end": 50178512}, {"filename": "/GameData/textures/common/trigger.png", "start": 50178512, "end": 50190388}, {"filename": "/GameData/textures/delvenPack/dlv_door1a.png", "start": 50190388, "end": 50198172}, {"filename": "/GameData/textures/delvenPack/dlv_door1b.png", "start": 50198172, "end": 50207362}, {"filename": "/GameData/textures/delvenPack/dlv_door1c.png", "start": 50207362, "end": 50215213}, {"filename": "/GameData/textures/delvenPack/dlv_door1d.png", "start": 50215213, "end": 50224385}, {"filename": "/GameData/textures/delvenPack/dlv_door2a.png", "start": 50224385, "end": 50232109}, {"filename": "/GameData/textures/delvenPack/dlv_door2b.png", "start": 50232109, "end": 50241234}, {"filename": "/GameData/textures/delvenPack/dlv_door2c.png", "start": 50241234, "end": 50249262}, {"filename": "/GameData/textures/delvenPack/dlv_door2d.png", "start": 50249262, "end": 50258490}, {"filename": "/GameData/textures/delvenPack/dlv_door3a.png", "start": 50258490, "end": 50262638}, {"filename": "/GameData/textures/delvenPack/dlv_door3b.png", "start": 50262638, "end": 50267416}, {"filename": "/GameData/textures/delvenPack/dlv_door3c.png", "start": 50267416, "end": 50271566}, {"filename": "/GameData/textures/delvenPack/dlv_door3d.png", "start": 50271566, "end": 50276358}, {"filename": "/GameData/textures/delvenPack/dlv_door4a.png", "start": 50276358, "end": 50280479}, {"filename": "/GameData/textures/delvenPack/dlv_door4b.png", "start": 50280479, "end": 50285270}, {"filename": "/GameData/textures/delvenPack/dlv_door4c.png", "start": 50285270, "end": 50289540}, {"filename": "/GameData/textures/delvenPack/dlv_door4d.png", "start": 50289540, "end": 50294376}, {"filename": "/GameData/textures/delvenPack/dlv_fabric1a.png", "start": 50294376, "end": 50299752}, {"filename": "/GameData/textures/delvenPack/dlv_fabric1b.png", "start": 50299752, "end": 50305880}, {"filename": "/GameData/textures/delvenPack/dlv_fabric1c.png", "start": 50305880, "end": 50312174}, {"filename": "/GameData/textures/delvenPack/dlv_fabric1d.png", "start": 50312174, "end": 50315810}, {"filename": "/GameData/textures/delvenPack/dlv_fabric1e.png", "start": 50315810, "end": 50319496}, {"filename": "/GameData/textures/delvenPack/dlv_fabric2a.png", "start": 50319496, "end": 50324221}, {"filename": "/GameData/textures/delvenPack/dlv_fabric2b.png", "start": 50324221, "end": 50329672}, {"filename": "/GameData/textures/delvenPack/dlv_fabric2c.png", "start": 50329672, "end": 50335350}, {"filename": "/GameData/textures/delvenPack/dlv_fabric2d.png", "start": 50335350, "end": 50338636}, {"filename": "/GameData/textures/delvenPack/dlv_fabric2e.png", "start": 50338636, "end": 50341997}, {"filename": "/GameData/textures/delvenPack/dlv_fabric3a.png", "start": 50341997, "end": 50346951}, {"filename": "/GameData/textures/delvenPack/dlv_fabric3b.png", "start": 50346951, "end": 50352622}, {"filename": "/GameData/textures/delvenPack/dlv_fabric3c.png", "start": 50352622, "end": 50358447}, {"filename": "/GameData/textures/delvenPack/dlv_fabric3d.png", "start": 50358447, "end": 50361827}, {"filename": "/GameData/textures/delvenPack/dlv_fabric3e.png", "start": 50361827, "end": 50365269}, {"filename": "/GameData/textures/delvenPack/dlv_fabric4a.png", "start": 50365269, "end": 50374687}, {"filename": "/GameData/textures/delvenPack/dlv_fabric4b.png", "start": 50374687, "end": 50384652}, {"filename": "/GameData/textures/delvenPack/dlv_fabric4c.png", "start": 50384652, "end": 50394781}, {"filename": "/GameData/textures/delvenPack/dlv_fabric4d.png", "start": 50394781, "end": 50400305}, {"filename": "/GameData/textures/delvenPack/dlv_fabric4e.png", "start": 50400305, "end": 50405875}, {"filename": "/GameData/textures/delvenPack/dlv_fabric5a.png", "start": 50405875, "end": 50414942}, {"filename": "/GameData/textures/delvenPack/dlv_fabric5b.png", "start": 50414942, "end": 50424589}, {"filename": "/GameData/textures/delvenPack/dlv_fabric5c.png", "start": 50424589, "end": 50434389}, {"filename": "/GameData/textures/delvenPack/dlv_fabric5d.png", "start": 50434389, "end": 50439799}, {"filename": "/GameData/textures/delvenPack/dlv_fabric5e.png", "start": 50439799, "end": 50445233}, {"filename": "/GameData/textures/delvenPack/dlv_fabric6a.png", "start": 50445233, "end": 50452367}, {"filename": "/GameData/textures/delvenPack/dlv_fabric6b.png", "start": 50452367, "end": 50460095}, {"filename": "/GameData/textures/delvenPack/dlv_fabric6c.png", "start": 50460095, "end": 50467990}, {"filename": "/GameData/textures/delvenPack/dlv_fabric6d.png", "start": 50467990, "end": 50472390}, {"filename": "/GameData/textures/delvenPack/dlv_fabric6e.png", "start": 50472390, "end": 50476848}, {"filename": "/GameData/textures/delvenPack/dlv_ground1a.png", "start": 50476848, "end": 50484770}, {"filename": "/GameData/textures/delvenPack/dlv_ground1b.png", "start": 50484770, "end": 50493606}, {"filename": "/GameData/textures/delvenPack/dlv_ground1c.png", "start": 50493606, "end": 50503102}, {"filename": "/GameData/textures/delvenPack/dlv_ground1d.png", "start": 50503102, "end": 50513145}, {"filename": "/GameData/textures/delvenPack/dlv_ground2a.png", "start": 50513145, "end": 50523013}, {"filename": "/GameData/textures/delvenPack/dlv_ground2b.png", "start": 50523013, "end": 50531920}, {"filename": "/GameData/textures/delvenPack/dlv_ground2c.png", "start": 50531920, "end": 50541645}, {"filename": "/GameData/textures/delvenPack/dlv_ground2d.png", "start": 50541645, "end": 50552066}, {"filename": "/GameData/textures/delvenPack/dlv_ground3a.png", "start": 50552066, "end": 50562125}, {"filename": "/GameData/textures/delvenPack/dlv_ground3b.png", "start": 50562125, "end": 50571837}, {"filename": "/GameData/textures/delvenPack/dlv_ground3c.png", "start": 50571837, "end": 50581273}, {"filename": "/GameData/textures/delvenPack/dlv_ground4a.png", "start": 50581273, "end": 50591464}, {"filename": "/GameData/textures/delvenPack/dlv_ground4b.png", "start": 50591464, "end": 50601914}, {"filename": "/GameData/textures/delvenPack/dlv_ground4c.png", "start": 50601914, "end": 50611855}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal1a.png", "start": 50611855, "end": 50615883}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal1b.png", "start": 50615883, "end": 50620595}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal1c.png", "start": 50620595, "end": 50624676}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal2a.png", "start": 50624676, "end": 50628729}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal2b.png", "start": 50628729, "end": 50633488}, {"filename": "/GameData/textures/delvenPack/dlv_litmetal2c.png", "start": 50633488, "end": 50637359}, {"filename": "/GameData/textures/delvenPack/dlv_litstone1a.png", "start": 50637359, "end": 50640737}, {"filename": "/GameData/textures/delvenPack/dlv_litstone1b.png", "start": 50640737, "end": 50644950}, {"filename": "/GameData/textures/delvenPack/dlv_litstone1c.png", "start": 50644950, "end": 50648581}, {"filename": "/GameData/textures/delvenPack/dlv_litstone2a.png", "start": 50648581, "end": 50653378}, {"filename": "/GameData/textures/delvenPack/dlv_litstone2b.png", "start": 50653378, "end": 50658904}, {"filename": "/GameData/textures/delvenPack/dlv_litstone2c.png", "start": 50658904, "end": 50663147}, {"filename": "/GameData/textures/delvenPack/dlv_metalgen1.png", "start": 50663147, "end": 50668972}, {"filename": "/GameData/textures/delvenPack/dlv_metalgen2.png", "start": 50668972, "end": 50675852}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan1a.png", "start": 50675852, "end": 50683910}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan1b.png", "start": 50683910, "end": 50691380}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan1c.png", "start": 50691380, "end": 50700582}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan2a.png", "start": 50700582, "end": 50708909}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan2b.png", "start": 50708909, "end": 50716321}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan2c.png", "start": 50716321, "end": 50725520}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan3a.png", "start": 50725520, "end": 50734127}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan3b.png", "start": 50734127, "end": 50742354}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan3c.png", "start": 50742354, "end": 50751656}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan4a.png", "start": 50751656, "end": 50760417}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan4b.png", "start": 50760417, "end": 50768742}, {"filename": "/GameData/textures/delvenPack/dlv_metalpan4c.png", "start": 50768742, "end": 50778197}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip1a.png", "start": 50778197, "end": 50784841}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip1b.png", "start": 50784841, "end": 50791353}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip2a.png", "start": 50791353, "end": 50798486}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip2b.png", "start": 50798486, "end": 50805504}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip3a.png", "start": 50805504, "end": 50812775}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip3b.png", "start": 50812775, "end": 50819853}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip4a.png", "start": 50819853, "end": 50827421}, {"filename": "/GameData/textures/delvenPack/dlv_metalpip4b.png", "start": 50827421, "end": 50834776}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm1a.png", "start": 50834776, "end": 50841774}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm1b.png", "start": 50841774, "end": 50848307}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm1c.png", "start": 50848307, "end": 50856495}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm2a.png", "start": 50856495, "end": 50863992}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm2b.png", "start": 50863992, "end": 50870580}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm2c.png", "start": 50870580, "end": 50878841}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm3a.png", "start": 50878841, "end": 50886117}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm3b.png", "start": 50886117, "end": 50893043}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm3c.png", "start": 50893043, "end": 50901038}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm4a.png", "start": 50901038, "end": 50907918}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm4b.png", "start": 50907918, "end": 50914626}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm4c.png", "start": 50914626, "end": 50922435}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm5.png", "start": 50922435, "end": 50926094}, {"filename": "/GameData/textures/delvenPack/dlv_metaltrm6.png", "start": 50926094, "end": 50929817}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr1a.png", "start": 50929817, "end": 50936609}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr1b.png", "start": 50936609, "end": 50944980}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr1c.png", "start": 50944980, "end": 50952650}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr1d.png", "start": 50952650, "end": 50960767}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr1e.png", "start": 50960767, "end": 50968659}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr2a.png", "start": 50968659, "end": 50976850}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr2b.png", "start": 50976850, "end": 50986362}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr2c.png", "start": 50986362, "end": 50995264}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr2d.png", "start": 50995264, "end": 51004515}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr2e.png", "start": 51004515, "end": 51013586}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr3a.png", "start": 51013586, "end": 51021717}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr3b.png", "start": 51021717, "end": 51031427}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr3c.png", "start": 51031427, "end": 51040399}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr3d.png", "start": 51040399, "end": 51049873}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr3e.png", "start": 51049873, "end": 51059100}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr4a.png", "start": 51059100, "end": 51067606}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr4b.png", "start": 51067606, "end": 51076066}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr4c.png", "start": 51076066, "end": 51084730}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr5a.png", "start": 51084730, "end": 51091909}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr5b.png", "start": 51091909, "end": 51099037}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr5c.png", "start": 51099037, "end": 51107177}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr6a.png", "start": 51107177, "end": 51115853}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr6b.png", "start": 51115853, "end": 51124268}, {"filename": "/GameData/textures/delvenPack/dlv_slateflr6c.png", "start": 51124268, "end": 51133001}, {"filename": "/GameData/textures/delvenPack/dlv_slategen1.png", "start": 51133001, "end": 51138962}, {"filename": "/GameData/textures/delvenPack/dlv_slategen2.png", "start": 51138962, "end": 51146521}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk1a.png", "start": 51146521, "end": 51152471}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk1b.png", "start": 51152471, "end": 51158287}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk1c.png", "start": 51158287, "end": 51164231}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk1d.png", "start": 51164231, "end": 51169877}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk2a.png", "start": 51169877, "end": 51178749}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk2b.png", "start": 51178749, "end": 51187423}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk2c.png", "start": 51187423, "end": 51196220}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk2d.png", "start": 51196220, "end": 51204516}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk3a.png", "start": 51204516, "end": 51212963}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk3b.png", "start": 51212963, "end": 51221610}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4a.png", "start": 51221610, "end": 51228229}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4b.png", "start": 51228229, "end": 51234915}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4c.png", "start": 51234915, "end": 51241532}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4d.png", "start": 51241532, "end": 51248146}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4e.png", "start": 51248146, "end": 51254730}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk4f.png", "start": 51254730, "end": 51261087}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5a.png", "start": 51261087, "end": 51270564}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5b.png", "start": 51270564, "end": 51280076}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5c.png", "start": 51280076, "end": 51289574}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5d.png", "start": 51289574, "end": 51299047}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5e.png", "start": 51299047, "end": 51308438}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk5f.png", "start": 51308438, "end": 51317575}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk6a.png", "start": 51317575, "end": 51326500}, {"filename": "/GameData/textures/delvenPack/dlv_stonebrk6b.png", "start": 51326500, "end": 51335725}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr1a.png", "start": 51335725, "end": 51341155}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr1b.png", "start": 51341155, "end": 51347577}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr1c.png", "start": 51347577, "end": 51354600}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr2a.png", "start": 51354600, "end": 51362488}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr2b.png", "start": 51362488, "end": 51371303}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr2c.png", "start": 51371303, "end": 51380584}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr3a.png", "start": 51380584, "end": 51386287}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr3b.png", "start": 51386287, "end": 51392765}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr3c.png", "start": 51392765, "end": 51399757}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr4a.png", "start": 51399757, "end": 51408149}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr4b.png", "start": 51408149, "end": 51417218}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr4c.png", "start": 51417218, "end": 51426318}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr5a.png", "start": 51426318, "end": 51432408}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr5b.png", "start": 51432408, "end": 51438821}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr5c.png", "start": 51438821, "end": 51445739}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr6a.png", "start": 51445739, "end": 51454284}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr6b.png", "start": 51454284, "end": 51463104}, {"filename": "/GameData/textures/delvenPack/dlv_stoneflr6c.png", "start": 51463104, "end": 51472388}, {"filename": "/GameData/textures/delvenPack/dlv_stonegen1.png", "start": 51472388, "end": 51477482}, {"filename": "/GameData/textures/delvenPack/dlv_stonegen2.png", "start": 51477482, "end": 51484912}, {"filename": "/GameData/textures/delvenPack/dlv_stonestep1a.png", "start": 51484912, "end": 51490355}, {"filename": "/GameData/textures/delvenPack/dlv_stonestep1b.png", "start": 51490355, "end": 51495895}, {"filename": "/GameData/textures/delvenPack/dlv_stonestep2a.png", "start": 51495895, "end": 51503744}, {"filename": "/GameData/textures/delvenPack/dlv_stonestep2b.png", "start": 51503744, "end": 51511494}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm1a.png", "start": 51511494, "end": 51516784}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm1b.png", "start": 51516784, "end": 51522884}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm1c.png", "start": 51522884, "end": 51528460}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm2a.png", "start": 51528460, "end": 51536206}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm2b.png", "start": 51536206, "end": 51545036}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm2c.png", "start": 51545036, "end": 51553344}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm3a.png", "start": 51553344, "end": 51558644}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm3b.png", "start": 51558644, "end": 51564717}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm4a.png", "start": 51564717, "end": 51572539}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm4b.png", "start": 51572539, "end": 51581070}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm5.png", "start": 51581070, "end": 51584227}, {"filename": "/GameData/textures/delvenPack/dlv_stonetrm6.png", "start": 51584227, "end": 51588582}, {"filename": "/GameData/textures/delvenPack/dlv_stonewaf1a.png", "start": 51588582, "end": 51594193}, {"filename": "/GameData/textures/delvenPack/dlv_stonewaf1b.png", "start": 51594193, "end": 51599810}, {"filename": "/GameData/textures/delvenPack/dlv_stonewaf2a.png", "start": 51599810, "end": 51607683}, {"filename": "/GameData/textures/delvenPack/dlv_stonewaf2b.png", "start": 51607683, "end": 51615792}, {"filename": "/GameData/textures/delvenPack/dlv_wood1a.png", "start": 51615792, "end": 51620491}, {"filename": "/GameData/textures/delvenPack/dlv_wood1b.png", "start": 51620491, "end": 51625450}, {"filename": "/GameData/textures/delvenPack/dlv_wood2a.png", "start": 51625450, "end": 51632146}, {"filename": "/GameData/textures/delvenPack/dlv_wood2b.png", "start": 51632146, "end": 51639103}, {"filename": "/GameData/textures/delvenPack/dlv_wood3a.png", "start": 51639103, "end": 51644307}, {"filename": "/GameData/textures/delvenPack/dlv_wood3b.png", "start": 51644307, "end": 51649765}, {"filename": "/GameData/textures/delvenPack/dlv_wood4a.png", "start": 51649765, "end": 51656597}, {"filename": "/GameData/textures/delvenPack/dlv_wood4b.png", "start": 51656597, "end": 51663642}, {"filename": "/GameData/textures/delvenPack/dlv_wood5a.png", "start": 51663642, "end": 51669655}, {"filename": "/GameData/textures/delvenPack/dlv_wood5b.png", "start": 51669655, "end": 51676490}, {"filename": "/GameData/textures/delvenPack/dlv_wood5c.png", "start": 51676490, "end": 51683500}, {"filename": "/GameData/textures/delvenPack/dlv_wood5d.png", "start": 51683500, "end": 51691090}, {"filename": "/GameData/textures/delvenPack/dlv_wood5e.png", "start": 51691090, "end": 51700223}, {"filename": "/GameData/textures/delvenPack/dlv_wood5f.png", "start": 51700223, "end": 51709252}, {"filename": "/GameData/textures/delvenPack/dlv_wood5g.png", "start": 51709252, "end": 51717986}, {"filename": "/GameData/textures/delvenPack/dlv_wood5h.png", "start": 51717986, "end": 51726714}, {"filename": "/GameData/textures/delvenPack/dlv_wood6a.png", "start": 51726714, "end": 51734344}, {"filename": "/GameData/textures/delvenPack/dlv_wood6b.png", "start": 51734344, "end": 51742792}, {"filename": "/GameData/textures/delvenPack/dlv_wood6c.png", "start": 51742792, "end": 51750360}, {"filename": "/GameData/textures/delvenPack/dlv_wood6d.png", "start": 51750360, "end": 51758757}, {"filename": "/GameData/textures/delvenPack/dlv_wood6e.png", "start": 51758757, "end": 51768514}, {"filename": "/GameData/textures/delvenPack/dlv_wood6f.png", "start": 51768514, "end": 51778460}, {"filename": "/GameData/textures/delvenPack/dlv_wood6g.png", "start": 51778460, "end": 51787663}, {"filename": "/GameData/textures/delvenPack/dlv_wood6h.png", "start": 51787663, "end": 51797102}, {"filename": "/GameData/textures/delvenPack/dlv_woodgen1.png", "start": 51797102, "end": 51801649}, {"filename": "/GameData/textures/delvenPack/dlv_woodgen2.png", "start": 51801649, "end": 51808342}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry1a.png", "start": 51808342, "end": 51816129}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry1b.png", "start": 51816129, "end": 51823276}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry1c.png", "start": 51823276, "end": 51827546}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry1d.png", "start": 51827546, "end": 51831556}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry2a.png", "start": 51831556, "end": 51838695}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry2b.png", "start": 51838695, "end": 51845254}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry2c.png", "start": 51845254, "end": 51849075}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry2d.png", "start": 51849075, "end": 51852713}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry3a.png", "start": 51852713, "end": 51859842}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry3b.png", "start": 51859842, "end": 51866428}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry3c.png", "start": 51866428, "end": 51870401}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry3d.png", "start": 51870401, "end": 51874173}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry4a.png", "start": 51874173, "end": 51885055}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry4b.png", "start": 51885055, "end": 51893893}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry4c.png", "start": 51893893, "end": 51899395}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry4d.png", "start": 51899395, "end": 51904105}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry5a.png", "start": 51904105, "end": 51914689}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry5b.png", "start": 51914689, "end": 51923315}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry5c.png", "start": 51923315, "end": 51928624}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry5d.png", "start": 51928624, "end": 51933174}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry6a.png", "start": 51933174, "end": 51942023}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry6b.png", "start": 51942023, "end": 51949301}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry6c.png", "start": 51949301, "end": 51953919}, {"filename": "/GameData/textures/delvenPack/{dlv_tapestry6d.png", "start": 51953919, "end": 51957931}, {"filename": "/GameData/textures/generic/__TB_empty.png", "start": 51957931, "end": 51958689}, {"filename": "/GameData/textures/generic/brick.png", "start": 51958689, "end": 52471603}, {"filename": "/GameData/textures/generic/brickPBR.png", "start": 52471603, "end": 53401735}, {"filename": "/GameData/textures/generic/brickPBR_orm.png", "start": 53401735, "end": 53826586}, {"filename": "/GameData/textures/generic/bricks.png", "start": 53826586, "end": 53837506}, {"filename": "/GameData/textures/generic/cat.png", "start": 53837506, "end": 54086944}, {"filename": "/GameData/textures/generic/foil.png", "start": 54086944, "end": 54344162}, {"filename": "/GameData/textures/generic/grass.png", "start": 54344162, "end": 54472287}, {"filename": "/GameData/textures/generic/hole_t.png", "start": 54472287, "end": 54474716}, {"filename": "/GameData/textures/generic/light.png", "start": 54474716, "end": 54474844}, {"filename": "/GameData/textures/generic/light_em.png", "start": 54474844, "end": 54474972}, {"filename": "/GameData/textures/generic/mask_test_m.png", "start": 54474972, "end": 54479034}, {"filename": "/GameData/textures/generic/mirror.png", "start": 54479034, "end": 54479154}, {"filename": "/GameData/textures/generic/mirror_orm.png", "start": 54479154, "end": 54479274}, {"filename": "/GameData/textures/generic/null_m.png", "start": 54479274, "end": 54481071}, {"filename": "/GameData/textures/generic/white.png", "start": 54481071, "end": 54481191}, {"filename": "/GameData/textures/gloves.png", "start": 54481191, "end": 54590866}, {"filename": "/GameData/textures/jacket.png", "start": 54590866, "end": 54754678}, {"filename": "/GameData/textures/lq_conc/conc1_1.png", "start": 54754678, "end": 54767791}, {"filename": "/GameData/textures/lq_conc/conc1_10.png", "start": 54767791, "end": 54781905}, {"filename": "/GameData/textures/lq_conc/conc1_2.png", "start": 54781905, "end": 54795946}, {"filename": "/GameData/textures/lq_conc/conc1_3.png", "start": 54795946, "end": 54810456}, {"filename": "/GameData/textures/lq_conc/conc1_4.png", "start": 54810456, "end": 54824842}, {"filename": "/GameData/textures/lq_conc/conc1_5.png", "start": 54824842, "end": 54839291}, {"filename": "/GameData/textures/lq_conc/conc1_6.png", "start": 54839291, "end": 54853908}, {"filename": "/GameData/textures/lq_conc/conc1_7.png", "start": 54853908, "end": 54868505}, {"filename": "/GameData/textures/lq_conc/conc1_8.png", "start": 54868505, "end": 54882853}, {"filename": "/GameData/textures/lq_conc/conc1_9.png", "start": 54882853, "end": 54897011}, {"filename": "/GameData/textures/lq_conc/conc1_a1.png", "start": 54897011, "end": 54911459}, {"filename": "/GameData/textures/lq_conc/conc2_1.png", "start": 54911459, "end": 54920887}, {"filename": "/GameData/textures/lq_conc/conc2_10.png", "start": 54920887, "end": 54930336}, {"filename": "/GameData/textures/lq_conc/conc2_2.png", "start": 54930336, "end": 54939730}, {"filename": "/GameData/textures/lq_conc/conc2_3.png", "start": 54939730, "end": 54949375}, {"filename": "/GameData/textures/lq_conc/conc2_4.png", "start": 54949375, "end": 54958882}, {"filename": "/GameData/textures/lq_conc/conc2_5.png", "start": 54958882, "end": 54968387}, {"filename": "/GameData/textures/lq_conc/conc2_6.png", "start": 54968387, "end": 54977900}, {"filename": "/GameData/textures/lq_conc/conc2_7.png", "start": 54977900, "end": 54987445}, {"filename": "/GameData/textures/lq_conc/conc2_8.png", "start": 54987445, "end": 54996855}, {"filename": "/GameData/textures/lq_conc/conc2_9.png", "start": 54996855, "end": 55006392}, {"filename": "/GameData/textures/lq_conc/conc2_a1.png", "start": 55006392, "end": 55015422}, {"filename": "/GameData/textures/lq_conc/conc3_1.png", "start": 55015422, "end": 55023286}, {"filename": "/GameData/textures/lq_conc/conc3_10.png", "start": 55023286, "end": 55032222}, {"filename": "/GameData/textures/lq_conc/conc3_2.png", "start": 55032222, "end": 55040616}, {"filename": "/GameData/textures/lq_conc/conc3_3.png", "start": 55040616, "end": 55050843}, {"filename": "/GameData/textures/lq_conc/conc3_4.png", "start": 55050843, "end": 55060122}, {"filename": "/GameData/textures/lq_conc/conc3_5.png", "start": 55060122, "end": 55069852}, {"filename": "/GameData/textures/lq_conc/conc3_6.png", "start": 55069852, "end": 55079412}, {"filename": "/GameData/textures/lq_conc/conc3_7.png", "start": 55079412, "end": 55088237}, {"filename": "/GameData/textures/lq_conc/conc3_8.png", "start": 55088237, "end": 55097018}, {"filename": "/GameData/textures/lq_conc/conc3_9.png", "start": 55097018, "end": 55106494}, {"filename": "/GameData/textures/lq_conc/conc3_a1.png", "start": 55106494, "end": 55114901}, {"filename": "/GameData/textures/lq_conc/conc4_1.png", "start": 55114901, "end": 55121060}, {"filename": "/GameData/textures/lq_conc/conc4_10.png", "start": 55121060, "end": 55127420}, {"filename": "/GameData/textures/lq_conc/conc4_2.png", "start": 55127420, "end": 55133685}, {"filename": "/GameData/textures/lq_conc/conc4_3.png", "start": 55133685, "end": 55140611}, {"filename": "/GameData/textures/lq_conc/conc4_4.png", "start": 55140611, "end": 55147101}, {"filename": "/GameData/textures/lq_conc/conc4_5.png", "start": 55147101, "end": 55153694}, {"filename": "/GameData/textures/lq_conc/conc4_6.png", "start": 55153694, "end": 55160218}, {"filename": "/GameData/textures/lq_conc/conc4_7.png", "start": 55160218, "end": 55166626}, {"filename": "/GameData/textures/lq_conc/conc4_8.png", "start": 55166626, "end": 55172909}, {"filename": "/GameData/textures/lq_conc/conc4_9.png", "start": 55172909, "end": 55179492}, {"filename": "/GameData/textures/lq_conc/conc4_a1.png", "start": 55179492, "end": 55185561}, {"filename": "/GameData/textures/lq_conc/conc5_1.png", "start": 55185561, "end": 55194323}, {"filename": "/GameData/textures/lq_conc/conc5_10.png", "start": 55194323, "end": 55203938}, {"filename": "/GameData/textures/lq_conc/conc5_2.png", "start": 55203938, "end": 55213254}, {"filename": "/GameData/textures/lq_conc/conc5_8.png", "start": 55213254, "end": 55222334}, {"filename": "/GameData/textures/lq_conc/conc5_9.png", "start": 55222334, "end": 55232048}, {"filename": "/GameData/textures/lq_conc/conc5_a1.png", "start": 55232048, "end": 55240423}, {"filename": "/GameData/textures/lq_conc/conc6_1.png", "start": 55240423, "end": 55246836}, {"filename": "/GameData/textures/lq_conc/conc6_10.png", "start": 55246836, "end": 55253245}, {"filename": "/GameData/textures/lq_conc/conc6_2.png", "start": 55253245, "end": 55259573}, {"filename": "/GameData/textures/lq_conc/conc6_3.png", "start": 55259573, "end": 55266434}, {"filename": "/GameData/textures/lq_conc/conc6_4.png", "start": 55266434, "end": 55273042}, {"filename": "/GameData/textures/lq_conc/conc6_5.png", "start": 55273042, "end": 55279718}, {"filename": "/GameData/textures/lq_conc/conc6_6.png", "start": 55279718, "end": 55286492}, {"filename": "/GameData/textures/lq_conc/conc6_7.png", "start": 55286492, "end": 55293195}, {"filename": "/GameData/textures/lq_conc/conc6_8.png", "start": 55293195, "end": 55299691}, {"filename": "/GameData/textures/lq_conc/conc6_9.png", "start": 55299691, "end": 55306313}, {"filename": "/GameData/textures/lq_conc/conc6_a1.png", "start": 55306313, "end": 55312957}, {"filename": "/GameData/textures/lq_conc/conc7_1.png", "start": 55312957, "end": 55320747}, {"filename": "/GameData/textures/lq_conc/conc7_10.png", "start": 55320747, "end": 55328425}, {"filename": "/GameData/textures/lq_conc/conc7_2.png", "start": 55328425, "end": 55336051}, {"filename": "/GameData/textures/lq_conc/conc7_3.png", "start": 55336051, "end": 55343995}, {"filename": "/GameData/textures/lq_conc/conc7_4.png", "start": 55343995, "end": 55351763}, {"filename": "/GameData/textures/lq_conc/conc7_5.png", "start": 55351763, "end": 55359587}, {"filename": "/GameData/textures/lq_conc/conc7_6.png", "start": 55359587, "end": 55367586}, {"filename": "/GameData/textures/lq_conc/conc7_7.png", "start": 55367586, "end": 55375566}, {"filename": "/GameData/textures/lq_conc/conc7_8.png", "start": 55375566, "end": 55383390}, {"filename": "/GameData/textures/lq_conc/conc7_9.png", "start": 55383390, "end": 55391190}, {"filename": "/GameData/textures/lq_conc/conc7_a1.png", "start": 55391190, "end": 55398923}, {"filename": "/GameData/textures/lq_conc/flr1_1.png", "start": 55398923, "end": 55401255}, {"filename": "/GameData/textures/lq_conc/flr1_2.png", "start": 55401255, "end": 55404519}, {"filename": "/GameData/textures/lq_conc/flr1_3.png", "start": 55404519, "end": 55408059}, {"filename": "/GameData/textures/lq_conc/flr1_4.png", "start": 55408059, "end": 55410836}, {"filename": "/GameData/textures/lq_conc/flr1_5.png", "start": 55410836, "end": 55414432}, {"filename": "/GameData/textures/lq_conc/flr1_6.png", "start": 55414432, "end": 55417488}, {"filename": "/GameData/textures/lq_conc/flr2_1.png", "start": 55417488, "end": 55419598}, {"filename": "/GameData/textures/lq_conc/flr2_2.png", "start": 55419598, "end": 55422811}, {"filename": "/GameData/textures/lq_conc/flr2_3.png", "start": 55422811, "end": 55426431}, {"filename": "/GameData/textures/lq_conc/flr2_4.png", "start": 55426431, "end": 55429829}, {"filename": "/GameData/textures/lq_conc/flr2_5.png", "start": 55429829, "end": 55433189}, {"filename": "/GameData/textures/lq_conc/flr2_6.png", "start": 55433189, "end": 55436388}, {"filename": "/GameData/textures/lq_conc/flr2_7.png", "start": 55436388, "end": 55439523}, {"filename": "/GameData/textures/lq_conc/flr2_8.png", "start": 55439523, "end": 55442464}, {"filename": "/GameData/textures/lq_dev/clip.png", "start": 55442464, "end": 55442970}, {"filename": "/GameData/textures/lq_dev/dot_blue_a.png", "start": 55442970, "end": 55443526}, {"filename": "/GameData/textures/lq_dev/dot_blue_b.png", "start": 55443526, "end": 55444081}, {"filename": "/GameData/textures/lq_dev/dot_blue_c.png", "start": 55444081, "end": 55444636}, {"filename": "/GameData/textures/lq_dev/dot_brown_a.png", "start": 55444636, "end": 55445191}, {"filename": "/GameData/textures/lq_dev/dot_brown_b.png", "start": 55445191, "end": 55445746}, {"filename": "/GameData/textures/lq_dev/dot_brown_c.png", "start": 55445746, "end": 55446301}, {"filename": "/GameData/textures/lq_dev/dot_green_a.png", "start": 55446301, "end": 55446857}, {"filename": "/GameData/textures/lq_dev/dot_green_b.png", "start": 55446857, "end": 55447412}, {"filename": "/GameData/textures/lq_dev/dot_green_c.png", "start": 55447412, "end": 55447967}, {"filename": "/GameData/textures/lq_dev/dot_grey_a.png", "start": 55447967, "end": 55448520}, {"filename": "/GameData/textures/lq_dev/dot_grey_b.png", "start": 55448520, "end": 55449076}, {"filename": "/GameData/textures/lq_dev/dot_grey_c.png", "start": 55449076, "end": 55449631}, {"filename": "/GameData/textures/lq_dev/dot_olive_a.png", "start": 55449631, "end": 55450186}, {"filename": "/GameData/textures/lq_dev/dot_olive_b.png", "start": 55450186, "end": 55450740}, {"filename": "/GameData/textures/lq_dev/dot_olive_c.png", "start": 55450740, "end": 55451295}, {"filename": "/GameData/textures/lq_dev/dot_orange_a.png", "start": 55451295, "end": 55451851}, {"filename": "/GameData/textures/lq_dev/dot_orange_b.png", "start": 55451851, "end": 55452406}, {"filename": "/GameData/textures/lq_dev/dot_orange_c.png", "start": 55452406, "end": 55452961}, {"filename": "/GameData/textures/lq_dev/dot_pink_a.png", "start": 55452961, "end": 55453517}, {"filename": "/GameData/textures/lq_dev/dot_pink_b.png", "start": 55453517, "end": 55454073}, {"filename": "/GameData/textures/lq_dev/dot_pink_c.png", "start": 55454073, "end": 55454628}, {"filename": "/GameData/textures/lq_dev/dot_purple_a.png", "start": 55454628, "end": 55455184}, {"filename": "/GameData/textures/lq_dev/dot_purple_b.png", "start": 55455184, "end": 55455739}, {"filename": "/GameData/textures/lq_dev/dot_purple_c.png", "start": 55455739, "end": 55456294}, {"filename": "/GameData/textures/lq_dev/dot_red_a.png", "start": 55456294, "end": 55456845}, {"filename": "/GameData/textures/lq_dev/dot_red_b.png", "start": 55456845, "end": 55457396}, {"filename": "/GameData/textures/lq_dev/dot_red_c.png", "start": 55457396, "end": 55457947}, {"filename": "/GameData/textures/lq_dev/dot_tan_a.png", "start": 55457947, "end": 55458503}, {"filename": "/GameData/textures/lq_dev/dot_tan_b.png", "start": 55458503, "end": 55459059}, {"filename": "/GameData/textures/lq_dev/dot_tan_c.png", "start": 55459059, "end": 55459614}, {"filename": "/GameData/textures/lq_dev/dot_yellow_a.png", "start": 55459614, "end": 55460170}, {"filename": "/GameData/textures/lq_dev/dot_yellow_b.png", "start": 55460170, "end": 55460726}, {"filename": "/GameData/textures/lq_dev/dot_yellow_c.png", "start": 55460726, "end": 55461280}, {"filename": "/GameData/textures/lq_dev/floor_blue_a.png", "start": 55461280, "end": 55462097}, {"filename": "/GameData/textures/lq_dev/floor_blue_b.png", "start": 55462097, "end": 55462915}, {"filename": "/GameData/textures/lq_dev/floor_blue_c.png", "start": 55462915, "end": 55463733}, {"filename": "/GameData/textures/lq_dev/floor_brown_a.png", "start": 55463733, "end": 55464555}, {"filename": "/GameData/textures/lq_dev/floor_brown_b.png", "start": 55464555, "end": 55465378}, {"filename": "/GameData/textures/lq_dev/floor_brown_c.png", "start": 55465378, "end": 55466200}, {"filename": "/GameData/textures/lq_dev/floor_green_a.png", "start": 55466200, "end": 55467021}, {"filename": "/GameData/textures/lq_dev/floor_green_b.png", "start": 55467021, "end": 55467843}, {"filename": "/GameData/textures/lq_dev/floor_green_c.png", "start": 55467843, "end": 55468665}, {"filename": "/GameData/textures/lq_dev/floor_grey_a.png", "start": 55468665, "end": 55469475}, {"filename": "/GameData/textures/lq_dev/floor_grey_b.png", "start": 55469475, "end": 55470288}, {"filename": "/GameData/textures/lq_dev/floor_grey_c.png", "start": 55470288, "end": 55471102}, {"filename": "/GameData/textures/lq_dev/floor_olive_a.png", "start": 55471102, "end": 55471920}, {"filename": "/GameData/textures/lq_dev/floor_olive_b.png", "start": 55471920, "end": 55472735}, {"filename": "/GameData/textures/lq_dev/floor_olive_c.png", "start": 55472735, "end": 55473552}, {"filename": "/GameData/textures/lq_dev/floor_orange_a.png", "start": 55473552, "end": 55474375}, {"filename": "/GameData/textures/lq_dev/floor_orange_b.png", "start": 55474375, "end": 55475198}, {"filename": "/GameData/textures/lq_dev/floor_orange_c.png", "start": 55475198, "end": 55476018}, {"filename": "/GameData/textures/lq_dev/floor_pink_a.png", "start": 55476018, "end": 55476841}, {"filename": "/GameData/textures/lq_dev/floor_pink_b.png", "start": 55476841, "end": 55477663}, {"filename": "/GameData/textures/lq_dev/floor_pink_c.png", "start": 55477663, "end": 55478485}, {"filename": "/GameData/textures/lq_dev/floor_purple_a.png", "start": 55478485, "end": 55479307}, {"filename": "/GameData/textures/lq_dev/floor_purple_b.png", "start": 55479307, "end": 55480129}, {"filename": "/GameData/textures/lq_dev/floor_purple_c.png", "start": 55480129, "end": 55480950}, {"filename": "/GameData/textures/lq_dev/floor_red_a.png", "start": 55480950, "end": 55481760}, {"filename": "/GameData/textures/lq_dev/floor_red_b.png", "start": 55481760, "end": 55482570}, {"filename": "/GameData/textures/lq_dev/floor_red_c.png", "start": 55482570, "end": 55483381}, {"filename": "/GameData/textures/lq_dev/floor_tan_a.png", "start": 55483381, "end": 55484201}, {"filename": "/GameData/textures/lq_dev/floor_tan_b.png", "start": 55484201, "end": 55485025}, {"filename": "/GameData/textures/lq_dev/floor_tan_c.png", "start": 55485025, "end": 55485846}, {"filename": "/GameData/textures/lq_dev/floor_yellow_a.png", "start": 55485846, "end": 55486667}, {"filename": "/GameData/textures/lq_dev/floor_yellow_b.png", "start": 55486667, "end": 55487489}, {"filename": "/GameData/textures/lq_dev/floor_yellow_c.png", "start": 55487489, "end": 55488308}, {"filename": "/GameData/textures/lq_dev/hint.png", "start": 55488308, "end": 55489174}, {"filename": "/GameData/textures/lq_dev/hintskip.png", "start": 55489174, "end": 55490067}, {"filename": "/GameData/textures/lq_dev/key_gold_1.png", "start": 55490067, "end": 55490822}, {"filename": "/GameData/textures/lq_dev/key_silver_1.png", "start": 55490822, "end": 55491576}, {"filename": "/GameData/textures/lq_dev/light_fbr.png", "start": 55491576, "end": 55492492}, {"filename": "/GameData/textures/lq_dev/origin.png", "start": 55492492, "end": 55492975}, {"filename": "/GameData/textures/lq_dev/plus_0_button_fbr.png", "start": 55492975, "end": 55493174}, {"filename": "/GameData/textures/lq_dev/plus_0_shoot_fbr.png", "start": 55493174, "end": 55493453}, {"filename": "/GameData/textures/lq_dev/plus_1_button_fbr.png", "start": 55493453, "end": 55493654}, {"filename": "/GameData/textures/lq_dev/plus_1_shoot_fbr.png", "start": 55493654, "end": 55493935}, {"filename": "/GameData/textures/lq_dev/plus_a_button_fbr.png", "start": 55493935, "end": 55494136}, {"filename": "/GameData/textures/lq_dev/plus_a_shoot_fbr.png", "start": 55494136, "end": 55494423}, {"filename": "/GameData/textures/lq_dev/skip.png", "start": 55494423, "end": 55494911}, {"filename": "/GameData/textures/lq_dev/sky_dev.png", "start": 55494911, "end": 55496883}, {"filename": "/GameData/textures/lq_dev/sky_dev.png.bak", "start": 55496883, "end": 55509237}, {"filename": "/GameData/textures/lq_dev/sky_dev_day_fbr.png", "start": 55509237, "end": 55513063}, {"filename": "/GameData/textures/lq_dev/sky_dev_void.png", "start": 55513063, "end": 55513974}, {"filename": "/GameData/textures/lq_dev/star_blood1.png", "start": 55513974, "end": 55514336}, {"filename": "/GameData/textures/lq_dev/star_lava1.png", "start": 55514336, "end": 55514707}, {"filename": "/GameData/textures/lq_dev/star_lavaskip.png", "start": 55514707, "end": 55515820}, {"filename": "/GameData/textures/lq_dev/star_slime1.png", "start": 55515820, "end": 55516190}, {"filename": "/GameData/textures/lq_dev/star_slimeskip.png", "start": 55516190, "end": 55517263}, {"filename": "/GameData/textures/lq_dev/star_smile.png", "start": 55517263, "end": 55517743}, {"filename": "/GameData/textures/lq_dev/star_teleport.png", "start": 55517743, "end": 55518067}, {"filename": "/GameData/textures/lq_dev/star_water1.png", "start": 55518067, "end": 55518439}, {"filename": "/GameData/textures/lq_dev/star_water2.png", "start": 55518439, "end": 55518808}, {"filename": "/GameData/textures/lq_dev/star_waterskip.png", "start": 55518808, "end": 55520548}, {"filename": "/GameData/textures/lq_dev/trigger.png", "start": 55520548, "end": 55521045}, {"filename": "/GameData/textures/lq_dev/wall_blue_a.png", "start": 55521045, "end": 55521725}, {"filename": "/GameData/textures/lq_dev/wall_blue_b.png", "start": 55521725, "end": 55522405}, {"filename": "/GameData/textures/lq_dev/wall_blue_c.png", "start": 55522405, "end": 55523085}, {"filename": "/GameData/textures/lq_dev/wall_brown_a.png", "start": 55523085, "end": 55523767}, {"filename": "/GameData/textures/lq_dev/wall_brown_b.png", "start": 55523767, "end": 55524449}, {"filename": "/GameData/textures/lq_dev/wall_brown_c.png", "start": 55524449, "end": 55525131}, {"filename": "/GameData/textures/lq_dev/wall_green_a.png", "start": 55525131, "end": 55525813}, {"filename": "/GameData/textures/lq_dev/wall_green_b.png", "start": 55525813, "end": 55526495}, {"filename": "/GameData/textures/lq_dev/wall_green_c.png", "start": 55526495, "end": 55527177}, {"filename": "/GameData/textures/lq_dev/wall_grey_a.png", "start": 55527177, "end": 55527851}, {"filename": "/GameData/textures/lq_dev/wall_grey_b.png", "start": 55527851, "end": 55528527}, {"filename": "/GameData/textures/lq_dev/wall_grey_c.png", "start": 55528527, "end": 55529204}, {"filename": "/GameData/textures/lq_dev/wall_olive_a.png", "start": 55529204, "end": 55529884}, {"filename": "/GameData/textures/lq_dev/wall_olive_b.png", "start": 55529884, "end": 55530561}, {"filename": "/GameData/textures/lq_dev/wall_olive_c.png", "start": 55530561, "end": 55531240}, {"filename": "/GameData/textures/lq_dev/wall_orange_a.png", "start": 55531240, "end": 55531922}, {"filename": "/GameData/textures/lq_dev/wall_orange_b.png", "start": 55531922, "end": 55532604}, {"filename": "/GameData/textures/lq_dev/wall_orange_c.png", "start": 55532604, "end": 55533285}, {"filename": "/GameData/textures/lq_dev/wall_pink_a.png", "start": 55533285, "end": 55533967}, {"filename": "/GameData/textures/lq_dev/wall_pink_b.png", "start": 55533967, "end": 55534649}, {"filename": "/GameData/textures/lq_dev/wall_pink_c.png", "start": 55534649, "end": 55535331}, {"filename": "/GameData/textures/lq_dev/wall_purple_a.png", "start": 55535331, "end": 55536014}, {"filename": "/GameData/textures/lq_dev/wall_purple_b.png", "start": 55536014, "end": 55536696}, {"filename": "/GameData/textures/lq_dev/wall_purple_c.png", "start": 55536696, "end": 55537378}, {"filename": "/GameData/textures/lq_dev/wall_red_a.png", "start": 55537378, "end": 55538051}, {"filename": "/GameData/textures/lq_dev/wall_red_b.png", "start": 55538051, "end": 55538724}, {"filename": "/GameData/textures/lq_dev/wall_red_c.png", "start": 55538724, "end": 55539398}, {"filename": "/GameData/textures/lq_dev/wall_tan_a.png", "start": 55539398, "end": 55540080}, {"filename": "/GameData/textures/lq_dev/wall_tan_b.png", "start": 55540080, "end": 55540762}, {"filename": "/GameData/textures/lq_dev/wall_tan_c.png", "start": 55540762, "end": 55541444}, {"filename": "/GameData/textures/lq_dev/wall_yellow_a.png", "start": 55541444, "end": 55542126}, {"filename": "/GameData/textures/lq_dev/wall_yellow_b.png", "start": 55542126, "end": 55542808}, {"filename": "/GameData/textures/lq_dev/wall_yellow_c.png", "start": 55542808, "end": 55543489}, {"filename": "/GameData/textures/lq_dev/{char_0_fbr.png", "start": 55543489, "end": 55543666}, {"filename": "/GameData/textures/lq_dev/{char_1_fbr.png", "start": 55543666, "end": 55543826}, {"filename": "/GameData/textures/lq_dev/{char_2_fbr.png", "start": 55543826, "end": 55543998}, {"filename": "/GameData/textures/lq_dev/{char_3_fbr.png", "start": 55543998, "end": 55544177}, {"filename": "/GameData/textures/lq_dev/{char_4_fbr.png", "start": 55544177, "end": 55544340}, {"filename": "/GameData/textures/lq_dev/{char_5_fbr.png", "start": 55544340, "end": 55544513}, {"filename": "/GameData/textures/lq_dev/{char_6_fbr.png", "start": 55544513, "end": 55544685}, {"filename": "/GameData/textures/lq_dev/{char_7_fbr.png", "start": 55544685, "end": 55544839}, {"filename": "/GameData/textures/lq_dev/{char_8_fbr.png", "start": 55544839, "end": 55545007}, {"filename": "/GameData/textures/lq_dev/{char_9_fbr.png", "start": 55545007, "end": 55545174}, {"filename": "/GameData/textures/lq_dev/{char_a_fbr.png", "start": 55545174, "end": 55545338}, {"filename": "/GameData/textures/lq_dev/{char_b_fbr.png", "start": 55545338, "end": 55545508}, {"filename": "/GameData/textures/lq_dev/{char_c_fbr.png", "start": 55545508, "end": 55545658}, {"filename": "/GameData/textures/lq_dev/{char_d_fbr.png", "start": 55545658, "end": 55545824}, {"filename": "/GameData/textures/lq_dev/{char_e_fbr.png", "start": 55545824, "end": 55545992}, {"filename": "/GameData/textures/lq_dev/{char_f_fbr.png", "start": 55545992, "end": 55546152}, {"filename": "/GameData/textures/lq_dev/{char_g_fbr.png", "start": 55546152, "end": 55546324}, {"filename": "/GameData/textures/lq_dev/{char_h_fbr.png", "start": 55546324, "end": 55546483}, {"filename": "/GameData/textures/lq_dev/{char_i_fbr.png", "start": 55546483, "end": 55546642}, {"filename": "/GameData/textures/lq_dev/{char_j_fbr.png", "start": 55546642, "end": 55546808}, {"filename": "/GameData/textures/lq_dev/{char_k_fbr.png", "start": 55546808, "end": 55547002}, {"filename": "/GameData/textures/lq_dev/{char_l_fbr.png", "start": 55547002, "end": 55547143}, {"filename": "/GameData/textures/lq_dev/{char_m_fbr.png", "start": 55547143, "end": 55547292}, {"filename": "/GameData/textures/lq_dev/{char_n_fbr.png", "start": 55547292, "end": 55547475}, {"filename": "/GameData/textures/lq_dev/{char_o_fbr.png", "start": 55547475, "end": 55547625}, {"filename": "/GameData/textures/lq_dev/{char_p_fbr.png", "start": 55547625, "end": 55547783}, {"filename": "/GameData/textures/lq_dev/{char_q_fbr.png", "start": 55547783, "end": 55547949}, {"filename": "/GameData/textures/lq_dev/{char_r_fbr.png", "start": 55547949, "end": 55548121}, {"filename": "/GameData/textures/lq_dev/{char_s_fbr.png", "start": 55548121, "end": 55548297}, {"filename": "/GameData/textures/lq_dev/{char_t_fbr.png", "start": 55548297, "end": 55548445}, {"filename": "/GameData/textures/lq_dev/{char_trans_fbr.png", "start": 55548445, "end": 55548568}, {"filename": "/GameData/textures/lq_dev/{char_u_fbr.png", "start": 55548568, "end": 55548714}, {"filename": "/GameData/textures/lq_dev/{char_v_fbr.png", "start": 55548714, "end": 55548882}, {"filename": "/GameData/textures/lq_dev/{char_w_fbr.png", "start": 55548882, "end": 55549031}, {"filename": "/GameData/textures/lq_dev/{char_x_fbr.png", "start": 55549031, "end": 55549213}, {"filename": "/GameData/textures/lq_dev/{char_y_fbr.png", "start": 55549213, "end": 55549382}, {"filename": "/GameData/textures/lq_dev/{char_z_fbr.png", "start": 55549382, "end": 55549557}, {"filename": "/GameData/textures/lq_dev/{charlow_a_fbr.png", "start": 55549557, "end": 55549712}, {"filename": "/GameData/textures/lq_dev/{charlow_b_fbr.png", "start": 55549712, "end": 55549871}, {"filename": "/GameData/textures/lq_dev/{charlow_c_fbr.png", "start": 55549871, "end": 55550025}, {"filename": "/GameData/textures/lq_dev/{charlow_d_fbr.png", "start": 55550025, "end": 55550186}, {"filename": "/GameData/textures/lq_dev/{charlow_e_fbr.png", "start": 55550186, "end": 55550340}, {"filename": "/GameData/textures/lq_dev/{charlow_f_fbr.png", "start": 55550340, "end": 55550500}, {"filename": "/GameData/textures/lq_dev/{charlow_g_fbr.png", "start": 55550500, "end": 55550658}, {"filename": "/GameData/textures/lq_dev/{charlow_h_fbr.png", "start": 55550658, "end": 55550812}, {"filename": "/GameData/textures/lq_dev/{charlow_i_fbr.png", "start": 55550812, "end": 55550958}, {"filename": "/GameData/textures/lq_dev/{charlow_j_fbr.png", "start": 55550958, "end": 55551118}, {"filename": "/GameData/textures/lq_dev/{charlow_k_fbr.png", "start": 55551118, "end": 55551297}, {"filename": "/GameData/textures/lq_dev/{charlow_l_fbr.png", "start": 55551297, "end": 55551435}, {"filename": "/GameData/textures/lq_dev/{charlow_m_fbr.png", "start": 55551435, "end": 55551588}, {"filename": "/GameData/textures/lq_dev/{charlow_n_fbr.png", "start": 55551588, "end": 55551738}, {"filename": "/GameData/textures/lq_dev/{charlow_o_fbr.png", "start": 55551738, "end": 55551892}, {"filename": "/GameData/textures/lq_dev/{charlow_p_fbr.png", "start": 55551892, "end": 55552047}, {"filename": "/GameData/textures/lq_dev/{charlow_q_fbr.png", "start": 55552047, "end": 55552197}, {"filename": "/GameData/textures/lq_dev/{charlow_r_fbr.png", "start": 55552197, "end": 55552350}, {"filename": "/GameData/textures/lq_dev/{charlow_s_fbr.png", "start": 55552350, "end": 55552502}, {"filename": "/GameData/textures/lq_dev/{charlow_t_fbr.png", "start": 55552502, "end": 55552668}, {"filename": "/GameData/textures/lq_dev/{charlow_u_fbr.png", "start": 55552668, "end": 55552819}, {"filename": "/GameData/textures/lq_dev/{charlow_v_fbr.png", "start": 55552819, "end": 55552993}, {"filename": "/GameData/textures/lq_dev/{charlow_w_fbr.png", "start": 55552993, "end": 55553147}, {"filename": "/GameData/textures/lq_dev/{charlow_x_fbr.png", "start": 55553147, "end": 55553334}, {"filename": "/GameData/textures/lq_dev/{charlow_y_fbr.png", "start": 55553334, "end": 55553508}, {"filename": "/GameData/textures/lq_dev/{charlow_z_fbr.png", "start": 55553508, "end": 55553679}, {"filename": "/GameData/textures/lq_dev/{chars_add_fbr.png", "start": 55553679, "end": 55553842}, {"filename": "/GameData/textures/lq_dev/{chars_and_fbr.png", "start": 55553842, "end": 55554034}, {"filename": "/GameData/textures/lq_dev/{chars_ardown_fbr.png", "start": 55554034, "end": 55554219}, {"filename": "/GameData/textures/lq_dev/{chars_arleft_fbr.png", "start": 55554219, "end": 55554403}, {"filename": "/GameData/textures/lq_dev/{chars_arright_fbr.png", "start": 55554403, "end": 55554588}, {"filename": "/GameData/textures/lq_dev/{chars_arup_fbr.png", "start": 55554588, "end": 55554766}, {"filename": "/GameData/textures/lq_dev/{chars_at_fbr.png", "start": 55554766, "end": 55554923}, {"filename": "/GameData/textures/lq_dev/{chars_brackc1_fbr.png", "start": 55554923, "end": 55555102}, {"filename": "/GameData/textures/lq_dev/{chars_brackc2_fbr.png", "start": 55555102, "end": 55555286}, {"filename": "/GameData/textures/lq_dev/{chars_brackr1_fbr.png", "start": 55555286, "end": 55555453}, {"filename": "/GameData/textures/lq_dev/{chars_brackr2_fbr.png", "start": 55555453, "end": 55555623}, {"filename": "/GameData/textures/lq_dev/{chars_bracks1_fbr.png", "start": 55555623, "end": 55555777}, {"filename": "/GameData/textures/lq_dev/{chars_bracks2_fbr.png", "start": 55555777, "end": 55555930}, {"filename": "/GameData/textures/lq_dev/{chars_caret_fbr.png", "start": 55555930, "end": 55556110}, {"filename": "/GameData/textures/lq_dev/{chars_colon_fbr.png", "start": 55556110, "end": 55556262}, {"filename": "/GameData/textures/lq_dev/{chars_colonsemi_fbr.png", "start": 55556262, "end": 55556429}, {"filename": "/GameData/textures/lq_dev/{chars_comma_fbr.png", "start": 55556429, "end": 55556585}, {"filename": "/GameData/textures/lq_dev/{chars_div_fbr.png", "start": 55556585, "end": 55556754}, {"filename": "/GameData/textures/lq_dev/{chars_dollar_fbr.png", "start": 55556754, "end": 55556930}, {"filename": "/GameData/textures/lq_dev/{chars_equ_fbr.png", "start": 55556930, "end": 55557089}, {"filename": "/GameData/textures/lq_dev/{chars_exclam_fbr.png", "start": 55557089, "end": 55557233}, {"filename": "/GameData/textures/lq_dev/{chars_grave_fbr.png", "start": 55557233, "end": 55557383}, {"filename": "/GameData/textures/lq_dev/{chars_hash_fbr.png", "start": 55557383, "end": 55557568}, {"filename": "/GameData/textures/lq_dev/{chars_heart_fbr.png", "start": 55557568, "end": 55557753}, {"filename": "/GameData/textures/lq_dev/{chars_multi_fbr.png", "start": 55557753, "end": 55557922}, {"filename": "/GameData/textures/lq_dev/{chars_percent_fbr.png", "start": 55557922, "end": 55558128}, {"filename": "/GameData/textures/lq_dev/{chars_perio_fbr.png", "start": 55558128, "end": 55558265}, {"filename": "/GameData/textures/lq_dev/{chars_pipe_fbr.png", "start": 55558265, "end": 55558412}, {"filename": "/GameData/textures/lq_dev/{chars_quest_fbr.png", "start": 55558412, "end": 55558585}, {"filename": "/GameData/textures/lq_dev/{chars_slaback_fbr.png", "start": 55558585, "end": 55558772}, {"filename": "/GameData/textures/lq_dev/{chars_slafoward_fbr.png", "start": 55558772, "end": 55558951}, {"filename": "/GameData/textures/lq_dev/{chars_smile_fbr.png", "start": 55558951, "end": 55559111}, {"filename": "/GameData/textures/lq_dev/{chars_sub_fbr.png", "start": 55559111, "end": 55559258}, {"filename": "/GameData/textures/lq_dev/{chars_sun_fbr.png", "start": 55559258, "end": 55559457}, {"filename": "/GameData/textures/lq_dev/{chars_thngreater_fbr.png", "start": 55559457, "end": 55559650}, {"filename": "/GameData/textures/lq_dev/{chars_thnless_fbr.png", "start": 55559650, "end": 55559837}, {"filename": "/GameData/textures/lq_dev/{chars_tilde_fbr.png", "start": 55559837, "end": 55560006}, {"filename": "/GameData/textures/lq_dev/{chars_unders_fbr.png", "start": 55560006, "end": 55560143}, {"filename": "/GameData/textures/lq_flesh/bone1_1.png", "start": 55560143, "end": 55567550}, {"filename": "/GameData/textures/lq_flesh/bone1_2.png", "start": 55567550, "end": 55576742}, {"filename": "/GameData/textures/lq_flesh/bone2_1.png", "start": 55576742, "end": 55587174}, {"filename": "/GameData/textures/lq_flesh/dopefish_fbr.png", "start": 55587174, "end": 55595404}, {"filename": "/GameData/textures/lq_flesh/flesh_gut1.png", "start": 55595404, "end": 55605719}, {"filename": "/GameData/textures/lq_flesh/flesh_gut2.png", "start": 55605719, "end": 55617519}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_1.png", "start": 55617519, "end": 55631269}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_2.png", "start": 55631269, "end": 55644249}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_3.png", "start": 55644249, "end": 55656282}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_4a.png", "start": 55656282, "end": 55669495}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_4b.png", "start": 55669495, "end": 55682635}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_5a.png", "start": 55682635, "end": 55696279}, {"filename": "/GameData/textures/lq_flesh/flesh_rot1_5b.png", "start": 55696279, "end": 55709852}, {"filename": "/GameData/textures/lq_flesh/flesh_rot2_1.png", "start": 55709852, "end": 55723078}, {"filename": "/GameData/textures/lq_flesh/flesh_rot3_1.png", "start": 55723078, "end": 55738024}, {"filename": "/GameData/textures/lq_flesh/flesh_rot3_2.png", "start": 55738024, "end": 55753063}, {"filename": "/GameData/textures/lq_flesh/flesh_rot4_1.png", "start": 55753063, "end": 55767700}, {"filename": "/GameData/textures/lq_flesh/flesh_rot5_1.png", "start": 55767700, "end": 55779968}, {"filename": "/GameData/textures/lq_flesh/flesh_rot6_1.png", "start": 55779968, "end": 55796375}, {"filename": "/GameData/textures/lq_flesh/flesh_wod1_1.png", "start": 55796375, "end": 55807893}, {"filename": "/GameData/textures/lq_flesh/flesh_wod1_2.png", "start": 55807893, "end": 55820955}, {"filename": "/GameData/textures/lq_flesh/flesh_wod1_3.png", "start": 55820955, "end": 55831958}, {"filename": "/GameData/textures/lq_flesh/flesh_wod1_4.png", "start": 55831958, "end": 55843370}, {"filename": "/GameData/textures/lq_flesh/flesh_wod1_5.png", "start": 55843370, "end": 55853969}, {"filename": "/GameData/textures/lq_flesh/flesh_wod2_1.png", "start": 55853969, "end": 55857625}, {"filename": "/GameData/textures/lq_flesh/flesh_wod2_2.png", "start": 55857625, "end": 55861722}, {"filename": "/GameData/textures/lq_flesh/flesh_wod2_3.png", "start": 55861722, "end": 55865098}, {"filename": "/GameData/textures/lq_flesh/flesh_wod2_4.png", "start": 55865098, "end": 55868696}, {"filename": "/GameData/textures/lq_flesh/flesh_wod2_5.png", "start": 55868696, "end": 55872086}, {"filename": "/GameData/textures/lq_flesh/fleshtile.png", "start": 55872086, "end": 55883055}, {"filename": "/GameData/textures/lq_flesh/marbred128.png", "start": 55883055, "end": 55895284}, {"filename": "/GameData/textures/lq_flesh/may_eat_eye2_fbr.png", "start": 55895284, "end": 55899157}, {"filename": "/GameData/textures/lq_flesh/may_eat_eye3_fbr.png", "start": 55899157, "end": 55903034}, {"filename": "/GameData/textures/lq_flesh/may_eat_eye_fbr.png", "start": 55903034, "end": 55906909}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_b.png", "start": 55906909, "end": 55919572}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_c.png", "start": 55919572, "end": 55932296}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_hol1.png", "start": 55932296, "end": 55936968}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_lit1_fbr.png", "start": 55936968, "end": 55941038}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_shut1.png", "start": 55941038, "end": 55956004}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_sp.png", "start": 55956004, "end": 55976146}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_sp2.png", "start": 55976146, "end": 56001769}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_te.png", "start": 56001769, "end": 56016875}, {"filename": "/GameData/textures/lq_flesh/may_flesh1_tet.png", "start": 56016875, "end": 56030095}, {"filename": "/GameData/textures/lq_flesh/may_flesh2_b.png", "start": 56030095, "end": 56043618}, {"filename": "/GameData/textures/lq_flesh/may_flesh3_1a.png", "start": 56043618, "end": 56057344}, {"filename": "/GameData/textures/lq_flesh/may_flesh3_1b.png", "start": 56057344, "end": 56093906}, {"filename": "/GameData/textures/lq_flesh/may_flesh3_b.png", "start": 56093906, "end": 56107792}, {"filename": "/GameData/textures/lq_flesh/may_flesh4_det.png", "start": 56107792, "end": 56118103}, {"filename": "/GameData/textures/lq_flesh/may_flesh4a_det.png", "start": 56118103, "end": 56132964}, {"filename": "/GameData/textures/lq_flesh/may_flesh5.png", "start": 56132964, "end": 56143300}, {"filename": "/GameData/textures/lq_flesh/may_flesh5_1a.png", "start": 56143300, "end": 56198035}, {"filename": "/GameData/textures/lq_flesh/may_flesh5_1b.png", "start": 56198035, "end": 56250753}, {"filename": "/GameData/textures/lq_flesh/may_flesh5_1c.png", "start": 56250753, "end": 56294395}, {"filename": "/GameData/textures/lq_flesh/may_flesh5_1lit_fbr.png", "start": 56294395, "end": 56311329}, {"filename": "/GameData/textures/lq_flesh/may_flesh_dr1a.png", "start": 56311329, "end": 56368222}, {"filename": "/GameData/textures/lq_flesh/may_skin_eye.png", "start": 56368222, "end": 56379439}, {"filename": "/GameData/textures/lq_flesh/meat-teeth0.png", "start": 56379439, "end": 56422810}, {"filename": "/GameData/textures/lq_flesh/meat-teeth1.png", "start": 56422810, "end": 56468002}, {"filename": "/GameData/textures/lq_flesh/meat_det1.png", "start": 56468002, "end": 56486733}, {"filename": "/GameData/textures/lq_flesh/meat_det2.png", "start": 56486733, "end": 56509304}, {"filename": "/GameData/textures/lq_flesh/meat_pipe1.png", "start": 56509304, "end": 56533728}, {"filename": "/GameData/textures/lq_flesh/plus_0eye.png", "start": 56533728, "end": 56537873}, {"filename": "/GameData/textures/lq_flesh/plus_0flesh2_gl.png", "start": 56537873, "end": 56553053}, {"filename": "/GameData/textures/lq_flesh/plus_0flesh_but1_fbr.png", "start": 56553053, "end": 56557348}, {"filename": "/GameData/textures/lq_flesh/plus_0flesh_but2_fbr.png", "start": 56557348, "end": 56561537}, {"filename": "/GameData/textures/lq_flesh/plus_0flsh_vent.png", "start": 56561537, "end": 56575788}, {"filename": "/GameData/textures/lq_flesh/plus_1eye.png", "start": 56575788, "end": 56579932}, {"filename": "/GameData/textures/lq_flesh/plus_1flesh2_gl.png", "start": 56579932, "end": 56595091}, {"filename": "/GameData/textures/lq_flesh/plus_1flesh_but2_fbr.png", "start": 56595091, "end": 56599286}, {"filename": "/GameData/textures/lq_flesh/plus_2eye.png", "start": 56599286, "end": 56603326}, {"filename": "/GameData/textures/lq_flesh/plus_2flesh2_gl.png", "start": 56603326, "end": 56618502}, {"filename": "/GameData/textures/lq_flesh/plus_2flesh_but2_fbr.png", "start": 56618502, "end": 56622685}, {"filename": "/GameData/textures/lq_flesh/plus_3eye.png", "start": 56622685, "end": 56626733}, {"filename": "/GameData/textures/lq_flesh/plus_3flesh2_gl.png", "start": 56626733, "end": 56641907}, {"filename": "/GameData/textures/lq_flesh/plus_3flesh_but2_fbr.png", "start": 56641907, "end": 56646078}, {"filename": "/GameData/textures/lq_flesh/plus_4eye.png", "start": 56646078, "end": 56650197}, {"filename": "/GameData/textures/lq_flesh/plus_4flesh2_gl.png", "start": 56650197, "end": 56665377}, {"filename": "/GameData/textures/lq_flesh/plus_5eye.png", "start": 56665377, "end": 56669430}, {"filename": "/GameData/textures/lq_flesh/plus_6eye.png", "start": 56669430, "end": 56673478}, {"filename": "/GameData/textures/lq_flesh/plus_7eye.png", "start": 56673478, "end": 56677549}, {"filename": "/GameData/textures/lq_flesh/plus_8eye.png", "start": 56677549, "end": 56681657}, {"filename": "/GameData/textures/lq_flesh/plus_9eye.png", "start": 56681657, "end": 56685805}, {"filename": "/GameData/textures/lq_flesh/plus_aeye.png", "start": 56685805, "end": 56689797}, {"filename": "/GameData/textures/lq_flesh/plus_aflesh_but1.png", "start": 56689797, "end": 56694363}, {"filename": "/GameData/textures/lq_flesh/plus_aflesh_but2.png", "start": 56694363, "end": 56698892}, {"filename": "/GameData/textures/lq_flesh/plus_aflsh_vent.png", "start": 56698892, "end": 56713292}, {"filename": "/GameData/textures/lq_greek/black.png", "start": 56713292, "end": 56713437}, {"filename": "/GameData/textures/lq_greek/grk_arch1.png", "start": 56713437, "end": 56718454}, {"filename": "/GameData/textures/lq_greek/grk_arch1_2.png", "start": 56718454, "end": 56724080}, {"filename": "/GameData/textures/lq_greek/grk_arch1_a.png", "start": 56724080, "end": 56736855}, {"filename": "/GameData/textures/lq_greek/grk_arch1_b.png", "start": 56736855, "end": 56750315}, {"filename": "/GameData/textures/lq_greek/grk_arch1_c.png", "start": 56750315, "end": 56763488}, {"filename": "/GameData/textures/lq_greek/grk_arch1_d.png", "start": 56763488, "end": 56777005}, {"filename": "/GameData/textures/lq_greek/grk_arch2.png", "start": 56777005, "end": 56780287}, {"filename": "/GameData/textures/lq_greek/grk_arch2_2.png", "start": 56780287, "end": 56783609}, {"filename": "/GameData/textures/lq_greek/grk_arch2_a.png", "start": 56783609, "end": 56794012}, {"filename": "/GameData/textures/lq_greek/grk_arch2_b.png", "start": 56794012, "end": 56804529}, {"filename": "/GameData/textures/lq_greek/grk_arch2_c.png", "start": 56804529, "end": 56814658}, {"filename": "/GameData/textures/lq_greek/grk_arch_end.png", "start": 56814658, "end": 56815414}, {"filename": "/GameData/textures/lq_greek/grk_arch_tcap.png", "start": 56815414, "end": 56816128}, {"filename": "/GameData/textures/lq_greek/grk_arch_trim.png", "start": 56816128, "end": 56817600}, {"filename": "/GameData/textures/lq_greek/grk_bl_arch1a.png", "start": 56817600, "end": 56825235}, {"filename": "/GameData/textures/lq_greek/grk_bl_arch1b.png", "start": 56825235, "end": 56833770}, {"filename": "/GameData/textures/lq_greek/grk_bl_brk1a.png", "start": 56833770, "end": 56836242}, {"filename": "/GameData/textures/lq_greek/grk_bl_brk1b.png", "start": 56836242, "end": 56838380}, {"filename": "/GameData/textures/lq_greek/grk_bl_brk2a.png", "start": 56838380, "end": 56840801}, {"filename": "/GameData/textures/lq_greek/grk_bl_brk2b.png", "start": 56840801, "end": 56843261}, {"filename": "/GameData/textures/lq_greek/grk_bl_flt1.png", "start": 56843261, "end": 56845485}, {"filename": "/GameData/textures/lq_greek/grk_bl_fsh1.png", "start": 56845485, "end": 56848411}, {"filename": "/GameData/textures/lq_greek/grk_bl_fsh2.png", "start": 56848411, "end": 56856503}, {"filename": "/GameData/textures/lq_greek/grk_bl_pil1.png", "start": 56856503, "end": 56859290}, {"filename": "/GameData/textures/lq_greek/grk_bl_trim1.png", "start": 56859290, "end": 56861702}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll1.png", "start": 56861702, "end": 56864422}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll2.png", "start": 56864422, "end": 56867148}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll3a.png", "start": 56867148, "end": 56869506}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll3b.png", "start": 56869506, "end": 56871974}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll4b.png", "start": 56871974, "end": 56874540}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll5a.png", "start": 56874540, "end": 56876757}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll5b.png", "start": 56876757, "end": 56879056}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll5c.png", "start": 56879056, "end": 56881379}, {"filename": "/GameData/textures/lq_greek/grk_bl_wll5d.png", "start": 56881379, "end": 56883786}, {"filename": "/GameData/textures/lq_greek/grk_brk15.png", "start": 56883786, "end": 56893292}, {"filename": "/GameData/textures/lq_greek/grk_brk15_b.png", "start": 56893292, "end": 56902540}, {"filename": "/GameData/textures/lq_greek/grk_brk15_c.png", "start": 56902540, "end": 56919408}, {"filename": "/GameData/textures/lq_greek/grk_brk15_f.png", "start": 56919408, "end": 56927701}, {"filename": "/GameData/textures/lq_greek/grk_brk15_g.png", "start": 56927701, "end": 56935350}, {"filename": "/GameData/textures/lq_greek/grk_brk16.png", "start": 56935350, "end": 56944626}, {"filename": "/GameData/textures/lq_greek/grk_brk16_a.png", "start": 56944626, "end": 56953905}, {"filename": "/GameData/textures/lq_greek/grk_brk16_f.png", "start": 56953905, "end": 56963610}, {"filename": "/GameData/textures/lq_greek/grk_brk17.png", "start": 56963610, "end": 56998754}, {"filename": "/GameData/textures/lq_greek/grk_brk17_f.png", "start": 56998754, "end": 57011356}, {"filename": "/GameData/textures/lq_greek/grk_but1.png", "start": 57011356, "end": 57012305}, {"filename": "/GameData/textures/lq_greek/grk_det1.png", "start": 57012305, "end": 57015378}, {"filename": "/GameData/textures/lq_greek/grk_door1.png", "start": 57015378, "end": 57030485}, {"filename": "/GameData/textures/lq_greek/grk_door1_f.png", "start": 57030485, "end": 57040520}, {"filename": "/GameData/textures/lq_greek/grk_door2.png", "start": 57040520, "end": 57051380}, {"filename": "/GameData/textures/lq_greek/grk_door3.png", "start": 57051380, "end": 57064037}, {"filename": "/GameData/textures/lq_greek/grk_ebrick10.png", "start": 57064037, "end": 57074664}, {"filename": "/GameData/textures/lq_greek/grk_ebrick10_bl.png", "start": 57074664, "end": 57089198}, {"filename": "/GameData/textures/lq_greek/grk_ebrick22.png", "start": 57089198, "end": 57103064}, {"filename": "/GameData/textures/lq_greek/grk_ebrick23.png", "start": 57103064, "end": 57112430}, {"filename": "/GameData/textures/lq_greek/grk_ebrick24.png", "start": 57112430, "end": 57120623}, {"filename": "/GameData/textures/lq_greek/grk_flr1.png", "start": 57120623, "end": 57120954}, {"filename": "/GameData/textures/lq_greek/grk_flr2.png", "start": 57120954, "end": 57121808}, {"filename": "/GameData/textures/lq_greek/grk_flr3.png", "start": 57121808, "end": 57124404}, {"filename": "/GameData/textures/lq_greek/grk_flr4_1.png", "start": 57124404, "end": 57126534}, {"filename": "/GameData/textures/lq_greek/grk_flr4_2.png", "start": 57126534, "end": 57128796}, {"filename": "/GameData/textures/lq_greek/grk_flr4_3.png", "start": 57128796, "end": 57131159}, {"filename": "/GameData/textures/lq_greek/grk_flr4_4.png", "start": 57131159, "end": 57133454}, {"filename": "/GameData/textures/lq_greek/grk_flr4_5.png", "start": 57133454, "end": 57137086}, {"filename": "/GameData/textures/lq_greek/grk_flr4_6.png", "start": 57137086, "end": 57140092}, {"filename": "/GameData/textures/lq_greek/grk_flr4_8.png", "start": 57140092, "end": 57143427}, {"filename": "/GameData/textures/lq_greek/grk_flr5_1.png", "start": 57143427, "end": 57146014}, {"filename": "/GameData/textures/lq_greek/grk_flr5_2.png", "start": 57146014, "end": 57148508}, {"filename": "/GameData/textures/lq_greek/grk_flr5_3.png", "start": 57148508, "end": 57151250}, {"filename": "/GameData/textures/lq_greek/grk_flr5_4.png", "start": 57151250, "end": 57153553}, {"filename": "/GameData/textures/lq_greek/grk_key_gl2.png", "start": 57153553, "end": 57154577}, {"filename": "/GameData/textures/lq_greek/grk_key_sl2.png", "start": 57154577, "end": 57155578}, {"filename": "/GameData/textures/lq_greek/grk_lion1.png", "start": 57155578, "end": 57166052}, {"filename": "/GameData/textures/lq_greek/grk_lion2.png", "start": 57166052, "end": 57175719}, {"filename": "/GameData/textures/lq_greek/grk_lion3.png", "start": 57175719, "end": 57186262}, {"filename": "/GameData/textures/lq_greek/grk_lion4.png", "start": 57186262, "end": 57197414}, {"filename": "/GameData/textures/lq_greek/grk_met1.png", "start": 57197414, "end": 57207795}, {"filename": "/GameData/textures/lq_greek/grk_met1_trim.png", "start": 57207795, "end": 57219070}, {"filename": "/GameData/textures/lq_greek/grk_met1a_trim.png", "start": 57219070, "end": 57230038}, {"filename": "/GameData/textures/lq_greek/grk_met1b_trim.png", "start": 57230038, "end": 57244151}, {"filename": "/GameData/textures/lq_greek/grk_met2_trim.png", "start": 57244151, "end": 57257420}, {"filename": "/GameData/textures/lq_greek/grk_met_plt.png", "start": 57257420, "end": 57272811}, {"filename": "/GameData/textures/lq_greek/grk_mural1.png", "start": 57272811, "end": 57292669}, {"filename": "/GameData/textures/lq_greek/grk_mural2.png", "start": 57292669, "end": 57315386}, {"filename": "/GameData/textures/lq_greek/grk_mural3.png", "start": 57315386, "end": 57404666}, {"filename": "/GameData/textures/lq_greek/grk_pl1_a.png", "start": 57404666, "end": 57406482}, {"filename": "/GameData/textures/lq_greek/grk_pl1_b.png", "start": 57406482, "end": 57408565}, {"filename": "/GameData/textures/lq_greek/grk_pl2_a.png", "start": 57408565, "end": 57410797}, {"filename": "/GameData/textures/lq_greek/grk_pl2_b.png", "start": 57410797, "end": 57413009}, {"filename": "/GameData/textures/lq_greek/grk_plat1_side.png", "start": 57413009, "end": 57414226}, {"filename": "/GameData/textures/lq_greek/grk_plat1_top.png", "start": 57414226, "end": 57418123}, {"filename": "/GameData/textures/lq_greek/grk_tile2_1.png", "start": 57418123, "end": 57420557}, {"filename": "/GameData/textures/lq_greek/grk_tile2_2.png", "start": 57420557, "end": 57422910}, {"filename": "/GameData/textures/lq_greek/grk_trim1.png", "start": 57422910, "end": 57429667}, {"filename": "/GameData/textures/lq_greek/grk_trim1_3.png", "start": 57429667, "end": 57436281}, {"filename": "/GameData/textures/lq_greek/grk_trim1_3_s.png", "start": 57436281, "end": 57438321}, {"filename": "/GameData/textures/lq_greek/grk_trim1_4_s.png", "start": 57438321, "end": 57440397}, {"filename": "/GameData/textures/lq_greek/grk_trim1_5.png", "start": 57440397, "end": 57447333}, {"filename": "/GameData/textures/lq_greek/grk_trim1_5_s.png", "start": 57447333, "end": 57449553}, {"filename": "/GameData/textures/lq_greek/grk_trim1_6_s.png", "start": 57449553, "end": 57451830}, {"filename": "/GameData/textures/lq_greek/grk_trim1_7_s.png", "start": 57451830, "end": 57455172}, {"filename": "/GameData/textures/lq_greek/grk_trim2.png", "start": 57455172, "end": 57457533}, {"filename": "/GameData/textures/lq_greek/grk_wall1.png", "start": 57457533, "end": 57460565}, {"filename": "/GameData/textures/lq_greek/grk_wall2.png", "start": 57460565, "end": 57463236}, {"filename": "/GameData/textures/lq_greek/grk_wall3.png", "start": 57463236, "end": 57469022}, {"filename": "/GameData/textures/lq_greek/grk_wall3b.png", "start": 57469022, "end": 57496056}, {"filename": "/GameData/textures/lq_greek/grk_win1_a.png", "start": 57496056, "end": 57505195}, {"filename": "/GameData/textures/lq_greek/grk_win1_b.png", "start": 57505195, "end": 57514538}, {"filename": "/GameData/textures/lq_greek/plus_0grk_but1_fbr.png", "start": 57514538, "end": 57515729}, {"filename": "/GameData/textures/lq_greek/plus_0grk_hbut_fbr.png", "start": 57515729, "end": 57516752}, {"filename": "/GameData/textures/lq_greek/plus_1grk_but1_fbr.png", "start": 57516752, "end": 57517927}, {"filename": "/GameData/textures/lq_greek/plus_1grk_hbut_fbr.png", "start": 57517927, "end": 57519008}, {"filename": "/GameData/textures/lq_greek/plus_2grk_but1_fbr.png", "start": 57519008, "end": 57520159}, {"filename": "/GameData/textures/lq_greek/plus_2grk_hbut_fbr.png", "start": 57520159, "end": 57521199}, {"filename": "/GameData/textures/lq_greek/plus_3grk_but1_fbr.png", "start": 57521199, "end": 57522352}, {"filename": "/GameData/textures/lq_greek/plus_3grk_hbut_fbr.png", "start": 57522352, "end": 57523394}, {"filename": "/GameData/textures/lq_greek/plus_agrk_but1.png", "start": 57523394, "end": 57524385}, {"filename": "/GameData/textures/lq_greek/plus_agrk_hbut.png", "start": 57524385, "end": 57525363}, {"filename": "/GameData/textures/lq_health_ammo/ammo_bottom.png", "start": 57525363, "end": 57526293}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_b1_fbr.png", "start": 57526293, "end": 57526901}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_b2_fbr.png", "start": 57526901, "end": 57527513}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_b3_fbr.png", "start": 57527513, "end": 57528320}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_s1_fbr.png", "start": 57528320, "end": 57528913}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_s2_fbr.png", "start": 57528913, "end": 57529903}, {"filename": "/GameData/textures/lq_health_ammo/ammo_c_s3_fbr.png", "start": 57529903, "end": 57530536}, {"filename": "/GameData/textures/lq_health_ammo/ammo_fl.png", "start": 57530536, "end": 57531095}, {"filename": "/GameData/textures/lq_health_ammo/ammo_fl2.png", "start": 57531095, "end": 57531650}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_b1_fbr.png", "start": 57531650, "end": 57532404}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_b2_fbr.png", "start": 57532404, "end": 57533089}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_b3.png", "start": 57533089, "end": 57533660}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_s1.png", "start": 57533660, "end": 57534231}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_s2_fbr.png", "start": 57534231, "end": 57534886}, {"filename": "/GameData/textures/lq_health_ammo/ammo_n_s3_fbr.png", "start": 57534886, "end": 57535423}, {"filename": "/GameData/textures/lq_health_ammo/ammo_r_b1_fbr.png", "start": 57535423, "end": 57536138}, {"filename": "/GameData/textures/lq_health_ammo/ammo_r_b2_fbr.png", "start": 57536138, "end": 57536764}, {"filename": "/GameData/textures/lq_health_ammo/ammo_r_b3_fbr.png", "start": 57536764, "end": 57537222}, {"filename": "/GameData/textures/lq_health_ammo/ammo_r_s1_fbr.png", "start": 57537222, "end": 57537836}, {"filename": "/GameData/textures/lq_health_ammo/ammo_r_s2_fbr.png", "start": 57537836, "end": 57538185}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_b1_fbr.png", "start": 57538185, "end": 57538844}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_b2_fbr.png", "start": 57538844, "end": 57539472}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_b3.png", "start": 57539472, "end": 57540249}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_s1_fbr.png", "start": 57540249, "end": 57540755}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_s2_fbr.png", "start": 57540755, "end": 57541333}, {"filename": "/GameData/textures/lq_health_ammo/ammo_s_s3_fbr.png", "start": 57541333, "end": 57541884}, {"filename": "/GameData/textures/lq_health_ammo/ammobotsmall.png", "start": 57541884, "end": 57542552}, {"filename": "/GameData/textures/lq_health_ammo/ammotop.png", "start": 57542552, "end": 57543297}, {"filename": "/GameData/textures/lq_health_ammo/ammotopsmall.png", "start": 57543297, "end": 57543848}, {"filename": "/GameData/textures/lq_health_ammo/boom.png", "start": 57543848, "end": 57544866}, {"filename": "/GameData/textures/lq_health_ammo/boomammo_bottom.png", "start": 57544866, "end": 57545232}, {"filename": "/GameData/textures/lq_health_ammo/boomammotop.png", "start": 57545232, "end": 57545574}, {"filename": "/GameData/textures/lq_health_ammo/boomsmall.png", "start": 57545574, "end": 57546249}, {"filename": "/GameData/textures/lq_health_ammo/epboxlarge_fbr.png", "start": 57546249, "end": 57547433}, {"filename": "/GameData/textures/lq_health_ammo/epboxsmall_fbr.png", "start": 57547433, "end": 57548330}, {"filename": "/GameData/textures/lq_health_ammo/explob_s2.png", "start": 57548330, "end": 57548826}, {"filename": "/GameData/textures/lq_health_ammo/hp15_side.png", "start": 57548826, "end": 57549411}, {"filename": "/GameData/textures/lq_health_ammo/hp25_top2.png", "start": 57549411, "end": 57549840}, {"filename": "/GameData/textures/lq_health_ammo/hp_bottom.png", "start": 57549840, "end": 57550747}, {"filename": "/GameData/textures/lq_health_ammo/hp_details.png", "start": 57550747, "end": 57551251}, {"filename": "/GameData/textures/lq_health_ammo/nails.png", "start": 57551251, "end": 57552298}, {"filename": "/GameData/textures/lq_health_ammo/nailssmall.png", "start": 57552298, "end": 57553039}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp100-winq_fbr.png", "start": 57553039, "end": 57555639}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp100_side_fbr.png", "start": 57555639, "end": 57556531}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp100_top_fbr.png", "start": 57556531, "end": 57557262}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp15_top_fbr.png", "start": 57557262, "end": 57557992}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp15winq_fbr.png", "start": 57557992, "end": 57560779}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp25-winq_fbr.png", "start": 57560779, "end": 57563333}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp25_side_fbr.png", "start": 57563333, "end": 57564155}, {"filename": "/GameData/textures/lq_health_ammo/plus_0_hp25_top_fbr.png", "start": 57564155, "end": 57564889}, {"filename": "/GameData/textures/lq_health_ammo/plus_0explob2_s1_fbr.png", "start": 57564889, "end": 57565992}, {"filename": "/GameData/textures/lq_health_ammo/plus_0explob_s1_fbr.png", "start": 57565992, "end": 57566625}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp100-winq_fbr.png", "start": 57566625, "end": 57569230}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp100_side_fbr.png", "start": 57569230, "end": 57570124}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp100_top_fbr.png", "start": 57570124, "end": 57570858}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp15_top_fbr.png", "start": 57570858, "end": 57571592}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp15winq_fbr.png", "start": 57571592, "end": 57574378}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp25-winq_fbr.png", "start": 57574378, "end": 57576942}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp25_side_fbr.png", "start": 57576942, "end": 57577768}, {"filename": "/GameData/textures/lq_health_ammo/plus_1_hp25_top_fbr.png", "start": 57577768, "end": 57578507}, {"filename": "/GameData/textures/lq_health_ammo/plus_1explob2_s1_fbr.png", "start": 57578507, "end": 57579614}, {"filename": "/GameData/textures/lq_health_ammo/plus_1explob_s1_fbr.png", "start": 57579614, "end": 57580247}, {"filename": "/GameData/textures/lq_health_ammo/plus_2_hp100-winq_fbr.png", "start": 57580247, "end": 57582851}, {"filename": "/GameData/textures/lq_health_ammo/plus_2_hp100_side_fbr.png", "start": 57582851, "end": 57583745}, {"filename": "/GameData/textures/lq_health_ammo/plus_2_hp25-winq_fbr.png", "start": 57583745, "end": 57586303}, {"filename": "/GameData/textures/lq_health_ammo/plus_2_hp25_side_fbr.png", "start": 57586303, "end": 57587128}, {"filename": "/GameData/textures/lq_health_ammo/plus_2_hp25_top_fbr.png", "start": 57587128, "end": 57587866}, {"filename": "/GameData/textures/lq_health_ammo/plus_2explob2_s1_fbr.png", "start": 57587866, "end": 57588962}, {"filename": "/GameData/textures/lq_health_ammo/plus_2explob_s1_fbr.png", "start": 57588962, "end": 57589604}, {"filename": "/GameData/textures/lq_health_ammo/plus_3_hp100-winq_fbr.png", "start": 57589604, "end": 57592214}, {"filename": "/GameData/textures/lq_health_ammo/plus_3_hp100_side_fbr.png", "start": 57592214, "end": 57593111}, {"filename": "/GameData/textures/lq_health_ammo/plus_3_hp25-winq_fbr.png", "start": 57593111, "end": 57595680}, {"filename": "/GameData/textures/lq_health_ammo/plus_3_hp25_side_fbr.png", "start": 57595680, "end": 57596511}, {"filename": "/GameData/textures/lq_health_ammo/plus_3_hp25_top_fbr.png", "start": 57596511, "end": 57597253}, {"filename": "/GameData/textures/lq_health_ammo/plus_3explob2_s1_fbr.png", "start": 57597253, "end": 57598349}, {"filename": "/GameData/textures/lq_health_ammo/plus_3explob_s1_fbr.png", "start": 57598349, "end": 57598991}, {"filename": "/GameData/textures/lq_health_ammo/shells.png", "start": 57598991, "end": 57599937}, {"filename": "/GameData/textures/lq_health_ammo/shellssmall.png", "start": 57599937, "end": 57600663}, {"filename": "/GameData/textures/lq_health_ammo/zap.png", "start": 57600663, "end": 57601656}, {"filename": "/GameData/textures/lq_health_ammo/zapsmall.png", "start": 57601656, "end": 57602420}, {"filename": "/GameData/textures/lq_legacy/brick7.png", "start": 57602420, "end": 57605353}, {"filename": "/GameData/textures/lq_legacy/brick8.png", "start": 57605353, "end": 57608440}, {"filename": "/GameData/textures/lq_legacy/button_0.png", "start": 57608440, "end": 57611912}, {"filename": "/GameData/textures/lq_legacy/button_0_grey.png", "start": 57611912, "end": 57615122}, {"filename": "/GameData/textures/lq_legacy/button_0_grn.png", "start": 57615122, "end": 57618457}, {"filename": "/GameData/textures/lq_legacy/button_1.png", "start": 57618457, "end": 57621912}, {"filename": "/GameData/textures/lq_legacy/button_1_grey.png", "start": 57621912, "end": 57625107}, {"filename": "/GameData/textures/lq_legacy/button_1_grn.png", "start": 57625107, "end": 57628408}, {"filename": "/GameData/textures/lq_legacy/floor_temp.png", "start": 57628408, "end": 57630466}, {"filename": "/GameData/textures/lq_legacy/flr.png", "start": 57630466, "end": 57633223}, {"filename": "/GameData/textures/lq_legacy/gardgrass_1.png", "start": 57633223, "end": 57653393}, {"filename": "/GameData/textures/lq_legacy/go-savgx.png", "start": 57653393, "end": 57655142}, {"filename": "/GameData/textures/lq_legacy/grass.png", "start": 57655142, "end": 57658543}, {"filename": "/GameData/textures/lq_legacy/grk_brk15_c_old.png", "start": 57658543, "end": 57669070}, {"filename": "/GameData/textures/lq_legacy/grk_brk17_f_old.png", "start": 57669070, "end": 57677779}, {"filename": "/GameData/textures/lq_legacy/grk_door1_old.png", "start": 57677779, "end": 57688796}, {"filename": "/GameData/textures/lq_legacy/grk_door2_old.png", "start": 57688796, "end": 57697969}, {"filename": "/GameData/textures/lq_legacy/grk_door3_old.png", "start": 57697969, "end": 57706775}, {"filename": "/GameData/textures/lq_legacy/grk_ebrick22_old.png", "start": 57706775, "end": 57716092}, {"filename": "/GameData/textures/lq_legacy/grk_trim1_7_s_old.png", "start": 57716092, "end": 57718297}, {"filename": "/GameData/textures/lq_legacy/ground_1.png", "start": 57718297, "end": 57720478}, {"filename": "/GameData/textures/lq_legacy/leaves.png", "start": 57720478, "end": 57723540}, {"filename": "/GameData/textures/lq_legacy/marble1_4.png", "start": 57723540, "end": 57726443}, {"filename": "/GameData/textures/lq_legacy/marble1_5.png", "start": 57726443, "end": 57729533}, {"filename": "/GameData/textures/lq_legacy/med_cflat1_3.png", "start": 57729533, "end": 57732014}, {"filename": "/GameData/textures/lq_legacy/med_csl_trm3.png", "start": 57732014, "end": 57733339}, {"filename": "/GameData/textures/lq_legacy/med_csl_trm3b.png", "start": 57733339, "end": 57734329}, {"filename": "/GameData/textures/lq_legacy/med_csl_trm3c.png", "start": 57734329, "end": 57735857}, {"filename": "/GameData/textures/lq_legacy/med_flat10.png", "start": 57735857, "end": 57739750}, {"filename": "/GameData/textures/lq_legacy/med_flat11.png", "start": 57739750, "end": 57742955}, {"filename": "/GameData/textures/lq_legacy/med_flat13.png", "start": 57742955, "end": 57753732}, {"filename": "/GameData/textures/lq_legacy/med_flat14.png", "start": 57753732, "end": 57764133}, {"filename": "/GameData/textures/lq_legacy/med_flat9a.png", "start": 57764133, "end": 57807713}, {"filename": "/GameData/textures/lq_legacy/med_flat9b.png", "start": 57807713, "end": 57849612}, {"filename": "/GameData/textures/lq_legacy/med_plaster1.png", "start": 57849612, "end": 57857654}, {"filename": "/GameData/textures/lq_legacy/med_ret_rock1.png", "start": 57857654, "end": 57861473}, {"filename": "/GameData/textures/lq_legacy/med_ret_wood1_old.png", "start": 57861473, "end": 57868554}, {"filename": "/GameData/textures/lq_legacy/med_rmet_key.png", "start": 57868554, "end": 57872515}, {"filename": "/GameData/textures/lq_legacy/med_rock6.png", "start": 57872515, "end": 57918118}, {"filename": "/GameData/textures/lq_legacy/med_rock7.png", "start": 57918118, "end": 57921002}, {"filename": "/GameData/textures/lq_legacy/med_rock8.png", "start": 57921002, "end": 57922887}, {"filename": "/GameData/textures/lq_legacy/med_rough_block.png", "start": 57922887, "end": 57971730}, {"filename": "/GameData/textures/lq_legacy/med_rough_block_f.png", "start": 57971730, "end": 58026275}, {"filename": "/GameData/textures/lq_legacy/med_tile.png", "start": 58026275, "end": 58028411}, {"filename": "/GameData/textures/lq_legacy/med_wall1.png", "start": 58028411, "end": 58038276}, {"filename": "/GameData/textures/lq_legacy/metground_1.png", "start": 58038276, "end": 58041207}, {"filename": "/GameData/textures/lq_legacy/note-savgx.png", "start": 58041207, "end": 58060227}, {"filename": "/GameData/textures/lq_legacy/plus_0button1.png", "start": 58060227, "end": 58063672}, {"filename": "/GameData/textures/lq_legacy/plus_0button2_fbr.png", "start": 58063672, "end": 58065159}, {"filename": "/GameData/textures/lq_legacy/plus_1button2_fbr.png", "start": 58065159, "end": 58066647}, {"filename": "/GameData/textures/lq_legacy/plus_1button3.png", "start": 58066647, "end": 58068527}, {"filename": "/GameData/textures/lq_legacy/plus_abutton1_fbr.png", "start": 58068527, "end": 58072129}, {"filename": "/GameData/textures/lq_legacy/plus_abutton2_fbr.png", "start": 58072129, "end": 58073627}, {"filename": "/GameData/textures/lq_legacy/readme.txt", "start": 58073627, "end": 58073703}, {"filename": "/GameData/textures/lq_legacy/riktoiflat.png", "start": 58073703, "end": 58087822}, {"filename": "/GameData/textures/lq_legacy/riktoiflat_blu.png", "start": 58087822, "end": 58100757}, {"filename": "/GameData/textures/lq_legacy/riktoiflat_grn.png", "start": 58100757, "end": 58112411}, {"filename": "/GameData/textures/lq_legacy/riktoilava.png", "start": 58112411, "end": 58116915}, {"filename": "/GameData/textures/lq_legacy/riktoislime.png", "start": 58116915, "end": 58120620}, {"filename": "/GameData/textures/lq_legacy/riktoitrim.png", "start": 58120620, "end": 58122107}, {"filename": "/GameData/textures/lq_legacy/riktoitrim__purp.png", "start": 58122107, "end": 58123823}, {"filename": "/GameData/textures/lq_legacy/riktoitrim_blu.png", "start": 58123823, "end": 58125553}, {"filename": "/GameData/textures/lq_legacy/riktoiwall.png", "start": 58125553, "end": 58165350}, {"filename": "/GameData/textures/lq_legacy/riktoiwall__purp.png", "start": 58165350, "end": 58201423}, {"filename": "/GameData/textures/lq_legacy/riktoiwall_blu.png", "start": 58201423, "end": 58237484}, {"filename": "/GameData/textures/lq_legacy/riktoiwater.png", "start": 58237484, "end": 58240375}, {"filename": "/GameData/textures/lq_legacy/rune1_fbr.png", "start": 58240375, "end": 58243388}, {"filename": "/GameData/textures/lq_legacy/rune2_fbr.png", "start": 58243388, "end": 58246454}, {"filename": "/GameData/textures/lq_legacy/rune3_fbr.png", "start": 58246454, "end": 58249850}, {"filename": "/GameData/textures/lq_legacy/rune4_fbr.png", "start": 58249850, "end": 58252402}, {"filename": "/GameData/textures/lq_legacy/sign_easy.png", "start": 58252402, "end": 58257354}, {"filename": "/GameData/textures/lq_legacy/sign_empty.png", "start": 58257354, "end": 58262111}, {"filename": "/GameData/textures/lq_legacy/sign_hard.png", "start": 58262111, "end": 58266984}, {"filename": "/GameData/textures/lq_legacy/sign_medium.png", "start": 58266984, "end": 58271994}, {"filename": "/GameData/textures/lq_legacy/sign_metal_1.png", "start": 58271994, "end": 58273206}, {"filename": "/GameData/textures/lq_legacy/sign_metal_2.png", "start": 58273206, "end": 58274801}, {"filename": "/GameData/textures/lq_legacy/sign_nmare.png", "start": 58274801, "end": 58279822}, {"filename": "/GameData/textures/lq_legacy/sky2.png", "start": 58279822, "end": 58295312}, {"filename": "/GameData/textures/lq_legacy/sky4.png", "start": 58295312, "end": 58304824}, {"filename": "/GameData/textures/lq_legacy/sky5_fbr.png", "start": 58304824, "end": 58321977}, {"filename": "/GameData/textures/lq_legacy/sky5a.png", "start": 58321977, "end": 58330988}, {"filename": "/GameData/textures/lq_legacy/sky7.png", "start": 58330988, "end": 58344903}, {"filename": "/GameData/textures/lq_legacy/sky8.png", "start": 58344903, "end": 58361576}, {"filename": "/GameData/textures/lq_legacy/sky8a_fbr.png", "start": 58361576, "end": 58370372}, {"filename": "/GameData/textures/lq_legacy/tile.png", "start": 58370372, "end": 58379188}, {"filename": "/GameData/textures/lq_legacy/tile_blu.png", "start": 58379188, "end": 58388194}, {"filename": "/GameData/textures/lq_legacy/tile_grn.png", "start": 58388194, "end": 58396457}, {"filename": "/GameData/textures/lq_legacy/trim2_blu.png", "start": 58396457, "end": 58397570}, {"filename": "/GameData/textures/lq_legacy/trim2_grn.png", "start": 58397570, "end": 58398634}, {"filename": "/GameData/textures/lq_legacy/ultrasteel1.png", "start": 58398634, "end": 58411524}, {"filename": "/GameData/textures/lq_legacy/ultrasteel2.png", "start": 58411524, "end": 58423318}, {"filename": "/GameData/textures/lq_legacy/vines1_old.png", "start": 58423318, "end": 58428172}, {"filename": "/GameData/textures/lq_legacy/wiz1_4.png", "start": 58428172, "end": 58431993}, {"filename": "/GameData/textures/lq_liquidsky/+0water_f3.png", "start": 58431993, "end": 58434934}, {"filename": "/GameData/textures/lq_liquidsky/+1water_f3.png", "start": 58434934, "end": 58437858}, {"filename": "/GameData/textures/lq_liquidsky/+2water_f3.png", "start": 58437858, "end": 58440776}, {"filename": "/GameData/textures/lq_liquidsky/+3water_f3.png", "start": 58440776, "end": 58443628}, {"filename": "/GameData/textures/lq_liquidsky/plus_0blood_f1.png", "start": 58443628, "end": 58445417}, {"filename": "/GameData/textures/lq_liquidsky/plus_0fslime.png", "start": 58445417, "end": 58459373}, {"filename": "/GameData/textures/lq_liquidsky/plus_0lava_fall3_fbr.png", "start": 58459373, "end": 58468366}, {"filename": "/GameData/textures/lq_liquidsky/plus_0water_f1.png", "start": 58468366, "end": 58470215}, {"filename": "/GameData/textures/lq_liquidsky/plus_0water_f2.png", "start": 58470215, "end": 58472033}, {"filename": "/GameData/textures/lq_liquidsky/plus_0wfall0.png", "start": 58472033, "end": 58481642}, {"filename": "/GameData/textures/lq_liquidsky/plus_1blood_f1.png", "start": 58481642, "end": 58483447}, {"filename": "/GameData/textures/lq_liquidsky/plus_1fslime.png", "start": 58483447, "end": 58497190}, {"filename": "/GameData/textures/lq_liquidsky/plus_1lava_fall3_fbr.png", "start": 58497190, "end": 58506757}, {"filename": "/GameData/textures/lq_liquidsky/plus_1water_f1.png", "start": 58506757, "end": 58508405}, {"filename": "/GameData/textures/lq_liquidsky/plus_1water_f2.png", "start": 58508405, "end": 58510234}, {"filename": "/GameData/textures/lq_liquidsky/plus_1wfall0.png", "start": 58510234, "end": 58519885}, {"filename": "/GameData/textures/lq_liquidsky/plus_2blood_f1.png", "start": 58519885, "end": 58521743}, {"filename": "/GameData/textures/lq_liquidsky/plus_2fslime.png", "start": 58521743, "end": 58535637}, {"filename": "/GameData/textures/lq_liquidsky/plus_2lava_fall3_fbr.png", "start": 58535637, "end": 58545186}, {"filename": "/GameData/textures/lq_liquidsky/plus_2water_f1.png", "start": 58545186, "end": 58546957}, {"filename": "/GameData/textures/lq_liquidsky/plus_2water_f2.png", "start": 58546957, "end": 58548843}, {"filename": "/GameData/textures/lq_liquidsky/plus_2wfall0.png", "start": 58548843, "end": 58558505}, {"filename": "/GameData/textures/lq_liquidsky/plus_3blood_f1.png", "start": 58558505, "end": 58560305}, {"filename": "/GameData/textures/lq_liquidsky/plus_3fslime.png", "start": 58560305, "end": 58573988}, {"filename": "/GameData/textures/lq_liquidsky/plus_3lava_fall3_fbr.png", "start": 58573988, "end": 58583325}, {"filename": "/GameData/textures/lq_liquidsky/plus_3water_f1.png", "start": 58583325, "end": 58584976}, {"filename": "/GameData/textures/lq_liquidsky/plus_3water_f2.png", "start": 58584976, "end": 58586803}, {"filename": "/GameData/textures/lq_liquidsky/plus_3wfall0.png", "start": 58586803, "end": 58596506}, {"filename": "/GameData/textures/lq_liquidsky/plus_4fslime.png", "start": 58596506, "end": 58610170}, {"filename": "/GameData/textures/lq_liquidsky/plus_4lava_fall3_fbr.png", "start": 58610170, "end": 58619543}, {"filename": "/GameData/textures/lq_liquidsky/plus_4wfall0.png", "start": 58619543, "end": 58629122}, {"filename": "/GameData/textures/lq_liquidsky/plus_5fslime.png", "start": 58629122, "end": 58642890}, {"filename": "/GameData/textures/lq_liquidsky/plus_5lava_fall3_fbr.png", "start": 58642890, "end": 58652253}, {"filename": "/GameData/textures/lq_liquidsky/plus_5wfall0.png", "start": 58652253, "end": 58661852}, {"filename": "/GameData/textures/lq_liquidsky/plus_6fslime.png", "start": 58661852, "end": 58675704}, {"filename": "/GameData/textures/lq_liquidsky/plus_6lava_fall3_fbr.png", "start": 58675704, "end": 58685089}, {"filename": "/GameData/textures/lq_liquidsky/plus_6wfall0.png", "start": 58685089, "end": 58694674}, {"filename": "/GameData/textures/lq_liquidsky/plus_7fslime.png", "start": 58694674, "end": 58708525}, {"filename": "/GameData/textures/lq_liquidsky/plus_7lava_fall3_fbr.png", "start": 58708525, "end": 58718040}, {"filename": "/GameData/textures/lq_liquidsky/plus_7wfall0.png", "start": 58718040, "end": 58727709}, {"filename": "/GameData/textures/lq_liquidsky/plus_8wfall0.png", "start": 58727709, "end": 58737371}, {"filename": "/GameData/textures/lq_liquidsky/plus_9wfall0.png", "start": 58737371, "end": 58746993}, {"filename": "/GameData/textures/lq_liquidsky/sky-test.png", "start": 58746993, "end": 58764589}, {"filename": "/GameData/textures/lq_liquidsky/sky-test.xcf", "start": 58764589, "end": 58897854}, {"filename": "/GameData/textures/lq_liquidsky/sky5_blu.png", "start": 58897854, "end": 58911167}, {"filename": "/GameData/textures/lq_liquidsky/sky5_dismal.png", "start": 58911167, "end": 58924350}, {"filename": "/GameData/textures/lq_liquidsky/sky_galx_fbr.png", "start": 58924350, "end": 58947105}, {"filename": "/GameData/textures/lq_liquidsky/sky_galx_spark_fbr.png", "start": 58947105, "end": 58966663}, {"filename": "/GameData/textures/lq_liquidsky/sky_orng.png", "start": 58966663, "end": 58981302}, {"filename": "/GameData/textures/lq_liquidsky/sky_pando.png", "start": 58981302, "end": 58997410}, {"filename": "/GameData/textures/lq_liquidsky/sky_pando2.png", "start": 58997410, "end": 59013945}, {"filename": "/GameData/textures/lq_liquidsky/sky_star.png", "start": 59013945, "end": 59015605}, {"filename": "/GameData/textures/lq_liquidsky/sky_void.png", "start": 59015605, "end": 59016300}, {"filename": "/GameData/textures/lq_liquidsky/sky_wfog_fbr.png", "start": 59016300, "end": 59017206}, {"filename": "/GameData/textures/lq_liquidsky/star_acid.png", "start": 59017206, "end": 59019385}, {"filename": "/GameData/textures/lq_liquidsky/star_blood1.png", "start": 59019385, "end": 59021458}, {"filename": "/GameData/textures/lq_liquidsky/star_lava1_fbr.png", "start": 59021458, "end": 59024663}, {"filename": "/GameData/textures/lq_liquidsky/star_lava2_fbr.png", "start": 59024663, "end": 59028745}, {"filename": "/GameData/textures/lq_liquidsky/star_lava3_fbr.png", "start": 59028745, "end": 59032714}, {"filename": "/GameData/textures/lq_liquidsky/star_lava_void_fbr.png", "start": 59032714, "end": 59036404}, {"filename": "/GameData/textures/lq_liquidsky/star_lavaskip.png", "start": 59036404, "end": 59037517}, {"filename": "/GameData/textures/lq_liquidsky/star_meatgoo2_fbr.png", "start": 59037517, "end": 59040705}, {"filename": "/GameData/textures/lq_liquidsky/star_meatgoo_fbr.png", "start": 59040705, "end": 59043502}, {"filename": "/GameData/textures/lq_liquidsky/star_slime1.png", "start": 59043502, "end": 59049793}, {"filename": "/GameData/textures/lq_liquidsky/star_slime2.png", "start": 59049793, "end": 59052982}, {"filename": "/GameData/textures/lq_liquidsky/star_slime3.png", "start": 59052982, "end": 59055014}, {"filename": "/GameData/textures/lq_liquidsky/star_slime_soul.png", "start": 59055014, "end": 59057804}, {"filename": "/GameData/textures/lq_liquidsky/star_slimeskip.png", "start": 59057804, "end": 59058877}, {"filename": "/GameData/textures/lq_liquidsky/star_soul_drain.png", "start": 59058877, "end": 59061845}, {"filename": "/GameData/textures/lq_liquidsky/star_tele1_fbr.png", "start": 59061845, "end": 59063725}, {"filename": "/GameData/textures/lq_liquidsky/star_tele2_fbr.png", "start": 59063725, "end": 59066526}, {"filename": "/GameData/textures/lq_liquidsky/star_tele3_fbr.png", "start": 59066526, "end": 59069562}, {"filename": "/GameData/textures/lq_liquidsky/star_tele4_fbr.png", "start": 59069562, "end": 59072254}, {"filename": "/GameData/textures/lq_liquidsky/star_water0.png", "start": 59072254, "end": 59075102}, {"filename": "/GameData/textures/lq_liquidsky/star_water1.png", "start": 59075102, "end": 59078211}, {"filename": "/GameData/textures/lq_liquidsky/star_water2.png", "start": 59078211, "end": 59080471}, {"filename": "/GameData/textures/lq_liquidsky/star_water3.png", "start": 59080471, "end": 59082852}, {"filename": "/GameData/textures/lq_liquidsky/star_water4.png", "start": 59082852, "end": 59087639}, {"filename": "/GameData/textures/lq_liquidsky/star_waterskip.png", "start": 59087639, "end": 59089379}, {"filename": "/GameData/textures/lq_liquidsky/star_wstill0.png", "start": 59089379, "end": 59092218}, {"filename": "/GameData/textures/lq_mayan/btn1.png", "start": 59092218, "end": 59095136}, {"filename": "/GameData/textures/lq_mayan/may_arrow.png", "start": 59095136, "end": 59096017}, {"filename": "/GameData/textures/lq_mayan/may_blok1_1.png", "start": 59096017, "end": 59098538}, {"filename": "/GameData/textures/lq_mayan/may_blok1_2.png", "start": 59098538, "end": 59100729}, {"filename": "/GameData/textures/lq_mayan/may_blok1_2_m.png", "start": 59100729, "end": 59103825}, {"filename": "/GameData/textures/lq_mayan/may_blok1_m.png", "start": 59103825, "end": 59113099}, {"filename": "/GameData/textures/lq_mayan/may_blok2_1.png", "start": 59113099, "end": 59115663}, {"filename": "/GameData/textures/lq_mayan/may_blok2_1_m.png", "start": 59115663, "end": 59118884}, {"filename": "/GameData/textures/lq_mayan/may_blok2_2.png", "start": 59118884, "end": 59129147}, {"filename": "/GameData/textures/lq_mayan/may_blok2_2_m.png", "start": 59129147, "end": 59141575}, {"filename": "/GameData/textures/lq_mayan/may_blud1_1.png", "start": 59141575, "end": 59144167}, {"filename": "/GameData/textures/lq_mayan/may_blud1_1m.png", "start": 59144167, "end": 59147387}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_1.png", "start": 59147387, "end": 59149822}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_1m.png", "start": 59149822, "end": 59152892}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_2.png", "start": 59152892, "end": 59155319}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_2m.png", "start": 59155319, "end": 59158347}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_3.png", "start": 59158347, "end": 59160606}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_3m.png", "start": 59160606, "end": 59164015}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_5.png", "start": 59164015, "end": 59166868}, {"filename": "/GameData/textures/lq_mayan/may_bnd1_5m.png", "start": 59166868, "end": 59170517}, {"filename": "/GameData/textures/lq_mayan/may_bnd_skull.png", "start": 59170517, "end": 59173149}, {"filename": "/GameData/textures/lq_mayan/may_brk1_0.png", "start": 59173149, "end": 59176253}, {"filename": "/GameData/textures/lq_mayan/may_brk1_0m.png", "start": 59176253, "end": 59179310}, {"filename": "/GameData/textures/lq_mayan/may_brk1_1.png", "start": 59179310, "end": 59181992}, {"filename": "/GameData/textures/lq_mayan/may_brk1_1m.png", "start": 59181992, "end": 59185191}, {"filename": "/GameData/textures/lq_mayan/may_brk1_2.png", "start": 59185191, "end": 59188142}, {"filename": "/GameData/textures/lq_mayan/may_brk1_2m.png", "start": 59188142, "end": 59191547}, {"filename": "/GameData/textures/lq_mayan/may_brk1_3.png", "start": 59191547, "end": 59194484}, {"filename": "/GameData/textures/lq_mayan/may_brk1_3m.png", "start": 59194484, "end": 59197660}, {"filename": "/GameData/textures/lq_mayan/may_brk2_0.png", "start": 59197660, "end": 59207515}, {"filename": "/GameData/textures/lq_mayan/may_brk2_0_m.png", "start": 59207515, "end": 59219848}, {"filename": "/GameData/textures/lq_mayan/may_brk_old.png", "start": 59219848, "end": 59222447}, {"filename": "/GameData/textures/lq_mayan/may_brk_oldm.png", "start": 59222447, "end": 59225786}, {"filename": "/GameData/textures/lq_mayan/may_deco1_1.png", "start": 59225786, "end": 59237015}, {"filename": "/GameData/textures/lq_mayan/may_deco1_1m.png", "start": 59237015, "end": 59251050}, {"filename": "/GameData/textures/lq_mayan/may_deco1_2.png", "start": 59251050, "end": 59253570}, {"filename": "/GameData/textures/lq_mayan/may_deco1_2my.png", "start": 59253570, "end": 59256958}, {"filename": "/GameData/textures/lq_mayan/may_deco1_3.png", "start": 59256958, "end": 59275531}, {"filename": "/GameData/textures/lq_mayan/may_deco1_3m.png", "start": 59275531, "end": 59295882}, {"filename": "/GameData/textures/lq_mayan/may_door1_1.png", "start": 59295882, "end": 59305349}, {"filename": "/GameData/textures/lq_mayan/may_door1_1m.png", "start": 59305349, "end": 59316440}, {"filename": "/GameData/textures/lq_mayan/may_door2_1.png", "start": 59316440, "end": 59328408}, {"filename": "/GameData/textures/lq_mayan/may_door2_2.png", "start": 59328408, "end": 59340186}, {"filename": "/GameData/textures/lq_mayan/may_drt2_1.png", "start": 59340186, "end": 59343023}, {"filename": "/GameData/textures/lq_mayan/may_flr1_1.png", "start": 59343023, "end": 59345928}, {"filename": "/GameData/textures/lq_mayan/may_flr1_2.png", "start": 59345928, "end": 59349151}, {"filename": "/GameData/textures/lq_mayan/may_flt1_1.png", "start": 59349151, "end": 59351440}, {"filename": "/GameData/textures/lq_mayan/may_flt1_1m.png", "start": 59351440, "end": 59354888}, {"filename": "/GameData/textures/lq_mayan/may_key1_1.png", "start": 59354888, "end": 59356475}, {"filename": "/GameData/textures/lq_mayan/may_key1_2.png", "start": 59356475, "end": 59358472}, {"filename": "/GameData/textures/lq_mayan/may_lite1_1_fbr.png", "start": 59358472, "end": 59359402}, {"filename": "/GameData/textures/lq_mayan/may_lite1_2.png", "start": 59359402, "end": 59359765}, {"filename": "/GameData/textures/lq_mayan/may_lite2_1.png", "start": 59359765, "end": 59362702}, {"filename": "/GameData/textures/lq_mayan/may_lite2_2.png", "start": 59362702, "end": 59363188}, {"filename": "/GameData/textures/lq_mayan/may_lite3_1_fbr.png", "start": 59363188, "end": 59364145}, {"filename": "/GameData/textures/lq_mayan/may_lite_f1.png", "start": 59364145, "end": 59364631}, {"filename": "/GameData/textures/lq_mayan/may_oldmtomb1_1_fbr.png", "start": 59364631, "end": 59378483}, {"filename": "/GameData/textures/lq_mayan/may_oldtomb1_2_fbr.png", "start": 59378483, "end": 59392375}, {"filename": "/GameData/textures/lq_mayan/may_plat_stem_m.png", "start": 59392375, "end": 59393241}, {"filename": "/GameData/textures/lq_mayan/may_plats.png", "start": 59393241, "end": 59395903}, {"filename": "/GameData/textures/lq_mayan/may_platst.png", "start": 59395903, "end": 59398330}, {"filename": "/GameData/textures/lq_mayan/may_platt.png", "start": 59398330, "end": 59400953}, {"filename": "/GameData/textures/lq_mayan/may_plr1_1.png", "start": 59400953, "end": 59404147}, {"filename": "/GameData/textures/lq_mayan/may_tomb1_1_fbr.png", "start": 59404147, "end": 59417999}, {"filename": "/GameData/textures/lq_mayan/may_tomb1_2_fbr.png", "start": 59417999, "end": 59431891}, {"filename": "/GameData/textures/lq_mayan/may_trm1_1.png", "start": 59431891, "end": 59440819}, {"filename": "/GameData/textures/lq_mayan/may_trm1_2.png", "start": 59440819, "end": 59443524}, {"filename": "/GameData/textures/lq_mayan/may_trm1_a.png", "start": 59443524, "end": 59445866}, {"filename": "/GameData/textures/lq_mayan/may_tskull.png", "start": 59445866, "end": 59456872}, {"filename": "/GameData/textures/lq_mayan/may_wall1_1.png", "start": 59456872, "end": 59459854}, {"filename": "/GameData/textures/lq_mayan/may_wall1_2.png", "start": 59459854, "end": 59462820}, {"filename": "/GameData/textures/lq_mayan/may_wall1_3.png", "start": 59462820, "end": 59466014}, {"filename": "/GameData/textures/lq_mayan/may_wall1_3a.png", "start": 59466014, "end": 59468595}, {"filename": "/GameData/textures/lq_mayan/may_wall1_4.png", "start": 59468595, "end": 59471583}, {"filename": "/GameData/textures/lq_mayan/may_wall1_4a.png", "start": 59471583, "end": 59473982}, {"filename": "/GameData/textures/lq_mayan/maya_end_dr1.png", "start": 59473982, "end": 59478368}, {"filename": "/GameData/textures/lq_mayan/maya_end_dr2.png", "start": 59478368, "end": 59482753}, {"filename": "/GameData/textures/lq_mayan/maya_end_trim1.png", "start": 59482753, "end": 59487143}, {"filename": "/GameData/textures/lq_mayan/plus_0_may_btn1.png", "start": 59487143, "end": 59490060}, {"filename": "/GameData/textures/lq_mayan/plus_0_may_mpiloilon_fbr.png", "start": 59490060, "end": 59491618}, {"filename": "/GameData/textures/lq_mayan/plus_0_may_mpilon_fbr.png", "start": 59491618, "end": 59493193}, {"filename": "/GameData/textures/lq_mayan/plus_0_may_mshoohoot_fbr.png", "start": 59493193, "end": 59494138}, {"filename": "/GameData/textures/lq_mayan/plus_0_may_mshoot_fbr.png", "start": 59494138, "end": 59495081}, {"filename": "/GameData/textures/lq_mayan/plus_1_may_btn1.png", "start": 59495081, "end": 59497989}, {"filename": "/GameData/textures/lq_mayan/plus_1_may_mpiloilon_fbr.png", "start": 59497989, "end": 59499562}, {"filename": "/GameData/textures/lq_mayan/plus_1_may_mpilon_fbr.png", "start": 59499562, "end": 59501155}, {"filename": "/GameData/textures/lq_mayan/plus_1_may_mshoohoot_fbr.png", "start": 59501155, "end": 59502140}, {"filename": "/GameData/textures/lq_mayan/plus_1_may_mshoot_fbr.png", "start": 59502140, "end": 59503124}, {"filename": "/GameData/textures/lq_mayan/plus_2_may_btn1.png", "start": 59503124, "end": 59506015}, {"filename": "/GameData/textures/lq_mayan/plus_2_may_mpilon.png", "start": 59506015, "end": 59507592}, {"filename": "/GameData/textures/lq_mayan/plus_2_may_mshoohoot_fbr.png", "start": 59507592, "end": 59508617}, {"filename": "/GameData/textures/lq_mayan/plus_2_may_mshoot_fbr.png", "start": 59508617, "end": 59509644}, {"filename": "/GameData/textures/lq_mayan/plus_3_may_btn1.png", "start": 59509644, "end": 59512706}, {"filename": "/GameData/textures/lq_mayan/plus_3_may_mpiloilon_fbr.png", "start": 59512706, "end": 59514279}, {"filename": "/GameData/textures/lq_mayan/plus_3_may_mpilon_fbr.png", "start": 59514279, "end": 59515872}, {"filename": "/GameData/textures/lq_mayan/plus_3_may_mshoohoot_fbr.png", "start": 59515872, "end": 59516857}, {"filename": "/GameData/textures/lq_mayan/plus_3_may_mshoot_fbr.png", "start": 59516857, "end": 59517841}, {"filename": "/GameData/textures/lq_mayan/plus_4_may_btn1.png", "start": 59517841, "end": 59520904}, {"filename": "/GameData/textures/lq_mayan/plus_5_may_btn1.png", "start": 59520904, "end": 59523645}, {"filename": "/GameData/textures/lq_mayan/plus_a_may_btn1.png", "start": 59523645, "end": 59526368}, {"filename": "/GameData/textures/lq_mayan/plus_a_may_mpiloilon_fbr.png", "start": 59526368, "end": 59527945}, {"filename": "/GameData/textures/lq_mayan/plus_a_may_mpilon_fbr.png", "start": 59527945, "end": 59529529}, {"filename": "/GameData/textures/lq_mayan/plus_a_may_mshoot.png", "start": 59529529, "end": 59530478}, {"filename": "/GameData/textures/lq_medieval/+0med_but1.png", "start": 59530478, "end": 59534733}, {"filename": "/GameData/textures/lq_medieval/+0med_but2.png", "start": 59534733, "end": 59536445}, {"filename": "/GameData/textures/lq_medieval/+0med_but3.png", "start": 59536445, "end": 59538148}, {"filename": "/GameData/textures/lq_medieval/+0med_but_s1.png", "start": 59538148, "end": 59542394}, {"filename": "/GameData/textures/lq_medieval/+0med_sht_but1.png", "start": 59542394, "end": 59543687}, {"filename": "/GameData/textures/lq_medieval/+1med_but3.png", "start": 59543687, "end": 59545389}, {"filename": "/GameData/textures/lq_medieval/+1med_but_s1.png", "start": 59545389, "end": 59549608}, {"filename": "/GameData/textures/lq_medieval/+1med_sht_but1.png", "start": 59549608, "end": 59550921}, {"filename": "/GameData/textures/lq_medieval/+2med_but_s1.png", "start": 59550921, "end": 59555195}, {"filename": "/GameData/textures/lq_medieval/+3med_but_s1.png", "start": 59555195, "end": 59559414}, {"filename": "/GameData/textures/lq_medieval/+amed_but1.png", "start": 59559414, "end": 59563497}, {"filename": "/GameData/textures/lq_medieval/+amed_but2.png", "start": 59563497, "end": 59565215}, {"filename": "/GameData/textures/lq_medieval/+amed_but3.png", "start": 59565215, "end": 59566825}, {"filename": "/GameData/textures/lq_medieval/+amed_but_s1.png", "start": 59566825, "end": 59571038}, {"filename": "/GameData/textures/lq_medieval/+amed_sht_but1.png", "start": 59571038, "end": 59572194}, {"filename": "/GameData/textures/lq_medieval/Art1.png", "start": 59572194, "end": 59705187}, {"filename": "/GameData/textures/lq_medieval/afloor1_4.png", "start": 59705187, "end": 59708188}, {"filename": "/GameData/textures/lq_medieval/afloor1_8.png", "start": 59708188, "end": 59711028}, {"filename": "/GameData/textures/lq_medieval/afloor3_1.png", "start": 59711028, "end": 59713838}, {"filename": "/GameData/textures/lq_medieval/altar1_1.png", "start": 59713838, "end": 59717726}, {"filename": "/GameData/textures/lq_medieval/altar1_3.png", "start": 59717726, "end": 59722207}, {"filename": "/GameData/textures/lq_medieval/altar1_4.png", "start": 59722207, "end": 59726260}, {"filename": "/GameData/textures/lq_medieval/brick0.png", "start": 59726260, "end": 59734482}, {"filename": "/GameData/textures/lq_medieval/brick1.png", "start": 59734482, "end": 59745236}, {"filename": "/GameData/textures/lq_medieval/brick4_s.png", "start": 59745236, "end": 59747169}, {"filename": "/GameData/textures/lq_medieval/brown1.png", "start": 59747169, "end": 59756345}, {"filename": "/GameData/textures/lq_medieval/med_block_1a.png", "start": 59756345, "end": 59802700}, {"filename": "/GameData/textures/lq_medieval/med_block_1b.png", "start": 59802700, "end": 59886064}, {"filename": "/GameData/textures/lq_medieval/med_block_1c.png", "start": 59886064, "end": 59984235}, {"filename": "/GameData/textures/lq_medieval/med_block_1d.png", "start": 59984235, "end": 60092904}, {"filename": "/GameData/textures/lq_medieval/med_block_1e.png", "start": 60092904, "end": 60201397}, {"filename": "/GameData/textures/lq_medieval/med_block_1f.png", "start": 60201397, "end": 60269256}, {"filename": "/GameData/textures/lq_medieval/med_block_1s.png", "start": 60269256, "end": 60386707}, {"filename": "/GameData/textures/lq_medieval/med_block_2a.png", "start": 60386707, "end": 60464091}, {"filename": "/GameData/textures/lq_medieval/med_block_2b.png", "start": 60464091, "end": 60541530}, {"filename": "/GameData/textures/lq_medieval/med_block_2c.png", "start": 60541530, "end": 60630405}, {"filename": "/GameData/textures/lq_medieval/med_block_2d.png", "start": 60630405, "end": 60728599}, {"filename": "/GameData/textures/lq_medieval/med_block_2e.png", "start": 60728599, "end": 60826836}, {"filename": "/GameData/textures/lq_medieval/med_block_2f.png", "start": 60826836, "end": 60891380}, {"filename": "/GameData/textures/lq_medieval/med_block_2s.png", "start": 60891380, "end": 60997132}, {"filename": "/GameData/textures/lq_medieval/med_brk9_ceil1a.png", "start": 60997132, "end": 61008940}, {"filename": "/GameData/textures/lq_medieval/med_brk9_ceil1b.png", "start": 61008940, "end": 61021475}, {"filename": "/GameData/textures/lq_medieval/med_brk9_wal2a.png", "start": 61021475, "end": 61034372}, {"filename": "/GameData/textures/lq_medieval/med_brk9_wal2b.png", "start": 61034372, "end": 61047628}, {"filename": "/GameData/textures/lq_medieval/med_brk9_wal2c.png", "start": 61047628, "end": 61060070}, {"filename": "/GameData/textures/lq_medieval/med_brk9_win1.png", "start": 61060070, "end": 61103068}, {"filename": "/GameData/textures/lq_medieval/med_brk9_win1b.png", "start": 61103068, "end": 61148394}, {"filename": "/GameData/textures/lq_medieval/med_but_side.png", "start": 61148394, "end": 61149392}, {"filename": "/GameData/textures/lq_medieval/med_cmet1.png", "start": 61149392, "end": 61152516}, {"filename": "/GameData/textures/lq_medieval/med_cmet2a.png", "start": 61152516, "end": 61155667}, {"filename": "/GameData/textures/lq_medieval/med_cmet2b.png", "start": 61155667, "end": 61158729}, {"filename": "/GameData/textures/lq_medieval/med_cmet2c.png", "start": 61158729, "end": 61161866}, {"filename": "/GameData/textures/lq_medieval/med_cmet3a.png", "start": 61161866, "end": 61165138}, {"filename": "/GameData/textures/lq_medieval/med_cmet3b.png", "start": 61165138, "end": 61168320}, {"filename": "/GameData/textures/lq_medieval/med_cmet4.png", "start": 61168320, "end": 61171161}, {"filename": "/GameData/textures/lq_medieval/med_cmet5a.png", "start": 61171161, "end": 61174299}, {"filename": "/GameData/textures/lq_medieval/med_cmet5c.png", "start": 61174299, "end": 61177442}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk10.png", "start": 61177442, "end": 61192168}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk10_f.png", "start": 61192168, "end": 61204456}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk10b.png", "start": 61204456, "end": 61219019}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk11.png", "start": 61219019, "end": 61230267}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk12.png", "start": 61230267, "end": 61247216}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk12_f.png", "start": 61247216, "end": 61264549}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk13.png", "start": 61264549, "end": 61320666}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk14.png", "start": 61320666, "end": 61336520}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk14_f.png", "start": 61336520, "end": 61349489}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk14b.png", "start": 61349489, "end": 61362979}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk15.png", "start": 61362979, "end": 61376805}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk15b.png", "start": 61376805, "end": 61390735}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk15f.png", "start": 61390735, "end": 61403933}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk16.png", "start": 61403933, "end": 61420154}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk16b.png", "start": 61420154, "end": 61437746}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk16f.png", "start": 61437746, "end": 61455017}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk17.png", "start": 61455017, "end": 61465124}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk17_f.png", "start": 61465124, "end": 61474149}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk17b.png", "start": 61474149, "end": 61484286}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_f.png", "start": 61484286, "end": 61492919}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_gb.png", "start": 61492919, "end": 61495667}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_gt.png", "start": 61495667, "end": 61498404}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_t.png", "start": 61498404, "end": 61506590}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_tb.png", "start": 61506590, "end": 61517216}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18_tc.png", "start": 61517216, "end": 61521745}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk18b.png", "start": 61521745, "end": 61529918}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk19_f.png", "start": 61529918, "end": 61541619}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk19_t.png", "start": 61541619, "end": 61553010}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk19b.png", "start": 61553010, "end": 61564330}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk1_1.png", "start": 61564330, "end": 61566815}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk1_2.png", "start": 61566815, "end": 61569986}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk1_3.png", "start": 61569986, "end": 61572229}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk2_1.png", "start": 61572229, "end": 61574778}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk2_2.png", "start": 61574778, "end": 61578151}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk5.png", "start": 61578151, "end": 61581318}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk6_1.png", "start": 61581318, "end": 61583961}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk6_2.png", "start": 61583961, "end": 61589023}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk7_1.png", "start": 61589023, "end": 61591663}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk7_1b.png", "start": 61591663, "end": 61594207}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk7_2.png", "start": 61594207, "end": 61597373}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk8_1c.png", "start": 61597373, "end": 61609978}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk8_1d.png", "start": 61609978, "end": 61622581}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk9_1.png", "start": 61622581, "end": 61635791}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk9_1b.png", "start": 61635791, "end": 61649448}, {"filename": "/GameData/textures/lq_medieval/med_csl_brk9_f.png", "start": 61649448, "end": 61662235}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr1_1.png", "start": 61662235, "end": 61672182}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr2_1.png", "start": 61672182, "end": 61674468}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr2_2.png", "start": 61674468, "end": 61676641}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr4_1.png", "start": 61676641, "end": 61680381}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr4_3.png", "start": 61680381, "end": 61683815}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr4_4.png", "start": 61683815, "end": 61696871}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr4_5.png", "start": 61696871, "end": 61711389}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr5_1.png", "start": 61711389, "end": 61726319}, {"filename": "/GameData/textures/lq_medieval/med_csl_flr5_2.png", "start": 61726319, "end": 61741847}, {"filename": "/GameData/textures/lq_medieval/med_csl_stp1.png", "start": 61741847, "end": 61744444}, {"filename": "/GameData/textures/lq_medieval/med_csl_stp2.png", "start": 61744444, "end": 61746987}, {"filename": "/GameData/textures/lq_medieval/med_csl_trm1.png", "start": 61746987, "end": 61748312}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1.png", "start": 61748312, "end": 61769785}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1_t.png", "start": 61769785, "end": 61781172}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1_t2.png", "start": 61781172, "end": 61794334}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1_t2b_fbr.png", "start": 61794334, "end": 61808284}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1_t3.png", "start": 61808284, "end": 61820024}, {"filename": "/GameData/textures/lq_medieval/med_dbrick1_t4.png", "start": 61820024, "end": 61832800}, {"filename": "/GameData/textures/lq_medieval/med_dbrick2.png", "start": 61832800, "end": 61838334}, {"filename": "/GameData/textures/lq_medieval/med_dbrick3.png", "start": 61838334, "end": 61843708}, {"filename": "/GameData/textures/lq_medieval/med_dbrick4.png", "start": 61843708, "end": 61849016}, {"filename": "/GameData/textures/lq_medieval/med_dbrick5.png", "start": 61849016, "end": 61854838}, {"filename": "/GameData/textures/lq_medieval/med_dbrick6.png", "start": 61854838, "end": 61863999}, {"filename": "/GameData/textures/lq_medieval/med_dbrick6b.png", "start": 61863999, "end": 61873007}, {"filename": "/GameData/textures/lq_medieval/med_dbrick6f.png", "start": 61873007, "end": 61882297}, {"filename": "/GameData/textures/lq_medieval/med_door1.png", "start": 61882297, "end": 61886980}, {"filename": "/GameData/textures/lq_medieval/med_door2.png", "start": 61886980, "end": 61891479}, {"filename": "/GameData/textures/lq_medieval/med_door3.png", "start": 61891479, "end": 61896128}, {"filename": "/GameData/textures/lq_medieval/med_door3b.png", "start": 61896128, "end": 61898669}, {"filename": "/GameData/textures/lq_medieval/med_door4.png", "start": 61898669, "end": 61903233}, {"filename": "/GameData/textures/lq_medieval/med_door4b.png", "start": 61903233, "end": 61905829}, {"filename": "/GameData/textures/lq_medieval/med_dr1a.png", "start": 61905829, "end": 61924613}, {"filename": "/GameData/textures/lq_medieval/med_dr1a_blu.png", "start": 61924613, "end": 61942977}, {"filename": "/GameData/textures/lq_medieval/med_dr1b.png", "start": 61942977, "end": 61967126}, {"filename": "/GameData/textures/lq_medieval/med_dr1b_blu.png", "start": 61967126, "end": 61986745}, {"filename": "/GameData/textures/lq_medieval/med_dr2a.png", "start": 61986745, "end": 62011122}, {"filename": "/GameData/textures/lq_medieval/med_dr2a_blu.png", "start": 62011122, "end": 62031016}, {"filename": "/GameData/textures/lq_medieval/med_dr3a.png", "start": 62031016, "end": 62044499}, {"filename": "/GameData/textures/lq_medieval/med_dr3a_blu.png", "start": 62044499, "end": 62057789}, {"filename": "/GameData/textures/lq_medieval/med_dr3b.png", "start": 62057789, "end": 62072097}, {"filename": "/GameData/textures/lq_medieval/med_dr3b_blu.png", "start": 62072097, "end": 62086303}, {"filename": "/GameData/textures/lq_medieval/med_dr3c.png", "start": 62086303, "end": 62099146}, {"filename": "/GameData/textures/lq_medieval/med_dr3c_blu.png", "start": 62099146, "end": 62112270}, {"filename": "/GameData/textures/lq_medieval/med_dwall1.png", "start": 62112270, "end": 62114746}, {"filename": "/GameData/textures/lq_medieval/med_ebrick1.png", "start": 62114746, "end": 62127328}, {"filename": "/GameData/textures/lq_medieval/med_ebrick10.png", "start": 62127328, "end": 62138213}, {"filename": "/GameData/textures/lq_medieval/med_ebrick10b.png", "start": 62138213, "end": 62148699}, {"filename": "/GameData/textures/lq_medieval/med_ebrick11.png", "start": 62148699, "end": 62163958}, {"filename": "/GameData/textures/lq_medieval/med_ebrick12.png", "start": 62163958, "end": 62181375}, {"filename": "/GameData/textures/lq_medieval/med_ebrick12b.png", "start": 62181375, "end": 62194614}, {"filename": "/GameData/textures/lq_medieval/med_ebrick13.png", "start": 62194614, "end": 62204512}, {"filename": "/GameData/textures/lq_medieval/med_ebrick14.png", "start": 62204512, "end": 62213527}, {"filename": "/GameData/textures/lq_medieval/med_ebrick15.png", "start": 62213527, "end": 62228328}, {"filename": "/GameData/textures/lq_medieval/med_ebrick16.png", "start": 62228328, "end": 62239692}, {"filename": "/GameData/textures/lq_medieval/med_ebrick16b.png", "start": 62239692, "end": 62248010}, {"filename": "/GameData/textures/lq_medieval/med_ebrick17.png", "start": 62248010, "end": 62261144}, {"filename": "/GameData/textures/lq_medieval/med_ebrick17b.png", "start": 62261144, "end": 62273410}, {"filename": "/GameData/textures/lq_medieval/med_ebrick17c.png", "start": 62273410, "end": 62286669}, {"filename": "/GameData/textures/lq_medieval/med_ebrick18.png", "start": 62286669, "end": 62302669}, {"filename": "/GameData/textures/lq_medieval/med_ebrick18b.png", "start": 62302669, "end": 62317057}, {"filename": "/GameData/textures/lq_medieval/med_ebrick2.png", "start": 62317057, "end": 62329549}, {"filename": "/GameData/textures/lq_medieval/med_ebrick20.png", "start": 62329549, "end": 62342949}, {"filename": "/GameData/textures/lq_medieval/med_ebrick21.png", "start": 62342949, "end": 62355091}, {"filename": "/GameData/textures/lq_medieval/med_ebrick22.png", "start": 62355091, "end": 62367252}, {"filename": "/GameData/textures/lq_medieval/med_ebrick3.png", "start": 62367252, "end": 62379712}, {"filename": "/GameData/textures/lq_medieval/med_ebrick4.png", "start": 62379712, "end": 62395262}, {"filename": "/GameData/textures/lq_medieval/med_ebrick5.png", "start": 62395262, "end": 62409078}, {"filename": "/GameData/textures/lq_medieval/med_ebrick6.png", "start": 62409078, "end": 62423789}, {"filename": "/GameData/textures/lq_medieval/med_ebrick7.png", "start": 62423789, "end": 62437434}, {"filename": "/GameData/textures/lq_medieval/med_ebrick8.png", "start": 62437434, "end": 62450173}, {"filename": "/GameData/textures/lq_medieval/med_ebrick9.png", "start": 62450173, "end": 62464728}, {"filename": "/GameData/textures/lq_medieval/med_etrim1.png", "start": 62464728, "end": 62468057}, {"filename": "/GameData/textures/lq_medieval/med_glass1.png", "start": 62468057, "end": 62480436}, {"filename": "/GameData/textures/lq_medieval/med_glass2.png", "start": 62480436, "end": 62492093}, {"filename": "/GameData/textures/lq_medieval/med_glass3.png", "start": 62492093, "end": 62503108}, {"filename": "/GameData/textures/lq_medieval/med_glass4.png", "start": 62503108, "end": 62534927}, {"filename": "/GameData/textures/lq_medieval/med_glass5.png", "start": 62534927, "end": 62544709}, {"filename": "/GameData/textures/lq_medieval/med_met_dec1.png", "start": 62544709, "end": 62548849}, {"filename": "/GameData/textures/lq_medieval/med_met_key1a.png", "start": 62548849, "end": 62550609}, {"filename": "/GameData/textures/lq_medieval/med_met_key1b.png", "start": 62550609, "end": 62551943}, {"filename": "/GameData/textures/lq_medieval/med_met_key2a.png", "start": 62551943, "end": 62553703}, {"filename": "/GameData/textures/lq_medieval/med_met_key2b.png", "start": 62553703, "end": 62555409}, {"filename": "/GameData/textures/lq_medieval/med_met_trim1.png", "start": 62555409, "end": 62556436}, {"filename": "/GameData/textures/lq_medieval/med_met_trim2.png", "start": 62556436, "end": 62557383}, {"filename": "/GameData/textures/lq_medieval/med_met_trim3.png", "start": 62557383, "end": 62558257}, {"filename": "/GameData/textures/lq_medieval/med_metw1a.png", "start": 62558257, "end": 62570565}, {"filename": "/GameData/textures/lq_medieval/med_metw1b.png", "start": 62570565, "end": 62579272}, {"filename": "/GameData/textures/lq_medieval/med_metw2a.png", "start": 62579272, "end": 62589179}, {"filename": "/GameData/textures/lq_medieval/med_metw2b.png", "start": 62589179, "end": 62601557}, {"filename": "/GameData/textures/lq_medieval/med_rmet.png", "start": 62601557, "end": 62614658}, {"filename": "/GameData/textures/lq_medieval/med_rmet_slat.png", "start": 62614658, "end": 62628969}, {"filename": "/GameData/textures/lq_medieval/med_rmet_tile.png", "start": 62628969, "end": 62642901}, {"filename": "/GameData/textures/lq_medieval/med_rmet_trim32.png", "start": 62642901, "end": 62656110}, {"filename": "/GameData/textures/lq_medieval/med_roof1.png", "start": 62656110, "end": 62668025}, {"filename": "/GameData/textures/lq_medieval/med_roof2.png", "start": 62668025, "end": 62683098}, {"filename": "/GameData/textures/lq_medieval/med_roof3.png", "start": 62683098, "end": 62698027}, {"filename": "/GameData/textures/lq_medieval/med_roof4.png", "start": 62698027, "end": 62715841}, {"filename": "/GameData/textures/lq_medieval/med_roof5.png", "start": 62715841, "end": 62726347}, {"filename": "/GameData/textures/lq_medieval/med_tanwall1.png", "start": 62726347, "end": 62768025}, {"filename": "/GameData/textures/lq_medieval/med_tanwall2.png", "start": 62768025, "end": 62827802}, {"filename": "/GameData/textures/lq_medieval/med_tanwall3.png", "start": 62827802, "end": 62878237}, {"filename": "/GameData/textures/lq_medieval/med_tanwall4.png", "start": 62878237, "end": 62927378}, {"filename": "/GameData/textures/lq_medieval/med_tanwall4_f.png", "start": 62927378, "end": 62971331}, {"filename": "/GameData/textures/lq_medieval/med_tanwall6.png", "start": 62971331, "end": 63015876}, {"filename": "/GameData/textures/lq_medieval/med_tanwall7.png", "start": 63015876, "end": 63061469}, {"filename": "/GameData/textures/lq_medieval/med_tanwall8.png", "start": 63061469, "end": 63099067}, {"filename": "/GameData/textures/lq_medieval/med_tanwall9.png", "start": 63099067, "end": 63148147}, {"filename": "/GameData/textures/lq_medieval/med_tanwall9_f.png", "start": 63148147, "end": 63192126}, {"filename": "/GameData/textures/lq_medieval/med_telepad.png", "start": 63192126, "end": 63196753}, {"filename": "/GameData/textures/lq_medieval/med_tile1.png", "start": 63196753, "end": 63269364}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_lit1_fbr.png", "start": 63269364, "end": 63270967}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_lit3_fbr.png", "start": 63270967, "end": 63272272}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_tele.png", "start": 63272272, "end": 63275454}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_trim1.png", "start": 63275454, "end": 63278620}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_trim1b.png", "start": 63278620, "end": 63280675}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_trim2.png", "start": 63280675, "end": 63284215}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_trim3.png", "start": 63284215, "end": 63287692}, {"filename": "/GameData/textures/lq_medieval/med_tmpl_trim4.png", "start": 63287692, "end": 63290805}, {"filename": "/GameData/textures/lq_medieval/med_trim1_1.png", "start": 63290805, "end": 63299509}, {"filename": "/GameData/textures/lq_medieval/med_trim2_1.png", "start": 63299509, "end": 63300623}, {"filename": "/GameData/textures/lq_medieval/med_trim3_1.png", "start": 63300623, "end": 63304218}, {"filename": "/GameData/textures/lq_medieval/med_trim3_2.png", "start": 63304218, "end": 63307777}, {"filename": "/GameData/textures/lq_medieval/med_trim3_3.png", "start": 63307777, "end": 63311474}, {"filename": "/GameData/textures/lq_medieval/med_trim3_4.png", "start": 63311474, "end": 63314845}, {"filename": "/GameData/textures/lq_medieval/med_trim4_1.png", "start": 63314845, "end": 63318269}, {"filename": "/GameData/textures/lq_medieval/med_trim4_2.png", "start": 63318269, "end": 63321793}, {"filename": "/GameData/textures/lq_medieval/med_trim4_3.png", "start": 63321793, "end": 63325153}, {"filename": "/GameData/textures/lq_medieval/med_trim4_4.png", "start": 63325153, "end": 63328376}, {"filename": "/GameData/textures/lq_medieval/plus_0_csl_brk14.png", "start": 63328376, "end": 63344689}, {"filename": "/GameData/textures/lq_medieval/plus_1_csl_brk14.png", "start": 63344689, "end": 63360982}, {"filename": "/GameData/textures/lq_medieval/plus_2_csl_brk14.png", "start": 63360982, "end": 63377281}, {"filename": "/GameData/textures/lq_medieval/plus_3_csl_brk14.png", "start": 63377281, "end": 63393523}, {"filename": "/GameData/textures/lq_medieval/plus_4_csl_brk14.png", "start": 63393523, "end": 63409864}, {"filename": "/GameData/textures/lq_medieval/sidewalk.png", "start": 63409864, "end": 63418620}, {"filename": "/GameData/textures/lq_medieval/sq_trim1_2.png", "start": 63418620, "end": 63426902}, {"filename": "/GameData/textures/lq_medieval/sq_trim1_2_s.png", "start": 63426902, "end": 63429477}, {"filename": "/GameData/textures/lq_medieval/tile.png", "start": 63429477, "end": 63432602}, {"filename": "/GameData/textures/lq_medieval/tile1.png", "start": 63432602, "end": 63505215}, {"filename": "/GameData/textures/lq_medieval/wall14_5.png", "start": 63505215, "end": 63509295}, {"filename": "/GameData/textures/lq_medieval/wbrick1_5.png", "start": 63509295, "end": 63513061}, {"filename": "/GameData/textures/lq_medieval/wswamp2_1.png", "start": 63513061, "end": 63516187}, {"filename": "/GameData/textures/lq_medieval/wswamp2_2.png", "start": 63516187, "end": 63519964}, {"filename": "/GameData/textures/lq_metal/gig1_bone.png", "start": 63519964, "end": 63529645}, {"filename": "/GameData/textures/lq_metal/gig1_bone_l.png", "start": 63529645, "end": 63560306}, {"filename": "/GameData/textures/lq_metal/gig1_skull.png", "start": 63560306, "end": 63562970}, {"filename": "/GameData/textures/lq_metal/gig1_skull_l.png", "start": 63562970, "end": 63571755}, {"filename": "/GameData/textures/lq_metal/gig1_spine.png", "start": 63571755, "end": 63620456}, {"filename": "/GameData/textures/lq_metal/gig2_bone.png", "start": 63620456, "end": 63628879}, {"filename": "/GameData/textures/lq_metal/gig2_bone_l.png", "start": 63628879, "end": 63654627}, {"filename": "/GameData/textures/lq_metal/gig2_bone_s.png", "start": 63654627, "end": 63657240}, {"filename": "/GameData/textures/lq_metal/gig2_mouth_s.png", "start": 63657240, "end": 63659655}, {"filename": "/GameData/textures/lq_metal/med_flat8.png", "start": 63659655, "end": 63662513}, {"filename": "/GameData/textures/lq_metal/med_flat9.png", "start": 63662513, "end": 63665501}, {"filename": "/GameData/textures/lq_metal/met_blc_block.png", "start": 63665501, "end": 63668840}, {"filename": "/GameData/textures/lq_metal/met_blc_diam.png", "start": 63668840, "end": 63671730}, {"filename": "/GameData/textures/lq_metal/met_blc_trim28.png", "start": 63671730, "end": 63676773}, {"filename": "/GameData/textures/lq_metal/met_blc_trim32.png", "start": 63676773, "end": 63679919}, {"filename": "/GameData/textures/lq_metal/met_blc_trim32r.png", "start": 63679919, "end": 63683174}, {"filename": "/GameData/textures/lq_metal/met_blc_trim32s.png", "start": 63683174, "end": 63686274}, {"filename": "/GameData/textures/lq_metal/met_blc_trim64.png", "start": 63686274, "end": 63689417}, {"filename": "/GameData/textures/lq_metal/met_blu_block.png", "start": 63689417, "end": 63694437}, {"filename": "/GameData/textures/lq_metal/met_blu_det1.png", "start": 63694437, "end": 63695593}, {"filename": "/GameData/textures/lq_metal/met_blu_diam.png", "start": 63695593, "end": 63698490}, {"filename": "/GameData/textures/lq_metal/met_blu_diam2.png", "start": 63698490, "end": 63701285}, {"filename": "/GameData/textures/lq_metal/met_blu_diamc.png", "start": 63701285, "end": 63704816}, {"filename": "/GameData/textures/lq_metal/met_blu_door1.png", "start": 63704816, "end": 63709499}, {"filename": "/GameData/textures/lq_metal/met_blu_door2.png", "start": 63709499, "end": 63713998}, {"filename": "/GameData/textures/lq_metal/met_blu_door3.png", "start": 63713998, "end": 63718647}, {"filename": "/GameData/textures/lq_metal/met_blu_door4.png", "start": 63718647, "end": 63723211}, {"filename": "/GameData/textures/lq_metal/met_blu_door5.png", "start": 63723211, "end": 63726154}, {"filename": "/GameData/textures/lq_metal/met_blu_door6.png", "start": 63726154, "end": 63728750}, {"filename": "/GameData/textures/lq_metal/met_blu_fac1.png", "start": 63728750, "end": 63731152}, {"filename": "/GameData/textures/lq_metal/met_blu_flat.png", "start": 63731152, "end": 63734586}, {"filename": "/GameData/textures/lq_metal/met_blu_flatst.png", "start": 63734586, "end": 63736815}, {"filename": "/GameData/textures/lq_metal/met_blu_gig1.png", "start": 63736815, "end": 63740934}, {"filename": "/GameData/textures/lq_metal/met_blu_gig2.png", "start": 63740934, "end": 63744547}, {"filename": "/GameData/textures/lq_metal/met_blu_gig2b.png", "start": 63744547, "end": 63750741}, {"filename": "/GameData/textures/lq_metal/met_blu_grate.png", "start": 63750741, "end": 63753796}, {"filename": "/GameData/textures/lq_metal/met_blu_grate2.png", "start": 63753796, "end": 63756627}, {"filename": "/GameData/textures/lq_metal/met_blu_grate3.png", "start": 63756627, "end": 63758287}, {"filename": "/GameData/textures/lq_metal/met_blu_lit1_fbr.png", "start": 63758287, "end": 63760358}, {"filename": "/GameData/textures/lq_metal/met_blu_lit2_fbr.png", "start": 63760358, "end": 63761547}, {"filename": "/GameData/textures/lq_metal/met_blu_lit3.png", "start": 63761547, "end": 63762733}, {"filename": "/GameData/textures/lq_metal/met_blu_lit4.png", "start": 63762733, "end": 63763994}, {"filename": "/GameData/textures/lq_metal/met_blu_lit5.png", "start": 63763994, "end": 63766087}, {"filename": "/GameData/textures/lq_metal/met_blu_pan1.png", "start": 63766087, "end": 63769569}, {"filename": "/GameData/textures/lq_metal/met_blu_pan2.png", "start": 63769569, "end": 63771855}, {"filename": "/GameData/textures/lq_metal/met_blu_pan3.png", "start": 63771855, "end": 63775282}, {"filename": "/GameData/textures/lq_metal/met_blu_rect.png", "start": 63775282, "end": 63779061}, {"filename": "/GameData/textures/lq_metal/met_blu_rivg.png", "start": 63779061, "end": 63782982}, {"filename": "/GameData/textures/lq_metal/met_blu_rivs.png", "start": 63782982, "end": 63786340}, {"filename": "/GameData/textures/lq_metal/met_blu_slat.png", "start": 63786340, "end": 63790126}, {"filename": "/GameData/textures/lq_metal/met_blu_sqr.png", "start": 63790126, "end": 63793897}, {"filename": "/GameData/textures/lq_metal/met_blu_sqrd.png", "start": 63793897, "end": 63797735}, {"filename": "/GameData/textures/lq_metal/met_blu_sqrs.png", "start": 63797735, "end": 63801489}, {"filename": "/GameData/textures/lq_metal/met_blu_stile.png", "start": 63801489, "end": 63804716}, {"filename": "/GameData/textures/lq_metal/met_blu_tile.png", "start": 63804716, "end": 63808165}, {"filename": "/GameData/textures/lq_metal/met_blu_trim16.png", "start": 63808165, "end": 63811869}, {"filename": "/GameData/textures/lq_metal/met_blu_trim16g.png", "start": 63811869, "end": 63815619}, {"filename": "/GameData/textures/lq_metal/met_blu_trim16h.png", "start": 63815619, "end": 63819236}, {"filename": "/GameData/textures/lq_metal/met_blu_trim16s.png", "start": 63819236, "end": 63822404}, {"filename": "/GameData/textures/lq_metal/met_blu_trim28.png", "start": 63822404, "end": 63827701}, {"filename": "/GameData/textures/lq_metal/met_blu_trim32.png", "start": 63827701, "end": 63831151}, {"filename": "/GameData/textures/lq_metal/met_blu_trim32r.png", "start": 63831151, "end": 63834807}, {"filename": "/GameData/textures/lq_metal/met_blu_trim32s.png", "start": 63834807, "end": 63838131}, {"filename": "/GameData/textures/lq_metal/met_blu_trim64.png", "start": 63838131, "end": 63841608}, {"filename": "/GameData/textures/lq_metal/met_blu_vtrim.png", "start": 63841608, "end": 63845178}, {"filename": "/GameData/textures/lq_metal/met_brn2_pat.png", "start": 63845178, "end": 63849027}, {"filename": "/GameData/textures/lq_metal/met_brn_block.png", "start": 63849027, "end": 63852738}, {"filename": "/GameData/textures/lq_metal/met_brn_blockl.png", "start": 63852738, "end": 63856267}, {"filename": "/GameData/textures/lq_metal/met_brn_det1.png", "start": 63856267, "end": 63857430}, {"filename": "/GameData/textures/lq_metal/met_brn_flat.png", "start": 63857430, "end": 63860951}, {"filename": "/GameData/textures/lq_metal/met_brn_grate.png", "start": 63860951, "end": 63864339}, {"filename": "/GameData/textures/lq_metal/met_brn_grate2.png", "start": 63864339, "end": 63867845}, {"filename": "/GameData/textures/lq_metal/met_brn_grate3.png", "start": 63867845, "end": 63869863}, {"filename": "/GameData/textures/lq_metal/met_brn_lit1_fbr.png", "start": 63869863, "end": 63871976}, {"filename": "/GameData/textures/lq_metal/met_brn_lit2_fbr.png", "start": 63871976, "end": 63873164}, {"filename": "/GameData/textures/lq_metal/met_brn_lit3.png", "start": 63873164, "end": 63874335}, {"filename": "/GameData/textures/lq_metal/met_brn_lit4.png", "start": 63874335, "end": 63875545}, {"filename": "/GameData/textures/lq_metal/met_brn_lit5.png", "start": 63875545, "end": 63877598}, {"filename": "/GameData/textures/lq_metal/met_brn_pan1.png", "start": 63877598, "end": 63881061}, {"filename": "/GameData/textures/lq_metal/met_brn_pan2.png", "start": 63881061, "end": 63883337}, {"filename": "/GameData/textures/lq_metal/met_brn_pan3.png", "start": 63883337, "end": 63886814}, {"filename": "/GameData/textures/lq_metal/met_brn_pan4.png", "start": 63886814, "end": 63890677}, {"filename": "/GameData/textures/lq_metal/met_brn_rect.png", "start": 63890677, "end": 63894355}, {"filename": "/GameData/textures/lq_metal/met_brn_rivg.png", "start": 63894355, "end": 63898383}, {"filename": "/GameData/textures/lq_metal/met_brn_rivs.png", "start": 63898383, "end": 63901806}, {"filename": "/GameData/textures/lq_metal/met_brn_signs.png", "start": 63901806, "end": 63906640}, {"filename": "/GameData/textures/lq_metal/met_brn_slat.png", "start": 63906640, "end": 63910283}, {"filename": "/GameData/textures/lq_metal/met_brn_sqr.png", "start": 63910283, "end": 63913974}, {"filename": "/GameData/textures/lq_metal/met_brn_sqrd.png", "start": 63913974, "end": 63917782}, {"filename": "/GameData/textures/lq_metal/met_brn_sqrs.png", "start": 63917782, "end": 63921486}, {"filename": "/GameData/textures/lq_metal/met_brn_stile.png", "start": 63921486, "end": 63925571}, {"filename": "/GameData/textures/lq_metal/met_brn_tile.png", "start": 63925571, "end": 63928952}, {"filename": "/GameData/textures/lq_metal/met_brn_tile2.png", "start": 63928952, "end": 63932285}, {"filename": "/GameData/textures/lq_metal/met_brn_trim16.png", "start": 63932285, "end": 63936391}, {"filename": "/GameData/textures/lq_metal/met_brn_trim16g.png", "start": 63936391, "end": 63940595}, {"filename": "/GameData/textures/lq_metal/met_brn_trim16h.png", "start": 63940595, "end": 63944792}, {"filename": "/GameData/textures/lq_metal/met_brn_trim16s.png", "start": 63944792, "end": 63948415}, {"filename": "/GameData/textures/lq_metal/met_brn_trim32.png", "start": 63948415, "end": 63951902}, {"filename": "/GameData/textures/lq_metal/met_brn_trim32s.png", "start": 63951902, "end": 63955288}, {"filename": "/GameData/textures/lq_metal/met_brn_trim64.png", "start": 63955288, "end": 63958928}, {"filename": "/GameData/textures/lq_metal/met_brn_vtrim.png", "start": 63958928, "end": 63962381}, {"filename": "/GameData/textures/lq_metal/met_cop_flat.png", "start": 63962381, "end": 63965458}, {"filename": "/GameData/textures/lq_metal/met_cop_riv.png", "start": 63965458, "end": 63969059}, {"filename": "/GameData/textures/lq_metal/met_dbrn_flat.png", "start": 63969059, "end": 63972439}, {"filename": "/GameData/textures/lq_metal/met_dbrn_rect.png", "start": 63972439, "end": 63976324}, {"filename": "/GameData/textures/lq_metal/met_dbrn_slat.png", "start": 63976324, "end": 63980139}, {"filename": "/GameData/textures/lq_metal/met_grate.png", "start": 63980139, "end": 63983489}, {"filename": "/GameData/textures/lq_metal/met_grn_block.png", "start": 63983489, "end": 63987414}, {"filename": "/GameData/textures/lq_metal/met_grn_blockl.png", "start": 63987414, "end": 63991143}, {"filename": "/GameData/textures/lq_metal/met_grn_det1.png", "start": 63991143, "end": 63992394}, {"filename": "/GameData/textures/lq_metal/met_grn_fac1.png", "start": 63992394, "end": 63995304}, {"filename": "/GameData/textures/lq_metal/met_grn_flat.png", "start": 63995304, "end": 63999189}, {"filename": "/GameData/textures/lq_metal/met_grn_grate.png", "start": 63999189, "end": 64002251}, {"filename": "/GameData/textures/lq_metal/met_grn_grate2.png", "start": 64002251, "end": 64005234}, {"filename": "/GameData/textures/lq_metal/met_grn_grate3.png", "start": 64005234, "end": 64006978}, {"filename": "/GameData/textures/lq_metal/met_grn_lit1_fbr.png", "start": 64006978, "end": 64009112}, {"filename": "/GameData/textures/lq_metal/met_grn_lit2_fbr.png", "start": 64009112, "end": 64010313}, {"filename": "/GameData/textures/lq_metal/met_grn_lit3.png", "start": 64010313, "end": 64011525}, {"filename": "/GameData/textures/lq_metal/met_grn_lit4.png", "start": 64011525, "end": 64012811}, {"filename": "/GameData/textures/lq_metal/met_grn_lit5.png", "start": 64012811, "end": 64015003}, {"filename": "/GameData/textures/lq_metal/met_grn_pan1.png", "start": 64015003, "end": 64018719}, {"filename": "/GameData/textures/lq_metal/met_grn_pan2.png", "start": 64018719, "end": 64021084}, {"filename": "/GameData/textures/lq_metal/met_grn_pan3.png", "start": 64021084, "end": 64024684}, {"filename": "/GameData/textures/lq_metal/met_grn_rect.png", "start": 64024684, "end": 64028721}, {"filename": "/GameData/textures/lq_metal/met_grn_rivg.png", "start": 64028721, "end": 64033133}, {"filename": "/GameData/textures/lq_metal/met_grn_rivs.png", "start": 64033133, "end": 64036900}, {"filename": "/GameData/textures/lq_metal/met_grn_slat.png", "start": 64036900, "end": 64040941}, {"filename": "/GameData/textures/lq_metal/met_grn_sqr.png", "start": 64040941, "end": 64044939}, {"filename": "/GameData/textures/lq_metal/met_grn_sqrd.png", "start": 64044939, "end": 64049049}, {"filename": "/GameData/textures/lq_metal/met_grn_sqrs.png", "start": 64049049, "end": 64053079}, {"filename": "/GameData/textures/lq_metal/met_grn_stile.png", "start": 64053079, "end": 64056363}, {"filename": "/GameData/textures/lq_metal/met_grn_tile.png", "start": 64056363, "end": 64059624}, {"filename": "/GameData/textures/lq_metal/met_grn_trim16.png", "start": 64059624, "end": 64063236}, {"filename": "/GameData/textures/lq_metal/met_grn_trim16g.png", "start": 64063236, "end": 64067390}, {"filename": "/GameData/textures/lq_metal/met_grn_trim16h.png", "start": 64067390, "end": 64070939}, {"filename": "/GameData/textures/lq_metal/met_grn_trim16s.png", "start": 64070939, "end": 64074450}, {"filename": "/GameData/textures/lq_metal/met_grn_trim28.png", "start": 64074450, "end": 64079385}, {"filename": "/GameData/textures/lq_metal/met_grn_trim28r.png", "start": 64079385, "end": 64084411}, {"filename": "/GameData/textures/lq_metal/met_grn_trim32.png", "start": 64084411, "end": 64088236}, {"filename": "/GameData/textures/lq_metal/met_grn_trim32r.png", "start": 64088236, "end": 64092106}, {"filename": "/GameData/textures/lq_metal/met_grn_trim32s.png", "start": 64092106, "end": 64095752}, {"filename": "/GameData/textures/lq_metal/met_grn_trim64.png", "start": 64095752, "end": 64099409}, {"filename": "/GameData/textures/lq_metal/met_grn_vtrim.png", "start": 64099409, "end": 64102659}, {"filename": "/GameData/textures/lq_metal/met_gry_beam.png", "start": 64102659, "end": 64105673}, {"filename": "/GameData/textures/lq_metal/met_gry_block.png", "start": 64105673, "end": 64108696}, {"filename": "/GameData/textures/lq_metal/met_gry_flat.png", "start": 64108696, "end": 64111430}, {"filename": "/GameData/textures/lq_metal/met_gry_lit1_fbr.png", "start": 64111430, "end": 64113241}, {"filename": "/GameData/textures/lq_metal/met_gry_lit2_fbr.png", "start": 64113241, "end": 64114364}, {"filename": "/GameData/textures/lq_metal/met_gry_lit2b.png", "start": 64114364, "end": 64117814}, {"filename": "/GameData/textures/lq_metal/met_gry_pan1.png", "start": 64117814, "end": 64120910}, {"filename": "/GameData/textures/lq_metal/met_gry_pan2.png", "start": 64120910, "end": 64123063}, {"filename": "/GameData/textures/lq_metal/met_gry_pan3.png", "start": 64123063, "end": 64126166}, {"filename": "/GameData/textures/lq_metal/met_gry_rect.png", "start": 64126166, "end": 64129363}, {"filename": "/GameData/textures/lq_metal/met_gry_signs.png", "start": 64129363, "end": 64133193}, {"filename": "/GameData/textures/lq_metal/met_gry_slat.png", "start": 64133193, "end": 64136609}, {"filename": "/GameData/textures/lq_metal/met_gry_sqr.png", "start": 64136609, "end": 64139443}, {"filename": "/GameData/textures/lq_metal/met_gry_sqrd.png", "start": 64139443, "end": 64142691}, {"filename": "/GameData/textures/lq_metal/met_gry_sqrs.png", "start": 64142691, "end": 64145652}, {"filename": "/GameData/textures/lq_metal/met_gry_trim64.png", "start": 64145652, "end": 64149353}, {"filename": "/GameData/textures/lq_metal/met_lbrn_flat.png", "start": 64149353, "end": 64152779}, {"filename": "/GameData/textures/lq_metal/met_lbrn_rect.png", "start": 64152779, "end": 64156667}, {"filename": "/GameData/textures/lq_metal/met_lbrn_slat.png", "start": 64156667, "end": 64160621}, {"filename": "/GameData/textures/lq_metal/met_lift.png", "start": 64160621, "end": 64163666}, {"filename": "/GameData/textures/lq_metal/met_met7_1.png", "start": 64163666, "end": 64166139}, {"filename": "/GameData/textures/lq_metal/met_mix_beam.png", "start": 64166139, "end": 64169791}, {"filename": "/GameData/textures/lq_metal/met_mix_diam.png", "start": 64169791, "end": 64172980}, {"filename": "/GameData/textures/lq_metal/met_mix_diam2.png", "start": 64172980, "end": 64176179}, {"filename": "/GameData/textures/lq_metal/met_mix_diamc.png", "start": 64176179, "end": 64180279}, {"filename": "/GameData/textures/lq_metal/met_mt1_flat.png", "start": 64180279, "end": 64194569}, {"filename": "/GameData/textures/lq_metal/met_mt1_rect.png", "start": 64194569, "end": 64210279}, {"filename": "/GameData/textures/lq_metal/met_mt1_slat.png", "start": 64210279, "end": 64226035}, {"filename": "/GameData/textures/lq_metal/met_mt1_sqr.png", "start": 64226035, "end": 64232575}, {"filename": "/GameData/textures/lq_metal/met_mt2_flat.png", "start": 64232575, "end": 64248789}, {"filename": "/GameData/textures/lq_metal/met_mt2_rect.png", "start": 64248789, "end": 64265515}, {"filename": "/GameData/textures/lq_metal/met_mt2_slat.png", "start": 64265515, "end": 64282153}, {"filename": "/GameData/textures/lq_metal/met_mt2_sqr.png", "start": 64282153, "end": 64290551}, {"filename": "/GameData/textures/lq_metal/met_mt3_flat.png", "start": 64290551, "end": 64307151}, {"filename": "/GameData/textures/lq_metal/met_mt3_rect.png", "start": 64307151, "end": 64324059}, {"filename": "/GameData/textures/lq_metal/met_mt3_slat.png", "start": 64324059, "end": 64340970}, {"filename": "/GameData/textures/lq_metal/met_mt3_sqr.png", "start": 64340970, "end": 64349505}, {"filename": "/GameData/textures/lq_metal/met_ora_trim64.png", "start": 64349505, "end": 64352284}, {"filename": "/GameData/textures/lq_metal/met_rail_flat.png", "start": 64352284, "end": 64354431}, {"filename": "/GameData/textures/lq_metal/met_rune1_fbr.png", "start": 64354431, "end": 64357235}, {"filename": "/GameData/textures/lq_metal/met_rune_trim32.png", "start": 64357235, "end": 64361576}, {"filename": "/GameData/textures/lq_metal/met_set1.png", "start": 64361576, "end": 64431421}, {"filename": "/GameData/textures/lq_metal/met_shm_flat.png", "start": 64431421, "end": 64435082}, {"filename": "/GameData/textures/lq_metal/met_shm_rect.png", "start": 64435082, "end": 64438954}, {"filename": "/GameData/textures/lq_metal/met_shm_slat.png", "start": 64438954, "end": 64443005}, {"filename": "/GameData/textures/lq_metal/met_shm_sqr.png", "start": 64443005, "end": 64446467}, {"filename": "/GameData/textures/lq_metal/met_teal_block.png", "start": 64446467, "end": 64449845}, {"filename": "/GameData/textures/lq_metal/met_teal_flat.png", "start": 64449845, "end": 64453191}, {"filename": "/GameData/textures/lq_metal/met_teal_trim32.png", "start": 64453191, "end": 64456467}, {"filename": "/GameData/textures/lq_metal/met_teal_trim32r.png", "start": 64456467, "end": 64459442}, {"filename": "/GameData/textures/lq_metal/met_teal_trim64.png", "start": 64459442, "end": 64462926}, {"filename": "/GameData/textures/lq_metal/met_wall3_1.png", "start": 64462926, "end": 64474192}, {"filename": "/GameData/textures/lq_metal/met_wall3_1_s.png", "start": 64474192, "end": 64477476}, {"filename": "/GameData/textures/lq_metal/metal4_4.png", "start": 64477476, "end": 64481817}, {"filename": "/GameData/textures/lq_metal/plus_0_sqbut1.png", "start": 64481817, "end": 64482899}, {"filename": "/GameData/textures/lq_metal/plus_0_sqbut2_fbr.png", "start": 64482899, "end": 64486135}, {"filename": "/GameData/textures/lq_metal/plus_0_sqshoot1_fbr.png", "start": 64486135, "end": 64487242}, {"filename": "/GameData/textures/lq_metal/plus_0gig2a_fbr.png", "start": 64487242, "end": 64487715}, {"filename": "/GameData/textures/lq_metal/plus_0gig_shot_fbr.png", "start": 64487715, "end": 64488191}, {"filename": "/GameData/textures/lq_metal/plus_0gig_sshot_fbr.png", "start": 64488191, "end": 64488635}, {"filename": "/GameData/textures/lq_metal/plus_0gig_ye_fbr.png", "start": 64488635, "end": 64489075}, {"filename": "/GameData/textures/lq_metal/plus_0met_blu_keyg_fbr.png", "start": 64489075, "end": 64489975}, {"filename": "/GameData/textures/lq_metal/plus_0met_blu_keys_fbr.png", "start": 64489975, "end": 64490858}, {"filename": "/GameData/textures/lq_metal/plus_1_sqbut1.png", "start": 64490858, "end": 64492024}, {"filename": "/GameData/textures/lq_metal/plus_1_sqbut2_fbr.png", "start": 64492024, "end": 64495212}, {"filename": "/GameData/textures/lq_metal/plus_1_sqshoot1.png", "start": 64495212, "end": 64496326}, {"filename": "/GameData/textures/lq_metal/plus_1met_blu_keyg_fbr.png", "start": 64496326, "end": 64497227}, {"filename": "/GameData/textures/lq_metal/plus_1met_blu_keys_fbr.png", "start": 64497227, "end": 64498104}, {"filename": "/GameData/textures/lq_metal/plus_2met_blu_keyg_fbr.png", "start": 64498104, "end": 64499013}, {"filename": "/GameData/textures/lq_metal/plus_2met_blu_keys_fbr.png", "start": 64499013, "end": 64499882}, {"filename": "/GameData/textures/lq_metal/plus_3met_blu_keyg_fbr.png", "start": 64499882, "end": 64500790}, {"filename": "/GameData/textures/lq_metal/plus_3met_blu_keys_fbr.png", "start": 64500790, "end": 64501646}, {"filename": "/GameData/textures/lq_metal/plus_4met_blu_keyg_fbr.png", "start": 64501646, "end": 64502554}, {"filename": "/GameData/textures/lq_metal/plus_4met_blu_keys_fbr.png", "start": 64502554, "end": 64503410}, {"filename": "/GameData/textures/lq_metal/plus_5met_blu_keyg_fbr.png", "start": 64503410, "end": 64504319}, {"filename": "/GameData/textures/lq_metal/plus_5met_blu_keys_fbr.png", "start": 64504319, "end": 64505188}, {"filename": "/GameData/textures/lq_metal/plus_6met_blu_keyg_fbr.png", "start": 64505188, "end": 64506089}, {"filename": "/GameData/textures/lq_metal/plus_6met_blu_keys_fbr.png", "start": 64506089, "end": 64506966}, {"filename": "/GameData/textures/lq_metal/plus_a_sqbut1.png", "start": 64506966, "end": 64508048}, {"filename": "/GameData/textures/lq_metal/plus_a_sqbut2_fbr.png", "start": 64508048, "end": 64511284}, {"filename": "/GameData/textures/lq_metal/plus_a_sqshoot1_fbr.png", "start": 64511284, "end": 64512391}, {"filename": "/GameData/textures/lq_metal/plus_agig2a.png", "start": 64512391, "end": 64512761}, {"filename": "/GameData/textures/lq_metal/plus_agig_shot_fbr.png", "start": 64512761, "end": 64513136}, {"filename": "/GameData/textures/lq_metal/plus_agig_sshot_fbr.png", "start": 64513136, "end": 64513499}, {"filename": "/GameData/textures/lq_metal/plus_agig_ye.png", "start": 64513499, "end": 64513911}, {"filename": "/GameData/textures/lq_metal/plus_amet_blu_keyg.png", "start": 64513911, "end": 64514781}, {"filename": "/GameData/textures/lq_metal/plus_amet_blu_keys.png", "start": 64514781, "end": 64515651}, {"filename": "/GameData/textures/lq_metal/ret_metal1_tile.png", "start": 64515651, "end": 64529583}, {"filename": "/GameData/textures/lq_metal/sq_lit1_fbr.png", "start": 64529583, "end": 64529956}, {"filename": "/GameData/textures/lq_metal/sq_lit2_fbr.png", "start": 64529956, "end": 64530217}, {"filename": "/GameData/textures/lq_palette/flat_01_a.png", "start": 64530217, "end": 64530765}, {"filename": "/GameData/textures/lq_palette/flat_01_b.png", "start": 64530765, "end": 64531291}, {"filename": "/GameData/textures/lq_palette/flat_01_c.png", "start": 64531291, "end": 64531817}, {"filename": "/GameData/textures/lq_palette/flat_01_d.png", "start": 64531817, "end": 64532343}, {"filename": "/GameData/textures/lq_palette/flat_01_e.png", "start": 64532343, "end": 64532869}, {"filename": "/GameData/textures/lq_palette/flat_01_f.png", "start": 64532869, "end": 64533395}, {"filename": "/GameData/textures/lq_palette/flat_01_g.png", "start": 64533395, "end": 64533921}, {"filename": "/GameData/textures/lq_palette/flat_01_h.png", "start": 64533921, "end": 64534447}, {"filename": "/GameData/textures/lq_palette/flat_01_i.png", "start": 64534447, "end": 64534974}, {"filename": "/GameData/textures/lq_palette/flat_01_j.png", "start": 64534974, "end": 64535501}, {"filename": "/GameData/textures/lq_palette/flat_01_k.png", "start": 64535501, "end": 64536028}, {"filename": "/GameData/textures/lq_palette/flat_01_l.png", "start": 64536028, "end": 64536555}, {"filename": "/GameData/textures/lq_palette/flat_01_m.png", "start": 64536555, "end": 64537082}, {"filename": "/GameData/textures/lq_palette/flat_01_n.png", "start": 64537082, "end": 64537607}, {"filename": "/GameData/textures/lq_palette/flat_01_o.png", "start": 64537607, "end": 64538132}, {"filename": "/GameData/textures/lq_palette/flat_01_p.png", "start": 64538132, "end": 64538657}, {"filename": "/GameData/textures/lq_palette/flat_02_a.png", "start": 64538657, "end": 64539183}, {"filename": "/GameData/textures/lq_palette/flat_02_b.png", "start": 64539183, "end": 64539709}, {"filename": "/GameData/textures/lq_palette/flat_02_c.png", "start": 64539709, "end": 64540235}, {"filename": "/GameData/textures/lq_palette/flat_02_d.png", "start": 64540235, "end": 64540761}, {"filename": "/GameData/textures/lq_palette/flat_02_e.png", "start": 64540761, "end": 64541287}, {"filename": "/GameData/textures/lq_palette/flat_02_f.png", "start": 64541287, "end": 64541813}, {"filename": "/GameData/textures/lq_palette/flat_02_g.png", "start": 64541813, "end": 64542339}, {"filename": "/GameData/textures/lq_palette/flat_02_h.png", "start": 64542339, "end": 64542865}, {"filename": "/GameData/textures/lq_palette/flat_02_i.png", "start": 64542865, "end": 64543391}, {"filename": "/GameData/textures/lq_palette/flat_02_j.png", "start": 64543391, "end": 64543917}, {"filename": "/GameData/textures/lq_palette/flat_02_k.png", "start": 64543917, "end": 64544443}, {"filename": "/GameData/textures/lq_palette/flat_02_l.png", "start": 64544443, "end": 64544969}, {"filename": "/GameData/textures/lq_palette/flat_02_m.png", "start": 64544969, "end": 64545495}, {"filename": "/GameData/textures/lq_palette/flat_02_n.png", "start": 64545495, "end": 64546021}, {"filename": "/GameData/textures/lq_palette/flat_02_o.png", "start": 64546021, "end": 64546547}, {"filename": "/GameData/textures/lq_palette/flat_02_p.png", "start": 64546547, "end": 64547073}, {"filename": "/GameData/textures/lq_palette/flat_03_a.png", "start": 64547073, "end": 64547599}, {"filename": "/GameData/textures/lq_palette/flat_03_b.png", "start": 64547599, "end": 64548125}, {"filename": "/GameData/textures/lq_palette/flat_03_c.png", "start": 64548125, "end": 64548651}, {"filename": "/GameData/textures/lq_palette/flat_03_d.png", "start": 64548651, "end": 64549177}, {"filename": "/GameData/textures/lq_palette/flat_03_e.png", "start": 64549177, "end": 64549703}, {"filename": "/GameData/textures/lq_palette/flat_03_f.png", "start": 64549703, "end": 64550229}, {"filename": "/GameData/textures/lq_palette/flat_03_g.png", "start": 64550229, "end": 64550755}, {"filename": "/GameData/textures/lq_palette/flat_03_h.png", "start": 64550755, "end": 64551281}, {"filename": "/GameData/textures/lq_palette/flat_03_i.png", "start": 64551281, "end": 64551807}, {"filename": "/GameData/textures/lq_palette/flat_03_j.png", "start": 64551807, "end": 64552333}, {"filename": "/GameData/textures/lq_palette/flat_03_k.png", "start": 64552333, "end": 64552859}, {"filename": "/GameData/textures/lq_palette/flat_03_l.png", "start": 64552859, "end": 64553385}, {"filename": "/GameData/textures/lq_palette/flat_03_m.png", "start": 64553385, "end": 64553912}, {"filename": "/GameData/textures/lq_palette/flat_03_n.png", "start": 64553912, "end": 64554439}, {"filename": "/GameData/textures/lq_palette/flat_03_o.png", "start": 64554439, "end": 64554966}, {"filename": "/GameData/textures/lq_palette/flat_03_p.png", "start": 64554966, "end": 64555493}, {"filename": "/GameData/textures/lq_palette/flat_04_a.png", "start": 64555493, "end": 64556041}, {"filename": "/GameData/textures/lq_palette/flat_04_b.png", "start": 64556041, "end": 64556567}, {"filename": "/GameData/textures/lq_palette/flat_04_c.png", "start": 64556567, "end": 64557093}, {"filename": "/GameData/textures/lq_palette/flat_04_d.png", "start": 64557093, "end": 64557619}, {"filename": "/GameData/textures/lq_palette/flat_04_e.png", "start": 64557619, "end": 64558145}, {"filename": "/GameData/textures/lq_palette/flat_04_f.png", "start": 64558145, "end": 64558671}, {"filename": "/GameData/textures/lq_palette/flat_04_g.png", "start": 64558671, "end": 64559197}, {"filename": "/GameData/textures/lq_palette/flat_04_h.png", "start": 64559197, "end": 64559723}, {"filename": "/GameData/textures/lq_palette/flat_04_i.png", "start": 64559723, "end": 64560249}, {"filename": "/GameData/textures/lq_palette/flat_04_j.png", "start": 64560249, "end": 64560775}, {"filename": "/GameData/textures/lq_palette/flat_04_k.png", "start": 64560775, "end": 64561301}, {"filename": "/GameData/textures/lq_palette/flat_04_l.png", "start": 64561301, "end": 64561827}, {"filename": "/GameData/textures/lq_palette/flat_04_m.png", "start": 64561827, "end": 64562353}, {"filename": "/GameData/textures/lq_palette/flat_04_n.png", "start": 64562353, "end": 64562879}, {"filename": "/GameData/textures/lq_palette/flat_04_o.png", "start": 64562879, "end": 64563405}, {"filename": "/GameData/textures/lq_palette/flat_04_p.png", "start": 64563405, "end": 64563931}, {"filename": "/GameData/textures/lq_palette/flat_05_a.png", "start": 64563931, "end": 64564457}, {"filename": "/GameData/textures/lq_palette/flat_05_b.png", "start": 64564457, "end": 64564983}, {"filename": "/GameData/textures/lq_palette/flat_05_c.png", "start": 64564983, "end": 64565509}, {"filename": "/GameData/textures/lq_palette/flat_05_d.png", "start": 64565509, "end": 64566035}, {"filename": "/GameData/textures/lq_palette/flat_05_e.png", "start": 64566035, "end": 64566561}, {"filename": "/GameData/textures/lq_palette/flat_05_f.png", "start": 64566561, "end": 64567087}, {"filename": "/GameData/textures/lq_palette/flat_05_g.png", "start": 64567087, "end": 64567613}, {"filename": "/GameData/textures/lq_palette/flat_05_h.png", "start": 64567613, "end": 64568139}, {"filename": "/GameData/textures/lq_palette/flat_05_i.png", "start": 64568139, "end": 64568665}, {"filename": "/GameData/textures/lq_palette/flat_05_j.png", "start": 64568665, "end": 64569191}, {"filename": "/GameData/textures/lq_palette/flat_05_k.png", "start": 64569191, "end": 64569717}, {"filename": "/GameData/textures/lq_palette/flat_05_l.png", "start": 64569717, "end": 64570243}, {"filename": "/GameData/textures/lq_palette/flat_05_m.png", "start": 64570243, "end": 64570769}, {"filename": "/GameData/textures/lq_palette/flat_05_n.png", "start": 64570769, "end": 64571295}, {"filename": "/GameData/textures/lq_palette/flat_05_o.png", "start": 64571295, "end": 64571821}, {"filename": "/GameData/textures/lq_palette/flat_05_p.png", "start": 64571821, "end": 64572347}, {"filename": "/GameData/textures/lq_palette/flat_06_a.png", "start": 64572347, "end": 64572873}, {"filename": "/GameData/textures/lq_palette/flat_06_b.png", "start": 64572873, "end": 64573399}, {"filename": "/GameData/textures/lq_palette/flat_06_c.png", "start": 64573399, "end": 64573925}, {"filename": "/GameData/textures/lq_palette/flat_06_d.png", "start": 64573925, "end": 64574451}, {"filename": "/GameData/textures/lq_palette/flat_06_e.png", "start": 64574451, "end": 64574977}, {"filename": "/GameData/textures/lq_palette/flat_06_f.png", "start": 64574977, "end": 64575503}, {"filename": "/GameData/textures/lq_palette/flat_06_g.png", "start": 64575503, "end": 64576029}, {"filename": "/GameData/textures/lq_palette/flat_06_h.png", "start": 64576029, "end": 64576555}, {"filename": "/GameData/textures/lq_palette/flat_06_i.png", "start": 64576555, "end": 64577081}, {"filename": "/GameData/textures/lq_palette/flat_06_j.png", "start": 64577081, "end": 64577607}, {"filename": "/GameData/textures/lq_palette/flat_06_k.png", "start": 64577607, "end": 64578133}, {"filename": "/GameData/textures/lq_palette/flat_06_l.png", "start": 64578133, "end": 64578659}, {"filename": "/GameData/textures/lq_palette/flat_06_m.png", "start": 64578659, "end": 64579185}, {"filename": "/GameData/textures/lq_palette/flat_06_n.png", "start": 64579185, "end": 64579711}, {"filename": "/GameData/textures/lq_palette/flat_06_o.png", "start": 64579711, "end": 64580237}, {"filename": "/GameData/textures/lq_palette/flat_06_p.png", "start": 64580237, "end": 64580763}, {"filename": "/GameData/textures/lq_palette/flat_07_a.png", "start": 64580763, "end": 64581289}, {"filename": "/GameData/textures/lq_palette/flat_07_b.png", "start": 64581289, "end": 64581815}, {"filename": "/GameData/textures/lq_palette/flat_07_c.png", "start": 64581815, "end": 64582341}, {"filename": "/GameData/textures/lq_palette/flat_07_d.png", "start": 64582341, "end": 64582867}, {"filename": "/GameData/textures/lq_palette/flat_07_e.png", "start": 64582867, "end": 64583393}, {"filename": "/GameData/textures/lq_palette/flat_07_f.png", "start": 64583393, "end": 64583919}, {"filename": "/GameData/textures/lq_palette/flat_07_g.png", "start": 64583919, "end": 64584445}, {"filename": "/GameData/textures/lq_palette/flat_07_h.png", "start": 64584445, "end": 64584971}, {"filename": "/GameData/textures/lq_palette/flat_07_i.png", "start": 64584971, "end": 64585497}, {"filename": "/GameData/textures/lq_palette/flat_07_j.png", "start": 64585497, "end": 64586023}, {"filename": "/GameData/textures/lq_palette/flat_07_k.png", "start": 64586023, "end": 64586549}, {"filename": "/GameData/textures/lq_palette/flat_07_l.png", "start": 64586549, "end": 64587075}, {"filename": "/GameData/textures/lq_palette/flat_07_m.png", "start": 64587075, "end": 64587601}, {"filename": "/GameData/textures/lq_palette/flat_07_n.png", "start": 64587601, "end": 64588127}, {"filename": "/GameData/textures/lq_palette/flat_07_o.png", "start": 64588127, "end": 64588653}, {"filename": "/GameData/textures/lq_palette/flat_07_p.png", "start": 64588653, "end": 64589179}, {"filename": "/GameData/textures/lq_palette/flat_08_a.png", "start": 64589179, "end": 64589705}, {"filename": "/GameData/textures/lq_palette/flat_08_b.png", "start": 64589705, "end": 64590231}, {"filename": "/GameData/textures/lq_palette/flat_08_c.png", "start": 64590231, "end": 64590757}, {"filename": "/GameData/textures/lq_palette/flat_08_d.png", "start": 64590757, "end": 64591283}, {"filename": "/GameData/textures/lq_palette/flat_08_e.png", "start": 64591283, "end": 64591809}, {"filename": "/GameData/textures/lq_palette/flat_08_f.png", "start": 64591809, "end": 64592335}, {"filename": "/GameData/textures/lq_palette/flat_08_g.png", "start": 64592335, "end": 64592861}, {"filename": "/GameData/textures/lq_palette/flat_08_h.png", "start": 64592861, "end": 64593387}, {"filename": "/GameData/textures/lq_palette/flat_08_i.png", "start": 64593387, "end": 64593913}, {"filename": "/GameData/textures/lq_palette/flat_08_j.png", "start": 64593913, "end": 64594439}, {"filename": "/GameData/textures/lq_palette/flat_08_k.png", "start": 64594439, "end": 64594965}, {"filename": "/GameData/textures/lq_palette/flat_08_l.png", "start": 64594965, "end": 64595491}, {"filename": "/GameData/textures/lq_palette/flat_08_m.png", "start": 64595491, "end": 64596017}, {"filename": "/GameData/textures/lq_palette/flat_08_n.png", "start": 64596017, "end": 64596544}, {"filename": "/GameData/textures/lq_palette/flat_08_o.png", "start": 64596544, "end": 64597071}, {"filename": "/GameData/textures/lq_palette/flat_08_p.png", "start": 64597071, "end": 64597598}, {"filename": "/GameData/textures/lq_palette/flat_09_a.png", "start": 64597598, "end": 64598125}, {"filename": "/GameData/textures/lq_palette/flat_09_b.png", "start": 64598125, "end": 64598652}, {"filename": "/GameData/textures/lq_palette/flat_09_c.png", "start": 64598652, "end": 64599179}, {"filename": "/GameData/textures/lq_palette/flat_09_d.png", "start": 64599179, "end": 64599705}, {"filename": "/GameData/textures/lq_palette/flat_09_e.png", "start": 64599705, "end": 64600231}, {"filename": "/GameData/textures/lq_palette/flat_09_f.png", "start": 64600231, "end": 64600757}, {"filename": "/GameData/textures/lq_palette/flat_09_g.png", "start": 64600757, "end": 64601283}, {"filename": "/GameData/textures/lq_palette/flat_09_h.png", "start": 64601283, "end": 64601809}, {"filename": "/GameData/textures/lq_palette/flat_09_i.png", "start": 64601809, "end": 64602335}, {"filename": "/GameData/textures/lq_palette/flat_09_j.png", "start": 64602335, "end": 64602861}, {"filename": "/GameData/textures/lq_palette/flat_09_k.png", "start": 64602861, "end": 64603387}, {"filename": "/GameData/textures/lq_palette/flat_09_l.png", "start": 64603387, "end": 64603913}, {"filename": "/GameData/textures/lq_palette/flat_09_m.png", "start": 64603913, "end": 64604439}, {"filename": "/GameData/textures/lq_palette/flat_09_n.png", "start": 64604439, "end": 64604965}, {"filename": "/GameData/textures/lq_palette/flat_09_o.png", "start": 64604965, "end": 64605491}, {"filename": "/GameData/textures/lq_palette/flat_09_p.png", "start": 64605491, "end": 64606017}, {"filename": "/GameData/textures/lq_palette/flat_10_a.png", "start": 64606017, "end": 64606544}, {"filename": "/GameData/textures/lq_palette/flat_10_b.png", "start": 64606544, "end": 64607070}, {"filename": "/GameData/textures/lq_palette/flat_10_c.png", "start": 64607070, "end": 64607596}, {"filename": "/GameData/textures/lq_palette/flat_10_d.png", "start": 64607596, "end": 64608122}, {"filename": "/GameData/textures/lq_palette/flat_10_e.png", "start": 64608122, "end": 64608648}, {"filename": "/GameData/textures/lq_palette/flat_10_f.png", "start": 64608648, "end": 64609174}, {"filename": "/GameData/textures/lq_palette/flat_10_g.png", "start": 64609174, "end": 64609700}, {"filename": "/GameData/textures/lq_palette/flat_10_h.png", "start": 64609700, "end": 64610226}, {"filename": "/GameData/textures/lq_palette/flat_10_i.png", "start": 64610226, "end": 64610752}, {"filename": "/GameData/textures/lq_palette/flat_10_j.png", "start": 64610752, "end": 64611278}, {"filename": "/GameData/textures/lq_palette/flat_10_k.png", "start": 64611278, "end": 64611804}, {"filename": "/GameData/textures/lq_palette/flat_10_l.png", "start": 64611804, "end": 64612330}, {"filename": "/GameData/textures/lq_palette/flat_10_m.png", "start": 64612330, "end": 64612856}, {"filename": "/GameData/textures/lq_palette/flat_10_n.png", "start": 64612856, "end": 64613382}, {"filename": "/GameData/textures/lq_palette/flat_10_o.png", "start": 64613382, "end": 64613908}, {"filename": "/GameData/textures/lq_palette/flat_10_p.png", "start": 64613908, "end": 64614434}, {"filename": "/GameData/textures/lq_palette/flat_11_a.png", "start": 64614434, "end": 64614961}, {"filename": "/GameData/textures/lq_palette/flat_11_b.png", "start": 64614961, "end": 64615488}, {"filename": "/GameData/textures/lq_palette/flat_11_c.png", "start": 64615488, "end": 64616015}, {"filename": "/GameData/textures/lq_palette/flat_11_d.png", "start": 64616015, "end": 64616542}, {"filename": "/GameData/textures/lq_palette/flat_11_e.png", "start": 64616542, "end": 64617069}, {"filename": "/GameData/textures/lq_palette/flat_11_f.png", "start": 64617069, "end": 64617595}, {"filename": "/GameData/textures/lq_palette/flat_11_g.png", "start": 64617595, "end": 64618121}, {"filename": "/GameData/textures/lq_palette/flat_11_h.png", "start": 64618121, "end": 64618647}, {"filename": "/GameData/textures/lq_palette/flat_11_i.png", "start": 64618647, "end": 64619173}, {"filename": "/GameData/textures/lq_palette/flat_11_j.png", "start": 64619173, "end": 64619699}, {"filename": "/GameData/textures/lq_palette/flat_11_k.png", "start": 64619699, "end": 64620225}, {"filename": "/GameData/textures/lq_palette/flat_11_l.png", "start": 64620225, "end": 64620751}, {"filename": "/GameData/textures/lq_palette/flat_11_m.png", "start": 64620751, "end": 64621277}, {"filename": "/GameData/textures/lq_palette/flat_11_n.png", "start": 64621277, "end": 64621803}, {"filename": "/GameData/textures/lq_palette/flat_11_o.png", "start": 64621803, "end": 64622329}, {"filename": "/GameData/textures/lq_palette/flat_11_p.png", "start": 64622329, "end": 64622855}, {"filename": "/GameData/textures/lq_palette/flat_12_a.png", "start": 64622855, "end": 64623381}, {"filename": "/GameData/textures/lq_palette/flat_12_b.png", "start": 64623381, "end": 64623907}, {"filename": "/GameData/textures/lq_palette/flat_12_c.png", "start": 64623907, "end": 64624433}, {"filename": "/GameData/textures/lq_palette/flat_12_d.png", "start": 64624433, "end": 64624959}, {"filename": "/GameData/textures/lq_palette/flat_12_e.png", "start": 64624959, "end": 64625485}, {"filename": "/GameData/textures/lq_palette/flat_12_f.png", "start": 64625485, "end": 64626011}, {"filename": "/GameData/textures/lq_palette/flat_12_g.png", "start": 64626011, "end": 64626537}, {"filename": "/GameData/textures/lq_palette/flat_12_h.png", "start": 64626537, "end": 64627063}, {"filename": "/GameData/textures/lq_palette/flat_12_i.png", "start": 64627063, "end": 64627589}, {"filename": "/GameData/textures/lq_palette/flat_12_j.png", "start": 64627589, "end": 64628115}, {"filename": "/GameData/textures/lq_palette/flat_12_k.png", "start": 64628115, "end": 64628641}, {"filename": "/GameData/textures/lq_palette/flat_12_l.png", "start": 64628641, "end": 64629167}, {"filename": "/GameData/textures/lq_palette/flat_12_m.png", "start": 64629167, "end": 64629693}, {"filename": "/GameData/textures/lq_palette/flat_12_n.png", "start": 64629693, "end": 64630219}, {"filename": "/GameData/textures/lq_palette/flat_12_o.png", "start": 64630219, "end": 64630745}, {"filename": "/GameData/textures/lq_palette/flat_12_p.png", "start": 64630745, "end": 64631271}, {"filename": "/GameData/textures/lq_palette/flat_13_a.png", "start": 64631271, "end": 64631797}, {"filename": "/GameData/textures/lq_palette/flat_13_b.png", "start": 64631797, "end": 64632323}, {"filename": "/GameData/textures/lq_palette/flat_13_c.png", "start": 64632323, "end": 64632849}, {"filename": "/GameData/textures/lq_palette/flat_13_d.png", "start": 64632849, "end": 64633375}, {"filename": "/GameData/textures/lq_palette/flat_13_e.png", "start": 64633375, "end": 64633901}, {"filename": "/GameData/textures/lq_palette/flat_13_f.png", "start": 64633901, "end": 64634427}, {"filename": "/GameData/textures/lq_palette/flat_13_g.png", "start": 64634427, "end": 64634953}, {"filename": "/GameData/textures/lq_palette/flat_13_h.png", "start": 64634953, "end": 64635479}, {"filename": "/GameData/textures/lq_palette/flat_13_i.png", "start": 64635479, "end": 64636005}, {"filename": "/GameData/textures/lq_palette/flat_13_j.png", "start": 64636005, "end": 64636531}, {"filename": "/GameData/textures/lq_palette/flat_13_k.png", "start": 64636531, "end": 64637057}, {"filename": "/GameData/textures/lq_palette/flat_13_l.png", "start": 64637057, "end": 64637583}, {"filename": "/GameData/textures/lq_palette/flat_13_m.png", "start": 64637583, "end": 64638109}, {"filename": "/GameData/textures/lq_palette/flat_13_n.png", "start": 64638109, "end": 64638635}, {"filename": "/GameData/textures/lq_palette/flat_13_o.png", "start": 64638635, "end": 64639161}, {"filename": "/GameData/textures/lq_palette/flat_13_p.png", "start": 64639161, "end": 64639687}, {"filename": "/GameData/textures/lq_palette/flat_14_a.png", "start": 64639687, "end": 64640235}, {"filename": "/GameData/textures/lq_palette/flat_14_b.png", "start": 64640235, "end": 64640761}, {"filename": "/GameData/textures/lq_palette/flat_14_c.png", "start": 64640761, "end": 64641287}, {"filename": "/GameData/textures/lq_palette/flat_14_d.png", "start": 64641287, "end": 64641813}, {"filename": "/GameData/textures/lq_palette/flat_14_e.png", "start": 64641813, "end": 64642339}, {"filename": "/GameData/textures/lq_palette/flat_14_f.png", "start": 64642339, "end": 64642865}, {"filename": "/GameData/textures/lq_palette/flat_14_g.png", "start": 64642865, "end": 64643391}, {"filename": "/GameData/textures/lq_palette/flat_14_h.png", "start": 64643391, "end": 64643917}, {"filename": "/GameData/textures/lq_palette/flat_14_i.png", "start": 64643917, "end": 64644443}, {"filename": "/GameData/textures/lq_palette/flat_14_j.png", "start": 64644443, "end": 64644969}, {"filename": "/GameData/textures/lq_palette/flat_14_k.png", "start": 64644969, "end": 64645495}, {"filename": "/GameData/textures/lq_palette/flat_14_l.png", "start": 64645495, "end": 64646021}, {"filename": "/GameData/textures/lq_palette/flat_14_m.png", "start": 64646021, "end": 64646547}, {"filename": "/GameData/textures/lq_palette/flat_14_n.png", "start": 64646547, "end": 64647073}, {"filename": "/GameData/textures/lq_palette/flat_14_o.png", "start": 64647073, "end": 64647599}, {"filename": "/GameData/textures/lq_palette/flat_14_p.png", "start": 64647599, "end": 64648125}, {"filename": "/GameData/textures/lq_palette/flat_15_a_fbr.png", "start": 64648125, "end": 64648651}, {"filename": "/GameData/textures/lq_palette/flat_15_b_fbr.png", "start": 64648651, "end": 64649177}, {"filename": "/GameData/textures/lq_palette/flat_15_c_fbr.png", "start": 64649177, "end": 64649703}, {"filename": "/GameData/textures/lq_palette/flat_15_d_fbr.png", "start": 64649703, "end": 64650229}, {"filename": "/GameData/textures/lq_palette/flat_15_e_fbr.png", "start": 64650229, "end": 64650755}, {"filename": "/GameData/textures/lq_palette/flat_15_f_fbr.png", "start": 64650755, "end": 64651281}, {"filename": "/GameData/textures/lq_palette/flat_15_g_fbr.png", "start": 64651281, "end": 64651807}, {"filename": "/GameData/textures/lq_palette/flat_15_h_fbr.png", "start": 64651807, "end": 64652333}, {"filename": "/GameData/textures/lq_palette/flat_15_i_fbr.png", "start": 64652333, "end": 64652859}, {"filename": "/GameData/textures/lq_palette/flat_15_j_fbr.png", "start": 64652859, "end": 64653385}, {"filename": "/GameData/textures/lq_palette/flat_15_k_fbr.png", "start": 64653385, "end": 64653911}, {"filename": "/GameData/textures/lq_palette/flat_15_l_fbr.png", "start": 64653911, "end": 64654437}, {"filename": "/GameData/textures/lq_palette/flat_15_m_fbr.png", "start": 64654437, "end": 64654963}, {"filename": "/GameData/textures/lq_palette/flat_15_n_fbr.png", "start": 64654963, "end": 64655489}, {"filename": "/GameData/textures/lq_palette/flat_15_o_fbr.png", "start": 64655489, "end": 64656016}, {"filename": "/GameData/textures/lq_palette/flat_15_p_fbr.png", "start": 64656016, "end": 64656543}, {"filename": "/GameData/textures/lq_palette/flat_16_a_fbr.png", "start": 64656543, "end": 64657069}, {"filename": "/GameData/textures/lq_palette/flat_16_b_fbr.png", "start": 64657069, "end": 64657595}, {"filename": "/GameData/textures/lq_palette/flat_16_c_fbr.png", "start": 64657595, "end": 64658121}, {"filename": "/GameData/textures/lq_palette/flat_16_d_fbr.png", "start": 64658121, "end": 64658647}, {"filename": "/GameData/textures/lq_palette/flat_16_e_fbr.png", "start": 64658647, "end": 64659174}, {"filename": "/GameData/textures/lq_palette/flat_16_f_fbr.png", "start": 64659174, "end": 64659701}, {"filename": "/GameData/textures/lq_palette/flat_16_g_fbr.png", "start": 64659701, "end": 64660226}, {"filename": "/GameData/textures/lq_palette/flat_16_h_fbr.png", "start": 64660226, "end": 64660752}, {"filename": "/GameData/textures/lq_palette/flat_16_i_fbr.png", "start": 64660752, "end": 64661278}, {"filename": "/GameData/textures/lq_palette/flat_16_j_fbr.png", "start": 64661278, "end": 64661804}, {"filename": "/GameData/textures/lq_palette/flat_16_k_fbr.png", "start": 64661804, "end": 64662330}, {"filename": "/GameData/textures/lq_palette/flat_16_l_fbr.png", "start": 64662330, "end": 64662856}, {"filename": "/GameData/textures/lq_palette/flat_16_m_fbr.png", "start": 64662856, "end": 64663383}, {"filename": "/GameData/textures/lq_palette/flat_16_n_fbr.png", "start": 64663383, "end": 64663910}, {"filename": "/GameData/textures/lq_palette/flat_16_o_fbr.png", "start": 64663910, "end": 64664435}, {"filename": "/GameData/textures/lq_palette/flat_16_p_fbr.png", "start": 64664435, "end": 64664961}, {"filename": "/GameData/textures/lq_props/JarBod1.png", "start": 64664961, "end": 64665308}, {"filename": "/GameData/textures/lq_props/JarBod2.png", "start": 64665308, "end": 64665626}, {"filename": "/GameData/textures/lq_props/JarTop1.png", "start": 64665626, "end": 64665851}, {"filename": "/GameData/textures/lq_props/JarTop2.png", "start": 64665851, "end": 64665991}, {"filename": "/GameData/textures/lq_props/crate-door-brn.png", "start": 64665991, "end": 64674629}, {"filename": "/GameData/textures/lq_props/crate-door-grn.png", "start": 64674629, "end": 64684685}, {"filename": "/GameData/textures/lq_props/crate-door-orn.png", "start": 64684685, "end": 64693784}, {"filename": "/GameData/textures/lq_props/crate-side-brn.png", "start": 64693784, "end": 64713910}, {"filename": "/GameData/textures/lq_props/crate-side-grn.png", "start": 64713910, "end": 64736823}, {"filename": "/GameData/textures/lq_props/crate-side-orn.png", "start": 64736823, "end": 64758152}, {"filename": "/GameData/textures/lq_props/go-ep0_fbr.png", "start": 64758152, "end": 64759927}, {"filename": "/GameData/textures/lq_props/med_book_blue.png", "start": 64759927, "end": 64760507}, {"filename": "/GameData/textures/lq_props/med_book_green.png", "start": 64760507, "end": 64761130}, {"filename": "/GameData/textures/lq_props/med_book_pink.png", "start": 64761130, "end": 64761785}, {"filename": "/GameData/textures/lq_props/med_book_red.png", "start": 64761785, "end": 64762502}, {"filename": "/GameData/textures/lq_props/med_book_teal.png", "start": 64762502, "end": 64763089}, {"filename": "/GameData/textures/lq_props/med_books_wood.png", "start": 64763089, "end": 64776019}, {"filename": "/GameData/textures/lq_props/med_dbrick4_p1.png", "start": 64776019, "end": 64807856}, {"filename": "/GameData/textures/lq_props/med_dbrick4_p2.png", "start": 64807856, "end": 64841277}, {"filename": "/GameData/textures/lq_props/med_ebrick9_p1.png", "start": 64841277, "end": 64873168}, {"filename": "/GameData/textures/lq_props/med_ebrick9_p2.png", "start": 64873168, "end": 64904534}, {"filename": "/GameData/textures/lq_props/note-e0_fbr.png", "start": 64904534, "end": 64923963}, {"filename": "/GameData/textures/lq_props/plus_0blink_fbr.png", "start": 64923963, "end": 64924192}, {"filename": "/GameData/textures/lq_props/plus_0tvnoise.png", "start": 64924192, "end": 64925178}, {"filename": "/GameData/textures/lq_props/plus_1blink_fbr.png", "start": 64925178, "end": 64925407}, {"filename": "/GameData/textures/lq_props/plus_1tvnoise.png", "start": 64925407, "end": 64926385}, {"filename": "/GameData/textures/lq_props/plus_2blink_fbr.png", "start": 64926385, "end": 64926616}, {"filename": "/GameData/textures/lq_props/plus_2tvnoise.png", "start": 64926616, "end": 64927589}, {"filename": "/GameData/textures/lq_props/plus_3blink_fbr.png", "start": 64927589, "end": 64927820}, {"filename": "/GameData/textures/lq_props/plus_3tvnoise.png", "start": 64927820, "end": 64928802}, {"filename": "/GameData/textures/lq_props/plus_4blink_fbr.png", "start": 64928802, "end": 64929032}, {"filename": "/GameData/textures/lq_props/plus_4tvnoise.png", "start": 64929032, "end": 64930016}, {"filename": "/GameData/textures/lq_props/plus_5tvnoise.png", "start": 64930016, "end": 64930980}, {"filename": "/GameData/textures/lq_props/plus_6tvnoise.png", "start": 64930980, "end": 64931970}, {"filename": "/GameData/textures/lq_props/plus_7tvnoise.png", "start": 64931970, "end": 64932944}, {"filename": "/GameData/textures/lq_props/plus_8tvnoise.png", "start": 64932944, "end": 64933945}, {"filename": "/GameData/textures/lq_props/plus_9tvnoise.png", "start": 64933945, "end": 64934922}, {"filename": "/GameData/textures/lq_props/plus_ablink_fbr.png", "start": 64934922, "end": 64935152}, {"filename": "/GameData/textures/lq_props/plus_atvnoise.png", "start": 64935152, "end": 64935712}, {"filename": "/GameData/textures/lq_props/plus_atvnoise64.png", "start": 64935712, "end": 64936982}, {"filename": "/GameData/textures/lq_props/qr.png", "start": 64936982, "end": 64938402}, {"filename": "/GameData/textures/lq_props/radio16.png", "start": 64938402, "end": 64939170}, {"filename": "/GameData/textures/lq_props/radio32.png", "start": 64939170, "end": 64940160}, {"filename": "/GameData/textures/lq_props/radio64.png", "start": 64940160, "end": 64941918}, {"filename": "/GameData/textures/lq_props/radiowood.png", "start": 64941918, "end": 64944061}, {"filename": "/GameData/textures/lq_props/secret_gem_1.png", "start": 64944061, "end": 64947097}, {"filename": "/GameData/textures/lq_props/secret_gem_2.png", "start": 64947097, "end": 64949801}, {"filename": "/GameData/textures/lq_props/secret_gem_3.png", "start": 64949801, "end": 64952861}, {"filename": "/GameData/textures/lq_props/secret_gem_4.png", "start": 64952861, "end": 64956619}, {"filename": "/GameData/textures/lq_props/secret_gem_h.png", "start": 64956619, "end": 64957170}, {"filename": "/GameData/textures/lq_tech/_t_fence01_fbr.png", "start": 64957170, "end": 64962887}, {"filename": "/GameData/textures/lq_tech/_t_flare01_fbr.png", "start": 64962887, "end": 64963273}, {"filename": "/GameData/textures/lq_tech/aqconc03.png", "start": 64963273, "end": 64973833}, {"filename": "/GameData/textures/lq_tech/aqconc04.png", "start": 64973833, "end": 64984889}, {"filename": "/GameData/textures/lq_tech/aqconc05.png", "start": 64984889, "end": 64989480}, {"filename": "/GameData/textures/lq_tech/aqf006b.png", "start": 64989480, "end": 64992107}, {"filename": "/GameData/textures/lq_tech/aqf032.png", "start": 64992107, "end": 64993378}, {"filename": "/GameData/textures/lq_tech/aqf049.png", "start": 64993378, "end": 64995655}, {"filename": "/GameData/textures/lq_tech/aqf074.png", "start": 64995655, "end": 64999077}, {"filename": "/GameData/textures/lq_tech/aqf075.png", "start": 64999077, "end": 65002236}, {"filename": "/GameData/textures/lq_tech/aqmetl01.png", "start": 65002236, "end": 65004964}, {"filename": "/GameData/textures/lq_tech/aqmetl07.png", "start": 65004964, "end": 65009735}, {"filename": "/GameData/textures/lq_tech/aqmetl14.png", "start": 65009735, "end": 65017467}, {"filename": "/GameData/textures/lq_tech/aqmetl28.png", "start": 65017467, "end": 65019119}, {"filename": "/GameData/textures/lq_tech/aqmetl30.png", "start": 65019119, "end": 65020771}, {"filename": "/GameData/textures/lq_tech/aqmetl33.png", "start": 65020771, "end": 65023207}, {"filename": "/GameData/textures/lq_tech/aqpanl09.png", "start": 65023207, "end": 65025999}, {"filename": "/GameData/textures/lq_tech/aqpanl10.png", "start": 65025999, "end": 65030893}, {"filename": "/GameData/textures/lq_tech/aqpipe01.png", "start": 65030893, "end": 65035081}, {"filename": "/GameData/textures/lq_tech/aqpipe04.png", "start": 65035081, "end": 65037545}, {"filename": "/GameData/textures/lq_tech/aqpipe05.png", "start": 65037545, "end": 65040711}, {"filename": "/GameData/textures/lq_tech/aqpipe08.png", "start": 65040711, "end": 65046343}, {"filename": "/GameData/textures/lq_tech/aqpipe09.png", "start": 65046343, "end": 65054447}, {"filename": "/GameData/textures/lq_tech/aqpipe12.png", "start": 65054447, "end": 65062567}, {"filename": "/GameData/textures/lq_tech/aqpipe13.png", "start": 65062567, "end": 65068878}, {"filename": "/GameData/textures/lq_tech/aqpipe14.png", "start": 65068878, "end": 65076231}, {"filename": "/GameData/textures/lq_tech/aqrust01.png", "start": 65076231, "end": 65078862}, {"filename": "/GameData/textures/lq_tech/aqrust02.png", "start": 65078862, "end": 65084163}, {"filename": "/GameData/textures/lq_tech/aqrust03.png", "start": 65084163, "end": 65089466}, {"filename": "/GameData/textures/lq_tech/aqrust03b.png", "start": 65089466, "end": 65092196}, {"filename": "/GameData/textures/lq_tech/aqrust04.png", "start": 65092196, "end": 65093589}, {"filename": "/GameData/textures/lq_tech/aqrust09.png", "start": 65093589, "end": 65097477}, {"filename": "/GameData/textures/lq_tech/aqrust10.png", "start": 65097477, "end": 65100643}, {"filename": "/GameData/textures/lq_tech/aqsect14.png", "start": 65100643, "end": 65103019}, {"filename": "/GameData/textures/lq_tech/aqsect15.png", "start": 65103019, "end": 65105966}, {"filename": "/GameData/textures/lq_tech/aqsect16.png", "start": 65105966, "end": 65109212}, {"filename": "/GameData/textures/lq_tech/aqsect16b.png", "start": 65109212, "end": 65114169}, {"filename": "/GameData/textures/lq_tech/aqsupp01.png", "start": 65114169, "end": 65116815}, {"filename": "/GameData/textures/lq_tech/aqsupp02.png", "start": 65116815, "end": 65121680}, {"filename": "/GameData/textures/lq_tech/aqsupp03.png", "start": 65121680, "end": 65125582}, {"filename": "/GameData/textures/lq_tech/aqsupp04.png", "start": 65125582, "end": 65127125}, {"filename": "/GameData/textures/lq_tech/aqsupp06.png", "start": 65127125, "end": 65128504}, {"filename": "/GameData/textures/lq_tech/aqsupp07.png", "start": 65128504, "end": 65129859}, {"filename": "/GameData/textures/lq_tech/aqsupp08.png", "start": 65129859, "end": 65132061}, {"filename": "/GameData/textures/lq_tech/aqsupp09.png", "start": 65132061, "end": 65134914}, {"filename": "/GameData/textures/lq_tech/aqtrim01.png", "start": 65134914, "end": 65136141}, {"filename": "/GameData/textures/lq_tech/aqtrim02.png", "start": 65136141, "end": 65137065}, {"filename": "/GameData/textures/lq_tech/aqtrim03.png", "start": 65137065, "end": 65137636}, {"filename": "/GameData/textures/lq_tech/aqtrim08.png", "start": 65137636, "end": 65138368}, {"filename": "/GameData/textures/lq_tech/butmet.png", "start": 65138368, "end": 65140279}, {"filename": "/GameData/textures/lq_tech/comp1_1.png", "start": 65140279, "end": 65143635}, {"filename": "/GameData/textures/lq_tech/comp1_2.png", "start": 65143635, "end": 65146314}, {"filename": "/GameData/textures/lq_tech/comp1_3.png", "start": 65146314, "end": 65148803}, {"filename": "/GameData/textures/lq_tech/comp1_3b.png", "start": 65148803, "end": 65151431}, {"filename": "/GameData/textures/lq_tech/comp1_4.png", "start": 65151431, "end": 65153036}, {"filename": "/GameData/textures/lq_tech/comp1_5.png", "start": 65153036, "end": 65156135}, {"filename": "/GameData/textures/lq_tech/comp1_6.png", "start": 65156135, "end": 65159577}, {"filename": "/GameData/textures/lq_tech/comp1_7.png", "start": 65159577, "end": 65161922}, {"filename": "/GameData/textures/lq_tech/comp1_8.png", "start": 65161922, "end": 65164251}, {"filename": "/GameData/textures/lq_tech/compbase.png", "start": 65164251, "end": 65166670}, {"filename": "/GameData/textures/lq_tech/crate.png", "start": 65166670, "end": 65169787}, {"filename": "/GameData/textures/lq_tech/crate0_bottom.png", "start": 65169787, "end": 65171903}, {"filename": "/GameData/textures/lq_tech/crate0_s_bottom.png", "start": 65171903, "end": 65172991}, {"filename": "/GameData/textures/lq_tech/crate0_s_sside.png", "start": 65172991, "end": 65173936}, {"filename": "/GameData/textures/lq_tech/crate0_s_top.png", "start": 65173936, "end": 65175375}, {"filename": "/GameData/textures/lq_tech/crate0_s_tside.png", "start": 65175375, "end": 65177038}, {"filename": "/GameData/textures/lq_tech/crate0_side.png", "start": 65177038, "end": 65180506}, {"filename": "/GameData/textures/lq_tech/crate0_top.png", "start": 65180506, "end": 65183252}, {"filename": "/GameData/textures/lq_tech/crate0_xs_bot.png", "start": 65183252, "end": 65183919}, {"filename": "/GameData/textures/lq_tech/crate0_xs_sside.png", "start": 65183919, "end": 65184864}, {"filename": "/GameData/textures/lq_tech/crate0_xs_top.png", "start": 65184864, "end": 65185734}, {"filename": "/GameData/textures/lq_tech/crate0_xs_tside.png", "start": 65185734, "end": 65186674}, {"filename": "/GameData/textures/lq_tech/crate1_bottom.png", "start": 65186674, "end": 65188638}, {"filename": "/GameData/textures/lq_tech/crate1_s_bottom.png", "start": 65188638, "end": 65189668}, {"filename": "/GameData/textures/lq_tech/crate1_s_sside.png", "start": 65189668, "end": 65190546}, {"filename": "/GameData/textures/lq_tech/crate1_s_top.png", "start": 65190546, "end": 65191679}, {"filename": "/GameData/textures/lq_tech/crate1_s_tside.png", "start": 65191679, "end": 65193144}, {"filename": "/GameData/textures/lq_tech/crate1_side.png", "start": 65193144, "end": 65196261}, {"filename": "/GameData/textures/lq_tech/crate1_top.png", "start": 65196261, "end": 65198382}, {"filename": "/GameData/textures/lq_tech/crate1_xs_bot.png", "start": 65198382, "end": 65199036}, {"filename": "/GameData/textures/lq_tech/crate1_xs_sside.png", "start": 65199036, "end": 65199914}, {"filename": "/GameData/textures/lq_tech/crate1_xs_top.png", "start": 65199914, "end": 65200621}, {"filename": "/GameData/textures/lq_tech/crate1_xs_tside.png", "start": 65200621, "end": 65201459}, {"filename": "/GameData/textures/lq_tech/dem4_1.png", "start": 65201459, "end": 65210858}, {"filename": "/GameData/textures/lq_tech/dem4_4.png", "start": 65210858, "end": 65218904}, {"filename": "/GameData/textures/lq_tech/dem5_3_fbr.png", "start": 65218904, "end": 65227721}, {"filename": "/GameData/textures/lq_tech/door02_1.png", "start": 65227721, "end": 65231853}, {"filename": "/GameData/textures/lq_tech/doorr02_1.png", "start": 65231853, "end": 65233419}, {"filename": "/GameData/textures/lq_tech/doortrak1.png", "start": 65233419, "end": 65235030}, {"filename": "/GameData/textures/lq_tech/doortrak2-corn.png", "start": 65235030, "end": 65236327}, {"filename": "/GameData/textures/lq_tech/doortrak2.png", "start": 65236327, "end": 65237992}, {"filename": "/GameData/textures/lq_tech/ecop1_1.png", "start": 65237992, "end": 65241069}, {"filename": "/GameData/textures/lq_tech/ecop1_4.png", "start": 65241069, "end": 65244670}, {"filename": "/GameData/textures/lq_tech/edoor01_1.png", "start": 65244670, "end": 65256816}, {"filename": "/GameData/textures/lq_tech/edoor02.png", "start": 65256816, "end": 65268867}, {"filename": "/GameData/textures/lq_tech/edoor02.png.png", "start": 65268867, "end": 65280918}, {"filename": "/GameData/textures/lq_tech/fddoor01.png", "start": 65280918, "end": 65289672}, {"filename": "/GameData/textures/lq_tech/fddoor01b.png", "start": 65289672, "end": 65300355}, {"filename": "/GameData/textures/lq_tech/fdoor02.png", "start": 65300355, "end": 65308909}, {"filename": "/GameData/textures/lq_tech/flat4.png", "start": 65308909, "end": 65310082}, {"filename": "/GameData/textures/lq_tech/floor5_2.png", "start": 65310082, "end": 65312808}, {"filename": "/GameData/textures/lq_tech/floor5_3.png", "start": 65312808, "end": 65314988}, {"filename": "/GameData/textures/lq_tech/laserfield1_fbr.png", "start": 65314988, "end": 65325997}, {"filename": "/GameData/textures/lq_tech/light2.png", "start": 65325997, "end": 65326595}, {"filename": "/GameData/textures/lq_tech/lit8nb.png", "start": 65326595, "end": 65326909}, {"filename": "/GameData/textures/lq_tech/lit8sfb_fbr.png", "start": 65326909, "end": 65327206}, {"filename": "/GameData/textures/lq_tech/met2.png", "start": 65327206, "end": 65339530}, {"filename": "/GameData/textures/lq_tech/metalstrip_1.png", "start": 65339530, "end": 65341813}, {"filename": "/GameData/textures/lq_tech/plat_side1.png", "start": 65341813, "end": 65342578}, {"filename": "/GameData/textures/lq_tech/plat_stem.png", "start": 65342578, "end": 65343224}, {"filename": "/GameData/textures/lq_tech/plat_top1.png", "start": 65343224, "end": 65346624}, {"filename": "/GameData/textures/lq_tech/plat_top2.png", "start": 65346624, "end": 65350339}, {"filename": "/GameData/textures/lq_tech/plat_top3.png", "start": 65350339, "end": 65353747}, {"filename": "/GameData/textures/lq_tech/plat_top4.png", "start": 65353747, "end": 65356443}, {"filename": "/GameData/textures/lq_tech/plat_top5.png", "start": 65356443, "end": 65358538}, {"filename": "/GameData/textures/lq_tech/plus_0_gkey.png", "start": 65358538, "end": 65359602}, {"filename": "/GameData/textures/lq_tech/plus_0_skey.png", "start": 65359602, "end": 65360618}, {"filename": "/GameData/textures/lq_tech/plus_0_tscrn0.png", "start": 65360618, "end": 65362731}, {"filename": "/GameData/textures/lq_tech/plus_0_tscrn1.png", "start": 65362731, "end": 65364856}, {"filename": "/GameData/textures/lq_tech/plus_0arrow2_d_fbr.png", "start": 65364856, "end": 65366110}, {"filename": "/GameData/textures/lq_tech/plus_0arrow2_h_fbr.png", "start": 65366110, "end": 65367379}, {"filename": "/GameData/textures/lq_tech/plus_0arrow2_u_fbr.png", "start": 65367379, "end": 65368637}, {"filename": "/GameData/textures/lq_tech/plus_0arrow_d_fbr.png", "start": 65368637, "end": 65369519}, {"filename": "/GameData/textures/lq_tech/plus_0arrow_h_fbr.png", "start": 65369519, "end": 65370398}, {"filename": "/GameData/textures/lq_tech/plus_0arrow_u_fbr.png", "start": 65370398, "end": 65371273}, {"filename": "/GameData/textures/lq_tech/plus_0basebtn1b_fbr.png", "start": 65371273, "end": 65372162}, {"filename": "/GameData/textures/lq_tech/plus_0basebtn2_fbr.png", "start": 65372162, "end": 65372508}, {"filename": "/GameData/textures/lq_tech/plus_0basebtn2b_fbr.png", "start": 65372508, "end": 65372848}, {"filename": "/GameData/textures/lq_tech/plus_0basebtn_fbr.png", "start": 65372848, "end": 65374129}, {"filename": "/GameData/textures/lq_tech/plus_0button3_fbr.png", "start": 65374129, "end": 65376005}, {"filename": "/GameData/textures/lq_tech/plus_0lit8s.png", "start": 65376005, "end": 65376319}, {"filename": "/GameData/textures/lq_tech/plus_0planet_a_fbr.png", "start": 65376319, "end": 65377910}, {"filename": "/GameData/textures/lq_tech/plus_0planet_b_fbr.png", "start": 65377910, "end": 65379020}, {"filename": "/GameData/textures/lq_tech/plus_0planet_c_fbr.png", "start": 65379020, "end": 65380120}, {"filename": "/GameData/textures/lq_tech/plus_0slipbot.png", "start": 65380120, "end": 65383403}, {"filename": "/GameData/textures/lq_tech/plus_0sliptop.png", "start": 65383403, "end": 65387046}, {"filename": "/GameData/textures/lq_tech/plus_0tek_jump1_fbr.png", "start": 65387046, "end": 65389529}, {"filename": "/GameData/textures/lq_tech/plus_0term128.png", "start": 65389529, "end": 65392333}, {"filename": "/GameData/textures/lq_tech/plus_0term64.png", "start": 65392333, "end": 65393525}, {"filename": "/GameData/textures/lq_tech/plus_0tlight1.png", "start": 65393525, "end": 65394123}, {"filename": "/GameData/textures/lq_tech/plus_0tlight2.png", "start": 65394123, "end": 65394736}, {"filename": "/GameData/textures/lq_tech/plus_0tlight3.png", "start": 65394736, "end": 65395307}, {"filename": "/GameData/textures/lq_tech/plus_1_gkey.png", "start": 65395307, "end": 65396357}, {"filename": "/GameData/textures/lq_tech/plus_1_skey.png", "start": 65396357, "end": 65397362}, {"filename": "/GameData/textures/lq_tech/plus_1arrow2_d_fbr.png", "start": 65397362, "end": 65398618}, {"filename": "/GameData/textures/lq_tech/plus_1arrow2_h_fbr.png", "start": 65398618, "end": 65399885}, {"filename": "/GameData/textures/lq_tech/plus_1arrow2_u_fbr.png", "start": 65399885, "end": 65401143}, {"filename": "/GameData/textures/lq_tech/plus_1arrow_d_fbr.png", "start": 65401143, "end": 65402029}, {"filename": "/GameData/textures/lq_tech/plus_1arrow_h_fbr.png", "start": 65402029, "end": 65402911}, {"filename": "/GameData/textures/lq_tech/plus_1arrow_u_fbr.png", "start": 65402911, "end": 65403789}, {"filename": "/GameData/textures/lq_tech/plus_1basebtn.png", "start": 65403789, "end": 65405068}, {"filename": "/GameData/textures/lq_tech/plus_1basebtn1b.png", "start": 65405068, "end": 65405946}, {"filename": "/GameData/textures/lq_tech/plus_1basebtn2.png", "start": 65405946, "end": 65406296}, {"filename": "/GameData/textures/lq_tech/plus_1basebtn2b.png", "start": 65406296, "end": 65406654}, {"filename": "/GameData/textures/lq_tech/plus_1planet_a_fbr.png", "start": 65406654, "end": 65408275}, {"filename": "/GameData/textures/lq_tech/plus_1planet_b_fbr.png", "start": 65408275, "end": 65409360}, {"filename": "/GameData/textures/lq_tech/plus_1planet_c_fbr.png", "start": 65409360, "end": 65410432}, {"filename": "/GameData/textures/lq_tech/plus_1tek_jump1_fbr.png", "start": 65410432, "end": 65412915}, {"filename": "/GameData/textures/lq_tech/plus_1term128.png", "start": 65412915, "end": 65415718}, {"filename": "/GameData/textures/lq_tech/plus_1term64.png", "start": 65415718, "end": 65416910}, {"filename": "/GameData/textures/lq_tech/plus_2_gkey.png", "start": 65416910, "end": 65417972}, {"filename": "/GameData/textures/lq_tech/plus_2_skey.png", "start": 65417972, "end": 65418978}, {"filename": "/GameData/textures/lq_tech/plus_2arrow2_d_fbr.png", "start": 65418978, "end": 65420221}, {"filename": "/GameData/textures/lq_tech/plus_2arrow2_h_fbr.png", "start": 65420221, "end": 65421482}, {"filename": "/GameData/textures/lq_tech/plus_2arrow2_u_fbr.png", "start": 65421482, "end": 65422728}, {"filename": "/GameData/textures/lq_tech/plus_2arrow_d_fbr.png", "start": 65422728, "end": 65423626}, {"filename": "/GameData/textures/lq_tech/plus_2arrow_h_fbr.png", "start": 65423626, "end": 65424516}, {"filename": "/GameData/textures/lq_tech/plus_2arrow_u_fbr.png", "start": 65424516, "end": 65425403}, {"filename": "/GameData/textures/lq_tech/plus_2planet_a_fbr.png", "start": 65425403, "end": 65427006}, {"filename": "/GameData/textures/lq_tech/plus_2planet_b_fbr.png", "start": 65427006, "end": 65428108}, {"filename": "/GameData/textures/lq_tech/plus_2planet_c_fbr.png", "start": 65428108, "end": 65429226}, {"filename": "/GameData/textures/lq_tech/plus_3planet_a_fbr.png", "start": 65429226, "end": 65430816}, {"filename": "/GameData/textures/lq_tech/plus_3planet_b_fbr.png", "start": 65430816, "end": 65431910}, {"filename": "/GameData/textures/lq_tech/plus_3planet_c_fbr.png", "start": 65431910, "end": 65433033}, {"filename": "/GameData/textures/lq_tech/plus_4planet_a_fbr.png", "start": 65433033, "end": 65434642}, {"filename": "/GameData/textures/lq_tech/plus_4planet_b_fbr.png", "start": 65434642, "end": 65435757}, {"filename": "/GameData/textures/lq_tech/plus_4planet_c_fbr.png", "start": 65435757, "end": 65436904}, {"filename": "/GameData/textures/lq_tech/plus_5planet_a_fbr.png", "start": 65436904, "end": 65438536}, {"filename": "/GameData/textures/lq_tech/plus_5planet_b_fbr.png", "start": 65438536, "end": 65439663}, {"filename": "/GameData/textures/lq_tech/plus_5planet_c_fbr.png", "start": 65439663, "end": 65440770}, {"filename": "/GameData/textures/lq_tech/plus_6planet_a_fbr.png", "start": 65440770, "end": 65442391}, {"filename": "/GameData/textures/lq_tech/plus_6planet_b_fbr.png", "start": 65442391, "end": 65443515}, {"filename": "/GameData/textures/lq_tech/plus_6planet_c_fbr.png", "start": 65443515, "end": 65444609}, {"filename": "/GameData/textures/lq_tech/plus_7planet_a_fbr.png", "start": 65444609, "end": 65446197}, {"filename": "/GameData/textures/lq_tech/plus_7planet_b_fbr.png", "start": 65446197, "end": 65447310}, {"filename": "/GameData/textures/lq_tech/plus_7planet_c_fbr.png", "start": 65447310, "end": 65448402}, {"filename": "/GameData/textures/lq_tech/plus_8planet_a_fbr.png", "start": 65448402, "end": 65450024}, {"filename": "/GameData/textures/lq_tech/plus_8planet_b_fbr.png", "start": 65450024, "end": 65451151}, {"filename": "/GameData/textures/lq_tech/plus_8planet_c_fbr.png", "start": 65451151, "end": 65452240}, {"filename": "/GameData/textures/lq_tech/plus_9planet_a_fbr.png", "start": 65452240, "end": 65453824}, {"filename": "/GameData/textures/lq_tech/plus_9planet_b_fbr.png", "start": 65453824, "end": 65454912}, {"filename": "/GameData/textures/lq_tech/plus_9planet_c_fbr.png", "start": 65454912, "end": 65455988}, {"filename": "/GameData/textures/lq_tech/plus_A_tscrn0.png", "start": 65455988, "end": 65457611}, {"filename": "/GameData/textures/lq_tech/plus_A_tscrn1.png", "start": 65457611, "end": 65460120}, {"filename": "/GameData/textures/lq_tech/plus_A_tscrn2.png", "start": 65460120, "end": 65461720}, {"filename": "/GameData/textures/lq_tech/plus_abasebtn.png", "start": 65461720, "end": 65463009}, {"filename": "/GameData/textures/lq_tech/plus_abasebtn1b.png", "start": 65463009, "end": 65464298}, {"filename": "/GameData/textures/lq_tech/plus_abasebtn2.png", "start": 65464298, "end": 65464669}, {"filename": "/GameData/textures/lq_tech/plus_abasebtn2b.png", "start": 65464669, "end": 65465040}, {"filename": "/GameData/textures/lq_tech/plus_abasebtnb.png", "start": 65465040, "end": 65465411}, {"filename": "/GameData/textures/lq_tech/plus_abutton3_fbr.png", "start": 65465411, "end": 65467299}, {"filename": "/GameData/textures/lq_tech/plus_alit8s_fbr.png", "start": 65467299, "end": 65467596}, {"filename": "/GameData/textures/lq_tech/plus_atek_jump1_fbr.png", "start": 65467596, "end": 65470089}, {"filename": "/GameData/textures/lq_tech/plus_atlight1_fbr.png", "start": 65470089, "end": 65470679}, {"filename": "/GameData/textures/lq_tech/plus_atlight2_fbr.png", "start": 65470679, "end": 65471252}, {"filename": "/GameData/textures/lq_tech/plus_atlight3_fbr.png", "start": 65471252, "end": 65471854}, {"filename": "/GameData/textures/lq_tech/rw33_1.png", "start": 65471854, "end": 65477419}, {"filename": "/GameData/textures/lq_tech/rw33_2.png", "start": 65477419, "end": 65483273}, {"filename": "/GameData/textures/lq_tech/rw33_3.png", "start": 65483273, "end": 65489019}, {"filename": "/GameData/textures/lq_tech/rw33_4.png", "start": 65489019, "end": 65491585}, {"filename": "/GameData/textures/lq_tech/rw33_4b_l.png", "start": 65491585, "end": 65494545}, {"filename": "/GameData/textures/lq_tech/rw33_5.png", "start": 65494545, "end": 65497417}, {"filename": "/GameData/textures/lq_tech/rw33_flat.png", "start": 65497417, "end": 65503368}, {"filename": "/GameData/textures/lq_tech/rw33_lit.png", "start": 65503368, "end": 65504192}, {"filename": "/GameData/textures/lq_tech/rw33b_1.png", "start": 65504192, "end": 65509655}, {"filename": "/GameData/textures/lq_tech/rw33b_2.png", "start": 65509655, "end": 65515497}, {"filename": "/GameData/textures/lq_tech/rw33b_3.png", "start": 65515497, "end": 65521484}, {"filename": "/GameData/textures/lq_tech/rw33b_4.png", "start": 65521484, "end": 65524246}, {"filename": "/GameData/textures/lq_tech/rw33b_5.png", "start": 65524246, "end": 65527295}, {"filename": "/GameData/textures/lq_tech/rw33b_flat.png", "start": 65527295, "end": 65533493}, {"filename": "/GameData/textures/lq_tech/rw33b_lit.png", "start": 65533493, "end": 65534445}, {"filename": "/GameData/textures/lq_tech/rw37_1.png", "start": 65534445, "end": 65540057}, {"filename": "/GameData/textures/lq_tech/rw37_2.png", "start": 65540057, "end": 65546187}, {"filename": "/GameData/textures/lq_tech/rw37_3.png", "start": 65546187, "end": 65552082}, {"filename": "/GameData/textures/lq_tech/rw37_4.png", "start": 65552082, "end": 65559093}, {"filename": "/GameData/textures/lq_tech/rw37_trim1.png", "start": 65559093, "end": 65561601}, {"filename": "/GameData/textures/lq_tech/rw37_trim2.png", "start": 65561601, "end": 65563384}, {"filename": "/GameData/textures/lq_tech/rw37_trim3.png", "start": 65563384, "end": 65565875}, {"filename": "/GameData/textures/lq_tech/rw39_1_fbr.png", "start": 65565875, "end": 65571820}, {"filename": "/GameData/textures/lq_tech/spotlight_fbr.png", "start": 65571820, "end": 65574432}, {"filename": "/GameData/textures/lq_tech/star_lasergrid.png", "start": 65574432, "end": 65574812}, {"filename": "/GameData/textures/lq_tech/t_band1a.png", "start": 65574812, "end": 65577316}, {"filename": "/GameData/textures/lq_tech/t_band1b.png", "start": 65577316, "end": 65579924}, {"filename": "/GameData/textures/lq_tech/t_blok01.png", "start": 65579924, "end": 65582602}, {"filename": "/GameData/textures/lq_tech/t_blok01a.png", "start": 65582602, "end": 65585374}, {"filename": "/GameData/textures/lq_tech/t_blok02.png", "start": 65585374, "end": 65590501}, {"filename": "/GameData/textures/lq_tech/t_blok02a.png", "start": 65590501, "end": 65595360}, {"filename": "/GameData/textures/lq_tech/t_blok03.png", "start": 65595360, "end": 65597802}, {"filename": "/GameData/textures/lq_tech/t_blok03a.png", "start": 65597802, "end": 65599657}, {"filename": "/GameData/textures/lq_tech/t_blok04.png", "start": 65599657, "end": 65602723}, {"filename": "/GameData/textures/lq_tech/t_blok04h.png", "start": 65602723, "end": 65605447}, {"filename": "/GameData/textures/lq_tech/t_blok05.png", "start": 65605447, "end": 65609735}, {"filename": "/GameData/textures/lq_tech/t_blok06.png", "start": 65609735, "end": 65612585}, {"filename": "/GameData/textures/lq_tech/t_blok06h.png", "start": 65612585, "end": 65614562}, {"filename": "/GameData/textures/lq_tech/t_blok07.png", "start": 65614562, "end": 65617348}, {"filename": "/GameData/textures/lq_tech/t_blok07a.png", "start": 65617348, "end": 65620134}, {"filename": "/GameData/textures/lq_tech/t_blok08.png", "start": 65620134, "end": 65624641}, {"filename": "/GameData/textures/lq_tech/t_blok09.png", "start": 65624641, "end": 65627961}, {"filename": "/GameData/textures/lq_tech/t_blok10.png", "start": 65627961, "end": 65633972}, {"filename": "/GameData/textures/lq_tech/t_blok10b.png", "start": 65633972, "end": 65639680}, {"filename": "/GameData/textures/lq_tech/t_blok10c.png", "start": 65639680, "end": 65642898}, {"filename": "/GameData/textures/lq_tech/t_blok11.png", "start": 65642898, "end": 65648839}, {"filename": "/GameData/textures/lq_tech/t_blok11b.png", "start": 65648839, "end": 65654444}, {"filename": "/GameData/textures/lq_tech/t_blok12c.png", "start": 65654444, "end": 65657651}, {"filename": "/GameData/textures/lq_tech/t_flat01.png", "start": 65657651, "end": 65660085}, {"filename": "/GameData/textures/lq_tech/t_flat02.png", "start": 65660085, "end": 65662450}, {"filename": "/GameData/textures/lq_tech/t_flat05.png", "start": 65662450, "end": 65670646}, {"filename": "/GameData/textures/lq_tech/t_flor1a.png", "start": 65670646, "end": 65674258}, {"filename": "/GameData/textures/lq_tech/t_flor1b.png", "start": 65674258, "end": 65677876}, {"filename": "/GameData/textures/lq_tech/t_flor2a.png", "start": 65677876, "end": 65680470}, {"filename": "/GameData/textures/lq_tech/t_flor2b.png", "start": 65680470, "end": 65682406}, {"filename": "/GameData/textures/lq_tech/t_flor2c.png", "start": 65682406, "end": 65683886}, {"filename": "/GameData/textures/lq_tech/t_flor2d.png", "start": 65683886, "end": 65684390}, {"filename": "/GameData/textures/lq_tech/t_lit01_fbr.png", "start": 65684390, "end": 65684570}, {"filename": "/GameData/textures/lq_tech/t_lit02_fbr.png", "start": 65684570, "end": 65684712}, {"filename": "/GameData/textures/lq_tech/t_lit03_fbr.png", "start": 65684712, "end": 65684847}, {"filename": "/GameData/textures/lq_tech/t_lit04_fbr.png", "start": 65684847, "end": 65684979}, {"filename": "/GameData/textures/lq_tech/t_lit05_fbr.png", "start": 65684979, "end": 65685145}, {"filename": "/GameData/textures/lq_tech/t_lit06_fbr.png", "start": 65685145, "end": 65685301}, {"filename": "/GameData/textures/lq_tech/t_lit07_fbr.png", "start": 65685301, "end": 65687604}, {"filename": "/GameData/textures/lq_tech/t_lit08_fbr.png", "start": 65687604, "end": 65687821}, {"filename": "/GameData/textures/lq_tech/t_metalsheeta.png", "start": 65687821, "end": 65694420}, {"filename": "/GameData/textures/lq_tech/t_metalsheetb.png", "start": 65694420, "end": 65705933}, {"filename": "/GameData/textures/lq_tech/t_num_0_fbr.png", "start": 65705933, "end": 65706707}, {"filename": "/GameData/textures/lq_tech/t_num_1_fbr.png", "start": 65706707, "end": 65707469}, {"filename": "/GameData/textures/lq_tech/t_num_2_fbr.png", "start": 65707469, "end": 65708264}, {"filename": "/GameData/textures/lq_tech/t_num_3_fbr.png", "start": 65708264, "end": 65709022}, {"filename": "/GameData/textures/lq_tech/t_num_4_fbr.png", "start": 65709022, "end": 65709827}, {"filename": "/GameData/textures/lq_tech/t_num_5_fbr.png", "start": 65709827, "end": 65710625}, {"filename": "/GameData/textures/lq_tech/t_num_6_fbr.png", "start": 65710625, "end": 65711422}, {"filename": "/GameData/textures/lq_tech/t_num_7_fbr.png", "start": 65711422, "end": 65712196}, {"filename": "/GameData/textures/lq_tech/t_num_8_fbr.png", "start": 65712196, "end": 65712976}, {"filename": "/GameData/textures/lq_tech/t_num_9_fbr.png", "start": 65712976, "end": 65713769}, {"filename": "/GameData/textures/lq_tech/t_num_x.png", "start": 65713769, "end": 65714577}, {"filename": "/GameData/textures/lq_tech/t_rivs01.png", "start": 65714577, "end": 65717005}, {"filename": "/GameData/textures/lq_tech/t_rivs01a.png", "start": 65717005, "end": 65719459}, {"filename": "/GameData/textures/lq_tech/t_sign1.png", "start": 65719459, "end": 65722409}, {"filename": "/GameData/textures/lq_tech/t_tech01.png", "start": 65722409, "end": 65726460}, {"filename": "/GameData/textures/lq_tech/t_tech02.png", "start": 65726460, "end": 65729563}, {"filename": "/GameData/textures/lq_tech/t_tech03.png", "start": 65729563, "end": 65745554}, {"filename": "/GameData/textures/lq_tech/t_tech04.png", "start": 65745554, "end": 65748409}, {"filename": "/GameData/textures/lq_tech/t_tech05.png", "start": 65748409, "end": 65751188}, {"filename": "/GameData/textures/lq_tech/t_tech06.png", "start": 65751188, "end": 65754000}, {"filename": "/GameData/textures/lq_tech/t_trim1a.png", "start": 65754000, "end": 65756488}, {"filename": "/GameData/textures/lq_tech/t_trim1aa.png", "start": 65756488, "end": 65759123}, {"filename": "/GameData/textures/lq_tech/t_trim1b.png", "start": 65759123, "end": 65761854}, {"filename": "/GameData/textures/lq_tech/t_trim1ba.png", "start": 65761854, "end": 65764478}, {"filename": "/GameData/textures/lq_tech/t_trim1c.png", "start": 65764478, "end": 65767263}, {"filename": "/GameData/textures/lq_tech/t_trim1ca.png", "start": 65767263, "end": 65770089}, {"filename": "/GameData/textures/lq_tech/t_trim1d.png", "start": 65770089, "end": 65772389}, {"filename": "/GameData/textures/lq_tech/t_trim1e.png", "start": 65772389, "end": 65774534}, {"filename": "/GameData/textures/lq_tech/t_trim2a.png", "start": 65774534, "end": 65777607}, {"filename": "/GameData/textures/lq_tech/t_trim2aa.png", "start": 65777607, "end": 65780259}, {"filename": "/GameData/textures/lq_tech/t_trim2b.png", "start": 65780259, "end": 65782887}, {"filename": "/GameData/textures/lq_tech/t_trim2ba.png", "start": 65782887, "end": 65785575}, {"filename": "/GameData/textures/lq_tech/t_trim2c.png", "start": 65785575, "end": 65788366}, {"filename": "/GameData/textures/lq_tech/t_trim2ca.png", "start": 65788366, "end": 65791245}, {"filename": "/GameData/textures/lq_tech/t_trim2d.png", "start": 65791245, "end": 65793707}, {"filename": "/GameData/textures/lq_tech/t_trim2e.png", "start": 65793707, "end": 65796214}, {"filename": "/GameData/textures/lq_tech/t_tris02.png", "start": 65796214, "end": 65799128}, {"filename": "/GameData/textures/lq_tech/t_wall05.png", "start": 65799128, "end": 65802454}, {"filename": "/GameData/textures/lq_tech/t_wall1a.png", "start": 65802454, "end": 65812981}, {"filename": "/GameData/textures/lq_tech/t_wall1aa.png", "start": 65812981, "end": 65823302}, {"filename": "/GameData/textures/lq_tech/t_wall1b.png", "start": 65823302, "end": 65834402}, {"filename": "/GameData/textures/lq_tech/t_wall1ba.png", "start": 65834402, "end": 65845241}, {"filename": "/GameData/textures/lq_tech/t_wall2a.png", "start": 65845241, "end": 65856025}, {"filename": "/GameData/textures/lq_tech/t_wall2aa.png", "start": 65856025, "end": 65867045}, {"filename": "/GameData/textures/lq_tech/t_wall2ab.png", "start": 65867045, "end": 65878802}, {"filename": "/GameData/textures/lq_tech/t_wall2b.png", "start": 65878802, "end": 65893040}, {"filename": "/GameData/textures/lq_tech/t_wall2ba.png", "start": 65893040, "end": 65906491}, {"filename": "/GameData/textures/lq_tech/t_wall3a.png", "start": 65906491, "end": 65914458}, {"filename": "/GameData/textures/lq_tech/t_wall3aa.png", "start": 65914458, "end": 65922725}, {"filename": "/GameData/textures/lq_tech/t_wall3b.png", "start": 65922725, "end": 65931047}, {"filename": "/GameData/textures/lq_tech/t_wall3ba.png", "start": 65931047, "end": 65940010}, {"filename": "/GameData/textures/lq_tech/t_wall6a.png", "start": 65940010, "end": 65943244}, {"filename": "/GameData/textures/lq_tech/t_wall6b.png", "start": 65943244, "end": 65946258}, {"filename": "/GameData/textures/lq_tech/t_wall6c.png", "start": 65946258, "end": 65949934}, {"filename": "/GameData/textures/lq_tech/t_wall6d.png", "start": 65949934, "end": 65953637}, {"filename": "/GameData/textures/lq_tech/t_wall6e.png", "start": 65953637, "end": 65957433}, {"filename": "/GameData/textures/lq_tech/t_wall7a.png", "start": 65957433, "end": 65968325}, {"filename": "/GameData/textures/lq_tech/t_wall7b.png", "start": 65968325, "end": 65975766}, {"filename": "/GameData/textures/lq_tech/t_wire01.png", "start": 65975766, "end": 65978702}, {"filename": "/GameData/textures/lq_tech/t_wire02.png", "start": 65978702, "end": 65982237}, {"filename": "/GameData/textures/lq_tech/t_wire03.png", "start": 65982237, "end": 65985837}, {"filename": "/GameData/textures/lq_tech/tech04_1.png", "start": 65985837, "end": 65986660}, {"filename": "/GameData/textures/lq_tech/tech04_3.png", "start": 65986660, "end": 65988129}, {"filename": "/GameData/textures/lq_tech/tech08_1.png", "start": 65988129, "end": 65999182}, {"filename": "/GameData/textures/lq_tech/tech08_2.png", "start": 65999182, "end": 66010235}, {"filename": "/GameData/textures/lq_tech/tech10_3.png", "start": 66010235, "end": 66014091}, {"filename": "/GameData/textures/lq_tech/tech14-1.png", "start": 66014091, "end": 66024362}, {"filename": "/GameData/textures/lq_tech/techbasetextures.txt", "start": 66024362, "end": 66024872}, {"filename": "/GameData/textures/lq_tech/techeye1_fbr.png", "start": 66024872, "end": 66028216}, {"filename": "/GameData/textures/lq_tech/techeye2_fbr.png", "start": 66028216, "end": 66031618}, {"filename": "/GameData/textures/lq_tech/tek_door1.png", "start": 66031618, "end": 66043764}, {"filename": "/GameData/textures/lq_tech/tek_door2.png", "start": 66043764, "end": 66055815}, {"filename": "/GameData/textures/lq_tech/tek_flr3.png", "start": 66055815, "end": 66059196}, {"filename": "/GameData/textures/lq_tech/tek_grate.png", "start": 66059196, "end": 66062074}, {"filename": "/GameData/textures/lq_tech/tek_lit1_fbr.png", "start": 66062074, "end": 66063709}, {"filename": "/GameData/textures/lq_tech/tek_lit2_fbr.png", "start": 66063709, "end": 66064695}, {"filename": "/GameData/textures/lq_tech/tek_lit3_fbr.png", "start": 66064695, "end": 66066537}, {"filename": "/GameData/textures/lq_tech/tek_lit4_fbr.png", "start": 66066537, "end": 66067664}, {"filename": "/GameData/textures/lq_tech/tek_pip1_fbr.png", "start": 66067664, "end": 66070746}, {"filename": "/GameData/textures/lq_tech/tek_pipe1.png", "start": 66070746, "end": 66073546}, {"filename": "/GameData/textures/lq_tech/tek_pipe2.png", "start": 66073546, "end": 66075205}, {"filename": "/GameData/textures/lq_tech/tek_trm1.png", "start": 66075205, "end": 66077663}, {"filename": "/GameData/textures/lq_tech/tek_trm3.png", "start": 66077663, "end": 66080273}, {"filename": "/GameData/textures/lq_tech/tek_wall4_1.png", "start": 66080273, "end": 66094718}, {"filename": "/GameData/textures/lq_tech/tele_frame1.png", "start": 66094718, "end": 66100873}, {"filename": "/GameData/textures/lq_tech/tele_frame2.png", "start": 66100873, "end": 66102824}, {"filename": "/GameData/textures/lq_tech/tele_frame3.png", "start": 66102824, "end": 66106811}, {"filename": "/GameData/textures/lq_tech/telepad1_fbr.png", "start": 66106811, "end": 66108787}, {"filename": "/GameData/textures/lq_tech/tlight11_fbr.png", "start": 66108787, "end": 66110413}, {"filename": "/GameData/textures/lq_tech/tlight12_fbr.png", "start": 66110413, "end": 66112108}, {"filename": "/GameData/textures/lq_tech/tlight13_fbr.png", "start": 66112108, "end": 66114167}, {"filename": "/GameData/textures/lq_tech/tlightblfb_fbr.png", "start": 66114167, "end": 66114740}, {"filename": "/GameData/textures/lq_tech/tlightfb_fbr.png", "start": 66114740, "end": 66115330}, {"filename": "/GameData/textures/lq_tech/tlightnb.png", "start": 66115330, "end": 66115928}, {"filename": "/GameData/textures/lq_tech/tlightrdfb_fbr.png", "start": 66115928, "end": 66116530}, {"filename": "/GameData/textures/lq_tech/treadplatemetal.png", "start": 66116530, "end": 66131284}, {"filename": "/GameData/textures/lq_tech/twall2_3.png", "start": 66131284, "end": 66134792}, {"filename": "/GameData/textures/lq_tech/w17_1.png", "start": 66134792, "end": 66151816}, {"filename": "/GameData/textures/lq_tech/w94_1.png", "start": 66151816, "end": 66164764}, {"filename": "/GameData/textures/lq_tech/z_exit_fbr.png", "start": 66164764, "end": 66166325}, {"filename": "/GameData/textures/lq_terra/afloor1_3.png", "start": 66166325, "end": 66169323}, {"filename": "/GameData/textures/lq_terra/asphalt.png", "start": 66169323, "end": 66186783}, {"filename": "/GameData/textures/lq_terra/azfloor1_1.png", "start": 66186783, "end": 66189802}, {"filename": "/GameData/textures/lq_terra/badlawn.png", "start": 66189802, "end": 66231611}, {"filename": "/GameData/textures/lq_terra/cracks1-1.png", "start": 66231611, "end": 66234630}, {"filename": "/GameData/textures/lq_terra/darkrock.png", "start": 66234630, "end": 66257199}, {"filename": "/GameData/textures/lq_terra/grass1.png", "start": 66257199, "end": 66267770}, {"filename": "/GameData/textures/lq_terra/gravel1.png", "start": 66267770, "end": 66281124}, {"filename": "/GameData/textures/lq_terra/gravel2.png", "start": 66281124, "end": 66296021}, {"filename": "/GameData/textures/lq_terra/grk_leaf1_1.png", "start": 66296021, "end": 66300014}, {"filename": "/GameData/textures/lq_terra/grk_leaf1_2.png", "start": 66300014, "end": 66303751}, {"filename": "/GameData/textures/lq_terra/marbbrn128.png", "start": 66303751, "end": 66313993}, {"filename": "/GameData/textures/lq_terra/may_drt1_1.png", "start": 66313993, "end": 66316889}, {"filename": "/GameData/textures/lq_terra/may_drt1_2.png", "start": 66316889, "end": 66319743}, {"filename": "/GameData/textures/lq_terra/may_drt2_2.png", "start": 66319743, "end": 66322487}, {"filename": "/GameData/textures/lq_terra/may_rck1_1.png", "start": 66322487, "end": 66336587}, {"filename": "/GameData/textures/lq_terra/may_rck1_2.png", "start": 66336587, "end": 66347265}, {"filename": "/GameData/textures/lq_terra/may_rck1_3.png", "start": 66347265, "end": 66360108}, {"filename": "/GameData/textures/lq_terra/may_slat1_1.png", "start": 66360108, "end": 66362956}, {"filename": "/GameData/textures/lq_terra/med_bigdirt.png", "start": 66362956, "end": 66517539}, {"filename": "/GameData/textures/lq_terra/med_bigdirt2.png", "start": 66517539, "end": 66671690}, {"filename": "/GameData/textures/lq_terra/med_bigdirt3.png", "start": 66671690, "end": 66825641}, {"filename": "/GameData/textures/lq_terra/med_cobstn1_1.png", "start": 66825641, "end": 66840022}, {"filename": "/GameData/textures/lq_terra/med_cobstn1_1a.png", "start": 66840022, "end": 66856996}, {"filename": "/GameData/textures/lq_terra/med_cobstn1_2.png", "start": 66856996, "end": 66871435}, {"filename": "/GameData/textures/lq_terra/med_cobstn1_2a.png", "start": 66871435, "end": 66888250}, {"filename": "/GameData/textures/lq_terra/med_cobstn2_1.png", "start": 66888250, "end": 66902219}, {"filename": "/GameData/textures/lq_terra/med_cobstn2_1a.png", "start": 66902219, "end": 66919530}, {"filename": "/GameData/textures/lq_terra/med_cobstn2_2.png", "start": 66919530, "end": 66934604}, {"filename": "/GameData/textures/lq_terra/med_cobstn2_2a.png", "start": 66934604, "end": 66951708}, {"filename": "/GameData/textures/lq_terra/med_cracks1.png", "start": 66951708, "end": 66965170}, {"filename": "/GameData/textures/lq_terra/med_flat1.png", "start": 66965170, "end": 66978079}, {"filename": "/GameData/textures/lq_terra/med_flat12.png", "start": 66978079, "end": 66989467}, {"filename": "/GameData/textures/lq_terra/med_flat15.png", "start": 66989467, "end": 67002568}, {"filename": "/GameData/textures/lq_terra/med_flat16.png", "start": 67002568, "end": 67015410}, {"filename": "/GameData/textures/lq_terra/med_flat2.png", "start": 67015410, "end": 67026687}, {"filename": "/GameData/textures/lq_terra/med_flat3.png", "start": 67026687, "end": 67040052}, {"filename": "/GameData/textures/lq_terra/med_flat4.png", "start": 67040052, "end": 67050001}, {"filename": "/GameData/textures/lq_terra/med_flat5.png", "start": 67050001, "end": 67061086}, {"filename": "/GameData/textures/lq_terra/med_flat5a.png", "start": 67061086, "end": 67071830}, {"filename": "/GameData/textures/lq_terra/med_flat6.png", "start": 67071830, "end": 67080815}, {"filename": "/GameData/textures/lq_terra/med_flat7.png", "start": 67080815, "end": 67092273}, {"filename": "/GameData/textures/lq_terra/med_plaster2.png", "start": 67092273, "end": 67101925}, {"filename": "/GameData/textures/lq_terra/med_rock1.png", "start": 67101925, "end": 67153182}, {"filename": "/GameData/textures/lq_terra/med_rock10.png", "start": 67153182, "end": 67162477}, {"filename": "/GameData/textures/lq_terra/med_rock10a.png", "start": 67162477, "end": 67171791}, {"filename": "/GameData/textures/lq_terra/med_rock10b.png", "start": 67171791, "end": 67185156}, {"filename": "/GameData/textures/lq_terra/med_rock10c.png", "start": 67185156, "end": 67197491}, {"filename": "/GameData/textures/lq_terra/med_rock2.png", "start": 67197491, "end": 67239368}, {"filename": "/GameData/textures/lq_terra/med_rock3.png", "start": 67239368, "end": 67285312}, {"filename": "/GameData/textures/lq_terra/med_rock3_bump.png", "start": 67285312, "end": 67401406}, {"filename": "/GameData/textures/lq_terra/med_rock4.png", "start": 67401406, "end": 67412762}, {"filename": "/GameData/textures/lq_terra/med_rock5.png", "start": 67412762, "end": 67425294}, {"filename": "/GameData/textures/lq_terra/med_rock9.png", "start": 67425294, "end": 67436318}, {"filename": "/GameData/textures/lq_terra/ret_plaster1.png", "start": 67436318, "end": 67447595}, {"filename": "/GameData/textures/lq_terra/rock1_1.png", "start": 67447595, "end": 67497966}, {"filename": "/GameData/textures/lq_terra/rock1_1b.png", "start": 67497966, "end": 67508981}, {"filename": "/GameData/textures/lq_terra/rock1_2.png", "start": 67508981, "end": 67551024}, {"filename": "/GameData/textures/lq_terra/rocks07.png", "start": 67551024, "end": 67562048}, {"filename": "/GameData/textures/lq_terra/rocks11d.png", "start": 67562048, "end": 67571362}, {"filename": "/GameData/textures/lq_terra/rocks11e.png", "start": 67571362, "end": 67580657}, {"filename": "/GameData/textures/lq_terra/sand.png", "start": 67580657, "end": 67606744}, {"filename": "/GameData/textures/lq_terra/snow1.png", "start": 67606744, "end": 67608607}, {"filename": "/GameData/textures/lq_terra/uwall1_2.png", "start": 67608607, "end": 67636258}, {"filename": "/GameData/textures/lq_terra/vines1.png", "start": 67636258, "end": 67641313}, {"filename": "/GameData/textures/lq_utility/black.png", "start": 67641313, "end": 67641861}, {"filename": "/GameData/textures/lq_utility/clip.png", "start": 67641861, "end": 67642367}, {"filename": "/GameData/textures/lq_utility/hint.png", "start": 67642367, "end": 67643233}, {"filename": "/GameData/textures/lq_utility/hintskip.png", "start": 67643233, "end": 67644126}, {"filename": "/GameData/textures/lq_utility/light_fbr.png", "start": 67644126, "end": 67645042}, {"filename": "/GameData/textures/lq_utility/origin.png", "start": 67645042, "end": 67645525}, {"filename": "/GameData/textures/lq_utility/skip.png", "start": 67645525, "end": 67646013}, {"filename": "/GameData/textures/lq_utility/star_lavaskip.png", "start": 67646013, "end": 67647126}, {"filename": "/GameData/textures/lq_utility/star_slimeskip.png", "start": 67647126, "end": 67648199}, {"filename": "/GameData/textures/lq_utility/star_waterskip.png", "start": 67648199, "end": 67649939}, {"filename": "/GameData/textures/lq_utility/trigger.png", "start": 67649939, "end": 67650436}, {"filename": "/GameData/textures/lq_wood/crate4.png", "start": 67650436, "end": 67653578}, {"filename": "/GameData/textures/lq_wood/crwdh6.png", "start": 67653578, "end": 67658586}, {"filename": "/GameData/textures/lq_wood/crwdl12.png", "start": 67658586, "end": 67661019}, {"filename": "/GameData/textures/lq_wood/crwds6.png", "start": 67661019, "end": 67662045}, {"filename": "/GameData/textures/lq_wood/may_crate3-small.png", "start": 67662045, "end": 67663036}, {"filename": "/GameData/textures/lq_wood/may_crate3.png", "start": 67663036, "end": 67666099}, {"filename": "/GameData/textures/lq_wood/may_wood1_1.png", "start": 67666099, "end": 67668939}, {"filename": "/GameData/textures/lq_wood/may_wood1_2.png", "start": 67668939, "end": 67671788}, {"filename": "/GameData/textures/lq_wood/med_ret_wood1.png", "start": 67671788, "end": 67678757}, {"filename": "/GameData/textures/lq_wood/med_wood1.png", "start": 67678757, "end": 67706125}, {"filename": "/GameData/textures/lq_wood/med_wood2.png", "start": 67706125, "end": 67713638}, {"filename": "/GameData/textures/lq_wood/med_wood2_plk1.png", "start": 67713638, "end": 67725336}, {"filename": "/GameData/textures/lq_wood/med_wood2_plk2.png", "start": 67725336, "end": 67737828}, {"filename": "/GameData/textures/lq_wood/med_wood3.png", "start": 67737828, "end": 67743828}, {"filename": "/GameData/textures/lq_wood/med_wood3_plk1.png", "start": 67743828, "end": 67757043}, {"filename": "/GameData/textures/lq_wood/med_wood4.png", "start": 67757043, "end": 67764167}, {"filename": "/GameData/textures/lq_wood/med_wood5.png", "start": 67764167, "end": 67771965}, {"filename": "/GameData/textures/lq_wood/med_wood6.png", "start": 67771965, "end": 67778745}, {"filename": "/GameData/textures/lq_wood/med_wood7.png", "start": 67778745, "end": 67787784}, {"filename": "/GameData/textures/lq_wood/med_wood8.png", "start": 67787784, "end": 67795748}, {"filename": "/GameData/textures/lq_wood/med_wood_riv1.png", "start": 67795748, "end": 67802373}, {"filename": "/GameData/textures/lq_wood/med_wood_riv1b.png", "start": 67802373, "end": 67808452}, {"filename": "/GameData/textures/lq_wood/med_wood_riv1c.png", "start": 67808452, "end": 67813576}, {"filename": "/GameData/textures/lq_wood/med_wood_riv2.png", "start": 67813576, "end": 67819986}, {"filename": "/GameData/textures/lq_wood/med_wood_riv2b.png", "start": 67819986, "end": 67826406}, {"filename": "/GameData/textures/lq_wood/med_wood_riv2c.png", "start": 67826406, "end": 67831778}, {"filename": "/GameData/textures/lq_wood/plank1.png", "start": 67831778, "end": 67838192}, {"filename": "/GameData/textures/lq_wood/plank1s.png", "start": 67838192, "end": 67840073}, {"filename": "/GameData/textures/lq_wood/plank2.png", "start": 67840073, "end": 67846653}, {"filename": "/GameData/textures/lq_wood/plank2s.png", "start": 67846653, "end": 67848542}, {"filename": "/GameData/textures/lq_wood/plank3.png", "start": 67848542, "end": 67855402}, {"filename": "/GameData/textures/lq_wood/plank3s.png", "start": 67855402, "end": 67857383}, {"filename": "/GameData/textures/lq_wood/plank4.png", "start": 67857383, "end": 67863829}, {"filename": "/GameData/textures/lq_wood/plank4s.png", "start": 67863829, "end": 67865719}, {"filename": "/GameData/textures/lq_wood/plank5.png", "start": 67865719, "end": 67871368}, {"filename": "/GameData/textures/lq_wood/sq_wood_1.png", "start": 67871368, "end": 67887409}, {"filename": "/GameData/textures/lq_wood/sq_wood_2.png", "start": 67887409, "end": 67890297}, {"filename": "/GameData/textures/lq_wood/sq_wood_2a.png", "start": 67890297, "end": 67893666}, {"filename": "/GameData/textures/lq_wood/wood_1.png", "start": 67893666, "end": 67897844}, {"filename": "/GameData/textures/lq_wood/wood_2.png", "start": 67897844, "end": 67901744}, {"filename": "/GameData/textures/lq_wood/woodbark128.png", "start": 67901744, "end": 67911951}, {"filename": "/GameData/textures/lq_wood/woodbark1m28.png", "start": 67911951, "end": 67921400}, {"filename": "/GameData/textures/lq_wood/woodbark64.png", "start": 67921400, "end": 67924337}, {"filename": "/GameData/textures/lq_wood/woodbarkA128.png", "start": 67924337, "end": 67933035}, {"filename": "/GameData/textures/lq_wood/woodbarkm64.png", "start": 67933035, "end": 67935712}, {"filename": "/GameData/textures/lq_wood/woodend.png", "start": 67935712, "end": 67938215}, {"filename": "/GameData/textures/lq_wood/woodring128.png", "start": 67938215, "end": 67946639}, {"filename": "/GameData/textures/lq_wood/woodring64.png", "start": 67946639, "end": 67949100}, {"filename": "/GameData/textures/lq_wood/woodringm128.png", "start": 67949100, "end": 67959399}, {"filename": "/GameData/textures/lq_wood/woodringm64.png", "start": 67959399, "end": 67962658}, {"filename": "/GameData/textures/metal/metal1.png", "start": 67962658, "end": 68386714}, {"filename": "/GameData/textures/muzzle_t.png", "start": 68386714, "end": 68396122}, {"filename": "/GameData/textures/particles/blood.png", "start": 68396122, "end": 68399835}, {"filename": "/GameData/textures/particles/smoke.png", "start": 68399835, "end": 68403705}, {"filename": "/GameData/textures/particles/trail.png", "start": 68403705, "end": 68422575}, {"filename": "/GameData/textures/particles/wood.png", "start": 68422575, "end": 68438058}, {"filename": "/GameData/textures/shirt.png", "start": 68438058, "end": 68629809}, {"filename": "/GameData/textures/tormentPack/+0str_bloodfall.png", "start": 68629809, "end": 68631886}, {"filename": "/GameData/textures/tormentPack/+1str_bloodfall.png", "start": 68631886, "end": 68633920}, {"filename": "/GameData/textures/tormentPack/+2str_bloodfall.png", "start": 68633920, "end": 68636018}, {"filename": "/GameData/textures/tormentPack/+3str_bloodfall.png", "start": 68636018, "end": 68638089}, {"filename": "/GameData/textures/tormentPack/+4str_bloodfall.png", "start": 68638089, "end": 68640149}, {"filename": "/GameData/textures/tormentPack/+5str_bloodfall.png", "start": 68640149, "end": 68642170}, {"filename": "/GameData/textures/tormentPack/+6str_bloodfall.png", "start": 68642170, "end": 68644216}, {"filename": "/GameData/textures/tormentPack/+7str_bloodfall.png", "start": 68644216, "end": 68646285}, {"filename": "/GameData/textures/tormentPack/str_blood.png", "start": 68646285, "end": 68648293}, {"filename": "/GameData/textures/tormentPack/str_blood_large.png", "start": 68648293, "end": 68670843}, {"filename": "/GameData/textures/tormentPack/str_bloodvein1.png", "start": 68670843, "end": 68697518}, {"filename": "/GameData/textures/tormentPack/str_bloodvein2.png", "start": 68697518, "end": 68729551}, {"filename": "/GameData/textures/tormentPack/str_bloodvein3.png", "start": 68729551, "end": 68763222}, {"filename": "/GameData/textures/tormentPack/str_bloodvein4.png", "start": 68763222, "end": 68794864}, {"filename": "/GameData/textures/tormentPack/str_bloodvein5.png", "start": 68794864, "end": 68828355}, {"filename": "/GameData/textures/tormentPack/str_bloodvein6.png", "start": 68828355, "end": 68862829}, {"filename": "/GameData/textures/tormentPack/str_bloodvein7.png", "start": 68862829, "end": 68897720}, {"filename": "/GameData/textures/tormentPack/str_bloodvein8.png", "start": 68897720, "end": 68925869}, {"filename": "/GameData/textures/tormentPack/str_bloodvein9.png", "start": 68925869, "end": 68961322}, {"filename": "/GameData/textures/tormentPack/str_metalflr1.png", "start": 68961322, "end": 68998016}, {"filename": "/GameData/textures/tormentPack/str_metalflr2.png", "start": 68998016, "end": 69034591}, {"filename": "/GameData/textures/tormentPack/str_metalflr3.png", "start": 69034591, "end": 69065584}, {"filename": "/GameData/textures/tormentPack/str_metalflr4.png", "start": 69065584, "end": 69095754}, {"filename": "/GameData/textures/tormentPack/str_metalflr5.png", "start": 69095754, "end": 69135204}, {"filename": "/GameData/textures/tormentPack/str_metalflr6.png", "start": 69135204, "end": 69174364}, {"filename": "/GameData/textures/tormentPack/str_metalflr7.png", "start": 69174364, "end": 69216056}, {"filename": "/GameData/textures/tormentPack/str_metalflr8.png", "start": 69216056, "end": 69256474}, {"filename": "/GameData/textures/tormentPack/str_metalgen1.png", "start": 69256474, "end": 69285744}, {"filename": "/GameData/textures/tormentPack/str_metalgen2.png", "start": 69285744, "end": 69315260}, {"filename": "/GameData/textures/tormentPack/str_metalgen3.png", "start": 69315260, "end": 69344736}, {"filename": "/GameData/textures/tormentPack/str_metalgen4.png", "start": 69344736, "end": 69369559}, {"filename": "/GameData/textures/tormentPack/str_metalgen5.png", "start": 69369559, "end": 69394429}, {"filename": "/GameData/textures/tormentPack/str_metalgen6.png", "start": 69394429, "end": 69418519}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl1.png", "start": 69418519, "end": 69447904}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl2.png", "start": 69447904, "end": 69480634}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl3.png", "start": 69480634, "end": 69507252}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl4.png", "start": 69507252, "end": 69536516}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl5.png", "start": 69536516, "end": 69570891}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl6.png", "start": 69570891, "end": 69609386}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl7.png", "start": 69609386, "end": 69645536}, {"filename": "/GameData/textures/tormentPack/str_metalgrbl8.png", "start": 69645536, "end": 69683529}, {"filename": "/GameData/textures/tormentPack/str_metalpan1.png", "start": 69683529, "end": 69718389}, {"filename": "/GameData/textures/tormentPack/str_metalpan2.png", "start": 69718389, "end": 69753930}, {"filename": "/GameData/textures/tormentPack/str_metalpan3.png", "start": 69753930, "end": 69784366}, {"filename": "/GameData/textures/tormentPack/str_metalpan4.png", "start": 69784366, "end": 69815326}, {"filename": "/GameData/textures/tormentPack/str_metalpan5.png", "start": 69815326, "end": 69851698}, {"filename": "/GameData/textures/tormentPack/str_metalpan6.png", "start": 69851698, "end": 69889740}, {"filename": "/GameData/textures/tormentPack/str_metalpan7.png", "start": 69889740, "end": 69928969}, {"filename": "/GameData/textures/tormentPack/str_metalpan8.png", "start": 69928969, "end": 69968063}, {"filename": "/GameData/textures/tormentPack/str_rotwoodgen1.png", "start": 69968063, "end": 69986642}, {"filename": "/GameData/textures/tormentPack/str_rotwoodgen2.png", "start": 69986642, "end": 70005938}, {"filename": "/GameData/textures/tormentPack/str_rotwoodgen3.png", "start": 70005938, "end": 70025992}, {"filename": "/GameData/textures/tormentPack/str_stonebrk1.png", "start": 70025992, "end": 70051384}, {"filename": "/GameData/textures/tormentPack/str_stonebrk2.png", "start": 70051384, "end": 70078386}, {"filename": "/GameData/textures/tormentPack/str_stonebrk3.png", "start": 70078386, "end": 70101016}, {"filename": "/GameData/textures/tormentPack/str_stonebrk4.png", "start": 70101016, "end": 70125069}, {"filename": "/GameData/textures/tormentPack/str_stonebrk5.png", "start": 70125069, "end": 70156153}, {"filename": "/GameData/textures/tormentPack/str_stonebrk6.png", "start": 70156153, "end": 70188825}, {"filename": "/GameData/textures/tormentPack/str_stonebrk7.png", "start": 70188825, "end": 70217349}, {"filename": "/GameData/textures/tormentPack/str_stonebrk8.png", "start": 70217349, "end": 70247601}, {"filename": "/GameData/textures/tormentPack/str_stoneflr1.png", "start": 70247601, "end": 70271757}, {"filename": "/GameData/textures/tormentPack/str_stoneflr2.png", "start": 70271757, "end": 70299663}, {"filename": "/GameData/textures/tormentPack/str_stoneflr3.png", "start": 70299663, "end": 70320668}, {"filename": "/GameData/textures/tormentPack/str_stoneflr4.png", "start": 70320668, "end": 70341812}, {"filename": "/GameData/textures/tormentPack/str_stoneflr5.png", "start": 70341812, "end": 70373344}, {"filename": "/GameData/textures/tormentPack/str_stoneflr6.png", "start": 70373344, "end": 70407627}, {"filename": "/GameData/textures/tormentPack/str_stonegen1.png", "start": 70407627, "end": 70426650}, {"filename": "/GameData/textures/tormentPack/str_stonegen2.png", "start": 70426650, "end": 70446021}, {"filename": "/GameData/textures/tormentPack/str_stonegen3.png", "start": 70446021, "end": 70467793}, {"filename": "/GameData/textures/tormentPack/str_stonegen4.png", "start": 70467793, "end": 70492838}, {"filename": "/GameData/textures/tormentPack/str_stonegen5.png", "start": 70492838, "end": 70518226}, {"filename": "/GameData/textures/tormentPack/str_stonegen6.png", "start": 70518226, "end": 70545663}, {"filename": "/GameData/textures/tormentPack/str_stonerubble.png", "start": 70545663, "end": 70574057}, {"filename": "/GameData/textures/tormentPack/str_stonewall1.png", "start": 70574057, "end": 70597314}, {"filename": "/GameData/textures/tormentPack/str_stonewall2.png", "start": 70597314, "end": 70622119}, {"filename": "/GameData/textures/tormentPack/str_stonewall3.png", "start": 70622119, "end": 70646782}, {"filename": "/GameData/textures/tormentPack/str_stonewall4.png", "start": 70646782, "end": 70673252}, {"filename": "/GameData/textures/tormentPack/str_stonewall5.png", "start": 70673252, "end": 70700719}, {"filename": "/GameData/textures/tormentPack/str_stonewall6.png", "start": 70700719, "end": 70728887}, {"filename": "/GameData/textures/tormentPack/str_stonewall7.png", "start": 70728887, "end": 70758276}, {"filename": "/GameData/textures/tormentPack/str_stonewall8.png", "start": 70758276, "end": 70789172}, {"filename": "/GameData/textures/tormentPack/{str_bloodgunk.png", "start": 70789172, "end": 70815621}, {"filename": "/GameData/textures/tormentPack/{str_bloodweb1.png", "start": 70815621, "end": 70832861}, {"filename": "/GameData/textures/tormentPack/{str_bloodweb2.png", "start": 70832861, "end": 70841812}, {"filename": "/GameData/textures/tormentPack/{str_bloodweb3.png", "start": 70841812, "end": 70847823}, {"filename": "/GameData/textures/tormentPack/{str_grating1.png", "start": 70847823, "end": 70856404}, {"filename": "/GameData/textures/tormentPack/{str_grating2.png", "start": 70856404, "end": 70871607}, {"filename": "/GameData/textures/tormentPack/{str_grating3.png", "start": 70871607, "end": 70890955}, {"filename": "/GameData/textures/tormentPack/{str_grating4.png", "start": 70890955, "end": 70903112}, {"filename": "/GameData/textures/tormentPack/{str_grating5.png", "start": 70903112, "end": 70924617}, {"filename": "/GameData/textures/tormentPack/{str_grating6.png", "start": 70924617, "end": 70952863}, {"filename": "/GameData/textures/wall/brickWall1.png", "start": 70952863, "end": 70961079}, {"filename": "/GameData/textures/wall/brickWall2.png", "start": 70961079, "end": 70969203}, {"filename": "/GameData/textures/wall/brickWall3.png", "start": 70969203, "end": 70977086}, {"filename": "/GameData/textures/water/Water1_t.png", "start": 70977086, "end": 71290187}, {"filename": "/GameData/textures/wood/wood1.png", "start": 71290187, "end": 71638539}, {"filename": "/GameData/title_bg_crop.png", "start": 71638539, "end": 72553647}, {"filename": "/GameData/yummy.ogg", "start": 72553647, "end": 72589238, "audio": 1}], "remote_package_size": 72589238});

  })();

// end include: C:\Users\Admin\AppData\Local\Temp\tmp21qreegl.js
// include: C:\Users\Admin\AppData\Local\Temp\tmpy0hcz4ld.js

    // All the pre-js content up to here must remain later on, we need to run
    // it.
    if (Module['$ww'] || (typeof ENVIRONMENT_IS_PTHREAD != 'undefined' && ENVIRONMENT_IS_PTHREAD)) Module['preRun'] = [];
    var necessaryPreJSTasks = Module['preRun'].slice();
  // end include: C:\Users\Admin\AppData\Local\Temp\tmpy0hcz4ld.js
// include: C:\Users\Admin\AppData\Local\Temp\tmpdqbeboco.js

    if (!Module['preRun']) throw 'Module.preRun should exist because file support used it; did a pre-js delete it?';
    necessaryPreJSTasks.forEach((task) => {
      if (Module['preRun'].indexOf(task) < 0) throw 'All preRun tasks that exist before user pre-js code should remain after; did you replace Module or modify Module.preRun?';
    });
  // end include: C:\Users\Admin\AppData\Local\Temp\tmpdqbeboco.js


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
  1520020: () => { if (typeof(AudioContext) !== 'undefined') { return true; } else if (typeof(webkitAudioContext) !== 'undefined') { return true; } return false; },  
 1520167: () => { if ((typeof(navigator.mediaDevices) !== 'undefined') && (typeof(navigator.mediaDevices.getUserMedia) !== 'undefined')) { return true; } else if (typeof(navigator.webkitGetUserMedia) !== 'undefined') { return true; } return false; },  
 1520401: ($0) => { if(typeof(Module['SDL2']) === 'undefined') { Module['SDL2'] = {}; } var SDL2 = Module['SDL2']; if (!$0) { SDL2.audio = {}; } else { SDL2.capture = {}; } if (!SDL2.audioContext) { if (typeof(AudioContext) !== 'undefined') { SDL2.audioContext = new AudioContext(); } else if (typeof(webkitAudioContext) !== 'undefined') { SDL2.audioContext = new webkitAudioContext(); } if (SDL2.audioContext) { if ((typeof navigator.userActivation) === 'undefined') { autoResumeAudioContext(SDL2.audioContext); } } } return SDL2.audioContext === undefined ? -1 : 0; },  
 1520953: () => { var SDL2 = Module['SDL2']; return SDL2.audioContext.sampleRate; },  
 1521021: ($0, $1, $2, $3) => { var SDL2 = Module['SDL2']; var have_microphone = function(stream) { if (SDL2.capture.silenceTimer !== undefined) { clearInterval(SDL2.capture.silenceTimer); SDL2.capture.silenceTimer = undefined; SDL2.capture.silenceBuffer = undefined } SDL2.capture.mediaStreamNode = SDL2.audioContext.createMediaStreamSource(stream); SDL2.capture.scriptProcessorNode = SDL2.audioContext.createScriptProcessor($1, $0, 1); SDL2.capture.scriptProcessorNode.onaudioprocess = function(audioProcessingEvent) { if ((SDL2 === undefined) || (SDL2.capture === undefined)) { return; } audioProcessingEvent.outputBuffer.getChannelData(0).fill(0.0); SDL2.capture.currentCaptureBuffer = audioProcessingEvent.inputBuffer; dynCall('vi', $2, [$3]); }; SDL2.capture.mediaStreamNode.connect(SDL2.capture.scriptProcessorNode); SDL2.capture.scriptProcessorNode.connect(SDL2.audioContext.destination); SDL2.capture.stream = stream; }; var no_microphone = function(error) { }; SDL2.capture.silenceBuffer = SDL2.audioContext.createBuffer($0, $1, SDL2.audioContext.sampleRate); SDL2.capture.silenceBuffer.getChannelData(0).fill(0.0); var silence_callback = function() { SDL2.capture.currentCaptureBuffer = SDL2.capture.silenceBuffer; dynCall('vi', $2, [$3]); }; SDL2.capture.silenceTimer = setInterval(silence_callback, ($1 / SDL2.audioContext.sampleRate) * 1000); if ((navigator.mediaDevices !== undefined) && (navigator.mediaDevices.getUserMedia !== undefined)) { navigator.mediaDevices.getUserMedia({ audio: true, video: false }).then(have_microphone).catch(no_microphone); } else if (navigator.webkitGetUserMedia !== undefined) { navigator.webkitGetUserMedia({ audio: true, video: false }, have_microphone, no_microphone); } },  
 1522714: ($0, $1, $2, $3) => { var SDL2 = Module['SDL2']; SDL2.audio.scriptProcessorNode = SDL2.audioContext['createScriptProcessor']($1, 0, $0); SDL2.audio.scriptProcessorNode['onaudioprocess'] = function (e) { if ((SDL2 === undefined) || (SDL2.audio === undefined)) { return; } if (SDL2.audio.silenceTimer !== undefined) { clearInterval(SDL2.audio.silenceTimer); SDL2.audio.silenceTimer = undefined; SDL2.audio.silenceBuffer = undefined; } SDL2.audio.currentOutputBuffer = e['outputBuffer']; dynCall('vi', $2, [$3]); }; SDL2.audio.scriptProcessorNode['connect'](SDL2.audioContext['destination']); if (SDL2.audioContext.state === 'suspended') { SDL2.audio.silenceBuffer = SDL2.audioContext.createBuffer($0, $1, SDL2.audioContext.sampleRate); SDL2.audio.silenceBuffer.getChannelData(0).fill(0.0); var silence_callback = function() { if ((typeof navigator.userActivation) !== 'undefined') { if (navigator.userActivation.hasBeenActive) { SDL2.audioContext.resume(); } } SDL2.audio.currentOutputBuffer = SDL2.audio.silenceBuffer; dynCall('vi', $2, [$3]); SDL2.audio.currentOutputBuffer = undefined; }; SDL2.audio.silenceTimer = setInterval(silence_callback, ($1 / SDL2.audioContext.sampleRate) * 1000); } },  
 1523889: ($0, $1) => { var SDL2 = Module['SDL2']; var numChannels = SDL2.capture.currentCaptureBuffer.numberOfChannels; for (var c = 0; c < numChannels; ++c) { var channelData = SDL2.capture.currentCaptureBuffer.getChannelData(c); if (channelData.length != $1) { throw 'Web Audio capture buffer length mismatch! Destination size: ' + channelData.length + ' samples vs expected ' + $1 + ' samples!'; } if (numChannels == 1) { for (var j = 0; j < $1; ++j) { setValue($0 + (j * 4), channelData[j], 'float'); } } else { for (var j = 0; j < $1; ++j) { setValue($0 + (((j * numChannels) + c) * 4), channelData[j], 'float'); } } } },  
 1524494: ($0, $1) => { var SDL2 = Module['SDL2']; var buf = $0 >>> 2; var numChannels = SDL2.audio.currentOutputBuffer['numberOfChannels']; for (var c = 0; c < numChannels; ++c) { var channelData = SDL2.audio.currentOutputBuffer['getChannelData'](c); if (channelData.length != $1) { throw 'Web Audio output buffer length mismatch! Destination size: ' + channelData.length + ' samples vs expected ' + $1 + ' samples!'; } for (var j = 0; j < $1; ++j) { channelData[j] = HEAPF32[buf + (j*numChannels + c)]; } } },  
 1524983: ($0) => { var SDL2 = Module['SDL2']; if ($0) { if (SDL2.capture.silenceTimer !== undefined) { clearInterval(SDL2.capture.silenceTimer); } if (SDL2.capture.stream !== undefined) { var tracks = SDL2.capture.stream.getAudioTracks(); for (var i = 0; i < tracks.length; i++) { SDL2.capture.stream.removeTrack(tracks[i]); } } if (SDL2.capture.scriptProcessorNode !== undefined) { SDL2.capture.scriptProcessorNode.onaudioprocess = function(audioProcessingEvent) {}; SDL2.capture.scriptProcessorNode.disconnect(); } if (SDL2.capture.mediaStreamNode !== undefined) { SDL2.capture.mediaStreamNode.disconnect(); } SDL2.capture = undefined; } else { if (SDL2.audio.scriptProcessorNode != undefined) { SDL2.audio.scriptProcessorNode.disconnect(); } if (SDL2.audio.silenceTimer !== undefined) { clearInterval(SDL2.audio.silenceTimer); } SDL2.audio = undefined; } if ((SDL2.audioContext !== undefined) && (SDL2.audio === undefined) && (SDL2.capture === undefined)) { SDL2.audioContext.close(); SDL2.audioContext = undefined; } },  
 1525989: ($0) => { var str = UTF8ToString($0) + '\n\n' + 'Abort/Retry/Ignore/AlwaysIgnore? [ariA] :'; var reply = window.prompt(str, "i"); if (reply === null) { reply = "i"; } return allocate(intArrayFromString(reply), 'i8', ALLOC_NORMAL); },  
 1526214: ($0, $1, $2) => { var w = $0; var h = $1; var pixels = $2; if (!Module['SDL2']) Module['SDL2'] = {}; var SDL2 = Module['SDL2']; if (SDL2.ctxCanvas !== Module['canvas']) { SDL2.ctx = Module['createContext'](Module['canvas'], false, true); SDL2.ctxCanvas = Module['canvas']; } if (SDL2.w !== w || SDL2.h !== h || SDL2.imageCtx !== SDL2.ctx) { SDL2.image = SDL2.ctx.createImageData(w, h); SDL2.w = w; SDL2.h = h; SDL2.imageCtx = SDL2.ctx; } var data = SDL2.image.data; var src = pixels / 4; var dst = 0; var num; if (typeof CanvasPixelArray !== 'undefined' && data instanceof CanvasPixelArray) { num = data.length; while (dst < num) { var val = HEAP32[src]; data[dst ] = val & 0xff; data[dst+1] = (val >> 8) & 0xff; data[dst+2] = (val >> 16) & 0xff; data[dst+3] = 0xff; src++; dst += 4; } } else { if (SDL2.data32Data !== data) { SDL2.data32 = new Int32Array(data.buffer); SDL2.data8 = new Uint8Array(data.buffer); SDL2.data32Data = data; } var data32 = SDL2.data32; num = data32.length; data32.set(HEAP32.subarray(src, src + num)); var data8 = SDL2.data8; var i = 3; var j = i + 4*num; if (num % 8 == 0) { while (i < j) { data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; data8[i] = 0xff; i = i + 4 | 0; } } else { while (i < j) { data8[i] = 0xff; i = i + 4 | 0; } } } SDL2.ctx.putImageData(SDL2.image, 0, 0); },  
 1527682: ($0, $1, $2, $3, $4) => { var w = $0; var h = $1; var hot_x = $2; var hot_y = $3; var pixels = $4; var canvas = document.createElement("canvas"); canvas.width = w; canvas.height = h; var ctx = canvas.getContext("2d"); var image = ctx.createImageData(w, h); var data = image.data; var src = pixels / 4; var dst = 0; var num; if (typeof CanvasPixelArray !== 'undefined' && data instanceof CanvasPixelArray) { num = data.length; while (dst < num) { var val = HEAP32[src]; data[dst ] = val & 0xff; data[dst+1] = (val >> 8) & 0xff; data[dst+2] = (val >> 16) & 0xff; data[dst+3] = (val >> 24) & 0xff; src++; dst += 4; } } else { var data32 = new Int32Array(data.buffer); num = data32.length; data32.set(HEAP32.subarray(src, src + num)); } ctx.putImageData(image, 0, 0); var url = hot_x === 0 && hot_y === 0 ? "url(" + canvas.toDataURL() + "), auto" : "url(" + canvas.toDataURL() + ") " + hot_x + " " + hot_y + ", auto"; var urlBuf = _malloc(url.length + 1); stringToUTF8(url, urlBuf, url.length + 1); return urlBuf; },  
 1528670: ($0) => { if (Module['canvas']) { Module['canvas'].style['cursor'] = UTF8ToString($0); } },  
 1528753: () => { if (Module['canvas']) { Module['canvas'].style['cursor'] = 'none'; } },  
 1528822: () => { return window.innerWidth; },  
 1528852: () => { return window.innerHeight; }
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

