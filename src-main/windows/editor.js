const fsPromises = require('fs/promises');
const path = require('path');
const nodeURL = require('url');
const zlib = require('zlib');
const nodeCrypto = require('crypto');
const {app, dialog} = require('electron');
const ProjectRunningWindow = require('./project-running-window');
const AddonsWindow = require('./addons');
const DesktopSettingsWindow = require('./desktop-settings');
const PrivacyWindow = require('./privacy');
const AboutWindow = require('./about');
const PackagerWindow = require('./packager');
const {createAtomicWriteStream} = require('../atomic-write-stream');
const {translate, updateLocale, getStrings} = require('../l10n');
const {APP_NAME} = require('../brand');
const prompts = require('../prompts');
const settings = require('../settings');
const privilegedFetch = require('../fetch');
const RichPresence = require('../rich-presence.js');
const FileAccessWindow = require('./file-access-window.js');
const ExtensionDocumentationWindow = require('./extension-documentation.js');
const MasterWindow = require('./master.js')
const ConnectWindow=require('./connect-device.js')
const DownloadCodeWindow = require('./download-code');
const BleConnectWindow = require('./ble-connect')
const {getWin,setWin} = require('../../utils/win.js')
const {setCode,setDown,setPlace} = require('../../utils/tempCode.js')
const {getPort} =require('../../utils/port')
const extensions = require('../../utils/extensionWho.js')
const socket =require('../../utils/socket')

const TYPE_FILE = 'file';
const TYPE_URL = 'url';
const TYPE_SCRATCH = 'scratch';
const TYPE_SAMPLE = 'sample';

//新增
const fs = require('fs');
const os = require("os");
const { spawn , exec} = require('child_process');
const { shell } = require("electron");

// const noble = require('@abandonware/noble');
const serialManager = require('../../utils/new/serialManager.js') //串口
const  { scanBleDevices,connectBleDevice,disBleDevice }  = require('../../utils/new/bleManager.js') //蓝牙


class OpenedFile {
  constructor (type, path) {
    /** @type {TYPE_FILE|TYPE_URL|TYPE_SCRATCH|TYPE_SAMPLE} */
    this.type = type;

    /**
     * Absolute file path or URL
     * @type {string}
     */
    this.path = path;
  }

  async read () {
    if (this.type === TYPE_FILE) {
      return {
        name: path.basename(this.path),
        data: await fsPromises.readFile(this.path)
      };
    }

    if (this.type === TYPE_URL) {
      const buffer = await privilegedFetch(this.path);
      return {
        name: decodeURIComponent(path.basename(this.path)),
        data: buffer
      };
    }

    if (this.type === TYPE_SCRATCH) {
      const metadata = await privilegedFetch.json(`https://api.scratch.mit.edu/projects/${this.path}`);
      const token = metadata.project_token;
      const title = metadata.title;

      const projectBuffer = await privilegedFetch(`https://projects.scratch.mit.edu/${this.path}?token=${token}`);
      return {
        name: title,
        data: projectBuffer
      };
    }

    if (this.type === TYPE_SAMPLE) {
      const sampleRoot = path.resolve(__dirname, '../../dist-extensions/samples/');
      const resolvedPath = path.join(sampleRoot, this.path);
      if (resolvedPath.startsWith(sampleRoot)) {
        const compressedPath = `${resolvedPath}.br`;
        const compressedData = await fsPromises.readFile(compressedPath);

        // dist-extensions is all brotli'd; must decompress
        const decompressedData = await new Promise((resolve, reject) => {
          zlib.brotliDecompress(compressedData, (err, res) => {
            if (err) {
              reject(err);
            } else {
              resolve(res);
            }
          });
        });

        return {
          name: this.path,
          data: decompressedData
        };
      }
      throw new Error('Unsafe join');
    }

    throw new Error(`Unknown type: ${this.type}`);
  }
}

/**
 * @param {string} file
 * @param {string|null} workingDirectory
 * @returns {OpenedFile}
 */
const parseOpenedFile = (file, workingDirectory) => {
  let url;
  try {
    url = new URL(file);
  } catch (e) {
    // Error means it was not a valid full URL
  }

  if (url) {
    if (url.protocol === 'http:' || url.protocol === 'https:') {
      // Scratch URLs require special treatment as they are not direct downloads.
      const scratchMatch = file.match(/^https?:\/\/scratch\.mit\.edu\/projects\/(\d+)\/?/);
      if (scratchMatch) {
        return new OpenedFile(TYPE_SCRATCH, scratchMatch[1]);
      }

      // Need to manually redirect extension samples to the copies we already have offline as the
      // fetching code will not go through web request handlers or custom protocols.
      const sampleMatch = file.match(/^https?:\/\/extensions\.turbowarp\.org\/samples\/(.+\.sb3)$/);
      if (sampleMatch) {
        return new OpenedFile(TYPE_SAMPLE, decodeURIComponent(sampleMatch[1]));
      }

      return new OpenedFile(TYPE_URL, file);
    }

    // Parse file:// URLs.
    // Notably we receive these in the flatpak version of the app when we can only access a file through
    // the XDG document portal instead of having direct access with eg. --filesystem=home
    if (url.protocol === 'file:') {
      let filePath;
      try {
        filePath = nodeURL.fileURLToPath(file);
      } catch (e) {
        // Very unlikely but possible
      }

      if (filePath) {
        return new OpenedFile(TYPE_FILE, path.resolve(workingDirectory, filePath));
      }
    }

    // Don't throw an error just because we don't recognize the URL protocol as
    // Windows paths look close enough to real URLs to be parsed successfully.
  }

  return new OpenedFile(TYPE_FILE, path.resolve(workingDirectory, file));
};

