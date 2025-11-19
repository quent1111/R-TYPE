# Conan Profile for R-Type Project

This directory contains example Conan profiles for different platforms.

## Quick Setup

After installing Conan, detect your system profile:

```bash
conan profile detect --force
```

This creates a default profile at `~/.conan2/profiles/default`.

## Custom Profiles

You can create custom profiles for specific build configurations:

### Linux GCC Release Profile

```ini
[settings]
os=Linux
arch=x86_64
compiler=gcc
compiler.version=11
compiler.libcxx=libstdc++11
build_type=Release

[conf]
tools.system.package_manager:mode=install
tools.system.package_manager:sudo=True
```

### Windows MSVC Release Profile

```ini
[settings]
os=Windows
arch=x86_64
compiler=msvc
compiler.version=193
build_type=Release
compiler.runtime=dynamic
```

## Using Custom Profiles

```bash
# Save profile to file
conan profile show default > profiles/my-profile

# Use custom profile
conan install . --output-folder=build --profile=profiles/my-profile
```

## Troubleshooting

If Conan can't detect your compiler:

1. Check installed compilers: `gcc --version` or `clang --version`
2. Manually edit profile: `conan profile path default`
3. Set correct compiler and version
