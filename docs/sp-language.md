# Sprout Script Language Overview

`*.sp` files are a lightweight wrapper around C++ designed for rapid iteration.
The language intentionally mirrors C++ syntax while omitting complex features.

## Key Features
- Implicit `Component` class for each file.
- `update()` entry point called every frame.
- Import other scripts via `import` statements.
- Generated C++ resides next to the source for easy debugging.

## Example
```sp
// PlayerCharacter.sp
import Transform

var speed : float = 5.0

fn update(dt : float) {
    Transform.position.x += speed * dt
}
```
