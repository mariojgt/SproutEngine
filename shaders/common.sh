#if BGFX_SHADER_LANGUAGE_GLSL
    #define mul(a,b) ((b) * (a))
#elif BGFX_SHADER_LANGUAGE_METAL
    #define mul(a,b) ((b) * (a))
#else
    // HLSL
#endif
