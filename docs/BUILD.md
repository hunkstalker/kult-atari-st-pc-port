# BUILD

> Fase M0: pendiente de definir con el código importado. Este documento se
> completará en T-002.

## Objetivo
- **Core** compilable aparte para las pruebas diferenciales (sin SDL).
- **Ejecutable** = core + `boot/` + `platform/` + `app/`, con **SDL3**.

## Herramientas previstas
- **CMake**.
- **SDL3** (ventana, entrada, audio, GPU).
- En Linux: paquete SDL3 del sistema o compilado; en Windows: SDL3 + DLLs.
- El arnés diferencial (kit) para el core, igual que en `kult-atari-st-c`.

## Flujo previsto
```
cmake -S . -B build
cmake --build build -j
```
Salidas: librería del core (tests) + ejecutable del juego.

## Datos
El ejecutable busca los datos del usuario en un directorio `data/` junto al
ejecutable (o en una ruta configurable). **No** se versionan assets.

## Cross (Windows) — a valorar
Compilación cruzada desde Linux con MinGW + SDL3, o build nativo en Windows.
