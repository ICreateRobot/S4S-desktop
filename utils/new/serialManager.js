/*  串口操作 */
const path = require('path');
const {app} = require('electron');
const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const usb = require('usb');
const fs = require('fs');
const {MicropythonFsHex }  = require('@microbit/microbit-fs');
const { microbitBoardId } = require('@microbit/microbit-universal-hex');
const DAPjs = require('dapjs');
const { DAPLink } = DAPjs;


// 连接串口设备状态管理
const serialDeviceState = {
    usbDevice: null,
    daplink: null,
    serialPort: null,
    parser: null,
    replActive: false,
    serialBuffer: '',
    currentResolve: null,
    _disconnecting:false//清理状态
};
  
  // 设备配置表
const DEVICE_CONFIGS = {
    Microbit: {
        name: 'Microbit',
        vendorId: 0x0d28,
        productIds: [0x0204, 0x0205],
        scanStrategy: 'serial-usb'
    },
    Arduino: {
        name: 'Arduino',
        vendorId: 0x2341, 
        productIds: [ 0x1002], 
        scanStrategy: 'serial-only'
    },
    ESP32: {
        name: 'ESP32',
        scanStrategy: 'serial-only'
    }
};

//######################################## 初始化 ########################################
let mainWindow = null;
// 初始化时设置 window
function serialInitialize(windowInstance) {
  mainWindow = windowInstance;
}

//######################################## 扫描 ########################################
async function scanSerialDevice(deviceType) {
    try {
        // 单独处理非设备情况
        if (deviceType === 'ALL') {
            return await scanAllDevices();
        }

        const config = DEVICE_CONFIGS[deviceType]

        if (config.scanStrategy === 'serial-only') {//只串口
          return await scanSerialOnly(config);
        }else if(config.scanStrategy === 'serial-usb'){//串口+ussb
          return await scanWithUsbMatching();
        }else{
          return await scanAllDevices();
        }
    } catch (err) {
        return { 
          success: false, 
          error: err.message,
          ...(process.env.NODE_ENV === 'development' && { stack: err.stack })
        };
    }
}

// 简单串口扫描
async function scanSerialOnly(config) {
    const ports = await SerialPort.list();

    // 根据设备类型进行过滤
    const devices = ports
      .filter(port => {
        // port.vendorId/productId 通常是字符串，需要转成十六进制数字比较
        const vid = port.vendorId ? parseInt(port.vendorId, 16) : null;
        const pid = port.productId ? parseInt(port.productId, 16) : null;

        return (
          vid === config.vendorId &&
          config.productIds.includes(pid)
        );
      })
      .map(port => ({
        comPort: port.path,
        name: port.path,
        vendorId: port.vendorId,
        productId: port.productId,
        mode: 'serial',
      }));

    return {
      success: true,
      devices,
      selectedDevice: devices[0] || null
    };
}

// 复杂扫描（+USB）
// async function scanWithUsbMatching() {
//     // 扫描USB设备
//     const usbDevices = usb.getDeviceList().filter(d => 
//       d.deviceDescriptor.idVendor === 0x0d28 && 
//       [0x0204, 0x0205].includes(d.deviceDescriptor.idProduct)
//     );

//     // 扫描串口设备
//     const ports = await SerialPort.list();
//     const microbitPorts = ports.filter(p => 
//       p.vendorId === '0D28' && 
//       ['0204', '0205'].includes(p.productId)
//     );

//     // 匹配
//     const devices = usbDevices.map(d => {
//       // Windows特殊处理
//       let port = null;
//       if (process.platform === 'win32') {
//         // 从locationId中提取设备号
//         const deviceNumber = `${d.busNumber}-${d.deviceAddress}`;
//         port = microbitPorts.find(p => 
//           p.locationId && p.locationId.includes(deviceNumber)
//         );
        
//         //匹配设备路径中的数字
//         if (!port) {
//             const usbPathMatch = d.device?.deviceAddress?.toString() || '';
//             port = microbitPorts.find(p => 
//                 p.path && p.path.includes(usbPathMatch)
//             );
//         }
//       } else {
//         // 非Windows系统使用常规匹配
//         port = microbitPorts.find(p => 
//           p.serialNumber && d.serialNumber &&
//           p.serialNumber === d.serialNumber
//         );
//       }
      
