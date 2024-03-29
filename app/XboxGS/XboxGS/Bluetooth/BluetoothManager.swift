//
//  BluetoothManager.swift
//  XboxGS
//
//  Created by Owen McGirr on 26/09/2019.
//  Copyright © 2019 Owen McGirr. All rights reserved.
//

import CoreBluetooth


protocol BluetoothManagerDelegate {
    func didChangeMode(to mode: String)
}


class BluetoothManager: NSObject, CBCentralManagerDelegate, CBPeripheralDelegate {
    
    static let shared = BluetoothManager()
    
    
    var delegate: BluetoothManagerDelegate?
    
    
    // variables
    var centralManager: CBCentralManager!
    var leftUSBPeripheral: CBPeripheral?
    var rightUSBPeripheral: CBPeripheral?
    
    // services and characteristics
    var uartServiceLeft: CBService?
    var uartServiceRight: CBService?
    let uartServiceId = CBUUID(string: "6e400001-b5a3-f393-e0a9-e50e24dcca9e")
    let txCharacteristicId = CBUUID(string: "6e400002-b5a3-f393-e0a9-e50e24dcca9e")
    var leftTxCharacteristic: CBCharacteristic?
    var rightTxCharacteristic: CBCharacteristic?
    let rxCharacteristicId = CBUUID(string: "6e400003-b5a3-f393-e0a9-e50e24dcca9e")
    var leftRxCharacteristic: CBCharacteristic?
    
    
    func start() {
        centralManager = CBCentralManager(delegate: self, queue: nil)
        DispatchQueue.main.asyncAfter(deadline: .now() + 1.0, execute: {
            self.centralManager.scanForPeripherals(withServices: [self.uartServiceId], options: nil)
        })
    }
    
    
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        print("state: \(central.state)")
    }
    
    
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        print("discovered: \(peripheral.name ?? "n/a")")
        if let n = peripheral.name {
            if n == PeripheralNames.left {
                centralManager.connect(peripheral, options: nil)
                leftUSBPeripheral = peripheral
            }
            if n == PeripheralNames.right {
                centralManager.connect(peripheral, options: nil)
                rightUSBPeripheral = peripheral
            }
            
            if leftUSBPeripheral != nil && rightUSBPeripheral != nil {
                centralManager.stopScan()
                
                leftUSBPeripheral?.delegate = self
                rightUSBPeripheral?.delegate = self
                DispatchQueue.main.asyncAfter(deadline: .now() + 3.0, execute: {
                    self.leftUSBPeripheral?.discoverServices([self.uartServiceId])
                    self.rightUSBPeripheral?.discoverServices([self.uartServiceId])
                })
            }
            else {
                centralManager.scanForPeripherals(withServices: nil, options: nil)
            }
        }
    }
    
    
    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        if let n = peripheral.name {
            if n == PeripheralNames.left {
                leftUSBPeripheral = nil
                uartServiceLeft = nil
                leftTxCharacteristic = nil
                leftRxCharacteristic = nil
            }
            if n == PeripheralNames.right {
                rightUSBPeripheral = nil
                uartServiceRight = nil
                rightTxCharacteristic = nil
            }
        }
        centralManager.scanForPeripherals(withServices: nil, options: nil)
    }
    
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        if let error = error {
            print("error: \(error)")
            return
        }
        let services = peripheral.services
        print("Found \(String(describing: services?.count)) services! :\(String(describing: services)) on \(String(describing: peripheral.name))")
        if let n = peripheral.name {
            if n == PeripheralNames.left {
                uartServiceLeft = peripheral.services?.first
                peripheral.discoverCharacteristics([txCharacteristicId, rxCharacteristicId], for: uartServiceLeft!)
            }
            if n == PeripheralNames.right {
                uartServiceRight = peripheral.services?.first
                peripheral.discoverCharacteristics([txCharacteristicId], for: uartServiceRight!)
            }
        }
    }
    
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        if let n = peripheral.name {
            if n == PeripheralNames.left {
                for c in service.characteristics! {
                    if c.uuid.uuidString == txCharacteristicId.uuidString {
                        leftTxCharacteristic = c
                    }
                    if c.uuid.uuidString == rxCharacteristicId.uuidString {
                        leftRxCharacteristic = c
                        leftUSBPeripheral?.setNotifyValue(true, for: leftRxCharacteristic!)
                    }
                }
            }
            if n == PeripheralNames.right {
                for c in service.characteristics! {
                    if c.uuid.uuidString == txCharacteristicId.uuidString {
                        rightTxCharacteristic = c
                    }
                }
            }
        }
    }
    
    
    func peripheral(_ peripheral: CBPeripheral, didUpdateNotificationStateFor characteristic: CBCharacteristic, error: Error?) {
        print("\(String(describing: peripheral.name)) notifications \(characteristic.isNotifying)")
    }
    
    
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        let val = String(bytes: characteristic.value!, encoding: .utf8)
        print("\(String(describing: val))")
        if val?.contains("Mode") ?? false {
            delegate?.didChangeMode(to: val!)
            return
        }
        write(to: PeripheralNames.right, str: val!)
    }
    
    
    func devicesNotFound() -> Bool {
        return leftUSBPeripheral == nil || rightUSBPeripheral == nil
    }
    
    
    func write(to peripheral: String, str: String) {
        if !devicesNotFound() {
            let data = NSData(bytes: str, length: str.count)
            switch peripheral {
            case PeripheralNames.left:
                if let p = leftUSBPeripheral,
                   let tx = leftTxCharacteristic {
                    p.writeValue(data as Data, for: tx, type: .withResponse)
                }
            case PeripheralNames.right:
                if let p = rightUSBPeripheral,
                   let tx = rightTxCharacteristic {
                    p.writeValue(data as Data, for: tx, type: .withResponse)
                }
            default:
                print("error")
            }
        }
    }
    
}


struct PeripheralNames {
    static let left = "GameSwitchLeftUSB"
    static let right = "GameSwitchRightUSB"
}
