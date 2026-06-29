/*  串口操作 */
const path = require('path');
const {app} = require('electron');
const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const usb = require('usb');
const fs = require('fs');
const os = require("os");
const {MicropythonFsHex }  = require('@microbit/microbit-fs');
const { microbitBoardId } = require('@microbit/microbit-universal-hex');
const DAPjs = require('dapjs');
const { DAPLink } = DAPjs;
const { spawn} = require('child_process');


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
        vendorId: 0x1A86, 
        productIds: [ 0x7523], 
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

//辅助函数--安全断开（是否静默断开-不通知前端）
async function safeDisconnect(silent = false) {
    try {
        // 保证不会重复断开
        if (serialDeviceState._disconnecting) {
            console.warn("断开过程已在进行中，跳过重复执行");
            return;
        }
        serialDeviceState._disconnecting = true;

        clearCurrentCommand();//清理数据阻塞

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
        throw e;
    }finally {
        serialDeviceState._disconnecting = false;
        //mainWindow.webContents.send('serial-disconnected');//(连接时也会清理一次串口，但是都会发送一个断开，后续应该改为根据不同情况来发送)
        if (!silent) {
            mainWindow.webContents.send('serial-disconnected');
        }
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
            if (!device) {
                return resolve();
            }
            // 已物理断开
            const stillConnected = usb.getDeviceList().some(d =>
                d.busNumber === device.busNumber &&
                d.deviceAddress === device.deviceAddress
            );

            // 即使断开了，也继续尝试释放
            const iface = device.interfaces?.[0];
            if (iface) {
                try {
                    iface.release(true, err => {
                        try {
                            device.close();
                        } catch(e){}
                        resolve();
                    });
                } catch(e) {
                    try {
                        device.close();
                    } catch(e){}
                    resolve();
                }
            } else {
                try {
                    device.close();
                } catch(e){}
                resolve();
            }
        } catch (err) {
            console.error('USB设备关闭错误', err);
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
            //console.log('ARDUINO DATA:', data);
            let resultData = Buffer.from(data);
            
            if (data.length === 3 && data.equals(Buffer.from([0x3e, 0x3e, 0x3e]))) {// 收到纯分隔符
                resultData = Buffer.alloc(0);// 空响应
            }else if(data.length > 3 && data.slice(data.length - 3).equals(Buffer.from([0x3e, 0x3e, 0x3e]))){
                resultData = data.slice(0, data.length - 3); // 去掉尾部>>>
            }else{
                //新增串口数据解析并回传给前端
                resultData = parseCleanedBuffer(resultData);
                mainWindow.webContents.send('serial-return',resultData );

                return
            }
            
            resultData = parseCleanedBuffer(resultData);
            //console.log(' result Data:', resultData);
            if (serialDeviceState.currentResolve) {
                serialDeviceState.currentResolve(resultData);
                serialDeviceState.currentResolve = null;
            }
            
        })
    }

    serialDeviceState.serialPort.once('close', () => {
        safeDisconnect()
        mainWindow.webContents.send('serial-disconnected');
    });

    // serialDeviceState.serialPort.once('error', err => {
    //     safeDisconnect();
    //     mainWindow.webContents.send('serial-disconnected');
    // });
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
    }else if (deviceType === 'ESP32') {
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
async function replSerial(type) {
    try {
        if (!serialDeviceState.serialPort ) {//|| serialDeviceState.replActive
          return { success: false, error: "串口未连接或已处于REPL模式"};
        }
        //console.log('replllll');

        // 中断当前程序
        await sendSerialCommand('\x03'); 
        
        
        await sendSerialCommand('from s4s import *\r',200);
        if(type === "Microbit"){
            await sendSerialCommand('from microbit import *\r',200);
            await sendSerialCommand('display.show(Image.HEART)\n\r', 200);
        }
        
        
        serialDeviceState.replActive = true;
        switchSerialParser(type,"repl")
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
        return { success: false, id: "001", error: "unconnected device" };//未连接设备
      }
      console.log(command)
      //发送
      serialDeviceState.serialBuffer = '';// 清空上次结果
      if(type === "Microbit"){
        serialDeviceState.serialPort.write( command + '\r' );
      }else if(type === "ESP32"){
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
      return { success: false, id: "", error: err.message };
    }
}
//清理数据阻塞
function clearCurrentCommand(result = null) {
    if (serialDeviceState.currentResolve) {
        serialDeviceState.currentResolve(result);
        serialDeviceState.currentResolve = null;
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
    //console.log('zf',command)

    if(type === "Microbit"){
        await sendSerialCommand(command  );
    }else if(type === "ESP32"){
        await sendSerialCommand(command  + '\r' );
    } else{
        serialDeviceState.serialPort.write(Buffer.from(command) );
    }
    
}


//######################################## microbit 运行长程序（粘贴模式） ########################################
async function mB_runCodeSerial(code) {
    // 中断当前程序
    await sendSerialCommand('\x03'); 
    await sendSerialCommand('\x05',200);
    await delay(100);
    serialDeviceState.serialPort.write( code );
    await delay(100);
    await sendSerialCommand('\x04');
}
//######################################## microbit 烧录固件（下载程序） ########################################
let abortCheckTimer = null;//检测终止下载
let isFlashing = false;//下载中
let flashAbort = false;

async function downloadCodeSerial(code) {//主函数代码，扩展列表
    try {
        if (!serialDeviceState.usbDevice) {
          //throw new Error("USB 设备未连接，无法烧录");
            throw { id: "001", error: "unconnected device", type: "toast"};
        }

        // flashAbort = false;
        // isFlashing = true;

        // 检测是否有对应的扩展文件
        //detectExtensionFile(packageList)

        // 递归检测扩展
        const extensionFiles = detectExtensionFile(code);


        // 生成 HEX 文件
        const hexPath = await generateV2Hex("Microbit_LinkBot_V1.0.0",code,extensionFiles);//固件名称未来需要提取到统一配置的文件中,统一到gui中
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
          mainWindow.webContents.send('flash-progress', {device: 'Microbit',stage: 'flashing',progress: percent});
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
        let errorResult;
        if ( typeof err === 'object'  && !(err instanceof Error)){
            errorResult = {
                success: false,
                id: err.id || "",
                error: err.error || "unknown error",
                type: err.type || "modal"
            };
        } else {
            errorResult = {
                success: false,
                id: "",
                error: err.message || err.error,
                type: ""
            };
        }
        mainWindow.webContents.send("flash-error", errorResult);

        // 出错后尝试断开连接
        if (serialDeviceState.daplink) {
          try { await serialDeviceState.daplink.disconnect(); } catch(e){}
          serialDeviceState.daplink = null;
        }

        return { 
            success: false, 
            id: err.id || "",
            error: err.message || err.error,
            type: err.type || "" 
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
      //return path.join(__dirname, '../../utils', relativePath);
      return path.join(process.resourcesPath, 'utils', relativePath);
    } else {
      // 开发环境
      return path.join(__dirname, '../../utils', relativePath);
    }
}


// 递归检测项目中所有扩展文件（会将文件直接读取出来，不需要再次读取了）
function detectExtensionFile(mainCode){
    const baseDir = getResourcePath('/PythonUploadFile');

    const visited = new Set();
    const result = new Map();

    function scan(code, parent = "main.py") {
        const modules = parsePythonImports(code);

        for (const mod of modules) {
            if (visited.has(mod)) continue;
            visited.add(mod);
            const filePath = path.join(baseDir, mod + ".py");

            // 如果不存在, 报错(后续需要统一报错格式与编码，方便前端进行识别与转换)
            if (!fs.existsSync(filePath)) {
                throw new Error(
                    `缺少Python扩展模块: ${mod}.py\n被 ${parent} 引用`
                );
            }

            // 读取文件
            const subCode = fs.readFileSync(filePath, 'utf8');
            result.set(mod, subCode);
            // 递归检测
            scan(subCode, mod + ".py");
        }
    }
    scan(mainCode);
    return result;
}

//解析导入的扩展python
function parsePythonImports(code) {
    const modules = new Set();

    const importRegex = /^\s*import\s+([a-zA-Z0-9_]+)/gm;
    const fromRegex = /^\s*from\s+([a-zA-Z0-9_]+)\s+import/gm;

    const exclude = new Set(['microbit', 's4s', "time"]); // 需要排除的模块

    let match;

    while ((match = importRegex.exec(code)) !== null) {
        modules.add(match[1]);
    }

    while ((match = fromRegex.exec(code)) !== null) {
        modules.add(match[1]);
    }

    // 过滤掉内置库
    return Array.from(modules).filter(m => !exclude.has(m));
}

//hex生成
async function generateV2Hex(firmwareName,code,extensionModules = new Map()) {
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

        // 写入扩展文件
        for (const [name, moduleCode] of extensionModules) {
            fsHex.write(name + '.py', moduleCode);
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


//######################################## ESP 下载程序 ########################################
async function downloadCodeSerial_ESP(code) {
    try {
        if (!serialDeviceState.usbDevice) {
          throw new Error("设备未连接，无法烧录");
        }

    } catch (err) {

        
    }finally {
       
    }
}

//######################################## Arduino 下载程序 ########################################
let isFlashing_arduino = false;//下载中
let stage_arduino = "";//阶段

// const projectDir = getResourcePath('platformio_cli');// 工程目录
// const inoFile = path.join(projectDir, 'Arduino1.ino');//主文件
// const libsRoot = getResourcePath('Arduino1/lib');// 自定义库
// const cliPath = getResourcePath('Arduino1/arduino-cli.exe');// arduino-cli.exe
// const configPath = getResourcePath('Arduino1/arduino-cli.yaml');// yaml

const rootDir_Arduino = getResourcePath('platformio_cli');// PlatformIO 根目录
const compileBat = path.join(rootDir_Arduino, 'pio_build.bat');//烧录入口
const uploadBat = path.join(rootDir_Arduino, 'pio_upload.bat');//上传入口
const projectDir_Arduino = path.join(rootDir_Arduino, 'project', 'src');//工程目录
const codeFile_Arduino = path.join(projectDir_Arduino, 'src', 'main.cpp');//主文件

async function downloadCodeSerial_Arduino(code) {
    let oldPort = null;
    try {
        if(isFlashing_arduino) return;//防止重复下载

        //判断是否有连接设备
        if (!serialDeviceState.serialPort) {
            throw { id: "001", error: "unconnected device", type: "toast"};
        }
        oldPort = serialDeviceState.serialPort.path;

        //断开旧的连接
        await safeDisconnect(true);//静默断开

        mainWindow.webContents.send('flash-progress', {device: 'Arduino',stage: 'compile',progress: 0});
        isFlashing_arduino = true;

        await setNotCustomBuild(false);

        // 写入代码
        fs.writeFileSync(codeFile_Arduino, code);

        // 编译
        stage_arduino = "compile";
        err_compile_arduino = "";
        await runPioCompile();

        //console.log('compile success');

        // 上传 
        stage_arduino = "upload";
        err_upload_arduino = "";
        await runPioUpload(oldPort);

        //console.log('PIO upload success');

        return { success: true };
        
   
    } catch (err) {
        let errorResult;
        if ( typeof err === 'object'  && !(err instanceof Error)){
            errorResult = {
                success: false,
                id: err.id || "",
                error: err.error || "unknown error",
                type: err.type || "modal"
            };
        } else {
            errorResult = {
                success: false,
                id: "",
                error: err.message || err.error,
                type: ""
            };
        }
        mainWindow.webContents.send("flash-error", errorResult);
    
        return { success: false, error: err.message };
    } finally {
        if (oldPort) {
            try {
                await delay(800);// 等待系统释放 COM

                //再重试连接
                let retry = 2;
                while (retry-- > 0) {
                    try {
                        await connSerialOnly({ comPort: oldPort }, "Arduino");
                        break;
                    } catch (e) {
                        console.error('重连失败，重试中...', e);
                        await delay(500);
                    }
                }

            } catch (e) {
                console.error('最终重连失败:', e);
            }
        }

        isFlashing_arduino = false;
    }
}

// 编译 
function runPioCompile() {
    return runBatProcess({
        batPath: compileBat,
        stage: 'compile'
    });
}

//上传
function runPioUpload(port) {
    const extraArgs = [];

    // 固定串口
    if (port) {
        extraArgs.push(port);
    }

    return runBatProcess({
        batPath: uploadBat,
        stage: 'upload',
        extraArgs
    });
}

let err_upload_arduino = "";//上传错误
let err_compile_arduino = "";//编译错误
//通用 bat 执行
function runBatProcess({ batPath, stage, extraArgs = [], useFirmwareUI = false}) {
    return new Promise((resolve, reject) => {
        const cli = spawn( 'cmd.exe',[  '/c', batPath, ...extraArgs  ],
            {
                cwd: rootDir_Arduino, 
                windowsHide: true,
                env: {
                    ...process.env,
                    PLATFORMIO_CORE_DIR: path.join(rootDir_Arduino, '.platformio'),
                    PATH: [
                        path.join(rootDir_Arduino, 'python'),
                        process.env.PATH
                    ].join(';')
                }
            }
        );

        // stdout
        cli.stdout.on('data', data => {
            const text = data.toString();
            // console.log("stdout");
            //console.log(text);
            
            const result = parseCliLine(text,stage);
            mainWindow.webContents.send('flash-progress', result );
        });

        // stderr
        cli.stderr.on('data', data => {
            const text = data.toString();
            // console.log("stderr");
            //console.error(text);
            if(stage === "compile"){
                err_compile_arduino += text;
            }else{
                if(text.includes("could not open")){
                    err_upload_arduino = text;
                }
            }
        });

        // close
        cli.on('close', code => {
           // console.log(`${stage} exit:`, code);
            if (code === 0) {
                resolve();
                if(stage_arduino == "upload"){
                    mainWindow.webContents.send("flash-done");
                }
            } else {
                if (stage === "upload") {
                    reject( new Error(  `${stage} failed: ${code}\n${err_upload_arduino}` ) );
                } else {
                    reject( new Error(  `${stage} failed: ${code}\n${err_compile_arduino}` ) );
                }
            }
        });

        // error
        cli.on('error', err => {
            console.error(`${stage} error:`, err);
            reject(err);
        });
    });
}

// 进度处理
function parseCliLine(text, stage) {
    const result = {
        device: 'Arduino',
        stage: stage,//阶段
        progress: null,//进度
        message: "",//消息
    };

    if(stage === "compile"){//编译不处理，直接返回
        result.message = text;
    }else{//上传阶段
        const percentMatch = text.match(/(\d+)%/);
        if (text.includes('Erase flash')) {
            result.progress = 0;
        }else if(percentMatch){//提取进度
            result.progress = parseInt( percentMatch[1], 10);
        }
    }
    //console.log("00000",result);
    return result;
}


// 正常版本的arduino才用下面这个
// 运行cli
// function runCli(args, cliPath) {
//     return new Promise((resolve, reject) => {
//         const cli = spawn(cliPath, args, {
//             windowsHide: true,
//             cwd: path.dirname(cliPath)
//         });

//         cli.stdout.on('data', data => {
//             const text = data.toString();
//             const result = parseCliLine(text);//处理数据
//             //console.log(result)
//             mainWindow.webContents.send('flash-progress', result);
//         });

//         cli.stderr.on('data', data => {
//             console.log(111)
//             console.error(data.toString());
//         });

//         // cli.on('spawn', () => {
//         //     console.log("CLI START");
//         // });

//         cli.on('close', code => {
//             // console.log("exit:", code);
//             if (code === 0) {
//                 resolve();
//                 if(stage_arduino == "upload"){
//                     mainWindow.webContents.send("flash-done");
//                 }
//             } else {
//                 reject(new Error(`CLI Error, exitCode= ${code}`));
//             }
//         });
//     });
// }
// // 进度处理
// function parseCliLine(text) {
//     const result = {
//         device: 'Arduino',
//         stage: 'compile',//阶段
//         progress: null,//进度
//         message: "",//消息
//     };

//     // 编译完成
//     if ( text.includes('Sketch uses') || text.includes('Global variables use') ) {
//         result.stage = 'compile';
//         result.message = text;
//     }else if ( text.includes('Write ') || text.includes('Erase flash') ) {// 刚刚进入烧录阶段
//         result.stage = 'flashing';
//         result.progress = 0;
//         //result.message = text;
//     }else if ( text.includes('Done in') || text.includes('New upload port') ) { // 成功
//         result.stage = 'flashing';
//         result.progress = 100;
//         //result.message = text;
//     }else{// 提取进度
//         const percentMatch = text.match(/(\d+)%/);
//         if (percentMatch) {
//             result.stage = 'flashing';
//             result.progress = parseInt(percentMatch[1]);
//         }
//     }

//     return result;
// }

//暂时不用的方案，舍不得删（创建临时文件且安装包）
// async function downloadCodeSerial_Arduino1(code) {
//     try {
//         const sketchDir = path.join(os.tmpdir(), "mysketch");
//         if (!fs.existsSync(sketchDir)) fs.mkdirSync(sketchDir);

//         const sketchFile = path.join(sketchDir, "mysketch.ino");
//         fs.writeFileSync(sketchFile, code);

//         // 安装 R4 支持包（可放在初始化位置）
//         //await runCli(["core", "install", "arduino:renesas_uno"]);
//         const libsPath = getResourcePath('/Arduino1/libraries');//自定义库路径

//         //编译 
//         await runCli([
//             'compile',
//             '--fqbn', 'arduino:renesas_uno:unor4wifi',
//             '--libraries', libsPath,
//             sketchDir
//         ]);


//         // 上传 
//         await runCli([
//             'upload',
//             '-p', "COM20",//serialDeviceState.serialPort.path,
//             '--fqbn', 'arduino:renesas_uno:unor4wifi',
//             sketchDir
//         ]);

//         return { success: true };

//     } catch (err) {
//         let errorResult;
//         if ( typeof err === 'object'  && !(err instanceof Error)){
//             errorResult = {
//                 success: false,
//                 id: err.id || "",
//                 error: err.error || "unknown error",
//                 type: err.type || "modal"
//             };
//         } else {
//             errorResult = {
//                 success: false,
//                 id: "",
//                 error: err.message || err.error,
//                 type: ""
//             };
//         }
//         mainWindow.webContents.send("flash-error", errorResult);

//         console.log("9999", err.message);
//         return { success: false, error: err.message };
//     } finally{
//         isFlashing_arduino = false;
//     }
// }

// // 调用cli
// function runCli(args) {
//     const config = getResourcePath('/Arduino1/arduino-cli.yaml');
//     const cliPath = getResourcePath('/Arduino1/arduino-cli.exe');
//     return new Promise((resolve, reject) => {
//         console.log("RUN:", cliPath, args.join(" ")); // ⭐ 打印执行命令
//         const proc = spawn(cliPath, ["--config-file", config, ...args], { shell: true });

//         proc.stdout.on('data', d => {//编译
//             console.log("[CLI stdout]:", d.toString());  // ⭐ 打印 stdout
//         });
//         proc.stderr.on('data', d => {
//             console.error("[CLI stderr]:", d.toString()); // ⭐ 打印 stderr
//         });

//         proc.on('close', code => {
//             console.log("CLI exit code:", code);          // ⭐ 打印退出码

//             if (code === 0) resolve();
//             else reject(new Error("CLI Error, exitCode=" + code));
//         });
//     });
// }

//######################################## 统一固件烧录接口 ########################################
// 分发
async function unifiedFlashFirmware(deviceType,  port) {
    try {
        if(deviceType === 'Microbit'){
            if (serialDeviceState.serialPort) {// 设备已连接时
                if(serialDeviceState.serialPort.path === port){//给当前设备
                    await flashWithCurrentDevice("Microbit_LinkBot_V1.0.0","");
                }else{//非当前设备
                    throw new Error("已有连接设备，请先断开");
                }
            }else{//未连接时
                await connectAndFlash("Microbit_LinkBot_V1.0.0",port, "");
            }
        }
        if (deviceType == 'Arduino') {
            await arduinoUploadFirmware(port);
        }

        if (deviceType == 'ESP32') {
            await espUploadFirmware(port);
        }
        
        // 烧录成功-通知前端结束流程
        mainWindow.webContents.send('flash-firmware-done');
        return { success: true };
    }catch(e){
        console.log(e)
        mainWindow.webContents.send( 'flash-firmware-error', e?.message || '烧录失败' );
        return { success: false, error: e.message };
    }
 
}
//--------------------------Arduino烧录流程-------------------------
async function arduinoUploadFirmware(port) {
    let fakeTimer = null;
    try {
        // 如果有连接，先断开
        const currentPort = serialDeviceState.serialPort?.path;
        if (currentPort) {
            await safeDisconnect(false);// 非静默断开
        }

        await setNotCustomBuild(true);

        //通知前端开始烧录
        mainWindow.webContents.send('flash-firmware-start');



         // 默认程序
        const code = `
#include "TinkerCode.h"

void app_setup() {

}

void app_loop() {

}`;
        // 写入 main.cpp
        fs.writeFileSync(codeFile_Arduino, code);

        // 编译阶段 0~50
        let compileProgress = 0;

        fakeTimer = setInterval(() => {
            // 随机步进
            let step = Math.floor(Math.random() * 4) + 1;

            // 偶尔卡一下
            if (Math.random() < 0.15) step = 0;

            compileProgress += step;

            // 到 50 前减速 + 抖动
            if (compileProgress > 49) {
                compileProgress = 49;
            }

            // // 再加一点“回退感”（更真实）
            // if (Math.random() < 0.05) {
            //     compileProgress -= 1;
            // }

            if (compileProgress < 0) compileProgress = 0;

            mainWindow.webContents.send( 'flash-firmware-progress', Math.round(compileProgress)  );
        }, 300 + Math.random() * 300); // 间隔也随机（300~600ms）

        await runFirmwareCompile();

        clearInterval(fakeTimer);
        mainWindow.webContents.send( 'flash-firmware-progress',  50 );

        // 上传阶段 50~100
        await runArduinoUploadFirmware(port);

        mainWindow.webContents.send('flash-firmware-done');
        return { success: true };
    }catch(e){
        if (fakeTimer) {
            clearInterval(fakeTimer);
        }
        console.log(e)
        throw e;
    }
}

function runFirmwareCompile() {
    return new Promise((resolve, reject) => {
        // const cli = spawn('cmd.exe', ['/c', compileBat], {
        //     cwd: path.dirname(compileBat),
        //     windowsHide: true
        // });
        const cli = spawn( 'cmd.exe',[ '/c', compileBat ],
            {
                cwd: rootDir_Arduino, 
                windowsHide: true,
                env: {
                    ...process.env,
                    PLATFORMIO_CORE_DIR: path.join(rootDir_Arduino, '.platformio'),
                    PATH: [
                        path.join(rootDir_Arduino, 'python'),
                        process.env.PATH
                    ].join(';')
                }
            }
        );

        cli.on('close', code => {
            if (code === 0) resolve();
            else reject(new Error('compile failed'));
        });

        cli.on('error', reject);
    });
}

//应该可以复用上面的烧录流程
function runArduinoUploadFirmware(port) {
    let progress =50;
    return new Promise((resolve, reject) => {

        const args = ['/c', uploadBat, port];
        // const cli = spawn('cmd.exe', args, {
        //     cwd: path.dirname(uploadBat),
        //     windowsHide: true
        // });
        const cli = spawn( 'cmd.exe',args,
            {
                cwd: rootDir_Arduino, 
                windowsHide: true,
                env: {
                    ...process.env,
                    PLATFORMIO_CORE_DIR: path.join(rootDir_Arduino, '.platformio'),
                    PATH: [
                        path.join(rootDir_Arduino, 'python'),
                        process.env.PATH
                    ].join(';')
                }
            }
        );
        

        cli.stdout.on('data', data => {
            const text = data.toString();
            const match = text.match(/(\d+)%/);

            if (match) {
                const uploadPercent = Number(match[1]);

                // 50~100 映射
                let uiPercent = 50 + uploadPercent * 0.5;
                if (uiPercent < progress) uiPercent = progress;
                progress = uiPercent;

                mainWindow.webContents.send( 'flash-firmware-progress',  Math.round(uiPercent));
            }
        });

        cli.stderr.on('data', data => {
            console.error('[Arduino upload]', data.toString());
        });

        cli.on('close', code => {
            if (code === 0) {
                resolve();
            } else {
                reject(new Error(`upload failed: ${code}`));
            }
        });

        cli.on('error', reject);
    });
}

// 控制编译配置文件，开启互动模式  false:注释掉
function setNotCustomBuild(enable) {
    const iniPath = path.join(
        getResourcePath('platformio_cli'),
        'project',
        'src',
        'platformio.ini'
    );

    let content = fs.readFileSync(iniPath, 'utf8');

    if (enable) {
        // 取消注释
        content = content.replace(
            /^\s*;\s*-D\s+NOT_CUSTOM_BUILD.*$/m,
            '    -D NOT_CUSTOM_BUILD               ; 当前不为自定义编译'
        );
    } else {
        // 注释掉
        content = content.replace(
            /^\s*-D\s+NOT_CUSTOM_BUILD.*$/m,
            '    ; -D NOT_CUSTOM_BUILD               ; 当前不为自定义编译'
        );
    }

    fs.writeFileSync(iniPath, content, 'utf8');
}

//--------------------------Microbit烧录流程--------------------------
// microbit 使用当前设备烧录
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
        throw error;
    }
}

// microbit 未连接设备，直接连接并烧录(哎，一团乱麻，先这么用吧)
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
    } catch (e) {
        throw e;   
    }  finally {
        await safeDisconnect();
    }
}

//--------------------------ESP32烧录流程--------------------------
// ESP 烧录
async function espUploadFirmware(port) {
    try {
        // 检测当前是否有串口连接
        const currentPort = serialDeviceState.serialPort?.path;
        if (currentPort) {
            // 非静默断开（通知 UI）
            await safeDisconnect(false);
        }

        const exePath = getResourcePath('esp32_firmware/esptool.exe');
        const binPath = getResourcePath('esp32_firmware/s4s_esp32_v1.0.0.bin');

        //通知前端开始烧录
        mainWindow.webContents.send('flash-firmware-start');

        //烧录
        const args = [
            '--port', port,
            '--baud', '1152000',
            'write_flash',
            '0x0',
            binPath
        ];
        const child = spawn(exePath, args);

        //进度解析
        child.stdout.on('data', (data) => {
            const msg = data.toString();
            console.log('[ESP32 OUT]', msg);
            parseEspProgress(msg);
        });

        // child.stderr.on('data', (data) => {
        //     const msg = data.toString();
        //     console.log('[ESP32 ERR]', msg);
        //     parseEspProgress(msg);
        // });

        //阻塞等待完成
        await new Promise((resolve, reject) => {
            child.on('close', (code) => {
                if (code === 0) {
                    resolve(code);
                } else {
                    reject(new Error(`烧录失败(code=${code})`));
                }
            });
            child.on('error', (err) => reject(err));
        });

        return { success: true };
    }catch(e){
        console.log(e)
        throw e; 
    }
}

// ESP 烧录进度解析
function parseEspProgress(text) {
    const match = text.match(/\((\d+)\s*%\)/);
    if (!match) return;
    const percent = Number(match[1]);
    mainWindow.webContents.send( 'flash-firmware-progress', percent );
}


// 写入ESP WiFi配置
async function writeEspWiFi(ssid, password, port) {
    try {
        // 当前已连接串口
        const currentPort =  serialDeviceState.serialPort?.path;

        // 未连接 或 连接的不是目标串口
        if (!currentPort || currentPort !== port) {
            // 有连接先断开
            if (currentPort) {
                await safeDisconnect(true); 
            }

            // 建立ESP32连接
            await connectSerialDevice( { comPort: port, vendorId: DEVICE_CONFIGS.ESP32.vendorId, productId: DEVICE_CONFIGS.ESP32.productIds[0] }, "ESP32" );
        }

        await new Promise(resolve => setTimeout(resolve, 500));

        // 进入REPL
        await replSerial("ESP32");

        await new Promise(resolve => setTimeout(resolve, 200));

        // 调用固件函数
        await sendCommandSerial(
            `update_wifi_config(${JSON.stringify(ssid)},${JSON.stringify(password)})`,
            "ESP32"
        ); 

        // 保存Flash
        await new Promise(resolve =>  setTimeout(resolve, 1000));

        // 重启ESP32
        await sendCommandSerial_D(
            'reset()',
            "ESP32"
        );

        // 等待一小段时间再断开本地状态
        // await new Promise(resolve => setTimeout(resolve, 500));

        // // 断开串口，清理状态
        // await safeDisconnect(true);

        return { success: true };
    } catch (err) {
        return {
            success: false,
            error: err.message
        };
    }
}


// 延时函数
function delay(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

// 辅助函数--解析串口返回的 Buffer 数据，尝试转换为数字或文本
function parseCleanedBuffer(cleanedData) {
    if (cleanedData.length === 0) return null;
    
    // 1. 尝试作为文本解析
    const asText = cleanedData.toString('ascii');
    
    // 2. 如果是纯数字
    if (/^\d+$/.test(asText)) {
        return Number(asText);
    }
    
    // 3. 如果是16位整数编码
    if (cleanedData.length === 2) {
        const value = cleanedData.readUInt16BE(0);
        const high = (value >> 8) & 0xFF;
        const low = value & 0xFF;
        
        if (high >= 0x20 && high <= 0x7E && low >= 0x20 && low <= 0x7E) {
            const str = String.fromCharCode(high) + String.fromCharCode(low);
            if (/^\d+$/.test(str)) {
                return Number(str);
            }
        }
    }
    
    // 4. 默认返回文本
    return asText;
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
    downloadCodeSerial_ESP,
    downloadCodeSerial_Arduino,
    cancelDownloadCodeSerial,
    unifiedFlashFirmware,
    writeEspWiFi,
    mB_runCodeSerial
};
  