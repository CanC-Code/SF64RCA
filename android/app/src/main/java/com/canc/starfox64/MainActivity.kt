package com.canc.starfox64

import android.app.Activity
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import androidx.appcompat.app.AlertDialog
import org.libsdl.app.SDLActivity

class MainActivity : SDLActivity() {

    companion object {
        init {
            System.loadLibrary("starfox64recompiled")
        }

        // JNI bridge function implemented in C++
        external fun pickRom(uri: String)
    }

    private val PICK_ROM_CODE = 12345
    private var pendingRomIntent: Intent? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        checkRom()
    }

    private fun checkRom() {
        // Launch SAF file picker if ROM does not exist
        val romFile = getInternalRomPath()
        if (!romFile.exists()) {
            promptUserForRom()
        }
    }

    private fun getInternalRomPath(): java.io.File {
        return java.io.File(filesDir, "Starfox64.z64")
    }

    private fun promptUserForRom() {
        val intent = Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
            addCategory(Intent.CATEGORY_OPENABLE)
            type = "*/*"
        }
        pendingRomIntent = intent
        startActivityForResult(intent, PICK_ROM_CODE)
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        if (requestCode == PICK_ROM_CODE && resultCode == Activity.RESULT_OK) {
            val uri: Uri? = data?.data
            if (uri != null) {
                // Persist URI permission for future access
                contentResolver.takePersistableUriPermission(uri, Intent.FLAG_GRANT_READ_URI_PERMISSION)
                // Pass the URI to native code (SDL_main.cpp)
                pickRom(uri.toString())
            } else {
                showRomError("No ROM selected. Cannot continue.")
            }
        } else {
            showRomError("ROM selection cancelled. Cannot continue.")
        }
        super.onActivityResult(requestCode, resultCode, data)
    }

    private fun showRomError(message: String) {
        AlertDialog.Builder(this)
            .setTitle("Starfox64 Error")
            .setMessage(message)
            .setPositiveButton("Exit") { _, _ -> finish() }
            .setCancelable(false)
            .show()
    }

    // Ensure SDLActivity loads our library
    override fun getLibraries(): Array<String> {
        return arrayOf("starfox64recompiled")
    }
}