package com.canc.starfox64

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import org.libsdl.app.SDLActivity

class MainActivity : SDLActivity() {
    companion object {
        init {
            // Load the native library
            System.loadLibrary("starfox64recompiled")
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
    }

    override fun getLibraries(): Array<String> {
        return arrayOf("starfox64recompiled")
    }
}