/**
 * @returns {Array<{path: string; app: string;}>}
 */
const getUnsafePaths = () => {
  if (process.platform !== 'win32') {
    // This problem doesn't really exist on other platforms
    return [];
  }

  const localPrograms = path.join(app.getPath('home'), 'AppData', 'Local', 'Programs');
  const appData = app.getPath('appData');
  return [
    // Current app, regardless of where it is installed or how modded it is
    {
      path: path.dirname(app.getPath('exe')),
      app: APP_NAME,
    },
    {
      path: app.getPath('userData'),
      app: APP_NAME,
    },

    // TurboWarp Desktop defaults
    {
      path: path.join(appData, 'turbowarp-desktop'),
      app: 'TurboWarp Desktop'
    },
    {
      path: path.join(localPrograms, 'TurboWarp'),
      app: 'TurboWarp Desktop'
    },

    // Scratch Desktop defaults
    {
      path: path.join(appData, 'Scratch'),
      app: 'Scratch Desktop'
    },
    {
      path: path.join(localPrograms, 'Scratch 3'),
      app: 'Scratch Desktop'
    }
  ];
};

/**
 * @param {string} parent
 * @param {string} child
 * @returns {boolean}
 */
const isChildPath = (parent, child) => {
  const relative = path.relative(parent, child);
  return !!relative && !relative.startsWith('..') && !path.isAbsolute(relative);
};

/**
 * @returns {string} A unique string.
 */
const generateFileId = () => {
  // Note that we can't use the randomUUID from web crypto as we need to support Electron 22.
  return `desktop_file_id{${nodeCrypto.randomUUID()}}`;
};

