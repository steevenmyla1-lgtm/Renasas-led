# Vendor headers for standalone build

Copy these folders from your Renesas package (e2 studio `rz/` tree):

- `rz/arm/CMSIS_6/CMSIS/Core/Include/` -> `vendor/CMSIS/Core/Include/`
- `rz/fsp/src/bsp/cmsis/Device/RENESAS/Include/` -> `vendor/Device/RENESAS/Include/`

The second copy must include the `cr/` subfolder.

After copying, configure/build normally with your existing CMake presets.
