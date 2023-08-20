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
    @IBOutlet var progressView: UIProgressView? 
    
    // variables
    var currentFacePose = ""
    var tongueOutStartTime: TimeInterval?
    var timingTongueOut = false
    var eyesClosedStartTime: TimeInterval?
    var timingEyesClosed = false
    var doneFirstEyesClosedAction = false
    var faceXSetupPoints: [Float] = []
    var faceXAveragePoint: Float = 0.0
    var isHeadRight = false
    var moving = false
    
    
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
        
        faceXSetupPoints = []
        faceXAveragePoint = 0.0
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
            
            let x = faceAnchor.transform.columns.3.x
            let numberOfSetupPoints = 50
            if faceXSetupPoints.count < numberOfSetupPoints {
                faceXSetupPoints.append(x)
                if faceXSetupPoints.count == numberOfSetupPoints {
                    var sum: Float = 0.0
                    for i in (0...(numberOfSetupPoints - 1)) {
                        sum += faceXSetupPoints[i]
                    }
                    faceXAveragePoint = sum / Float(numberOfSetupPoints)
                    print("average \(faceXAveragePoint)")
                }
                return
            }
            
            checkFacePosition(x)
            
            // analyze face
            facePoseAnalyzer(anchor: faceAnchor)
        }
    }

    // Function to recalibrate the head position after one second of not moving
    func recalibrate() {
        DispatchQueue.main.asyncAfter(deadline: .now() + 1.0, execute: {
            if !self.isHeadRight {
                self.faceXSetupPoints = []
                self.faceXAveragePoint = 0.0
                print("recalibrated")
            }
        })
    }
    
    func checkFacePosition(_ x: Float) {
        let headRightThreshold = faceXAveragePoint + 0.022
        if x > headRightThreshold {
            if !isHeadRight && moving {
                switchBView?.press()
                isHeadRight = true
            }
            print("right")
        } else {
            if moving {
                if isHeadRight {
                    switchBView?.release()
                    isHeadRight = false
                    recalibrate()
                }
            }
        }
    }
    
    func facePoseAnalyzer(anchor: ARFaceAnchor) {
        let tongue = anchor.blendShapes[.tongueOut]
        let leftEyeBlink = anchor.blendShapes[.eyeBlinkLeft]
        let rightEyeBlink = anchor.blendShapes[.eyeBlinkRight]
        
        var newFacePose = ""
        
        // check for tongue out
        if tongue?.decimalValue ?? 0.0 > 0.08 {
            newFacePose = "tongue"
        }
        else if leftEyeBlink?.decimalValue ?? 0.0 > 0.4 && rightEyeBlink?.decimalValue ?? 0.0 > 0.4 {
            newFacePose = "eyes"
        }
        else {
            newFacePose = "other"
            timingTongueOut = false
            timingEyesClosed = false
            doneFirstEyesClosedAction = false
        }
        
        if currentFacePose != newFacePose {
            // if tongue out, start timing
            if newFacePose == "tongue" {
                tongueOutStartTime = Date().timeIntervalSinceReferenceDate
                timingTongueOut = true
                print("tongue")
            }
            else {
                DispatchQueue.main.async {
                    self.progressView?.setProgress(0, animated: true)
                }
            }
            if newFacePose == "eyes" {
                eyesClosedStartTime = Date().timeIntervalSinceReferenceDate
                timingEyesClosed = true
                print("eyes")
            }
            currentFacePose = newFacePose
        }
        
        // check timing for tongue out
        if timingTongueOut {
            let duration = Date().timeIntervalSinceReferenceDate - tongueOutStartTime!
            DispatchQueue.main.async {
                if duration <= 0.5 {
                    self.progressView?.setProgress(Float(duration / 0.5), animated: true)
                }
                else {
                    self.showHUD(text: "Switch C (tongue)")
                    self.progressView?.setProgress(0, animated: true)
                }
            }
            if duration > 0.5 {
                switchCView?.performVirtualTap()
                timingTongueOut = false
            }
        }
        
        // check timing for eyes closed
        if timingEyesClosed {
            let duration = Date().timeIntervalSinceReferenceDate - eyesClosedStartTime!
            DispatchQueue.main.async {
                if duration <= 0.280 {
                    self.progressView?.setProgress(Float(duration / 0.280), animated: true)
                }
                else {
                    self.showHUD(text: "Eyes")
                    self.progressView?.setProgress(0, animated: true)
                }
                if duration >= 1.2 {
                    self.moving.toggle()
                    if self.moving {
                        self.showHUD(text: "Moving")
                    } else {
                        self.showHUD(text: "Not moving")
                    }
                    self.faceXSetupPoints = []
                }
            }
            if duration > 0.280 && !self.doneFirstEyesClosedAction {
                if !BluetoothManager.shared.devicesNotFound() {
                    BluetoothManager.shared.write(to: PeripheralNames.left, str: "7")
                }
                self.doneFirstEyesClosedAction = true
            }
            if duration > 1.2 {
                timingEyesClosed = false
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