class EditorWindow extends ProjectRunningWindow {
  /**
   * @param {OpenedFile|null} initialFile
   * @param {boolean} isInitiallyFullscreen
   */
  constructor (initialFile, isInitiallyFullscreen) {
    super();
    /**
     * Ideally we would revoke access after loading a new project, but our file handle handling in
     * the GUI isn't robust enough for that yet. We do at least use random file handle IDs which
     * makes it much harder for malicious code in the renderer process to enumerate all previously
     * opened IDs and overwrite them.
     * @type {Map<string, OpenedFile>}
     */
    EditorWindow.instance = this; // 保存最新的实例
    this.robotData=[
      [1],
      [1],
      [1],
      [1],
      [1],
      [1],
      [1],
      [1],
      [1],
      [1],
      [1],
      [1],
      [1],
      [1],
    ]
     
    this.window.webContents.setBackgroundThrottling(false);//禁止节流
    this.openedFiles = new Map();
    this.activeFileId = null;

    if (initialFile !== null) {
      this.activeFileId = generateFileId();
      this.openedFiles.set(this.activeFileId, initialFile);
    }

    this.openedProjectAt = Date.now();

    /**
     * @param {string} id
     * @returns {OpenedFile}
     * @throws if invalid ID
     */
    const getFileById = (id) => {
      if (!this.openedFiles.has(id)) {
        throw new Error('Invalid file ID');
      }
      return this.openedFiles.get(id);
    };

    this.window.webContents.on('will-prevent-unload', (event) => {
      const choice = dialog.showMessageBoxSync(this.window, {
        title: APP_NAME,
        type: 'info',
        buttons: [
          translate('unload.stay'),
          translate('unload.leave')
        ],
        cancelId: 0,
        defaultId: 0,
        message: translate('unload.message'),
        detail: translate('unload.detail'),
        noLink: true
      });
      if (choice === 1) {
        event.preventDefault();
      }
    });

    this.window.on('page-title-updated', (event, title, explicitSet) => {
      event.preventDefault();
      if (explicitSet && title) {
        this.window.setTitle(`${title} - ${APP_NAME}`);
        this.projectTitle = title;
      } else {
        this.window.setTitle(APP_NAME);
        this.projectTitle = '';
      }

      this.updateRichPresence();
    });
    this.window.setTitle(APP_NAME);

    this.window.on('focus', () => {
      this.updateRichPresence();
    });

    this.ipc.on('is-initially-fullscreen', (e) => {
      e.returnValue = isInitiallyFullscreen;
    });

    this.ipc.handle('get-initial-file', () => {
      return this.activeFileId;
    });

    this.ipc.handle('get-file', async (event, id) => {
      const file = getFileById(id);
      const {name, data} = await file.read();
      return {
        name,
        type: file.type,
        data
      };
    });

    this.ipc.on('set-locale', async (event, locale) => {
      if (settings.locale !== locale) {
        settings.locale = locale;
        updateLocale(locale);

        // Imported late due to circular dependency
        const rebuildMenuBar = require('../menu-bar');
        rebuildMenuBar();

        // Let the save happen in the background, not important
        Promise.resolve().then(() => settings.save());
      }
      event.returnValue = {
        strings: getStrings(),
        mas: !!process.mas
      };
    });

    this.ipc.handle('set-changed', (event, changed) => {
      this.window.setDocumentEdited(changed);
    });

    this.ipc.handle('opened-file', (event, id) => {
      const file = getFileById(id);
      if (file.type !== TYPE_FILE) {
        throw new Error('Not a file');
      }
      this.activeFileId = id;
      this.openedProjectAt = Date.now();
      this.window.setRepresentedFilename(file.path);
    });

    this.ipc.handle('closed-file', () => {
      this.activeFileId = null;
      this.window.setRepresentedFilename('');
    });

    this.ipc.handle('show-open-file-picker', async () => {
      const result = await dialog.showOpenDialog(this.window, {
        properties: ['openFile'],
        defaultPath: settings.lastDirectory,
        filters: [
          {
            name: 'Scratch Project',
            extensions: ['sb3', 'sb2', 'sb'],
          }
        ]
      });
      if (result.canceled) {
        return null;
      }

      const filePath = result.filePaths[0];
      settings.lastDirectory = path.dirname(filePath);
      await settings.save();

      const id = generateFileId();
      this.openedFiles.set(id, new OpenedFile(TYPE_FILE, filePath));

      return {
        id,
        name: path.basename(filePath)
      };
    });

    this.ipc.handle('show-save-file-picker', async (event, suggestedName) => {
      const result = await dialog.showSaveDialog(this.window, {
        defaultPath: path.join(settings.lastDirectory, suggestedName),
        filters: [
          {
            name: 'Scratch 3 Project',
            extensions: ['sb3'],
          }
        ]
      });
      if (result.canceled) {
        return null;
      }

      const filePath = result.filePath;

      const unsafePath = getUnsafePaths().find(i => isChildPath(i.path, filePath));
      if (unsafePath) {
        // No need to block until the message box is closed
        dialog.showMessageBox(this.window, {
          type: 'error',
          title: APP_NAME,
          message: translate('unsafe-path.title'),
          detail: translate(`unsafe-path.details`)
            .replace('{APP_NAME}', unsafePath.app)
            .replace('{file}', filePath),
          noLink: true
        });  
        return null;
      }

      settings.lastDirectory = path.dirname(filePath);
      await settings.save();

      const id = generateFileId();
      this.openedFiles.set(id, new OpenedFile(TYPE_FILE, filePath));

      return {
        id,
        name: path.basename(filePath)
      };
    });

    this.ipc.handle('get-preferred-media-devices', () => {
      return {
        microphone: settings.microphone,
        camera: settings.camera
      };
    });

    this.ipc.on('start-write-stream', async (startEvent, id) => {
      const file = getFileById(id);
      if (file.type !== TYPE_FILE) {
        throw new Error('Not a file');
      }

      const port = startEvent.ports[0];

      /** @type {NodeJS.WritableStream|null} */
      let writeStream = null;

      const handleError = (error) => {
        console.error('Write stream error', error);
        port.postMessage({
          error
        });

        // Make sure the port is started in case we encounter an error before we normally
        // begin to accept messages.
        port.start();
      };

      try {
        writeStream = await createAtomicWriteStream(file.path);
      } catch (error) {
        handleError(error);
        return;
      }

      writeStream.on('atomic-error', handleError);

      const handleMessage = (data) => {
        if (data.write) {
          if (writeStream.write(data.write)) {
            // Still more space in the buffer. Ask for more immediately.
            return;
          }
          // Wait for the buffer to become empty before asking for more.
          return new Promise(resolve => {
            writeStream.once('drain', resolve);
          });
        } else if (data.finish) {
          // Wait for the atomic file write to complete.
          return new Promise(resolve => {
            writeStream.once('atomic-finish', resolve);
            writeStream.end();
          });
        } else if (data.abort) {
          writeStream.emit('error', new Error('Aborted by renderer process'));
          return;
        }
        throw new Error('Unknown message from renderer');
      };

      port.on('message', async (messageEvent) => {
        try {
          const data = messageEvent.data;
          const id = data.id;
          const result = await handleMessage(data);
          port.postMessage({
            response: {
              id,
              result
            }
          });
        } catch (error) {
          handleError(error);
        }
      });

      port.start();
    });

    this.ipc.on('alert', (event, message) => {
      event.returnValue = prompts.alert(this.window, message);
    });

    this.ipc.on('confirm', (event, message) => {
      event.returnValue = prompts.confirm(this.window, message);
    });

    this.ipc.handle('open-packager', () => {
      PackagerWindow.forEditor(this);
    });

    this.ipc.handle('open-new-window', () => {
      EditorWindow.newWindow();
    });

    this.ipc.handle('open-addon-settings', (event, search) => {
      AddonsWindow.show(search);
    });

    this.ipc.handle('open-desktop-settings', () => {
      DesktopSettingsWindow.show();
    });

    this.ipc.handle('open-privacy', () => {
      PrivacyWindow.show();
    });

    this.ipc.handle('open-about', () => {
      AboutWindow.show();
    });

    // this.ipc.handle('open-master-window', () => {
    //   MasterWindow.show();
      
    // });
    // this.ipc.handle('open-connect-window', () => {
    //   // console.log('#######################')
    //   ConnectWindow.show();
      
    // });
    // this.ipc.handle('open-download-settings', (event,code) => {
    //   DownloadCodeWindow.show(code);
    // });
    // this.ipc.handle('open-ble-settings', () => {
    //   if(getWin()){
    //     getWin().show()
    //   }else{
    //     BleConnectWindow.show();
    //   }
      
      
    // });
    this.ipc.on('get-robot-data', (event) => {
      // console.log(this.robotData)
      event.returnValue = this.robotData
    });
    this.ipc.handle('download', (event,code,args) => {
      console.log('--------------')
      console.log(code)
      setCode(code)
      setDown(1)
      setPlace(args)
    });
    let PORT=getPort()
     // 发送数据并等待接收特定数据后再继续
     async function sendDataAndWait(dataToSend) {
      return new Promise(async (resolve, reject) => {
        // 发送数据
        await PORT.write(dataToSend, (err) => {
          if (err) {
            return reject('Error on write: ' + err.message);
          }

          console.log(`Data sent: ${dataToSend}`);
        });

        const onDataReceived = (data) => {
          console.log('Data received:', data.toString());
          console.log(typeof (data.toString()));
        
          // 检查是否是我们想要的响应（例如，'0'）
          if(data.toString().includes('74')){
            console.log('Received 74, completed');
            PORT.removeListener('data', onDataReceived); // 使用 removeListener 停止监听
            resolve(); // 继续执行
          }else if (data.toString().includes('71')) {
            console.log('Received 71, continuing...');
            PORT.removeListener('data', onDataReceived); // 使用 removeListener 停止监听
            resolve(); // 继续执行
          }
        };
        
        PORT.on('data', onDataReceived); // 添加 data 事件的监听器

        // 可选：添加一个超时机制，防止长时间等待
        setTimeout(() => {
          reject('Timeout: No response received in time.');
        }, 5000); // 5秒超时
      });
    }


    function crc16(arr) {
        let crc = 0xFFFF;
        for (let i = 0; i < arr.length; i++) {
          crc ^= arr[i];
          for (let j = 0; j < 8; j++) {
            if (crc & 0x0001) {
              crc = (crc >> 1) ^ 0xA001;
            } else {
              crc >>= 1;
            }
            crc &= 0xFFFF; // 保持 16 位
          }
        }
        return crc;
      }


      let bufferData = '';
      function createPromiseForSerial(port) {
        return new Promise((resolve, reject) => {
          const onData = (data) => {
            // const value = data[0]; // 假设返回单字节
            // console.log("收到串口数据:", value);

            let value
            bufferData += data.toString();
            if (bufferData.endsWith('\r\n')) {
              const message = bufferData.trim();
              bufferData = '';

              try {
                // const parsed = JSON.parse(message);
                // console.log(parsed)

                  let parsed;

                // 判断是否是 ESP32 特殊格式 {[…]}
                if (/^\{\[.*\]\}$/.test(message)) {
                  const match = message.match(/\[(.*?)\]/);
                  if (match) {
                    parsed = match[1].split(',').map(n => Number(n.trim()));
                  }
                } else {
                  // 如果是 JSON 就解析，否则丢异常走 catch
                  parsed = JSON.parse(message);
                }
                value=parsed
              } catch {
                value=message
              }
            }

            if ((Array.isArray(value) && value.length==1 && value[0] === 0) || (typeof value === "string" && value.includes("[0]"))) {
              port.off('data', onData); // 收到 0 就解绑
              resolve(0);
            }
            // 如果不是 0，继续等，不 resolve
          };

          port.on('data', onData);
        });
      }



    this.ipc.handle('serial-download', async(event,code) => {
      console.log(extensions.getExtension())
      PORT=getPort()
      console.log(PORT)
      if(extensions.getExtension()==1){
        console.log('#########################################')
        sendDataAndWait('Lua:').then(() => {
          sendDataAndWait(code.code).then(() => {
            sendDataAndWait('endLua')
          })
        })

      }else if(extensions.getExtension()==2){
        let p1 = createPromiseForSerial(PORT);
        let downloadCode=code.code
        // if (!downloadCode.includes('while')) {
        //     // 2. 如果没有 'while' 循环，拼接一个
        //     downloadCode += '\nwhile True:\n    pass';
        // }
        downloadCode+='\n'

        let jsonData={
          "command": "upload_script",
          "params": 
              {
                  "name": `${code.place}.py`,            // 字符串：1-5.py
                  "script":downloadCode,            //字符串：程序内容 
              }
        }
        let str=JSON.stringify(jsonData)
        str+='\n'
        console.log(str)

        await PORT.write(str, async(err) => {
          if (err) {
            return reject('Error on write: ' + err.message);
          }

          await p1;
          console.log(`Data sent: ${str}`);
          console.log("所有数据包发送完毕 ✅");
          if (socket.getSocket()) {
            socket.getSocket().send(JSON.stringify({
              type: 'serialSuccess',
              data: { message: true }
            }));
          }
        });
      }
      
      
    });
    this.ipc.handle('cancelload', () => {
      setDown(2)

    });
    this.ipc.handle('get-advanced-customizations', async () => {
      const USERSCRIPT_PATH = path.join(app.getPath('userData'), 'userscript.js');
      const USERSTYLE_PATH = path.join(app.getPath('userData'), 'userstyle.css');

      const [userscript, userstyle] = await Promise.all([
        fsPromises.readFile(USERSCRIPT_PATH, 'utf-8').catch(() => ''),
        fsPromises.readFile(USERSTYLE_PATH, 'utf-8').catch(() => '')
      ]);

      return {
        userscript,
        userstyle
      };
    });

    this.ipc.handle('check-drag-and-drop-path', (event, filePath) => {
      FileAccessWindow.check(filePath);
    });

    /**
     * Refers to the full screen button in the editor, not the OS-level fullscreen through
     * F11/Alt+Enter (Windows, Linux) or buttons provided by the OS (macOS).
     */
    this.isInEditorFullScreen = false;

    this.ipc.handle('set-is-full-screen', (event, isFullScreen) => {
      this.isInEditorFullScreen = !!isFullScreen;
    });


    // 其他的功能没有仔细探究，下面为新增
    // -----------------------------------------------------------------------------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------------

    //初始化
    serialManager.serialInitialize(EditorWindow.instance.window)
    // 扫描可用串口（根据设备类型自由扫描）
    this.ipc.handle('serial-scan', async (event,deviceType) => {
      return serialManager.scanSerialDevice(deviceType)
    });    

    // 连接指定串口
    this.ipc.handle('serial-connect', async (event, deviceInfo,deviceType) => {
      return serialManager.connectSerialDevice(deviceInfo,deviceType)
    });

    // 断开串口连接
    this.ipc.handle('serial-disconnect', async () => {
      return serialManager.disconnectSerialDevice()
    });

    //进入烧录模式
    this.ipc.handle('serial-exit-repl', async () => {
      return serialManager.replExitSerial();
    });
 
    //进入repl模式
    this.ipc.handle('serial-enter-repl', async () => {
      return serialManager.replSerial("Microbit");
    });

    //进入repl模式esp
    this.ipc.handle('serial-enter-repl-esp', async () => {
      return serialManager.replSerial("ESP32");
    });

    

    // 发送数据
    this.ipc.handle('serial-send-command', async (_, command,type) => {
      return serialManager.sendCommandSerial(command,type);
    });
    // 发送数据
    this.ipc.handle('serial-send-command-direct', async (_, command,type) => {
      return serialManager.sendCommandSerial_D(command,type);
    });
    

    // microbit USB 直接运行程序
    this.ipc.handle('microbit-usb-run', async (_, code) => {
      return serialManager.mB_runCodeSerial(code);
    });
    
    // USB 程序下载
    this.ipc.handle('usb-download-flash', async (_, code) => {
      return serialManager.downloadCodeSerial(code);
    });

    // 中止USB 程序下载
    this.ipc.handle('flash-cancel', async () => {
      return serialManager.cancelDownloadCodeSerial(); 
    });

    // 固件烧录
    this.ipc.handle('flash-firmware-all', async (_,device, firmwareName, port) => {
      return serialManager.unifiedFlashFirmware(device, firmwareName, port); 
    });
    
  
    // -----------------------------------------------------------------------------------------------------------------------------------


    // ############################################### Arduino相关 ###############################################
    //烧录Arduino代码
    this.ipc.handle('download-arduino-code', async (_, code) => {
      // if (!serialDeviceState.serialPort) {
      //   return { success: false, error: "未连接设备" };
      // }
      // console.log(serialDeviceState.serialPort.path)

      try {
        const sketchDir = path.join(os.tmpdir(), "mysketch");
        if (!fs.existsSync(sketchDir)) fs.mkdirSync(sketchDir);

        const sketchFile = path.join(sketchDir, "mysketch.ino");
        fs.writeFileSync(sketchFile, code);

        // 安装 R4 支持包（可放在初始化位置）
        //await runCli(["core", "install", "arduino:renesas_uno"]);


        // 编译 
        await runCli([
          'compile',
          '--fqbn', 'arduino:renesas_uno:unor4wifi',
          sketchDir
        ]);

        // 上传 
        await runCli([
          'upload',
          '-p', "COM20",//serialDeviceState.serialPort.path,
          '--fqbn', 'arduino:renesas_uno:unor4wifi',
          sketchDir
        ]);

        return { success: true };

      } catch (err) {
        console.log("1111", err.message);
        return { success: false, error: err.message };
      }
    });

    // 调用cli
    function runCli(args) {
      const config = getResourcePath1('/Arduino/arduino-cli.yaml');
      const cliPath = getResourcePath1('/Arduino/arduino-cli.exe');
      return new Promise((resolve, reject) => {
        console.log("RUN:", cliPath, args.join(" ")); // ⭐ 打印执行命令
        const proc = spawn(cliPath, ["--config-file", config, ...args], { shell: true });
    
        proc.stdout.on('data', d => {
          console.log("[CLI stdout]:", d.toString());  // ⭐ 打印 stdout
        });
        proc.stderr.on('data', d => {
          console.error("[CLI stderr]:", d.toString()); // ⭐ 打印 stderr
        });
    
        proc.on('close', code => {
          console.log("CLI exit code:", code);          // ⭐ 打印退出码
    
          if (code === 0) resolve();
          else reject(new Error("CLI Error, exitCode=" + code));
        });
      });
    }

    // ###########################################################################################################

    // ------------------------------------python相关--------------------------------------------------------------------------------------
    // 获取用户数据目录
    function getResourcePath1(relativePath) {
      if (app.isPackaged) {
        // 打包后
        return path.join(__dirname, '../../../utils', relativePath);
      } else {
        // 开发环境
        return path.join(__dirname, '../../utils', relativePath);
      }
    }
    //const projectsDir = path.join(app.getPath('userData'), 'PythonProjects');//缓存中方案
    const projectsDir = getResourcePath1("/PythonProjects");//path.join(__dirname, '../../utils/PythonProjects');//项目中

    const pythonPath = getResourcePath1("/Python313/python.exe");//path.join(__dirname, '../../utils/Python313/python.exe');
    
    // 读取文件树
    function readDirectoryRecursive(dir) {
      const items = fs.readdirSync(dir, { withFileTypes: true });
  
      let folders = [];
      let files = [];
  
      for (const item of items) {
          const fullPath = path.join(dir, item.name);
  
          if (item.isDirectory()) {
              folders.push({
                  type: "folder",
                  name: item.name,
                  children: readDirectoryRecursive(fullPath)
              });
          } else if (item.isFile()) {
              const isPy = item.name.endsWith(".py");
  
              files.push({
                  type: isPy ? "file" : "other",
                  name: item.name,
                  path: fullPath
              });
          }
      }
  
      // 分别排序，文件夹在前，文件在后
      folders.sort((a, b) => a.name.localeCompare(b.name));
      files.sort((a, b) => a.name.localeCompare(b.name));
  
      return [...folders, ...files];
    }
  
    
    // 获取项目文件列表(带文件树结构)
    this.ipc.handle('python-files:list', async () => {
        return readDirectoryRecursive(projectsDir);
    });
    
    // 读取文件内容
    this.ipc.handle('python-files:read', async (event, fileName) => {
        const filePath = path.join(projectsDir, fileName);
        if (fs.existsSync(filePath)) {
            return fs.readFileSync(filePath, 'utf-8');
        }
        return '';
    });
    
    // 保存文件
    this.ipc.handle('python-files:save', async (event, fileName, content) => {
        const filePath = path.join(projectsDir, fileName);
        fs.writeFileSync(filePath, content, 'utf-8');
        return true;
    });
    
    // 删除操作
    this.ipc.handle('python-files:delete', async (event, fileName) => {
        const filePath = path.join(projectsDir, fileName);
        if (!fs.existsSync(filePath)) return true;

        const stat = fs.statSync(filePath);

        if (stat.isDirectory()) {// 删除目录
            fs.rmSync(filePath, { recursive: true, force: true });
        } else {// 删除文件
            fs.unlinkSync(filePath);
        }

        return true;
    });
    
    // 新建py文件
    this.ipc.handle('python-files:new', async (event, baseName = 'newfile') => {
      /* 自动添加后缀，但是目前不使用了 */
        // let i = 1;
        // let fileName;
        // do {
        //     fileName = `${baseName}(${i}).py`;
        //     i++;
        // } while (fs.existsSync(path.join(projectsDir, fileName)));
    
        fs.writeFileSync(path.join(projectsDir, baseName), '\n', 'utf-8');
        return baseName;
    });

    //改名
    this.ipc.handle('python-files:rename', async (event, oldName, newName) => {
      const oldPath = path.join(projectsDir, oldName);
      const newPath = path.join(projectsDir, newName);
      fs.renameSync(oldPath, newPath);
      return true;
    });

    // 新建文件夹
    this.ipc.handle('python-folders:new', async (event, dirName) => {
      const folderPath = path.join(projectsDir, dirName);
      if (!fs.existsSync(folderPath)) {
          fs.mkdirSync(folderPath);
      }
      return true;
    });


    //重命名文件夹
    this.ipc.handle('python-folders:rename', async (event, oldName, newName, type) => {
      const oldPath = path.join(projectsDir, oldName);
      const newPath = path.join(projectsDir, newName);
      fs.renameSync(oldPath, newPath);
      return true;
    });

    // 上传文件/文件夹
    this.ipc.handle("python-files:upload", async (event, targetDir, relativePath, arrayBuffer) => {
      try {
          // 转成可写入的 Uint8Array
          const uint8 = new Uint8Array(arrayBuffer);
  
          const baseDir = path.join(projectsDir, targetDir);
          const finalPath = path.join(baseDir, relativePath);
  
          fs.mkdirSync(path.dirname(finalPath), { recursive: true });
          fs.writeFileSync(finalPath, uint8);
  
          return true;
      } catch (err) {
          console.error("Upload error:", err);
          return false;
      }
    });

    // 打开文件 / 文件夹所在位置
    this.ipc.handle("python-files:show-in-folder", async (event, relativePath) => {
      try {
          const fullPath = path.join(projectsDir, relativePath);

          if (!fs.existsSync(fullPath)) return false;

          shell.showItemInFolder(fullPath);

          return true;
      } catch (e) {
          console.error("show-in-folder error:", e);
          return false;
      }
    });

  
  
  
  
  
    






    let pyProcess = null;
    let runId = 0;          // 每次 run +1
    let currentRunId = 0;  // 当前进程归属


    //运行python  
    this.ipc.handle('python:run', async (event, fileName) => {
      try {
        const filePath = path.join(projectsDir, fileName);
        if (!fs.existsSync(filePath)) {
          throw new Error(`文件不存在: ${filePath}`);
        }

        // 清理已有进程
        if (pyProcess) {
          pyProcess.kill();
          pyProcess = null;
        }
        // 新 run
        runId++;
        currentRunId = runId;

        // 调用 Python 执行
        pyProcess = spawn(pythonPath, ['-u',filePath], {//'-i',
          cwd: projectsDir, 
          env: {
            ...process.env,
            PYTHONPATH: projectsDir,
            PYTHONIOENCODING: 'utf-8',
          },
          //detached: true,        // 成为独立进程
          stdio: ['pipe', 'pipe', 'pipe']
          // env: process.env,
          // stdio: 'pipe'
        });
        //pyProcess.unref();
        const myRunId = currentRunId;


        let stdoutBuffer = '';
        let lastFlush = Date.now();
        pyProcess.stdout.on('data', (data) => {
          if (myRunId !== currentRunId) return;
          stdoutBuffer += data.toString();
          const now = Date.now();
          if (now - lastFlush > 50) { // 20 FPS
              EditorWindow.instance.window.webContents.send( 'python-log', stdoutBuffer );
              stdoutBuffer = '';
              lastFlush = now;
          }        
        });

        pyProcess.stderr.on('data', (data) => {
          if (myRunId !== currentRunId) return;
          EditorWindow.instance.window.webContents.send('python-error', data.toString());
        });

        pyProcess.on('close', (code) => {
          if (myRunId !== currentRunId) return;
          if (stdoutBuffer.length > 0 ) {
            EditorWindow.instance.window.webContents.send('python-log', stdoutBuffer );
          }
          stdoutBuffer = '';
          EditorWindow.instance.window.webContents.send('python-exit', code);
        });

        return { success: true };
      } catch (err) {
        console.error('Run Python error:', err);
        return { success: false, error: err.message };
      }
    });


    // python停止
    this.ipc.handle('python-stop', async () => {
      if (!pyProcess) return false;
      currentRunId++;
  
      try {
          if (process.platform === 'win32') {
              // 强制杀整个进程树
              exec(`taskkill /pid ${pyProcess.pid} /T /F`);
          } else {
              pyProcess.kill('SIGKILL');
          }
      } catch (e) {
          console.error('kill error', e);
      } finally {
          pyProcess = null;
      }
  
      return true;
    });
  

    // python单独的输入命令
    this.ipc.on('python:input', (event, input) => {
      if (pyProcess && pyProcess.stdin.writable) {
        pyProcess.stdin.write(input + '\n'); // 将命令写入 Python stdin
      }
    });


    let currentPipProcess = null; // 当前 pip 进程引用，用于终止
    // pip命令
    this.ipc.handle('pip-action', (event, action, pkg) => {
      return new Promise((resolve) => {

        if (action === 'cancel') {
          if (currentPipProcess) {
            currentPipProcess.kill('SIGTERM');
            currentPipProcess = null;
            resolve({ success: true, data: '已终止安装' });
          } else {
            resolve({ success: false, error: '没有正在运行的 pip 进程' });
          }
          return;
        }

        let args = ['-m', 'pip'];
    
        if (action === 'list') {
          args.push('list', '--format=json'); // ✅ 用 JSON 格式返回
        } else if (action === 'install') {
          args.push('install', pkg);
        } else if (action === 'uninstall') {
          args.push('uninstall', '-y', pkg);
        }
    
        const pip = spawn(pythonPath, args);
        currentPipProcess = pip;
        let output = '';
        let errorOutput = '';
    
        pip.stdout.on('data', (data) => {
          output += data.toString();
          const text = data.toString();
          EditorWindow.instance.window.webContents.send('pip-progress', { raw: text });
        });
    
        pip.stderr.on('data', (data) => {
          errorOutput += data.toString();
          const text = data.toString();
          EditorWindow.instance.window.webContents.send('pip-progress', { raw: text });
        });
    
        pip.on('close', (code) => {
          currentPipProcess = null;
          if (code === 0) {
            try {
              if (action === 'list') {
                const list = JSON.parse(output); // ✅ 直接解析 JSON
                resolve({ success: true, data: list });
              } else {
                resolve({ success: true, data: output });
              }
            } catch (err) {
              resolve({ success: false, error: '解析失败: ' + err.message });
            }
          } else {
            resolve({ success: false, error: errorOutput || '命令执行失败' });
          }
        });
      });
    });

    // ###########################################################################################################

    // ------------------------------------蓝牙相关--------------------------------------------------------------------------------------
    // 扫描蓝牙
    this.ipc.handle('ble-scan', async (event, deviceType) => {
      try {
        return await scanBleDevices(3000);
      } catch (err) {
        return {
          success: false,
          error: err.message,
          ...(process.env.NODE_ENV === 'development' && { stack: err.stack })
        };
      }
    });
    
    //连接蓝牙
    this.ipc.handle('ble-connect', async (event, deviceInfo, deviceType) => {
      try {
        return await connectBleDevice(deviceInfo);
      } catch (err) {
        return {
          success: false,
          message: err.message
        };
      }
    });

    //连接蓝牙
    this.ipc.handle('ble-disconnect', async (event) => {
      try {
        return await disBleDevice();
      } catch (err) {
        return {
          success: false,
          message: err.message
        };
      }
    });

    // -----------------------------------------------------------------------------------------------------------------------------------












    this.loadURL('tw-editor://./gui/gui.html');
    this.show();
  }