//       return {
//         comPort: port?.path || null,
//         name: port?.path || null,
//         vendorId: d.deviceDescriptor.idVendor, 
//         productId: d.deviceDescriptor.idProduct,
//         mode: 'serial',
//         deviceAddress: d.deviceAddress,
//       };
//     });

//     return { 
//       success: true, 
//       devices,
//       selectedDevice: devices[0] || null
//     };
// }
//根据串口绑定usb，事实证明两种方式都不行（官方应该使用DAPLink HID会话独占来实现的，但是目前再改太麻烦了，姑且如此吧）
async function scanWithUsbMatching() {
    //  扫描 USB 设备 
    const usbDevices = usb.getDeviceList().filter(d =>
        d.deviceDescriptor.idVendor === 0x0d28 &&
        [0x0204, 0x0205].includes(d.deviceDescriptor.idProduct)
    );

    //  扫描串口设备 
    const ports = await SerialPort.list();
    const microbitPorts = ports.filter(p =>
        p.vendorId === '0D28' &&
        ['0204', '0205'].includes(p.productId)
    );

    // 以串口为唯一实体
    const deviceMap = new Map();

    for (const port of microbitPorts) {
        deviceMap.set(port.path, {
            comPort: port.path,
            name: port.path,
            vendorId: parseInt(port.vendorId, 16),
            productId: parseInt(port.productId, 16),
            mode: 'serial',
            deviceAddress: null, 
        });
    }

    // 将 USB device “挂载”到正确的 COM 上
    for (const d of usbDevices) {
        let matchedPort = null;

        if (process.platform === 'win32') {
            matchedPort = microbitPorts.find(p =>
                p.serialNumber &&
                d.serialNumber &&
                p.serialNumber === d.serialNumber
            );
        } else {
            // macOS / Linux
            matchedPort = microbitPorts.find(p =>
                p.serialNumber &&
                d.serialNumber &&
                p.serialNumber === d.serialNumber
            );
        }

        // 如果成功匹配到串口，则记录 deviceAddress
        if (matchedPort) {
            const device = deviceMap.get(matchedPort.path);
            if (device && device.deviceAddress === null) {
                device.deviceAddress = d.deviceAddress;
            }
        }
    }

    const devices = Array.from(deviceMap.values());

    return {
        success: true,
        devices,
        selectedDevice: devices[0] || null
    };
}


// 扫描全部设备（不区分类型）
async function scanAllDevices() {
    try {
        console.log(111)
        const ports = await SerialPort.list();
        
        // 统一格式转换
        const devices = ports.map(port => ({
            comPort: port.path,
            name: port.path,
            vendorId: port.vendorId,
            productId: port.productId,
            mode: 'serial',
        }));

        return {
            success: true,
            devices: devices,
            selectedDevice: devices[0] || null
        };
    } catch (err) {
        return { 
            success: false, 
            error: err.message, 
            devices: [],
            selectedDevice: null
        };
    }
}

//######################################## 连接 ########################################
async function connectSerialDevice(deviceInfo,deviceType) {
    try {
        // 先安全断开已有串口
        await safeDisconnect();
    
        if (!deviceInfo.comPort) throw new Error("串口路径无效");
    
        const config = DEVICE_CONFIGS[deviceType]
        if (config.scanStrategy === 'serial-only') {//只串口
          return await connSerialOnly(deviceInfo,deviceType);
        }else{//串口+ussb
          return await connWithUsbMatching(deviceInfo,deviceType);
        }
    } catch (err) {
        console.log(err)
        return { success: false, message: err.message };
    }
}

// 简单串口连接
async function connSerialOnly(deviceInfo,deviceType) {
    serialDeviceState.serialPort = new SerialPort({
        path: deviceInfo.comPort,
        baudRate: 115200,
        autoOpen: false,
        rts: true//rts打开
    });

    // 手动打开串口
    await new Promise((resolve, reject) => {
        serialDeviceState.serialPort.open(err => err ? reject(err) : resolve());
    });

    // 数据解析器
    serialDeviceState.parser = serialDeviceState.serialPort;//.pipe(new DelimiterParser({ delimiter: Buffer.from([0x3e, 0x3e, 0x3e]) }));
    setupSerialListeners(deviceType);

    return { success: true, info: { comPort: deviceInfo.comPort} };
}

