# KNOWN BUGS AND ISSUES

## Bugs

- Though most likely fixed, the engine may crash when loading a scene in the editor, due to a lifetime management issue with D3D12.
- Loading an asset, deleting that asset from disk and pressing the Play button will crash the engine.
- Whilst Rigidbodies kinda work, their behaviour with play/stop mode are broken.

## Optimization - Pain points

- Cascaded Shadow Maps and G-Buffer pass can be quite slow due to lack of indirect + meshlet culling. Amplification shaders are also used which are not ideal
- Compresison times for textures are slow, even for cuda-accelerated BC3 compression.
- Loading times for assets are slow, due to us not having a custom binary model format for faster load times.

## Potential new features

- More editor enhancements:
    - Copy/Paste/Cut/Duplicate
    - More themes
    - Better UX for asset browser
- World Management:
    - Prefabs
- Renderer:
    - Occlusion/Frustum/Visibility/Cone culling
    - SSAO
    - Wider lines for debug renderer
    - Point shadows
    - SSR
    - Raytracing
    - Shader graph
- RHI:
    - Support for execute indirect
    - Acceleration structure compaction
    - Metal support
- Core:
    - Exporting runtime
    - Gamepad input
    - MacOS support
- Script:
    - Full support for all engine types and more utils
    - Compute shader API just like Unity
    - Serialized variables in the editor
- Audio:
    - Spatialization
    - Node graph for effects and filters
    - Raytracing for occlusion and propagation
- AI:
    - Just actually finish the feature
- Asset:
    - Hot reloading of assets
    - Asset packaging via a VFS
- Physics:
    - Actually finish the feature too...
    - Add support for narrow phase raycast