  getPreload () {
    return 'editor';
  }

  getDimensions () {
    return {
      width: 1280,
      height: 800
    };
  }

  getBackgroundColor () {
    return '#333333';
  }

  applySettings () {
    this.window.webContents.setBackgroundThrottling(settings.backgroundThrottling);
  }

  enumerateMediaDevices () {
    // Used by desktop settings
    return new Promise((resolve, reject) => {
      this.ipc.once('enumerated-media-devices', (event, result) => {
        if (typeof result.error !== 'undefined') {
          reject(result.error);
        } else {
          resolve(result.devices);
        }
      });
      this.window.webContents.send('enumerate-media-devices');
    });
  }

  handleWindowOpen (details) {
    const url = new URL(details.url);
    const params = new URLSearchParams(url.search);

    // Open extension sample projects in-app
    if (
      url.protocol === 'tw-editor:' &&
      url.host === '.' &&
      params.has('project_url')
    ) {
      const projectUrl = params.get('project_url');
      const parsedFile = parseOpenedFile(projectUrl, null);
      if (parsedFile.type === TYPE_SAMPLE) {
        new EditorWindow(parsedFile, null);
        return {
          action: 'deny'
        };
      }
    }

    // Open extension documentation in-app
    const extensionsDocsMatch = details.url.match(
      /^https:\/\/extensions\.turbowarp\.org\/([\w_\-.\/]+)$/
    );
    if (extensionsDocsMatch) {
      ExtensionDocumentationWindow.open(extensionsDocsMatch[1]);
      return {
        action: 'deny'
      };
    }

    return super.handleWindowOpen(details);
  }