// 复杂连接（+USB）
async function connWithUsbMatching(deviceInfo,deviceType) {

    // 初始化串口-repl使用
    serialDeviceState.serialPort = new SerialPort({
        path: deviceInfo.comPort,
        baudRate: 115200,
        autoOpen: false,
    });

    //手动打开串口
    await new Promise((resolve, reject) => {
        serialDeviceState.serialPort.open(err => err ? reject(err) : resolve());
    });


    // 初始化USB设备
    serialDeviceState.usbDevice = usb.findByIds(deviceInfo.vendorId, deviceInfo.productId);
    if (!serialDeviceState.usbDevice) throw new Error('设备未找到');

    // 打开设备-烧录使用
    try {
        serialDeviceState.usbDevice.open();
        if (serialDeviceState.usbDevice.interfaces?.length > 0) {
            serialDeviceState.usbDevice.interfaces[0].claim();
        }
    } catch (openErr) {
        console.warn('设备打开警告:', openErr.message);
    }

    // 数据解析器(按>>>分割数据流)
    //serialDeviceState.parser = serialDeviceState.serialPort.pipe(new ReadlineParser({ delimiter: '>>>' }));//\r\n 
    setupSerialListeners(deviceType);// 监听数据变化、断开

    return { success: true, info: { comPort: deviceInfo.comPort } };
}

//######################################## 断开 ########################################

async function disconnectSerialDevice() {
    try {
        // 检查是否有活动连接
        if (!serialDeviceState.usbDevice && !serialDeviceState.serialPort) {
          throw new Error('没有已连接的设备');
        }
        // 保存当前状态用于通知
        const wasReplActive = serialDeviceState.replActive;
        // 执行断开操作
        await safeDisconnect();

        //notifyRenderer('usb-device-disconnected', { wasReplActive });
        return { 
          success: true, 
          message: '设备已断开连接',
          wasReplActive // 返回断开前的REPL状态
        };
    } catch (err) {
        return { 
          success: false, 
          error: err.message,
          ...(process.env.NODE_ENV === 'development' && { stack: err.stack })
        };
    }
}

//辅助函数--安全断开
async function safeDisconnect() {
    try {
        // 保证不会重复断开
        if (serialDeviceState._disconnecting) {
            console.warn("断开过程已在进行中，跳过重复执行");
            return;
        }
        serialDeviceState._disconnecting = true;

        const stateCopy = { ...serialDeviceState };
        serialDeviceState.serialPort = null;
        serialDeviceState.usbDevice = null;
        serialDeviceState.daplink = null;
        serialDeviceState.replActive = false;
        serialDeviceState.serialBuffer = '';
        serialDeviceState.currentResolve = null;

        // 关闭串口
        if (stateCopy.serialPort) {
            await closeSerialPort(stateCopy.serialPort);
        }
    
        // 释放 USB
        // if (stateCopy.usbDevice) {
        //   await closeUsbDevice(serialDeviceState.usbDevice);
        // }

        // 断开 DAPLink
        if (stateCopy.daplink) {
            console.log("close daplink")
            await closeDapLink(stateCopy.daplink);
        } else if (stateCopy.usbDevice) { // 如果没有 DAPLink，但有独立 USB 设备，再关闭
            console.log("close usb")
            await closeUsbDevice(stateCopy.usbDevice);
        }
    }catch(e){
        console.log(e)
    }finally {
        serialDeviceState._disconnecting = false;
        mainWindow.webContents.send('serial-disconnected');//(连接时也会清理一次串口，但是都会发送一个断开，后续应该改为根据不同情况来发送)
    }
}

//辅助函数--清理串口
async function closeSerialPort(port) {
    return new Promise(resolve => {
      try {
        // 移除所有监听器
        port.removeAllListeners();
        
        if (port.isOpen) {
          port.close(err => {
            if (err) console.error('串口关闭错误:', err);
            resolve();
          });
        } else {
          resolve();
        }
      } catch (err) {
        console.error('串口清理异常:', err);
        resolve();
      }
    });
}

//辅助函数--清理usb
async function closeUsbDevice(device) {
    return new Promise(resolve => {
      try {
            // 检查设备是否还连接
            const stillConnected = usb.getDeviceList().some(d =>
            d.busNumber === device.busNumber && d.deviceAddress === device.deviceAddress
            );
    
            if (!stillConnected) {
            console.warn('USB设备已物理断开');
            return resolve();
            }
    
            // 尝试关闭设备
            try {
            device.close();
            } catch (err) {
            console.warn('USB关闭异常', err.message);
            }
    
            // 等待少量时间确保资源释放
            setTimeout(resolve, 50);
      } catch (err) {
            console.error('USB设备检查错误', err);
            resolve();
      }
    });
}

