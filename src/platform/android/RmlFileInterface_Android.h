#ifndef __RML_FILE_INTERFACE_ANDROID_H__
#define __RML_FILE_INTERFACE_ANDROID_H__

#include <RmlUi/Core/FileInterface.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <string>

class RmlFileInterface_Android : public Rml::FileInterface {
private:
    AAssetManager* asset_manager_;
public:
    explicit RmlFileInterface_Android(AAssetManager* mgr) : asset_manager_(mgr) {}

    Rml::FileHandle Open(const Rml::String& path) override {
        AAsset* asset = AAssetManager_open(asset_manager_, path.c_str(), AASSET_MODE_BUFFER);
        return reinterpret_cast<Rml::FileHandle>(asset);
    }

    void Close(Rml::FileHandle file) override {
        if (file) {
            AAsset_close(reinterpret_cast<AAsset*>(file));
        }
    }

    size_t Read(void* buffer, size_t size, Rml::FileHandle file) override {
        if (!file) return 0;
        return static_cast<size_t>(AAsset_read(reinterpret_cast<AAsset*>(file), buffer, size));
    }

    bool Seek(Rml::FileHandle file, long offset, int origin) override {
        if (!file) return false;
        int whence = SEEK_SET;
        switch (origin) {
            case SEEK_SET: whence = SEEK_SET; break;
            case SEEK_CUR: whence = SEEK_CUR; break;
            case SEEK_END: whence = SEEK_END; break;
        }
        return AAsset_seek(reinterpret_cast<AAsset*>(file), offset, whence) != -1;
    }

    size_t Tell(Rml::FileHandle file) override {
        if (!file) return 0;
        return static_cast<size_t>(AAsset_getLength(reinterpret_cast<AAsset*>(file)) - AAsset_getRemainingLength(reinterpret_cast<AAsset*>(file)));
    }
};

#endif