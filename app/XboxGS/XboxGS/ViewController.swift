//
//  ViewController.swift
//  XboxGS
//
//  Created by Owen McGirr on 26/09/2019.
//  Copyright © 2019 Owen McGirr. All rights reserved.
//

import UIKit
import ARKit
import PKHUD

class ViewController: UIViewController, BluetoothManagerDelegate, ARSCNViewDelegate {
    
    // outlets
    @IBOutlet var modeLabel: UILabel?
    @IBOutlet var switchAView: ATSwitchView?
    @IBOutlet var switchBView: ATSwitchView?
    @IBOutlet var switchCView: ATSwitchView?
    @IBOutlet var sceneView: ARSCNView?
    @IBOutlet var bottomView: UIView?
    
    // variables
    var currentFacePose = ""
    var tongueOutStartTime: TimeInterval?
    var timingTongueOut = false
    
    
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
    
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        
        let configuration = ARFaceTrackingConfiguration()
        
        sceneView?.session.run(configuration)
        
        sceneView?.delegate = self
        sceneView?.showsStatistics = true
    }
    
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        
        sceneView?.session.pause()
    }
    
    
    func renderer(_ renderer: SCNSceneRenderer, nodeFor anchor: ARAnchor) -> SCNNode? {
        let faceMesh = ARSCNFaceGeometry(device: ((sceneView?.device)!))
        let node = SCNNode(geometry: faceMesh)
        node.geometry?.firstMaterial?.fillMode = .fill
        return node
    }
    
    func renderer(_ renderer: SCNSceneRenderer, didUpdate node: SCNNode, for anchor: ARAnchor) {
        if let faceAnchor = anchor as? ARFaceAnchor, let faceGeometry = node.geometry as? ARSCNFaceGeometry {
            faceGeometry.update(from: faceAnchor.geometry)
            facePoseAnalyzer(anchor: faceAnchor)
        }
    }
    
    func facePoseAnalyzer(anchor: ARFaceAnchor) {
        let tongue = anchor.blendShapes[.tongueOut]
        
        var newFacePose = ""
        
        if tongue?.decimalValue ?? 0.0 > 0.08 {
            newFacePose = "tongue"
        }
        else {
            newFacePose = "other"
            timingTongueOut = false
        }
        
        if currentFacePose != newFacePose {
            if newFacePose == "tongue" {
                tongueOutStartTime = Date().timeIntervalSinceReferenceDate
                timingTongueOut = true
                print("tongue")
            }
            currentFacePose = newFacePose
        }
        
        if timingTongueOut {
            let duration = Date().timeIntervalSinceReferenceDate - tongueOutStartTime!
            if duration > 0.7 {
                switchCView?.performVirtualTap()
                DispatchQueue.main.sync {
                    showHUD(text: "Switch C (tongue)")
                }
                timingTongueOut = false
            }
        }
    }
    
    
    func showHUD(text: String) {
        PKHUD.sharedHUD.dimsBackground = true
        PKHUD.sharedHUD.contentView = PKHUDTextView.init(text: text)
        PKHUD.sharedHUD.show(onView: bottomView!)
        PKHUD.sharedHUD.hide(afterDelay: 1.0)
    }
    
    
    func didChangeMode(to mode: String) {
        modeLabel?.text = mode
    }
    
}

