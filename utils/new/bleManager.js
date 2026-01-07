/*  蓝牙操作 */
const noble = require('@abandonware/noble');

let initialized = false;
let scanning = false;

const devices = new Map();
const peripheralMap = new Map();
let connectedPeripheral = null;

// 扫描指定服务
const TARGET_SERVICE_UUIDS = [
  '108f94d5-570b-4a6c-9a47-12f428f362e6'
];

/* ------------------------- 初始化 ------------------------- */

function initBleOnce() {
  if (initialized) return;
  initialized = true;

  noble.on('stateChange', state => {
    if (state === 'poweredOn') {
      console.log('[BLE] powered on');
    } else {
      console.log('[BLE] state:', state);
      noble.stopScanning();
    }
  });

  noble.on('discover', handleDiscover);
}

// 处理扫描
function handleDiscover(peripheral) {
  const { id, advertisement, address, rssi } = peripheral;
  console.log(peripheral)
  devices.set(id, {
    comPort: id,
    name: advertisement.localName || id,
    mode: 'bluetooth',
    address,
    rssi
  });

  peripheralMap.set(id, peripheral);
}

initBleOnce();

/* ------------------------- 扫描 ------------------------- */

async function scanBleDevices(timeout = 3000) {
  devices.clear();

  if (noble.state !== 'poweredOn') {
    return {
      success: false,
      error: 'bluetooth not powered on',
      devices: []
    };
  }

  if (!scanning) {
    scanning = true;
    await noble.startScanningAsync(TARGET_SERVICE_UUIDS, false);
  }

  await delay(timeout);

  await noble.stopScanningAsync();
  scanning = false;

  return {
    success: true,
    devices: Array.from(devices.values())
  };
}

/* ------------------------- 连接 ------------------------- */

async function connectBleDevice(deviceInfo) {
  const id = deviceInfo.comPort;
  const peripheral = peripheralMap.get(id);

  if (!peripheral) {
    throw new Error('BLE device not found');
  }

  await disconnectIfNeeded(id);

  if (peripheral.state !== 'connected') {
    await peripheral.connectAsync();
  }

  connectedPeripheral = peripheral;
  console.log('[BLE] connected:', id);

  // 连接后读取 GATT Device Name
  let deviceName = deviceInfo.name;
  const gattName = await readBleDeviceName(peripheral);

  if (gattName) {
    console.log('[BLE] GATT device name:', gattName);
  } else {
    console.log('[BLE] GATT device name not found');
  }

  return {
    success: true,
    info: {
      comPort: id,
      name: deviceName ,
      type: 'bluetooth'
    }
  };
}


/* ------------------------- 断开 ------------------------- */

async function disBleDevice() {
  // 1. 停止扫描（如果有）
  if (scanning) {
    try {
      await noble.stopScanningAsync();
    } catch (e) {}
    scanning = false;
  }

  // 2. 断开当前连接
  if (connectedPeripheral) {
    try {
      if (connectedPeripheral.state === 'connected') {
        await connectedPeripheral.disconnectAsync();
        console.log('[BLE] disconnected:', connectedPeripheral.id);
      }
    } catch (err) {
      console.warn('[BLE] disconnect failed:', err.message);
    } finally {
      connectedPeripheral = null;
    }
  }

  // 3. 清空缓存（可选，但推荐）
  devices.clear();
  peripheralMap.clear();

  return {
    success: true
  };
}

//安全断开
async function disconnectIfNeeded(nextId = null) {
  if (!connectedPeripheral) return;

  // 如果是同一个设备，不断
  if (nextId && connectedPeripheral.id === nextId) {
    return;
  }

  try {
    if (connectedPeripheral.state === 'connected') {
      await connectedPeripheral.disconnectAsync();
      console.log('[BLE] disconnected:', connectedPeripheral.id);
    }
  } catch (e) {
    console.warn('[BLE] disconnect error:', e.message);
  } finally {
    connectedPeripheral = null;
  }
}

/* ------------------------- helpers ------------------------- */

async function disconnectIfNeeded(targetId) {
  if (
    connectedPeripheral &&
    connectedPeripheral.id !== targetId
  ) {
    try {
      await connectedPeripheral.disconnectAsync();
    } catch (_) {}
    connectedPeripheral = null;
  }
}

async function readBleDeviceName(peripheral) {
  try {
    await peripheral.discoverServicesAsync(['1800']);
    const service = peripheral.services?.[0];
    if (!service) return null;

    await service.discoverCharacteristicsAsync(['2a00']);
    const ch = service.characteristics?.[0];
    if (!ch) return null;

    const data = await ch.readAsync();
    return data
      .toString('utf8')
      .replace(/\0/g, '')
      .trim();
  } catch (err) {
    console.warn('[BLE] read device name failed:', err.message);
    return null;
  }
}

function delay(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

/* ------------------------- exports ------------------------- */

module.exports = {
  scanBleDevices,
  connectBleDevice,
  disBleDevice
};
