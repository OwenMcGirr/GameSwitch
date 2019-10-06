//
//  ViewController.swift
//  XboxGS
//
//  Created by Owen McGirr on 26/09/2019.
//  Copyright © 2019 Owen McGirr. All rights reserved.
//

import UIKit

class ViewController: UIViewController, BluetoothManagerDelegate {
    
    // outlets
    @IBOutlet var modeLabel: UILabel?
    @IBOutlet var switchAView: ATSwitchView?
    @IBOutlet var switchBView: ATSwitchView?
    @IBOutlet var switchCView: ATSwitchView?
    

    override func viewDidLoad() {
        super.viewDidLoad()
        
        // start BT
        BluetoothManager.shared.delegate = self
        BluetoothManager.shared.start()
        
        // setup switches
        switchAView?.pressCode = "1"
        switchAView?.releaseCode = "2"
        switchBView?.pressCode = "3"
        switchBView?.releaseCode = "4"
        switchCView?.pressCode = "5"
        switchCView?.releaseCode = "6"
    }

    
    func didChangeMode(to mode: String) {
        modeLabel?.text = mode
    }

}

