#ifndef __RML_SYSTEM_INTERFACE_ANDROID_H__
#define __RML_SYSTEM_INTERFACE_ANDROID_H__

#include <RmlUi/Core/SystemInterface.h>
#include <chrono>
#include <android/log.h>

class RmlSystemInterface_Android : public Rml::SystemInterface {
private:
    std::chrono::steady_clock::time_point start_time_;
public:
    RmlSystemInterface_Android() {
        start_time_ = std::chrono::steady_clock::now();
    }

    double GetElapsedTime() override {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration<double>(now - start_time_).count();
    }

    void LogMessage(Rml::Log::Type type, const Rml::String& message) override {
        int priority = ANDROID_LOG_INFO;
        switch (type) {
            case Rml::Log::LT_ERROR: priority = ANDROID_LOG_ERROR; break;
            case Rml::Log::LT_ASSERT: priority = ANDROID_LOG_FATAL; break;
            case Rml::Log::LT_WARNING: priority = ANDROID_LOG_WARN; break;
            case Rml::Log::LT_INFO: priority = ANDROID_LOG_INFO; break;
            case Rml::Log::LT_DEBUG: priority = ANDROID_LOG_DEBUG; break;
        }
        __android_log_print(priority, "RmlUi", "%s", message.c_str());
    }

    // Optional stubs:
    bool SetClipboardText(const Rml::String& text) override { return false; }
    void GetClipboardText(Rml::String& text) override { text.clear(); }
    void ActivateKeyboard() override {}
    void DeactivateKeyboard() override {}
};

#endif