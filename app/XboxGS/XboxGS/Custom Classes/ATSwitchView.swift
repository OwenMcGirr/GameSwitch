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
    
    var defaultColor: UIColor?
    
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        defaultColor = self.backgroundColor
    }
    
    
    func press() {
        if !BluetoothManager.shared.devicesNotFound() {
            BluetoothManager.shared.write(to: PeripheralNames.left, str: pressCode)
            DispatchQueue.main.async() {
                self.backgroundColor = UIColor(red: 0.5, green: 0.5, blue: 0.5, alpha: 1)
            }
        }
    }

    
    func release() {
        if !BluetoothManager.shared.devicesNotFound() {
            BluetoothManager.shared.write(to: PeripheralNames.left, str: releaseCode)
            DispatchQueue.main.async() {
                self.backgroundColor = self.defaultColor
            }
        }
    }
    

    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        press()
    }
    
    
    override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
        release()
    }
    
    
    func performVirtualTap() {
        if !BluetoothManager.shared.devicesNotFound() {
            self.press()
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.3, execute: {
                self.release()
            })
        }
    }
    
}
