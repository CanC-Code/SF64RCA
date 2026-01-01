package com.canc.starfox64

import android.app.Activity
import android.os.Bundle

class Starfox64Activity : Activity() {

    // Load native library
    companion object {
        init {
            System.loadLibrary("Starfox64Recompiled")
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        // Call native entry
        nativeStart()
    }

    private external fun nativeStart()
}