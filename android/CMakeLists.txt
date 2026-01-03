// -------------------------------
// External native build (CMake)
// -------------------------------
externalNativeBuild {
    cmake {
        path = file("CMakeLists.txt")  // points to android/CMakeLists.txt
        version = "3.22.1"

        // Pass Android-specific options
        arguments += [
            "-DANDROID_PLATFORM=android-24",
            "-DANDROID_STL=c++_shared"
        ]
    }
}

// -------------------------------
// Dynamic asset preprocessing
// -------------------------------
def fileToCExecutable = "../file_to_c" // adjust if different path
def dynamicDirs = [
    "cshaders",
    "cmods",
    "cpatches"
]

// Loop through each dynamic asset directory
dynamicDirs.each { dirName ->
    def dir = file("../app/src/main/${dirName}")
    fileTree(dir).matching { include "**/*.nrm" }.files.each { File f ->
        def outC = file("${buildDir}/generated/${dirName}/${f.nameWithoutExtension}.c")
        def outH = file("${buildDir}/generated/${dirName}/${f.nameWithoutExtension}.h")

        // Create a Gradle task for each file
        tasks.register("generate${f.nameWithoutExtension.capitalize()}", Exec) {
            inputs.file f
            outputs.files outC, outH
            commandLine fileToCExecutable, f.absolutePath, f.nameWithoutExtension, outC.absolutePath, outH.absolutePath
        }

        // Add generated files to CMake via arguments
        afterEvaluate {
            android.externalNativeBuild.cmake.arguments += [
                "-D${dirName.toUpperCase()}_GENERATED=${outC.absolutePath}"
            ]
        }
    }
}