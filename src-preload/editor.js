const {contextBridge, ipcRenderer} = require('electron');

contextBridge.exposeInMainWorld('EditorPreload', {
  isInitiallyFullscreen: () => ipcRenderer.sendSync('is-initially-fullscreen'),
  getInitialFile: () => ipcRenderer.invoke('get-initial-file'),
  getFile: (id) => ipcRenderer.invoke('get-file', id),
  openedFile: (id) => ipcRenderer.invoke('opened-file', id),
  closedFile: () => ipcRenderer.invoke('closed-file'),
  showSaveFilePicker: (suggestedName) => ipcRenderer.invoke('show-save-file-picker', suggestedName),
  showOpenFilePicker: () => ipcRenderer.invoke('show-open-file-picker'),
  setLocale: (locale) => ipcRenderer.sendSync('set-locale', locale),
  setChanged: (changed) => ipcRenderer.invoke('set-changed', changed),
  openNewWindow: () => ipcRenderer.invoke('open-new-window'),
  openAddonSettings: (search) => ipcRenderer.invoke('open-addon-settings', search),
  openPackager: () => ipcRenderer.invoke('open-packager'),
  openDesktopSettings: () => ipcRenderer.invoke('open-desktop-settings'),
  openPrivacy: () => ipcRenderer.invoke('open-privacy'),
  openAbout: () => ipcRenderer.invoke('open-about'),
  getPreferredMediaDevices: () => ipcRenderer.invoke('get-preferred-media-devices'),
  getAdvancedCustomizations: () => ipcRenderer.invoke('get-advanced-customizations'),
  setExportForPackager: (callback) => {
    exportForPackager = callback;
  },
  setIsFullScreen: (isFullScreen) => ipcRenderer.invoke('set-is-full-screen', isFullScreen),
  // openMasterWindow: () => ipcRenderer.invoke('open-master-window'),
  // openConnectWindow:() =>ipcRenderer.invoke('open-connect-window'),
  // openBleSettings: () => ipcRenderer.invoke('open-ble-settings'),
  // openDownloadSettings: (code) => ipcRenderer.invoke('open-download-settings',code),
  sendStateData: (callback) => ipcRenderer.on('send-state', (event, state) => callback(state)),
  sendSenorData: (callback) => ipcRenderer.on('send-senor', (event, senor) => callback(senor)),
  getRobotData: () => ipcRenderer.sendSync('get-robot-data'),
  download: (code,args) => ipcRenderer.invoke('download',code,args),
  SerialDownload: (code) => ipcRenderer.invoke('serial-download',code),
  cancelload: () => ipcRenderer.invoke('cancelload'),

  // 浦东第一帅-----------------------------------------------
  //串口连接相关
  serialScan: (deviceType) => ipcRenderer.invoke('serial-scan',deviceType),
  serialConnect: (deviceInfo,deviceType) => ipcRenderer.invoke('serial-connect', deviceInfo,deviceType),
  serialDisconnect: () => ipcRenderer.invoke('serial-disconnect'),
  serialDisconnect_silent: () => ipcRenderer.invoke('serial-disconnect-silent'),
  

  onSerialDisconnected: (callback) => ipcRenderer.on('serial-disconnected', (event) => callback()),//串口断开监听

  sendBaudRateChange: (baudRate) => ipcRenderer.invoke('serial-change-baudrate', baudRate),//改变波特率
  serialSendCommand: (command,type) => ipcRenderer.invoke('serial-send-command', command,type),//发送数据
  enterReplMode: () => ipcRenderer.invoke('serial-enter-repl'),//进入repl模式
  exitReplMode: () => ipcRenderer.invoke('serial-exit-repl'),//退出repl模式（进入烧录）
  enterReplModeESP: () => ipcRenderer.invoke('serial-enter-repl-esp'),//进入repl模式（esp）
  exitReplModeESP: () => ipcRenderer.invoke('serial-exit-repl-esp'),//退出repl模式（进入烧录）（esp）

  mBUsbRunCode: (code) => ipcRenderer.invoke('microbit-usb-run',code),//microbit直接运行代码
  usbdownloadCode: (code,device) => ipcRenderer.invoke('usb-download-flash',code,device),//下载代码
  onFlashProgress: (callback) => ipcRenderer.on('flash-progress', (_, percent) => callback(percent)),// 监听烧录进度
  onFlashDone: cb => ipcRenderer.on("flash-done", cb),//下载完成
  onFlashError: cb => ipcRenderer.on("flash-error", (_, m) => cb(m)),//下载报错
  cancelFlash: () => ipcRenderer.invoke("flash-cancel"),//中止下载(暂时没有使用)

  onSerialReturn: (callback) => ipcRenderer.on('serial-return', (_, text) => callback(text)),// 监听串口返回
  sendSerialCommand: (command,type) => ipcRenderer.invoke('serial-send-command-direct', command,type),//串口数据直发

  flashFirmwareAll: (device, port) => ipcRenderer.invoke('flash-firmware-all',device,  port),//固件烧录
  onFlashFirmwareProgress: (callback) => {
    const listener = (_, percent) => callback(percent);
    ipcRenderer.on('flash-firmware-progress', listener);
    return () => { ipcRenderer.removeListener('flash-firmware-progress', listener); };
  },
  onFlashFirmwareDone: (callback) => {
    const listener = (_, result) => callback(result);
    ipcRenderer.on('flash-firmware-done', listener);
    return () => { ipcRenderer.removeListener('flash-firmware-done', listener); };
  },
  onFlashFirmwareError: (callback) => {
    const listener = (_, error) => callback(error);
    ipcRenderer.on('flash-firmware-error', listener);
    return () => { ipcRenderer.removeListener('flash-firmware-error', listener); };
  },
  writeEspWiFi: (ssid, password, port) => ipcRenderer.invoke('write-esp-wifi', ssid, password, port),//写入ESP WiFi配置


  download_ArduinoCode: (code) => ipcRenderer.invoke('download-arduino-code',code),//烧录Arduino代码

  /* python所有 */
  listPythonTree: () => ipcRenderer.invoke('python-files:list'),//遍历获取文件树
  newPythonFile: (baseName) => ipcRenderer.invoke('python-files:new', baseName),//新文件
  newPythonFolder: (name) => ipcRenderer.invoke('python-folders:new', name),//新文件夹
  renamePythonItem: (oldName, newName, type) => ipcRenderer.invoke('python-files:rename', oldName, newName,type),//重命名
  deletePythonFile: (name) => ipcRenderer.invoke('python-files:delete', name),//删除
  uploadPythonFile: (targetDir, relativePath, arrayBuffer) => ipcRenderer.invoke("python-files:upload", targetDir, relativePath, arrayBuffer),//上传
  showInFolder: (relativePath) => ipcRenderer.invoke("python-files:show-in-folder", relativePath),//打开文件所在位置

  readPythonFile: (name) => ipcRenderer.invoke('python-files:read', name),
  savePythonFile: (name, content) => ipcRenderer.invoke('python-files:save', name, content),
  


  runPython: (fileName) => ipcRenderer.invoke('python:run', fileName),//运行py
  stopPython: () => ipcRenderer.invoke('python-stop'),//停止运行
  sendPythonInput: (cmd) => ipcRenderer.send('python:input', cmd),//运行输入的指令

  //python运行时监听
  onPythonLog: (callback) => {
      const handler = (_, data) => callback(data);
      ipcRenderer.on('python-log', handler);
      return () => ipcRenderer.off('python-log', handler);
  },
  onPythonError: (callback) => {
      const handler = (_, data) => callback(data);
      ipcRenderer.on('python-error', handler);
      return () => ipcRenderer.off('python-error', handler);
  },
  onPythonExit: (callback) => {
      const handler = (_, code) => callback(code);
      ipcRenderer.on('python-exit', handler);
      return () => ipcRenderer.off('python-exit', handler);
  },


  pipPython: (action,pkg) => ipcRenderer.invoke('pip-action', action,pkg),//运行pip
  onPipProgress: (callback) => {
    const listener = (event, data) => callback(data);
    ipcRenderer.on('pip-progress', listener);
    return () => {
        ipcRenderer.removeListener('pip-progress', listener);
    };
  },

  // 蓝牙相关
  bleScan: (deviceType) => ipcRenderer.invoke('ble-scan', deviceType),
  bleConnect: (deviceInfo, deviceType) => ipcRenderer.invoke('ble-connect', deviceInfo, deviceType),
  blelDisconnect: () => ipcRenderer.invoke('ble-disconnect'),



  // 浦东第一帅-----------------------------------------------
  //下面这些可能不用了

  //烧录原始固件
  //  flashFirmware: () => ipcRenderer.invoke('usb-flash-firmware'),
 
   flashHexFile: (hexData, boardId) => ipcRenderer.invoke('usb-flash-hex', { hexData, boardId }),
   getStorageInfo: () => ipcRenderer.invoke('usb-get-storage-info'),
   
   // 添加USB设备事件监听
   onUSBDeviceEvent: (callback) => {
     ipcRenderer.on('usb-device-connected', (event, device) => callback('connected', device));
     ipcRenderer.on('usb-device-disconnected', (event, device) => callback('disconnected', device));
     ipcRenderer.on('usb-device-error', (event, error) => callback('error', null, error));
    //  ipcRenderer.on('flash-progress', (event, progress) => callback('progress',null,progress));
   },
   //enterReplMode: () => ipcRenderer.invoke('usb-enter-repl'),
  //  exitReplMode: () => ipcRenderer.invoke('usb-exit-repl'),
   sendCommandToDevice: (command) => ipcRenderer.invoke('usb-send-command', command),
   
   downloadCode: (code) => ipcRenderer.invoke('usb-download-flash',code),
   // 添加REPL数据接收监听
   onReplData: (callback) => {
     ipcRenderer.on('repl-data-received', (event, data) => {
       callback({
         ...data,
         isPrompt: data.text.includes('>>>') 
       });
     });
   },
   // 移除监听
   offReplData: () => {
     ipcRenderer.removeAllListeners('repl-data-received');
   }
});