//辅助函数--DapLink
async function closeDapLink(daplink) {
    return new Promise(resolve => {
      try {
        daplink.disconnect().then(resolve).catch(err => {
          console.error('DAPLink断开错误:', err);
          resolve();
        });
      } catch (err) {
        console.error('DAPLink断开异常:', err);
        resolve();
      }
    });
}

//######################################## 监听(数据解析) ########################################
//辅助函数--监听数据，设备断开或异常
function setupSerialListeners(deviceType) {
    // 先移除旧监听器
    // if (serialDeviceState.parser) {
    //     serialDeviceState.parser.removeAllListeners();
    // }

    if(deviceType=="Microbit"){//microbit数据监听
        // serialDeviceState.parser.on('data', data => {
        //     console.log('Microbit DATA:', data);

        //     // 以下为microbit
        //     serialDeviceState.serialBuffer += data;
        //     //console.log('RAW DATA:', JSON.stringify(data));
        //     // console.log('BUFFER NOW:', JSON.stringify(serialDeviceState.serialBuffer));
        
        //     if (!serialDeviceState.currentResolve) return;
        
        //     // 按行拆分，并去掉空行
        //     const lines = serialDeviceState.serialBuffer
        //     .split(/\r?\n/)
        //     .map(l => l.trim())
        //     .filter(l => l && l !== '>>>'); // 去掉空行和多余提示符
        
        //     // 去掉首行命令回显
        //     const resultLines = lines.slice(1);
        
        //     // 处理多行或单行结果
        //     const result = resultLines.length === 0
        //     ? ''                   // 没有返回值
        //     : resultLines.length === 1
        //         ? resultLines[0]      // 单行返回值
        //         : resultLines;        // 多行返回值
        
        //     // console.log('COMMAND RESULT:', result);
        
        //     // 清空 buffer 并 resolve
        //     serialDeviceState.serialBuffer = '';
        //     serialDeviceState.currentResolve(result);
        //     serialDeviceState.currentResolve = null;
        // });
    }else if(deviceType === "Arduino"){
        serialDeviceState.parser.on('data', data => {
            ///console.log('ARDUINO DATA:', data);
            let resultData = Buffer.from(data);

            if (data.length === 3 && data.equals(Buffer.from([0x3e, 0x3e, 0x3e]))) {// 收到纯分隔符
                resultData = Buffer.alloc(0);// 空响应
            }else if(data.length > 3 && data.slice(data.length - 3).equals(Buffer.from([0x3e, 0x3e, 0x3e]))){
                resultData = data.slice(0, data.length - 3); // 去掉尾部>>>
            }else{
                return
            }
            
            if (serialDeviceState.currentResolve) {
            serialDeviceState.currentResolve(resultData);
            serialDeviceState.currentResolve = null;
            }
            
        })
    }

    serialDeviceState.serialPort.once('close', () => {
        serialDeviceState.serialPort = null;
        mainWindow.webContents.send('serial-disconnected');
    });

    serialDeviceState.serialPort.once('error', err => {
        serialDeviceState.serialPort = null;
        mainWindow.webContents.send('serial-disconnected');
    });
}

// 切换解析器
function switchSerialParser(deviceType, mode) {
    // 清理旧 parser
    if (serialDeviceState.parser) {
        serialDeviceState.parser.removeAllListeners();
        // try {
        //     serialDeviceState.serialPort.unpipe(serialDeviceState.parser);
        // } catch (e) {}
        // serialDeviceState.parser = null;
    }

    serialDeviceState.serialMode = mode;

    if (deviceType === 'Microbit') {
        if (mode === 'repl') {
            setupMicrobitReplParser();
        } else {
            setupMicrobitNormalParser();
        }
    }
}

