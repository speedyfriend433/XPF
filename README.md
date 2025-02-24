# TSPF (TrollStore Patching Framework)

A flexible framework for patching TrollStore apps with custom modifications!

## Features

- Plugin-based architecture for easy extensibility
- Built-in plugins for common modifications:
  - Entitlement patching
  - Binary patching
  - Function/symbol patching
- Backup functionality
- Verification system
- Error handling and logging

## Building

```bash
make clean && make
```

This will build:
- `libxpf.dylib` - Core library
- `patch_app` - Main patching utility

## Usage

Basic usage:
```bash
./patch_app /path/to/App.app/AppBinary
```

## Example Plugins

### Entitlement Plugin
Adds custom entitlements to the app:
examples:
- No container restrictions - com.apple.private.security.no-container
- Full filesystem access - com.apple.private.security.filesystem-access
- Disabled container requirements - com.apple.private.security.container-requirements
- add more entitlements

### Binary Patch Plugin

Allows patching specific bytes at given offsets with verification.

### Example Plugin
This is just an example plugin that patches a specific symbol with a custom value. 

- Jailbreak detection bypass
- Geometry dash noclip (byte-patch)

## Creating Custom Plugins

1. Copy the template files:
   - `src/tspf/plugins/TSPFPluginTemplate.h`
   - `src/tspf/plugins/TSPFPluginTemplate.c`

2. Implement the required functions:
   - `initialize()`
   - `patch()`
   - `cleanup()`

3. Define config structure:
```c
typedef struct {
    char *targetSymbol;
    uint64_t targetAddress;
    uint8_t *patchBytes;
    size_t patchSize;
    // other configs etc
} pluginConfig;
```

4. Register your plugin with the TSPF manager.

## Plugin Features

- Symbol lookup
- Direct address patching
- Original bytes verification
- Automatic backup
- Post-patch verification
- Error handling & Logging support

## License

MIT License