package com.example.gameswitch

import android.Manifest
import android.bluetooth.BluetoothAdapter
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.view.KeyEvent
import android.widget.Button
import android.widget.ProgressBar
import android.widget.TextView
import android.widget.Toast
import android.view.View
import android.view.animation.AnimationUtils
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageAnalysis
import androidx.camera.core.Preview
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.camera.view.PreviewView
import androidx.core.content.ContextCompat
import androidx.lifecycle.lifecycleScope
import com.example.gameswitch.bluetooth.GameSwitchBluetoothManager
import com.example.gameswitch.face.FaceDetectionAnalyzer
import com.google.android.material.snackbar.Snackbar
import kotlinx.coroutines.launch
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors

class MainActivity : AppCompatActivity(), GameSwitchBluetoothManager.ConnectionListener {
    companion object {
        private const val TAG = "MainActivity"
        private const val REQUEST_ENABLE_BT = 1001
        private val REQUIRED_PERMISSIONS = mutableListOf(
            Manifest.permission.CAMERA
        ).apply {
            if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.P) {
                add(Manifest.permission.WRITE_EXTERNAL_STORAGE)
            }
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                add(Manifest.permission.BLUETOOTH_SCAN)
                add(Manifest.permission.BLUETOOTH_CONNECT)
            } else {
                // Location permission needed for BLE scanning on Android 6-11
                add(Manifest.permission.ACCESS_FINE_LOCATION)
            }
        }.toTypedArray()
    }

    private lateinit var bluetoothManager: GameSwitchBluetoothManager
    private lateinit var cameraExecutor: ExecutorService
    private var imageAnalyzer: ImageAnalysis? = null
    private var faceDetectionAnalyzer: FaceDetectionAnalyzer? = null
    
    // State tracking for switches
    private var isBPressed = false
    private var isCPressed = false
    private var isLeftConnected = false
    private var isRightConnected = false
    
    // UI elements
    private lateinit var previewView: PreviewView
    private lateinit var statusText: TextView
    private lateinit var modeText: TextView
    private lateinit var progressBar: ProgressBar
    private lateinit var switchAIndicator: TextView
    private lateinit var switchBIndicator: TextView
    private lateinit var switchCIndicator: TextView
    private lateinit var leftDeviceStatus: TextView
    private lateinit var rightDeviceStatus: TextView
    private lateinit var connectButton: Button

    private val activityResultLauncher =
        registerForActivityResult(ActivityResultContracts.RequestMultiplePermissions()) { permissions ->
            val permissionGranted = permissions.entries.all { it.value }
            if (permissionGranted) {
                startCamera()
                startBluetooth()
            } else {
                Toast.makeText(this, "Permissions not granted", Toast.LENGTH_SHORT).show()
                finish()
            }
        }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        initializeViews()
        bluetoothManager = GameSwitchBluetoothManager(this)
        bluetoothManager.connectionListener = this
        cameraExecutor = Executors.newSingleThreadExecutor()

        if (allPermissionsGranted()) {
            startCamera()
            startBluetooth()
        } else {
            requestPermissions()
        }

        setupUI()
    }

    private fun initializeViews() {
        previewView = findViewById(R.id.preview_view)
        statusText = findViewById(R.id.status_text)
        modeText = findViewById(R.id.mode_text)
        progressBar = findViewById(R.id.progress_bar)
        switchAIndicator = findViewById(R.id.switch_a_indicator)
        switchBIndicator = findViewById(R.id.switch_b_indicator)
        switchCIndicator = findViewById(R.id.switch_c_indicator)
        leftDeviceStatus = findViewById(R.id.left_device_status)
        rightDeviceStatus = findViewById(R.id.right_device_status)
        connectButton = findViewById(R.id.connect_button)
    }

    private fun setupUI() {
        statusText.text = "A: Any key | B: Head right | C: Head down"
        progressBar.visibility = View.GONE
        
        // Set initial indicator colors
        switchAIndicator.setBackgroundColor(getColor(android.R.color.darker_gray))
        switchBIndicator.setBackgroundColor(getColor(android.R.color.darker_gray))
        switchCIndicator.setBackgroundColor(getColor(android.R.color.darker_gray))
        
        // Set up connect button
        connectButton.setOnClickListener {
            // Check if Bluetooth is enabled
            val bluetoothAdapter = BluetoothAdapter.getDefaultAdapter()
            if (bluetoothAdapter == null) {
                Toast.makeText(this, "Bluetooth not supported on this device", Toast.LENGTH_LONG).show()
                return@setOnClickListener
            }
            
            if (!bluetoothAdapter.isEnabled) {
                // Request to enable Bluetooth
                val enableBtIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
                startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT)
            } else {
                // Bluetooth is enabled, start scanning
                startBluetoothScan()
            }
        }
    }

    override fun onKeyDown(keyCode: Int, event: KeyEvent?): Boolean {
        // Any key press triggers switch A
        sendSwitchCommand("A", true)
        switchAIndicator.setBackgroundColor(getColor(android.R.color.holo_green_light))
        return true
    }

    override fun onKeyUp(keyCode: Int, event: KeyEvent?): Boolean {
        // Any key release triggers switch A release
        sendSwitchCommand("A", false)
        switchAIndicator.setBackgroundColor(getColor(android.R.color.darker_gray))
        return true
    }

    private fun sendSwitchCommand(switch: String, isPressed: Boolean) {
        val command = when (switch) {
            "A" -> if (isPressed) "1" else "2"
            "B" -> if (isPressed) "3" else "4"
            "C" -> if (isPressed) "5" else "6"
            else -> return
        }
        
        bluetoothManager.sendCommand(command)
        Log.d(TAG, "Sent command: $command for switch $switch (${if (isPressed) "pressed" else "released"})")
    }

    private fun startCamera() {
        val cameraProviderFuture = ProcessCameraProvider.getInstance(this)

        cameraProviderFuture.addListener({
            val cameraProvider: ProcessCameraProvider = cameraProviderFuture.get()

            val preview = Preview.Builder().build().also {
                it.setSurfaceProvider(previewView.surfaceProvider)
            }

            faceDetectionAnalyzer = FaceDetectionAnalyzer { faces, headRotation ->
                runOnUiThread {
                    if (faces.isNotEmpty()) {
                        val face = faces.first()
                        
                        // B - Head movement right (negative Y rotation from viewer's perspective)
                        headRotation?.let { rotation ->
                            if (rotation.y < -8f) { // Head turned right (viewer's perspective)
                                if (!isBPressed) {
                                    isBPressed = true
                                    sendSwitchCommand("B", true)
                                    switchBIndicator.setBackgroundColor(getColor(android.R.color.holo_blue_light))
                                }
                            } else {
                                if (isBPressed) {
                                    isBPressed = false
                                    sendSwitchCommand("B", false)
                                    switchBIndicator.setBackgroundColor(getColor(android.R.color.darker_gray))
                                }
                            }
                        }
                        
                        // C - Head down detection (negative X rotation from viewer's perspective)
                        headRotation?.let { rotation ->
                            if (rotation.x < -8f) { // Head tilted down
                                if (!isCPressed) {
                                    isCPressed = true
                                    sendSwitchCommand("C", true)
                                    switchCIndicator.setBackgroundColor(getColor(android.R.color.holo_orange_light))
                                }
                            } else {
                                if (isCPressed) {
                                    isCPressed = false
                                    sendSwitchCommand("C", false)
                                    switchCIndicator.setBackgroundColor(getColor(android.R.color.darker_gray))
                                }
                            }
                        }
                    }
                }
            }

            imageAnalyzer = ImageAnalysis.Builder()
                .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
                .build()
                .also {
                    it.setAnalyzer(cameraExecutor, faceDetectionAnalyzer!!)
                }

            val cameraSelector = CameraSelector.DEFAULT_FRONT_CAMERA

            try {
                cameraProvider.unbindAll()
                cameraProvider.bindToLifecycle(
                    this, cameraSelector, preview, imageAnalyzer
                )
            } catch (exc: Exception) {
                Log.e(TAG, "Use case binding failed", exc)
            }

        }, ContextCompat.getMainExecutor(this))
    }

    private fun startBluetooth() {
        // Check if Bluetooth is enabled before auto-scanning
        val bluetoothAdapter = BluetoothAdapter.getDefaultAdapter()
        if (bluetoothAdapter?.isEnabled == true) {
            bluetoothManager.startScanning()
        }
    }
    
    private fun startBluetoothScan() {
        // Check permissions for Android 12+
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED ||
                ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                Toast.makeText(this, "Bluetooth permissions required", Toast.LENGTH_LONG).show()
                requestPermissions()
                return
            }
        }
        
        connectButton.isEnabled = false
        connectButton.text = "Scanning..."
        bluetoothManager.startScanning()
        
        // Re-enable after 5 seconds
        connectButton.postDelayed({
            if (!isLeftConnected || !isRightConnected) {
                connectButton.isEnabled = true
                connectButton.text = "Connect"
            }
        }, 5000)
    }
    
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        when (requestCode) {
            REQUEST_ENABLE_BT -> {
                if (resultCode == RESULT_OK) {
                    // Bluetooth was enabled, start scanning
                    startBluetoothScan()
                } else {
                    // User declined to enable Bluetooth
                    Toast.makeText(this, "Bluetooth is required for this app", Toast.LENGTH_LONG).show()
                }
            }
        }
    }

    override fun onDeviceConnected(deviceName: String) {
        runOnUiThread {
            when (deviceName) {
                "GameSwitchLeftUSB" -> {
                    isLeftConnected = true
                    leftDeviceStatus.text = "Left: ●"
                    leftDeviceStatus.setTextColor(getColor(android.R.color.holo_green_dark))
                    val pulseAnimation = AnimationUtils.loadAnimation(this@MainActivity, R.anim.pulse)
                    leftDeviceStatus.startAnimation(pulseAnimation)
                }
                "GameSwitchRightUSB" -> {
                    isRightConnected = true
                    rightDeviceStatus.text = "Right: ●"
                    rightDeviceStatus.setTextColor(getColor(android.R.color.holo_green_dark))
                    val pulseAnimation = AnimationUtils.loadAnimation(this@MainActivity, R.anim.pulse)
                    rightDeviceStatus.startAnimation(pulseAnimation)
                }
            }
            
            // Update status text when both devices are connected
            if (isLeftConnected && isRightConnected) {
                statusText.text = "✓ Both devices connected - Ready to use!"
                statusText.setTextColor(getColor(android.R.color.holo_green_dark))
                connectButton.text = "Connected"
                connectButton.isEnabled = false
            }
            
            Snackbar.make(findViewById(android.R.id.content), "Connected to $deviceName", Snackbar.LENGTH_SHORT).show()
        }
    }

    override fun onDeviceDisconnected(deviceName: String) {
        runOnUiThread {
            when (deviceName) {
                "GameSwitchLeftUSB" -> {
                    isLeftConnected = false
                    leftDeviceStatus.text = "Left: ●"
                    leftDeviceStatus.setTextColor(getColor(android.R.color.holo_red_dark))
                    leftDeviceStatus.clearAnimation()
                }
                "GameSwitchRightUSB" -> {
                    isRightConnected = false
                    rightDeviceStatus.text = "Right: ●"
                    rightDeviceStatus.setTextColor(getColor(android.R.color.holo_red_dark))
                    rightDeviceStatus.clearAnimation()
                }
            }
            
            // Update status text when a device disconnects
            statusText.text = "A: Any key | B: Head right | C: Head down"
            statusText.setTextColor(getColor(android.R.color.black))
            connectButton.text = "Connect"
            connectButton.isEnabled = true
            
            Snackbar.make(findViewById(android.R.id.content), "Disconnected from $deviceName", Snackbar.LENGTH_SHORT).show()
        }
    }

    override fun onConnectionFailed(deviceName: String) {
        runOnUiThread {
            when (deviceName) {
                "GameSwitchLeftUSB" -> {
                    leftDeviceStatus.text = "Left: ✗"
                    leftDeviceStatus.setTextColor(getColor(android.R.color.holo_red_dark))
                }
                "GameSwitchRightUSB" -> {
                    rightDeviceStatus.text = "Right: ✗"
                    rightDeviceStatus.setTextColor(getColor(android.R.color.holo_red_dark))
                }
                "Bluetooth is disabled" -> {
                    connectButton.isEnabled = true
                    connectButton.text = "Enable BT"
                    Toast.makeText(this, "Please enable Bluetooth", Toast.LENGTH_LONG).show()
                }
                "Bluetooth not available" -> {
                    connectButton.isEnabled = false
                    connectButton.text = "No BT"
                    Toast.makeText(this, "Bluetooth not available on this device", Toast.LENGTH_LONG).show()
                }
                else -> {
                    connectButton.isEnabled = true
                    connectButton.text = "Retry"
                }
            }
            
            if (deviceName.startsWith("GameSwitch")) {
                Snackbar.make(findViewById(android.R.id.content), "Failed to connect to $deviceName", Snackbar.LENGTH_SHORT).show()
            } else {
                Snackbar.make(findViewById(android.R.id.content), deviceName, Snackbar.LENGTH_LONG).show()
            }
        }
    }
    
    override fun onModeChanged(mode: String) {
        runOnUiThread {
            modeText.text = mode
            Log.d(TAG, "Mode changed to: $mode")
        }
    }

    private fun allPermissionsGranted() = REQUIRED_PERMISSIONS.all {
        ContextCompat.checkSelfPermission(baseContext, it) == PackageManager.PERMISSION_GRANTED
    }

    private fun requestPermissions() {
        activityResultLauncher.launch(REQUIRED_PERMISSIONS)
    }

    override fun onDestroy() {
        super.onDestroy()
        cameraExecutor.shutdown()
        bluetoothManager.disconnect()
    }
}