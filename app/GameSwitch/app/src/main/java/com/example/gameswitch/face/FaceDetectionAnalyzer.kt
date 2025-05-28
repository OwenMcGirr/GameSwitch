package com.example.gameswitch.face

import android.annotation.SuppressLint
import androidx.camera.core.ImageAnalysis
import androidx.camera.core.ImageProxy
import com.google.mlkit.vision.common.InputImage
import com.google.mlkit.vision.face.Face
import com.google.mlkit.vision.face.FaceDetection
import com.google.mlkit.vision.face.FaceDetectorOptions

data class HeadRotation(val x: Float, val y: Float, val z: Float)

class FaceDetectionAnalyzer(
    private val onFacesDetected: (List<Face>, HeadRotation?) -> Unit
) : ImageAnalysis.Analyzer {
    
    private val options = FaceDetectorOptions.Builder()
        .setPerformanceMode(FaceDetectorOptions.PERFORMANCE_MODE_FAST)
        .setLandmarkMode(FaceDetectorOptions.LANDMARK_MODE_ALL)
        .setClassificationMode(FaceDetectorOptions.CLASSIFICATION_MODE_ALL)
        .setMinFaceSize(0.15f)
        .enableTracking()
        .build()
    
    private val detector = FaceDetection.getClient(options)
    
    @SuppressLint("UnsafeOptInUsageError")
    override fun analyze(imageProxy: ImageProxy) {
        val mediaImage = imageProxy.image
        if (mediaImage != null) {
            val image = InputImage.fromMediaImage(mediaImage, imageProxy.imageInfo.rotationDegrees)
            
            detector.process(image)
                .addOnSuccessListener { faces ->
                    val headRotation = if (faces.isNotEmpty()) {
                        val face = faces.first()
                        HeadRotation(
                            x = face.headEulerAngleX, // Pitch (up/down)
                            y = face.headEulerAngleY, // Yaw (left/right)
                            z = face.headEulerAngleZ  // Roll (tilt)
                        )
                    } else null
                    onFacesDetected(faces, headRotation)
                }
                .addOnFailureListener { e ->
                    // Handle failure
                    e.printStackTrace()
                }
                .addOnCompleteListener {
                    imageProxy.close()
                }
        } else {
            imageProxy.close()
        }
    }
}