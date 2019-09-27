//
//  BluetoothManager.swift
//  XboxGS
//
//  Created by Owen McGirr on 26/09/2019.
//  Copyright © 2019 Owen McGirr. All rights reserved.
//

import CoreBluetooth

class BluetoothManager: NSObject, CBCentralManagerDelegate, CBPeripheralDelegate {
    
    static let shared = BluetoothManager()
    
    
    struct PeripheralNames {
        static let left = "GameSwitchLeftUSB"
        static let right = "GameSwitchRightUSB"
    }
    
    
    // variables
    var centralManager: CBCentralManager!
    var leftUSBPeripheral: CBPeripheral?
    var rightUSBPeripheral: CBPeripheral?
    
    // services and characteristics
    var uartServiceLeft: CBService?
    var uartServiceRight: CBService?
    let uartServiceId = CBUUID(string: "6e400001-b5a3-f393-e0a9-e50e24dcca9e")
    
    
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
            }
            if n == PeripheralNames.right {
                uartServiceRight = peripheral.services?.first
            }
        }
    }
    
    
    
    
}
