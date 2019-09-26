//
//  BluetoothManager.swift
//  XboxGS
//
//  Created by Owen McGirr on 26/09/2019.
//  Copyright © 2019 Owen McGirr. All rights reserved.
//

import CoreBluetooth

class BluetoothManager: NSObject, CBCentralManagerDelegate {
    
    static let shared = BluetoothManager()
    
    
    // variables
    var centralManager: CBCentralManager!
    var leftUSBPeripheral: CBPeripheral?
    var rightUSBPeripheral: CBPeripheral?
    
    // services
    //let uart = CBUUID(string: "6E400001-B5A3-F393-­E0A9-­E50E24DCCA9E")
    
    
    func start() {
        centralManager = CBCentralManager(delegate: self, queue: nil)
        DispatchQueue.main.asyncAfter(deadline: .now() + 1.0, execute: {
            self.centralManager.scanForPeripherals(withServices: nil, options: nil)
        })
        
    }
    
    
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        print("state: \(central.state)")
    }
    
    
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        print("discovered: \(peripheral.name ?? "n/a")")
        if let n = peripheral.name {
            if n == "GameSwitchLeftUSB" {
                centralManager.connect(peripheral, options: nil)
                leftUSBPeripheral = peripheral
            }
            if n == "GameSwitchRightUSB" {
                centralManager.connect(peripheral, options: nil)
                rightUSBPeripheral = peripheral
            }
            
            if leftUSBPeripheral != nil && rightUSBPeripheral != nil {
                centralManager.stopScan()
            }
            else {
                centralManager.scanForPeripherals(withServices: nil, options: nil)
            }
        }
    }
    
}
