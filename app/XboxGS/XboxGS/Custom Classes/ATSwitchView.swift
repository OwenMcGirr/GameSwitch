//
//  ATSwitchView.swift
//  XboxGS
//
//  Created by Owen McGirr on 02/10/2019.
//  Copyright © 2019 Owen McGirr. All rights reserved.
//

import UIKit

class ATSwitchView: UIView {

    var pressCode = ""
    var releaseCode = ""
    
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
    }
    
    
    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        BluetoothManager.shared.write(to: BluetoothManager.shared.leftUSBPeripheral!, for: BluetoothManager.shared.leftTxCharacteristic!, str: pressCode)
    }
    
    override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
        BluetoothManager.shared.write(to: BluetoothManager.shared.leftUSBPeripheral!, for: BluetoothManager.shared.leftTxCharacteristic!, str: releaseCode)
    }
    
}
