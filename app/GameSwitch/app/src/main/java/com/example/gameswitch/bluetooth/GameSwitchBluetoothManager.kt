package com.example.gameswitch.bluetooth

import android.Manifest
import android.bluetooth.*
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanFilter
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.os.ParcelUuid
import android.util.Log
import androidx.annotation.RequiresPermission
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import java.util.*

class GameSwitchBluetoothManager(private val context: Context) {
    companion object {
        private const val TAG = "GameSwitchBluetoothManager"
        private const val UART_SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
        private const val TX_CHARACTERISTIC_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
        private const val RX_CHARACTERISTIC_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
        private const val LEFT_DEVICE_NAME = "GameSwitchLeftUSB"
        private const val RIGHT_DEVICE_NAME = "GameSwitchRightUSB"
    }

    private var bluetoothAdapter: BluetoothAdapter? = null
    private var leftGatt: BluetoothGatt? = null
    private var rightGatt: BluetoothGatt? = null
    private var leftTxCharacteristic: BluetoothGattCharacteristic? = null
    private var rightTxCharacteristic: BluetoothGattCharacteristic? = null
    
    private var isLeftConnected = false
    private var isRightConnected = false
    
    interface ConnectionListener {
        fun onDeviceConnected(deviceName: String)
        fun onDeviceDisconnected(deviceName: String)
        fun onConnectionFailed(deviceName: String)
        fun onModeChanged(mode: String)
    }
    
    var connectionListener: ConnectionListener? = null

    init {
        val bluetoothManager = context.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothAdapter = bluetoothManager.adapter
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_SCAN)
    fun startScanning() {
        if (bluetoothAdapter == null) {
            Log.e(TAG, "Bluetooth adapter is null")
            connectionListener?.onConnectionFailed("Bluetooth not available")
            return
        }
        
        if (!bluetoothAdapter!!.isEnabled) {
            Log.e(TAG, "Bluetooth is not enabled")
            connectionListener?.onConnectionFailed("Bluetooth is disabled")
            return
        }

        val scanner = bluetoothAdapter?.bluetoothLeScanner
        if (scanner == null) {
            Log.e(TAG, "Bluetooth LE scanner is null")
            connectionListener?.onConnectionFailed("BLE scanner not available")
            return
        }
        
        try {
            // Stop any ongoing scan first
            scanner.stopScan(scanCallback)
            
            val scanFilters = listOf(
                ScanFilter.Builder().setDeviceName(LEFT_DEVICE_NAME).build(),
                ScanFilter.Builder().setDeviceName(RIGHT_DEVICE_NAME).build()
            )
            
            val scanSettings = ScanSettings.Builder()
                .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
                .build()

            scanner.startScan(scanFilters, scanSettings, scanCallback)
            Log.i(TAG, "Started scanning for devices")
        } catch (e: Exception) {
            Log.e(TAG, "Failed to start scan", e)
            connectionListener?.onConnectionFailed("Scan failed: ${e.message}")
        }
    }

    private val scanCallback = object : ScanCallback() {
        override fun onScanResult(callbackType: Int, result: ScanResult) {
            result.device?.let { device ->
                when (device.name) {
                    LEFT_DEVICE_NAME -> {
                        if (!isLeftConnected) {
                            connectDevice(device, true)
                        }
                    }
                    RIGHT_DEVICE_NAME -> {
                        if (!isRightConnected) {
                            connectDevice(device, false)
                        }
                    }
                }
            }
        }

        override fun onScanFailed(errorCode: Int) {
            Log.e(TAG, "Scan failed with error: $errorCode")
        }
    }

    private fun connectDevice(device: BluetoothDevice, isLeft: Boolean) {
        val gattCallback = object : BluetoothGattCallback() {
            override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
                when (newState) {
                    BluetoothProfile.STATE_CONNECTED -> {
                        Log.i(TAG, "Connected to ${device.name}")
                        gatt.discoverServices()
                        if (isLeft) {
                            isLeftConnected = true
                        } else {
                            isRightConnected = true
                        }
                        connectionListener?.onDeviceConnected(device.name)
                    }
                    BluetoothProfile.STATE_DISCONNECTED -> {
                        Log.i(TAG, "Disconnected from ${device.name}")
                        if (isLeft) {
                            isLeftConnected = false
                            leftGatt?.close()
                            leftGatt = null
                            leftTxCharacteristic = null
                        } else {
                            isRightConnected = false
                            rightGatt?.close()
                            rightGatt = null
                            rightTxCharacteristic = null
                        }
                        connectionListener?.onDeviceDisconnected(device.name)
                        // Attempt reconnection after delay
                        CoroutineScope(Dispatchers.IO).launch {
                            delay(3000)
                            startScanning()
                        }
                    }
                }
            }

            override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
                if (status == BluetoothGatt.GATT_SUCCESS) {
                    val service = gatt.getService(UUID.fromString(UART_SERVICE_UUID))
                    service?.let {
                        val txChar = it.getCharacteristic(UUID.fromString(TX_CHARACTERISTIC_UUID))
                        val rxChar = it.getCharacteristic(UUID.fromString(RX_CHARACTERISTIC_UUID))
                        
                        if (isLeft) {
                            leftTxCharacteristic = txChar
                        } else {
                            rightTxCharacteristic = txChar
                        }
                        
                        // Enable notifications for RX characteristic
                        rxChar?.let { rx ->
                            gatt.setCharacteristicNotification(rx, true)
                            val descriptor = rx.getDescriptor(UUID.fromString("00002902-0000-1000-8000-00805f9b34fb"))
                            descriptor?.value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
                            gatt.writeDescriptor(descriptor)
                        }
                    }
                }
            }

            override fun onCharacteristicChanged(gatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic) {
                // Handle incoming data
                val data = characteristic.value
                val dataString = String(data, Charsets.UTF_8)
                Log.d(TAG, "Received data from ${gatt.device.name}: $dataString")
                
                // Check if it's a mode update
                if (dataString.contains("Mode")) {
                    connectionListener?.onModeChanged(dataString)
                } else {
                    // Forward data from left to right device (if needed)
                    if (gatt.device.name == LEFT_DEVICE_NAME) {
                        sendToRightDevice(dataString)
                    }
                }
            }
        }

        val gatt = device.connectGatt(context, false, gattCallback)
        if (isLeft) {
            leftGatt = gatt
        } else {
            rightGatt = gatt
        }
    }

    fun sendCommand(command: String) {
        val data = command.toByteArray()
        
        leftTxCharacteristic?.let { char ->
            char.value = data
            leftGatt?.writeCharacteristic(char)
        }
        
        rightTxCharacteristic?.let { char ->
            char.value = data
            rightGatt?.writeCharacteristic(char)
        }
    }
    
    private fun sendToRightDevice(data: String) {
        val bytes = data.toByteArray()
        
        rightTxCharacteristic?.let { char ->
            char.value = bytes
            rightGatt?.writeCharacteristic(char)
        }
    }

    fun disconnect() {
        bluetoothAdapter?.bluetoothLeScanner?.stopScan(scanCallback)
        leftGatt?.close()
        rightGatt?.close()
        leftGatt = null
        rightGatt = null
        isLeftConnected = false
        isRightConnected = false
    }
}