// microbit切换>>>解析器处理数据（repl模式使用）
function setupMicrobitReplParser() {
    // ⚠️ 注意：REPL 不要用 ReadlineParser
    //serialDeviceState.parser = serialDeviceState.serialPort;
    serialDeviceState.parser = serialDeviceState.serialPort.pipe(new ReadlineParser({ delimiter: '>>>' }));

    serialDeviceState.parser.on('data', data => {
        const text = data.toString('utf8');
        console.log('data',text)

        serialDeviceState.serialBuffer += text;

        if (!serialDeviceState.currentResolve) return;

        // 按行拆，并去掉空行
        const lines = serialDeviceState.serialBuffer
            .split(/\r?\n/)
            .map(l => l.trim())
            .filter(l => l && l !== '>>>');

        // 去掉命令回显
        const resultLines = lines.slice(1);

         // 处理多行或单行结果
        const result =
            resultLines.length === 0 ? '' :
            resultLines.length === 1 ? resultLines[0] :
            resultLines;

        // 清空 buffer 并 resolve
        serialDeviceState.serialBuffer = '';
        serialDeviceState.currentResolve(result);
        serialDeviceState.currentResolve = null;
    });
}

// microbit切换\r\n解析器处理数据（烧录模式使用）
function setupMicrobitNormalParser() {
    serialDeviceState.parser =  serialDeviceState.serialPort.pipe(new ReadlineParser({ delimiter: '\r\n' }));

    serialDeviceState.parser.on('data', line => {
        const text = line.toString().trim();
        if (!text) return;

        console.log('Microbit NORMAL:', text);

        mainWindow.webContents.send('serial-return',text );
    });
}


//######################################## 模式切换 ########################################

// 进入repl(microbit用)
async function replSerial() {
    try {
        if (!serialDeviceState.serialPort ) {//|| serialDeviceState.replActive
          return { success: false, error: "串口未连接或已处于REPL模式"};
        }
        //console.log('replllll');

        // 中断当前程序
        await sendSerialCommand('\x03'); 
        await sendSerialCommand('from microbit import *\r',200);
        await sendSerialCommand('from s4s import *\r',200);
        await sendSerialCommand('display.show(Image.HEART)\n\r', 200);
        
        serialDeviceState.replActive = true;
        switchSerialParser("Microbit","repl")
        return { success: true , message: 'REPL模式已激活'};
    } catch (err) {
        return { success: false, error: err.message };
    }
}

// 退出repl（进入烧录模式）(microbit用)
async function replExitSerial() {
    try {
        if (!serialDeviceState.serialPort ) {//|| !serialDeviceState.replActive
          return { success: false, error: "串口未连接"};
        }
    
        await sendSerialCommand('\x03');
        await sendSerialCommand('\x04');
    
        serialDeviceState.replActive = false;
    
        switchSerialParser("Microbit","norepl")
        return { success: true };
    } catch (err) {
        return { success: false, error: err.message };
    }
}

//######################################## 串口发送数据 ########################################
// 队列发送数据
async function sendCommandSerial(command,type) {
    return enqueueCommand(async () => {//加入队列
        return await runSerialCommand(command, type);
    });
}

// 串口发送
async function runSerialCommand(command, type){
    try {
      if (!serialDeviceState.serialPort) {
        return { success: false, error: "未连接设备" };
      }
      console.log(command)
      //发送
      serialDeviceState.serialBuffer = '';// 清空上次结果
      if(type === "Microbit"){
        serialDeviceState.serialPort.write( command + '\r' );
      } else{
        serialDeviceState.serialPort.write(Buffer.from(command) );
      }
      
      // 等待响应(Promise 挂起)
      const response = await new Promise((resolve) => {
        serialDeviceState.currentResolve = (data) => {
          serialDeviceState.currentResolve = null;
          resolve(data);
        };
        
        // 超时
        // setTimeout(() => {
        //   if (deviceState.currentResolve) {
        //     deviceState.currentResolve = null;
        //     resolve(' 未收到响应');
        //   }
        // }, 2000);
      });
  
      // 控制发送速率
      await new Promise(resolve => setTimeout(resolve, 50));

      let output = response;
      if (Buffer.isBuffer(output)) {// Buffer 
          //output = output.toString("utf8");
          //output = response.toString("hex")

          output = bufferToDecimal(response);
      }else if (typeof output === "string") {// 字符串才 trim
          output = output.trim();
      }
  
      return { success: true, response: output };
    } catch (err) {
      return { success: false, error: err.message };
    }
}

// ………………队列，避免混乱………………………………
const commandQueue = [];
let isProcessing = false;

async function enqueueCommand(fn) {
    return new Promise((resolve, reject) => {
      commandQueue.push({ fn, resolve, reject });
      processQueue();
    });
}

