package com.canc.starfox64

import android.os.Bundle
import org.libsdl.app.SDLActivity

class MainActivity : SDLActivity() {

    companion object {
        init {
            System.loadLibrary("Starfox64Recompiled")
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
    }
}
