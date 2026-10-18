# ADR 0002 — `SDL_Renderer` base y `SDL_GPU` para CRT

- **Estado:** aceptada
- **Fecha:** sesión 1

## Contexto
El juego produce un framebuffer **planar 320×200, 16 colores**. Lo básico es
convertirlo a RGBA y escalarlo. Los filtros CRT necesitan shaders.

## Decisión
- **Base:** `SDL_Renderer` (textura + escalado entero). Simple y suficiente.
- **Filtros CRT:** `SDL_GPU` + shaders (hito M5), sin cambiar de librería.

## Consecuencias
- Empezar simple y añadir CRT después sin reescribir.
- No se usa Vulkan puro (desproporcionado para este caso).