async function processQueue() {
    if (isProcessing) return;
    const item = commandQueue.shift();
    if (!item) return;

    isProcessing = true;
    try {
      const result = await item.fn();  // 执行真正的串口过程
      item.resolve(result);
    } catch (err) {
      item.reject(err);
    }
    isProcessing = false;

    // 继续执行队列
    if (commandQueue.length > 0) processQueue();
}
// …………………………………………………………


async function sendSerialCommand(command, delay = 50) {
    return new Promise((resolve, reject) => {
      serialDeviceState.serialPort.write(command, err => {
        if (err) return reject(err);
        setTimeout(resolve, delay);
      });
    });
}

// 简单的十进制转换函数
function bufferToDecimal(buffer) {
    if (buffer.length === 0) return 0;
    
    // 使用 BigInt 避免所有溢出问题
    let unsignedValue = 0n;
    for (let i = 0; i < buffer.length; i++) {
        unsignedValue = (unsignedValue << 8n) | BigInt(buffer[i]);
    }
    
    // 有符号转换
    let signedValue = unsignedValue;
    if (buffer.length === 1 && unsignedValue > 127n) {
        signedValue = unsignedValue - 256n;  // 8位有符号
    } else if (buffer.length === 2 && unsignedValue > 32767n) {
        signedValue = unsignedValue - 65536n; // 16位有符号
    } else if (buffer.length === 4 && unsignedValue > 2147483647n) {
        signedValue = unsignedValue - 4294967296n; // 32位有符号
    }
    
    // 返回有符号值
    if (signedValue <= BigInt(Number.MAX_SAFE_INTEGER) && signedValue >= BigInt(Number.MIN_SAFE_INTEGER)) {
        return Number(signedValue);
    } else {
        return signedValue.toString();
    }
}

// 直接发送数据
async function sendCommandSerial_D(command,type) {
    console.log('zf',command)
    if(type === "Microbit"){
        await sendSerialCommand(command  );
    } else{
        serialDeviceState.serialPort.write(Buffer.from(command) );
    }
    
}

//######################################## microbit 烧录固件（下载程序） ########################################
let abortCheckTimer = null;//检测终止下载
let isFlashing = false;//下载中
let flashAbort = false;

async function downloadCodeSerial(code) {
    try {
        if (!serialDeviceState.usbDevice) {
          throw new Error("USB 设备未连接，无法烧录");
        }

        // flashAbort = false;
        // isFlashing = true;

        // 生成 HEX 文件
        const hexPath = await generateV2Hex("Microbit_LinkBot_V1.0.0",code);//固件名称未来需要提取到统一配置的文件中,统一到gui中
        const hexData = fs.readFileSync(hexPath);

        // 创建 DAPLink 传输层
        const transport = new DAPjs.USB(serialDeviceState.usbDevice);
        serialDeviceState.daplink = new DAPLink(transport);
        //console.log("开始连接 DAPLink...");

        // 连接 DAPLink
        await serialDeviceState.daplink.connect();

        // 监听进度
        serialDeviceState.daplink.on(DAPLink.EVENT_PROGRESS, progress => {
          // if (flashAbort) return;
          const percent = Math.round(progress * 100);
          mainWindow.webContents.send('flash-progress', percent);
        });


        // 执行烧录
        const flashPromise = await serialDeviceState.daplink.flash(hexData); //  

        // 检测是否点击取消
        // const result = await Promise.race([
        //     flashPromise,
        //     new Promise((_, reject) => {
        //           abortCheckTimer = setInterval(() => {
        //             if (flashAbort) {
        //                 clearInterval(abortCheckTimer );
        //                 abortCheckTimer = null;
        //                 reject(new Error("用户取消烧录"));
        //             }
        //         }, 100);
        //     })
        // ]);
        //console.log("烧录完成!");

        // 固定写法：烧录完成后必须断开
        await serialDeviceState.daplink.disconnect();
        serialDeviceState.daplink = null;

        mainWindow.webContents.send("flash-done");
        return { success: true };
    } catch (err) {
        console.error("烧录失败:", err);
        mainWindow.webContents.send("flash-error", err.message);

        // 出错后尝试断开连接
        if (serialDeviceState.daplink) {
          try { await serialDeviceState.daplink.disconnect(); } catch(e){}
          serialDeviceState.daplink = null;
        }

        return { 
          success: false, 
          error: err.message 
        };
    }finally {
        // isFlashing = false;
        // flashAbort = false;
        if (abortCheckTimer) {
            clearInterval(abortCheckTimer);  
            abortCheckTimer = null;
        }
    }
}

