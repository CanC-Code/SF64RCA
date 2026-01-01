// Forward declaration of your real desktop main()
int main(int argc, char** argv);

// Android-safe entry point called from SDL_main
extern "C"
int StarfoxMain(int argc, char** argv) {
    return main(argc, argv);
}