let exportForPackager = () => Promise.reject(new Error('exportForPackager missing'));

ipcRenderer.on('export-project-to-port', (e) => {
  const port = e.ports[0];
  exportForPackager()
    .then(({data, name}) => {
      port.postMessage({ data, name });
    })
    .catch((error) => {
      console.error(error);
      port.postMessage({ error: true });
    });
});

window.addEventListener('message', (e) => {
  if (e.source === window) {
    const data = e.data;
    if (data && typeof data.ipcStartWriteStream === 'string') {
      ipcRenderer.postMessage('start-write-stream', data.ipcStartWriteStream, e.ports);
    }
  }
});

ipcRenderer.on('enumerate-media-devices', (e) => {
  navigator.mediaDevices.enumerateDevices()
    .then((devices) => {
      e.sender.send('enumerated-media-devices', {
        devices: devices.map((device) => ({
          deviceId: device.deviceId,
          kind: device.kind,
          label: device.label
        }))
      });
    })
    .catch((error) => {
      console.error(error);
      e.sender.send('enumerated-media-devices', {
        error: `${error}`
      });
    });
});

contextBridge.exposeInMainWorld('PromptsPreload', {
  alert: (message) => ipcRenderer.sendSync('alert', message),
  confirm: (message) => ipcRenderer.sendSync('confirm', message),
});

// In some Linux environments, people may try to drag & drop files that we don't have access to.
// Remove when https://github.com/electron/electron/issues/30650 is fixed.
if (navigator.userAgent.includes('Linux')) {
  document.addEventListener('drop', (e) => {
    if (e.isTrusted) {
      for (const file of e.dataTransfer.files) {
        // Using webUtils is safe as we don't have a legacy build for Linux
        const {webUtils} = require('electron');
        const path = webUtils.getPathForFile(file);
        ipcRenderer.invoke('check-drag-and-drop-path', path);
      }
    }
  }, {
    capture: true
  });
}