//获取文件位置
function getResourcePath(relativePath) {
    if (app.isPackaged) {
      // 打包后
      return path.join(__dirname, '../../utils', relativePath);
    } else {
      // 开发环境
      return path.join(__dirname, '../../utils', relativePath);
    }
}

//hex生成
async function generateV2Hex(firmwareName,code) {
    try {
        const baseHexPath = getResourcePath('/microbit_firmware/'+firmwareName+'.hex')
        const hexContent = fs.readFileSync(baseHexPath, 'utf8');
    
        const fsHex = new MicropythonFsHex([
            { hex: hexContent, boardId: microbitBoardId.V2 }
        ]);
    
        // 写入 main.py
        if (code && code.trim() !== '') {
            fsHex.write('main.py', code);
        } else if (fsHex.exists('main.py')) {
            fsHex.remove('main.py');
        }
    
        // 生成 HEX 数据
        const boardHex = fsHex.getIntelHex();
    
        // 输出路径（与你当前一致）
        const outputPath = path.join(process.cwd(), 'output_v2.hex');
        fs.writeFileSync(outputPath, boardHex);
    
        return outputPath;
  
    } catch (e) {
        console.error("生成 HEX 失败:", e);
        throw e;
    }
}

//终止usb固件烧录
async function cancelDownloadCodeSerial() {
    flashAbort = true;
    if (serialDeviceState.daplink) {
        try { 
            await serialDeviceState.daplink.disconnect(); 
        } catch {}
        serialDeviceState.daplink = null;
    }
    return { success: true };
}


//######################################## 统一固件烧录接口 ########################################
// 分发
async function unifiedFlashFirmware(deviceType, firmwareName, port) {
    try {
        if(deviceType === 'Microbit'){
            if (serialDeviceState.serialPort) {// 设备已连接时
                if(serialDeviceState.serialPort.path === port){//给当前设备
                    await flashWithCurrentDevice(firmwareName,"");
                }else{//非当前设备
                    throw new Error("已有连接设备，请先断开");
                }
            }else{//未连接时
                await connectAndFlash(firmwareName,port, "");
            }
        }

        if (deviceType !== 'Microbit') {
            throw new Error('不支持的设备类型');
        }
        
        mainWindow.webContents.send('flash-firmware-done');
        return { success: true };
    }catch(e){
        console.log(e)
        mainWindow.webContents.send( 'flash-firmware-error', e?.message || '烧录失败' );
        return { success: false, error: e.message };
    }
 
}

// 使用当前设备烧录
async function flashWithCurrentDevice(firmwareName,code) {
    try {
        // 生成HEX
        const hexPath = await generateV2Hex(firmwareName,code);
        const hexData = fs.readFileSync(hexPath);

        // 创建DAPLink
        const transport = new DAPjs.USB(serialDeviceState.usbDevice);
        serialDeviceState.daplink = new DAPLink(transport);

        // 连接并监听
        await serialDeviceState.daplink.connect();
        serialDeviceState.daplink.on(DAPLink.EVENT_PROGRESS, progress => {
            const percent = Math.round(progress * 100);
            mainWindow.webContents.send('flash-firmware-progress', percent);
        });

        // 烧录
        await serialDeviceState.daplink.flash(hexData);

        // 断开DAPLink
        await serialDeviceState.daplink.disconnect();
        serialDeviceState.daplink = null;

        // mainWindow.webContents.send('flash-firmware-done');
        // return { success: true };

    } catch (error) {
        // 出错后尝试断开连接
        if (serialDeviceState.daplink) {
        try { await serialDeviceState.daplink.disconnect(); } catch(e){}
        serialDeviceState.daplink = null;
        }
    }
}

//未连接设备，直接连接并烧录(哎，一团乱麻，先这么用吧)
async function connectAndFlash(firmwareName,port, code) {
    let deviceInfo = {
        comPort:port,
        vendorId:0x0D28,
        productId:0x0204
    }
    //先连接
    let result = await connWithUsbMatching(deviceInfo,"Microbit")
    if (!result.success) {
        throw new Error("设备连接失败");
    }

    try {
        await flashWithCurrentDevice(firmwareName, code);
    } finally {
        await safeDisconnect();
    }
}