  canExitFullscreenByPressingEscape () {
    return !this.isInEditorFullScreen;
  }

  updateRichPresence () {
    RichPresence.setActivity(this.projectTitle, this.openedProjectAt);
  }

  /**
   * @param {string[]} files
   * @param {boolean} fullscreen
   * @param {string|null} workingDirectory
   */
  static openFiles (files, fullscreen, workingDirectory) {
    if (files.length === 0) {
      EditorWindow.newWindow(fullscreen);
    } else {
      for (const file of files) {
        new EditorWindow(parseOpenedFile(file, workingDirectory), fullscreen);
      }
    }
  }

  /**
   * Open a new window with the default project.
   * @param {boolean} fullscreen
   */
  static newWindow (fullscreen) {
    new EditorWindow(null, fullscreen);
  }

  static dataSend(data){
    console.log(data)
    if (EditorWindow.instance) {
      EditorWindow.instance.window.webContents.send('send-state', data);
    }
  }

  static setRobotData(data){
    // console.log('33333333',data)
     if (EditorWindow.instance) {
      // console.log('111111',data)
      // console.log('2222222',EditorWindow.instance.robotData)
      EditorWindow.instance.robotData=data
      EditorWindow.instance.window.webContents.send('send-senor', data);
    }
  } 
}















module.exports = EditorWindow;