// 临时连接新设备烧录 (未使用，ai写的未验证)
// 临时状态管理（独立于现有状态）
let tempFlashState = {
    usbDevice: null,
    daplink: null,
    serialPort: null,
    abortTimer: null,
    shouldAbort: false,
    isFlashing: false
};
async function tempConnectAndFlash(port, code) {
    // 保存当前状态
    const originalState = {
        usbDevice: serialDeviceState.usbDevice,
        serialPort: serialDeviceState.serialPort,
        daplink: serialDeviceState.daplink
    };

    try {
        // 1. 清理当前连接（不销毁，只是临时断开）
        if (serialDeviceState.daplink) {
            try { await serialDeviceState.daplink.disconnect(); } catch (e) {}
            serialDeviceState.daplink = null;
        }
        if (serialDeviceState.serialPort && serialDeviceState.serialPort.isOpen) {
            try { await serialDeviceState.serialPort.close(); } catch (e) {}
        }
        if (serialDeviceState.usbDevice && serialDeviceState.usbDevice.opened) {
            // 注意：这里不关闭USB设备，避免彻底断开
            // 只是释放接口
            try {
                if (serialDeviceState.usbDevice.interfaces?.length > 0) {
                    await serialDeviceState.usbDevice.interfaces[0].release();
                }
            } catch (e) {}
        }

        // 2. 连接新设备
        const device = usb.getDevices().find(d =>
            d.deviceDescriptor.idVendor === 0x0D28 &&
            d.deviceDescriptor.idProduct === 0x0204 &&
            d.deviceDescriptor.iSerialNumber === port // 通过端口匹配
        );

        if (!device) throw new Error(`未找到设备: ${port}`);

        await device.open();
        if (device.interfaces?.length > 0) {
            await device.interfaces[0].claim();
        }

        const serialPort = new SerialPort({ path: port, baudRate: 115200, autoOpen: false });
        await new Promise((resolve, reject) => {
            serialPort.open(err => err ? reject(err) : resolve());
        });

        // 3. 烧录（使用临时状态）
        tempFlashState.usbDevice = device;
        tempFlashState.serialPort = serialPort;

        const hexPath = await generateV2Hex(code);
        const hexData = fs.readFileSync(hexPath);

        const transport = new DAPjs.USB(device);
        tempFlashState.daplink = new DAPLink(transport);

        await tempFlashState.daplink.connect();
        tempFlashState.daplink.on(DAPLink.EVENT_PROGRESS, progress => {
            const percent = Math.round(progress * 100);
            mainWindow.webContents.send('flash-progress', percent);
        });

        await tempFlashState.daplink.flash(hexData);

        // 4. 烧录完成，断开新设备
        await tempFlashState.daplink.disconnect();
        await serialPort.close();
        await device.close();

        // 5. 恢复原始连接
        if (originalState.usbDevice && originalState.serialPort) {
            try {
                await originalState.usbDevice.open();
                if (originalState.usbDevice.interfaces?.length > 0) {
                    await originalState.usbDevice.interfaces[0].claim();
                }
                await originalState.serialPort.open();

                // 恢复到主状态
                serialDeviceState.usbDevice = originalState.usbDevice;
                serialDeviceState.serialPort = originalState.serialPort;
                setupSerialListeners('Microbit');
            } catch (e) {
                console.warn('恢复原始连接失败:', e.message);
            }
        }

        mainWindow.webContents.send('flash-done');
        return { success: true, tempConnection: true };

    } catch (error) {
        // 出错尝试恢复原始连接
        if (originalState.usbDevice && originalState.serialPort) {
            try {
                await originalState.usbDevice.open();
                if (originalState.usbDevice.interfaces?.length > 0) {
                    await originalState.usbDevice.interfaces[0].claim();
                }
                await originalState.serialPort.open();
                serialDeviceState.usbDevice = originalState.usbDevice;
                serialDeviceState.serialPort = originalState.serialPort;
                setupSerialListeners('Microbit');
            } catch (e) {
                console.warn('恢复原始连接失败:', e.message);
            }
        }
        throw error;
    }
}



module.exports = {
    serialInitialize,
    scanSerialDevice,
    connectSerialDevice,
    disconnectSerialDevice,
    replSerial,
    replExitSerial,
    sendCommandSerial,
    sendCommandSerial_D,
    downloadCodeSerial,
    cancelDownloadCodeSerial,
    unifiedFlashFirmware
};
  